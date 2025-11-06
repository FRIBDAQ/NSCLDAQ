/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <config.h>
#include <CNADC2530.h>
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <stdio.h>
#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
#endif

using namespace std;

#define Const(name) static const int name = 

////////////////////////////////////////////////////////////////////////
//
//                  Constants:

//  Register offsets for the NADC2530 module
//  (I've only bothered with the registers I may need).
//
Const(REG_ID)        0;		// Manufacturer's id.
Const(REG_TYPE)      2;		// Module model number.
Const(REG_CSR)       4;		// Control and status register.
Const(REG_MEM)       6;		// Memory offset for list mode memory.
Const(REG_LISTWL)    8;           // Low order number of words in list memory.
Const(REG_LISTWH)    0xa;         // High order number of words in list memory.
Const(REG_VECTOR)    0xc;         // Interrupt vector
Const(REG_FULLNESS)  0x10;
Const(REG_EVENTSREQ) 0x12;        // Requested event count
Const(REG_IRQMASK)   0x12;        // or could be interrupt mask depending on csr
Const(REG_LLD)       0x14;        // Lower level discriminator value.
Const(REG_HLD)       0x16;        // Upper level discrminator value.
Const(REG_EVENTSL)   0x18;        // Low 16 bits of number of events acquired.
Const(REG_EVENTSH)   0x1a;        // High 8 bits of number of events acquired.

// Identification Register values:

Const(ID_VALUE)      0x8063;      // Manufacturer code.
Const(TYPE_VALUE)    2530;        // Model number.

// CSR bits (we're really only going to set them).

Const(CSR_HE)       0x8000;       // 1 Enables histogram mode 0 list mode.
Const(CSR_GE)       0x4000;       // 1 Enables gate mode 0 auto trigger.
Const(CSR_ZE)       0x2000;       // 1 Disables zero supresssion 0 Enables zero supression.
Const(CSR_IT)       0x1000;       // 1 Interrupt on event count 0 Interrupt on full/half full.
Const(CSR_IPLMASK)  0x0e00;       // Interrupt priority level mask.
Const(CSR_IPLSHIFT) 9;            // Lowest bit position for IPL mask.
Const(CSR_ARM)      0x0100;       // 1 ARM Cleared automatically when full in list mode.
Const(CSR_IE)       0x0080;      // 1 Enable ineterrupts 0 disable.
Const(CSR_IS)       0x0040;       // 1 if an interrupt condition is set.
Const(CSR_CLTS)     0x0040;       // Set this to clear the timestamp.
Const(CSR_FFCLR)    0x0010;       // Force a fast clear
Const(CSR_SC)       0x0008;       // 1 Set in calibration mode.
Const(CSR_iDR)      0x0004;       // 1 Data ready when event count hit else on fullness flags.
Const(CSR_SS)       0x0002;       // 1 Disables sliding scale (should never be set!).
Const(CSR_RESET)    0x0001;       // Resets the csr.


// Address modifiers we will use to access the module:

static const uint8_t initamod(CVMUSBReadoutList::a24UserData);
static const uint8_t readamod(CVMUSBReadoutList::a32UserBlock);

// Parameter limits:

// Useful common values:

static XXUSB::CConfigurableObject::limit zero(0);
static XXUSB::CConfigurableObject::limit maxint16(0xffff);

// The register base addresss is in A24 space so it must  live between
// 0x00000000 and 0x00ffffff.
//



static XXUSB::CConfigurableObject::limit maxCsr(0x00ffffff);
static XXUSB::CConfigurableObject::Limits CsrLimits(zero, maxCsr);


// The IPL must be in the range [0..7]

static XXUSB::CConfigurableObject::limit maxIPL(7);
static XXUSB::CConfigurableObject::Limits IPLLimits(zero, maxIPL);

// The 16 bit vector must be in the range [0..7].

static XXUSB::CConfigurableObject::Limits VectorLimits(zero, maxint16);

// For now we won't accept more than 


// For now we won't accept more than 10 events at a time...

static XXUSB::CConfigurableObject::limit minEvents(1);
static XXUSB::CConfigurableObject::limit maxEvents(10);
static XXUSB::CConfigurableObject::Limits EventLimits(minEvents, maxEvents);


// Limits on the thresholds...these are set in V/mv or V and converted so these
// are floating point limits.


XXUSB::CConfigurableObject::flimit fzero(0.0);
XXUSB::CConfigurableObject::flimit lldhigh(819.2); // some slop for fp compares
XXUSB::CConfigurableObject::flimit hldhigh(8.192); // some slop for fp compares.

static XXUSB::CConfigurableObject::FloatingLimits lldLimits(fzero, lldhigh);
static XXUSB::CConfigurableObject::FloatingLimits hldLimits(fzero, hldhigh);

// Limits on the value of the ID 16 bit word:

static XXUSB::CConfigurableObject::Limits idLimits(zero, maxint16);


// The mask on the word count.  A spectroscopy event should
// have at most 8 channels a header, a trailer and 2 longs
// of timestamp (12 longs)... however the ADC is a multhihit
// adc so we'll allow up to 31 longs:
//
static const uint16_t transferMask(0x1f);


///////////////////////////////////////////////////////////////
//   Constructors and other canonical methods

/*!
  Construction is largely a no-op.  The realwork is done
  in the onAttach method when the object gets a configuration
  and sets up the parameters it recognizes.
*/
CNADC2530::CNADC2530() : 
  m_pConfiguration(0)
{
}
#ifndef MVLC_GENERATOR
/*!
  Copy construction snapshots  the configuration

*/
CNADC2530::CNADC2530(const CNADC2530& rhs) :
  m_pConfiguration(0),
  m_csr(rhs.m_csr),
  m_csrValue(rhs.m_csrValue),
  m_eventBase(rhs.m_eventBase),
  m_eventCount(rhs.m_eventCount)
{
 if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
#endif
/*!
   
Destructor leaks for now... if copy constructed.
*/
CNADC2530::~CNADC2530()
{
}
#ifndef MVLC_GENERATOR
/*!
  Assignment is also bogus for now due to danger of recursion loops.
*/
/*!
  Assignment is also bogus for now due to danger of recursion loops.
*/
CNADC2530&
CNADC2530::operator=(const CNADC2530& rhs)
{
  return *this;
}

#endif 
/////////////////////////////////////////////////////////////////////////////
// Implementing the base class interface:
//


/*!
  Attach a configuration to this object.  We create the
  configuration parameters, set them to default values, and provide limits where
  appropriate.  
  
  See the class header for a description of the configuration parameters.

  \param configuration - reference to the configuration of the module. 
*/
void
#ifdef MVLC_GENERATOR
CNADC2530::onAttach(XXUSB::CConfigurableObject& configuration) 
#else
CNADC2530::onAttach(CReadoutModule& configuration) 
#endif
{
  // Save our configuration so we can query it later.

  m_pConfiguration = &configuration;

  // Now setup  our configuration parameter set.

  // The module has two address spaces.  The register space is defined by jumpers on the
  // board and is in A24 space, while the data memory is in A32 space and programmed.
  // the -csr parameter must match the jumpers, the -memory parameter is used to 
  // program the data memory addresss.  Only the top 11 bits are meaningful.
  // the rest will be forced -> 0.

  m_pConfiguration->addParameter("-csr", XXUSB::CConfigurableObject::isInteger, 
				 static_cast<void*>(&CsrLimits), "0");
  m_pConfiguration->addParameter("-memory", XXUSB::CConfigurableObject::isInteger,
				 NULL, "0");

  //  Next set of parameters determine how the module interrupts.  This includes
  // the values of the IPn and IE bits in the CSR as well as the
  // value programmed in the vector register.
  //
  // -ipl - The interrupt priority level, the value programmed inthe Ipn bits
  //        If nonzero, IE is also set which enables the interrpts.
  // -vector - Determines the value set in the Vector register, which is the status/Id 
  //           response of the module to the interrupt acknowledge cycle.
  // 

  m_pConfiguration->addParameter("-ipl", XXUSB::CConfigurableObject::isInteger,
				 static_cast<void*>(&IPLLimits), "0");
  m_pConfiguration->addParameter("-vector", XXUSB::CConfigurableObject::isInteger,
				 static_cast<void*>(&VectorLimits), "0");

  //  Set up the discriminator values.
  // -lld  - The low level discriminator.
  // -hld  - The high level discriminator.
  //  These are floating parameters, so that they can be set in 'sensible' units.

  m_pConfiguration->addParameter("-lld", XXUSB::CConfigurableObject::isFloat,
				 static_cast<void*>(&lldLimits), "0.0");
  m_pConfiguration->addParameter("-hld", XXUSB::CConfigurableObject::isFloat,
				 static_cast<void*>(&hldLimits), "8.191");

  // The module is a multi-event adc, in interrupt mode, we can determine how many events
  // are accepted before the module interrutps.  This allows for somewhat faster processing
  // by amortizing interrupt overhead across several events.
  // -events determines how many events are read per interrupt.  It's up to the
  // analysis software to deal with the fact that multiple events can occur in one VM-USB superevent.
  //

  m_pConfiguration->addParameter("-events", XXUSB::CConfigurableObject::isInteger,
				 static_cast<void*>(&EventLimits), "1");

  // The module can zero suppress the data  That's controlled by the 
  // -zerosuppress bool:

  m_pConfiguration->addParameter("-zerosuppress", XXUSB::CConfigurableObject::isBool,
				 static_cast<void*>(0), "on");

  // A marker word will precede the ADC data in order to unambiguously identify
  // the data from an adc:

  m_pConfiguration->addParameter("-id", XXUSB::CConfigurableObject::isInteger,
				static_cast<void*>(&idLimits), "0");

}

/*!
   When the system is about ready to start data taking, the Initialize member is called
   to set up the module.  The module is set up in accordance with the
   configuration parameters defined  in onAttach. 
   Once thec csr address has been extracted, we'll verify that the module is, in fact
   a hytec adc by reading the ID and TYPE registers... comparing them
   with ID_VALUE and TYPE_VALUE respectively.

   If the -csr parameter does not specify the address of a hytec module CSR page,
   a descriptive string exception is thrown which should abort the begin of run.

   \param  controller - VM-USB controller object that allows us to manipulate the module.
*/
void
CNADC2530::Initialize(CVMUSB& controller)
{
  // Get the register page and list mode data addresses.

  uint32_t csr  = m_pConfiguration->getUnsignedParameter("-csr");
  uint32_t list = m_pConfiguration->getUnsignedParameter("-memory");
  uint16_t listRegister = (uint16_t)((list >> 16) & 0xffe0); // Value for list register.

  // Check the validity of the -csr. If valid, program the memory base.
  // read failures are either VM-USB problem or, potentially I'm pointed into
  // VME addresses with no responders.
  //  In the MVLC case we're not connected to a module so we have to take the user's word.
#ifndef MVLC_GENERATOR
  uint16_t   id, model;
  int status  = controller.vmeRead16(csr + REG_ID, initamod, &id);
  if (status) {
    throw "Could not read ID register of NADC2530";
  }
  status = controller.vmeRead16(csr + REG_TYPE, initamod, &model);
  if (status) {
    throw "Could not read TYPE register of NADC 2530";
  }
  if ((id != ID_VALUE) || (model != TYPE_VALUE)) {
    char message[1024];
    sprintf(message,"Module at 0x%08x has an invalid ID (was %x sb %x) or Model (was %d sb %d)",
	    csr, id, ID_VALUE, model, TYPE_VALUE);
    throw string(message);
  }
#endif
  // Disable the module and set its counters to something reasonable.

  controller.vmeWrite16(csr + REG_CSR, initamod, (uint16_t)(CSR_RESET | CSR_CLTS));
  controller.vmeWrite16(csr + REG_MEM, initamod, listRegister);
  controller.vmeWrite16(csr + REG_LISTWL, initamod, (uint16_t)0);	// offset where data will be put.
  controller.vmeWrite16(csr + REG_LISTWH, initamod, (uint16_t)0);
  controller.vmeWrite16(csr + REG_EVENTSL, initamod, (uint16_t)0); // Events acquired counter.
  controller.vmeWrite16(csr + REG_EVENTSH, initamod, (uint16_t)0);



  // Set up all values but the CSR

  int    vector       = m_pConfiguration->getIntegerParameter("-vector");
  int    events       = m_pConfiguration->getIntegerParameter("-events");
  double lld          = m_pConfiguration->getFloatParameter("-lld");
  double hld          = m_pConfiguration->getFloatParameter("-hld");
  bool   zerosuppress = m_pConfiguration->getBoolParameter("-zerosuppress");

  controller.vmeWrite16(csr + REG_VECTOR, initamod, (uint16_t)vector);
  controller.vmeWrite16(csr + REG_EVENTSREQ, initamod, (uint16_t)(events - 1)); // IRQ when > than this
  controller.vmeWrite16(csr + REG_LLD, initamod, lldToRegister(lld));
  controller.vmeWrite16(csr + REG_HLD, initamod, hldToRegister(hld));
  controller.vmeWrite16(csr + REG_FULLNESS, initamod, (uint16_t)0);


  // Figure out the CSR value based on the IPL etc, and set it.. which enables the module.
  // 

  uint16_t csrValue = CSR_GE | CSR_IT | CSR_ARM;
  if (!zerosuppress) {
    csrValue |= CSR_ZE;		// Setting this bit turns off zero suppression.
  }
  int ipl = m_pConfiguration->getIntegerParameter("-ipl");
  if (ipl) {
    csrValue |= CSR_IE;
    csrValue |= (ipl << CSR_IPLSHIFT) & CSR_IPLMASK;
  }
  controller.vmeWrite16(csr + REG_CSR, initamod, csrValue);

  // Save a few values that will be useful later on:

  m_csr       = csr;
  m_csrValue  = csrValue;	// What to set the CSR to after an event.
  m_eventBase = list;		// Where the events will live.
  m_eventCount= events;         // Determines how many events are read on a trigger.

}
/*!
   Add the VME transactions needed to read the module. Since I'm lazy, the following
   data elements have been saved so that I don't have to recompute them from the configuration:
   - m_csr        - Address of the register page.
   - m_csrValue   - Value to program into the CSR to re-arm the module.
   - m_eventBase  - Base address of the list mode data.
   - m_eventCount - Number of events in each trigger.

   \param list  - The CVMUSBReadoutList that will be loaded into the VM-USB to 
                  read out this 'stack'.

*/
void
CNADC2530::addReadoutList(CVMUSBReadoutList& list)
{

  // Disarm for the durationof the read:

  list.addWrite16(m_csr + REG_CSR, initamod, (uint16_t)(m_csrValue & (~CSR_ARM)));


  // insert the marker words, the id and the transfer count mask:


  list.addMarker(static_cast<uint16_t>(m_pConfiguration->getUnsignedParameter("-id")));
  //list.addDelay(1);
  list.addMarker(transferMask);
  //  list.addDelay(1);


  // The number of words to read is held by the List Mode Memory Address Counter 
  // I'm going to make the assumption that only the low order 16 bits have set data
  // as otherwise I can't make this work with the VM-USB
  // 


  list.addBlockCountRead16(m_csr +  REG_LISTWL, 
			   static_cast<uint32_t>(transferMask), initamod); // Get the block read count


  list.addMaskedCountBlockRead32(m_eventBase, readamod);	    // count set up above.

  
  // We're going to zero the offset regiseter so the next set of events is also
  // at the bottom of the memory, and then reset the CSR with the m_csrValue:

  list.addWrite16(m_csr + REG_LISTWL, initamod, (uint16_t)0);
  list.addWrite16(m_csr + REG_LISTWH, initamod, (uint16_t)0);

  // if interruptiung: Zero the event count register so we don't just interrupt again:

  if (m_pConfiguration->getIntegerParameter("-ipl") > 0) {

    list.addWrite16(m_csr + REG_EVENTSL, initamod, (uint16_t)0);
    list.addWrite16(m_csr + REG_EVENTSH, initamod, (uint16_t)0);
  }

  // Rearm the module.

  list.addWrite16(m_csr + REG_CSR, initamod, m_csrValue);
}
#ifndef MVLC_GENERATOR
/*!

  Clone the object... produce an exact duplicate of this.
  This is effectively a virtual copy constructor.

*/
CReadoutHardware*
CNADC2530::clone() const
{
  return new CNADC2530(*this);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////
//  Private utility functions.
//

/* 
  Convert the low level threshold value (millivolts) to 
  the value to program in the lld register.
  Note that the configuration subsystem has range checked the value.
*/
uint16_t
CNADC2530::lldToRegister(double lld)
{
  double volts = lld/1000.0;	// Millivolts -> volts.

  // This is taken from section 5.11 of the hytec manual.

  int intermediate = (int)((volts/3.2764)/(0.25/4095));
  return (uint16_t)((intermediate << 2) & 0x3ffc);
}
/*
  Convert the high level threshold value (volts) to the value to program in the hld register.
  Note that the configuration subsystem has already range checked the value.
*/
uint16_t
CNADC2530::hldToRegister(double hld)
{
  // hld is already in volts:

  int intermediate = (int)(((hld/3.2764) - 2)/(0.5/4095));
  return (uint16_t)((intermediate << 2) & 0x3ffc);
}

#ifdef MVLC_GENERATOR
////////////////////////////// implmenet the Nadc2530Command.

/**
 *  constructor - nothing special, register as "hytec"
 */
Nadc2530Command::Nadc2530Command(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "hytec", parser) {}


/**
 * destructor:
 */
Nadc2530Command::~Nadc2530Command() {}

/** 
 * createDevice.
 * 
 */
CReadoutModule* 
Nadc2530Command::createDevice(std::string name) {
  auto result = new CReadoutModule;
  result->SetDriver(new CNADC2530);

  return result;
}
#endif