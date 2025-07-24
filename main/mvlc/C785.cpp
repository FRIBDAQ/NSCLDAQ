/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

// Implementation of the C785 class VM-USB support for the CAEN V785.



#include "C785.h"

#include "CReadoutModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include <XXUSBConfigurableObject.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

#include <iostream>
using namespace std;

////////////////////////////////////////////////////////////////////////
///////////////////////// Local constants //////////////////////////////
////////////////////////////////////////////////////////////////////////
#define Const(name) static const int name  = 

// Address modifiers used to access the module:

static const  uint8_t initamod(CVMUSBReadoutList::a32UserData);
static const uint8_t readamod(CVMUSBReadoutList::a32PrivBlock);
static const uint8_t cbltamod(CVMUSBReadoutList::a32PrivBlock);

// Register map (offsets in bytes) for the V785
//   Not an exhaustive list as the various test registers are omitted.
//

Const(eventBuffer)      0;
Const(firmware)    0x1000;
Const(GEO)         0x1002;
Const(McastAddr)   0x1004;
Const(BSet1)       0x1006;
Const(BClear1)     0x1008;
Const(IPL)         0x100a;
Const(Vector)      0x100c;
Const(Status1)    0x100e;
Const(Control1)    0x1010;
Const(AderHigh)    0x1012;
Const(AderLow)     0x1014;
Const(RESET)       0x1016;	// Better to use Bset according to manual.
Const(McastCtl)    0x101a;
Const(EventTrig)   0x1020;
Const(Status2)     0x1022;
Const(EventCountL) 0x1024;
Const(EventCountH) 0x1026;
Const(IncrementEC) 0x1028;
Const(IncrementO)  0x102a;
Const(LoadTest)    0x102c;
Const(FastClear)   0x102e;
Const(BSet2)       0x1032;
Const(BClear2)     0x1034;
Const(LogicalCrate) 0x103c;
Const(ECountReset) 0x1040;
Const(FSRange)     0x1060;
Const(Iped)        0x1060;
Const(Thresholds)   0x1080;	// Continues through 10bf, 32 D16 words.

// Prom locations: We'll want to be sure the module really is a V785:
// Our mnemonics are a bit different from the CAEN ones:

Const(BoardIDHSB) 0x8036;	// Highest significant byte of the id
Const(BoardIDMSB) 0x803a;	// Middle significant byte of the id.
Const(BoardIDLSB) 0x803e;	// Lowest significant byte of the id.

// Bits in the mcast/cblt control register:

Const(CbltLeft)   2;
Const(CbltMiddle) 3;
Const(CbltRight)  1;
Const(CbltDisabled) 0;

// One last set of constants. This is the maximum number of longwords  a
// module might store:

Const(MaxEventSize)  34;           // Longwords in the largest event.
Const(MEBDepth)      32;	   // Max events the adc can buffer.
Const(MaxLongwordsBuffered) (MaxEventSize*MEBDepth);
Const(EventsPerRead)  1;


/////////////////////////////////////////////////////////////////////
///////// Parameters for validity checkers (e.g. ranges)  ///////////
/////////////////////////////////////////////////////////////////////

// -geo is a 5 bit unsigned field:

static XXUSB::CConfigurableObject::limit GeoLow(0);
static XXUSB::CConfigurableObject::limit GeoHigh(0x1f);
static XXUSB::CConfigurableObject::Limits GeoLimits(GeoLow, GeoHigh);

// -threshold requires 32 integers in the range [0, 0xff]

static XXUSB::CConfigurableObject::limit ThreshLow(0);  
static XXUSB::CConfigurableObject::limit ThreshHi(0xff);
static XXUSB::CConfigurableObject::Limits ThreshLimits(ThreshLow, ThreshHi);
static XXUSB::CConfigurableObject::limit  thresholdSize(32);
static XXUSB::CConfigurableObject::ListSizeConstraint threshListSize = {thresholdSize,
                                                                 thresholdSize};
XXUSB::CConfigurableObject::TypeCheckInfo threshValueOk(XXUSB::CConfigurableObject::isInteger,
						 &ThreshLimits);
static XXUSB::CConfigurableObject::isListParameter threshValidity(thresholdSize,
							   thresholdSize,
							   threshValueOk);

							      
// -ipl is in the range 0-7

static XXUSB::CConfigurableObject::limit iplLow(0);
static XXUSB::CConfigurableObject::limit iplHigh(7);
static XXUSB::CConfigurableObject::Limits iplRange(iplLow, iplHigh);

// - vector is in the range 0 0xff

static XXUSB::CConfigurableObject::limit vectorLow(0);
static XXUSB::CConfigurableObject::limit vectorHigh(0xff);
static XXUSB::CConfigurableObject::Limits vectorRange(vectorLow, vectorHigh);

// -highwater is in the range 0 through MEBDepth.

static XXUSB::CConfigurableObject::limit hwlow(0);
static XXUSB::CConfigurableObject::limit hwhigh(MEBDepth);
static XXUSB::CConfigurableObject::Limits hwRange(hwlow, hwhigh);

// -fastclear is in the range 0 through 0x3ff.

static XXUSB::CConfigurableObject::limit  fcLow(0);
static XXUSB::CConfigurableObject::limit  fcHigh(0x3ff);
static XXUSB::CConfigurableObject::Limits fcRange(fcLow, fcHigh);

// -timescale is from 140-1200.

static XXUSB::CConfigurableObject::limit tsLow(140);
static XXUSB::CConfigurableObject::limit tsHigh(1200);
static XXUSB::CConfigurableObject::Limits tsRange(tsLow, tsHigh);

// -iped is from 0-255.

static XXUSB::CConfigurableObject::limit ipedLow(0);
static XXUSB::CConfigurableObject::limit ipedHigh(255);
static XXUSB::CConfigurableObject::Limits ipedRange(ipedLow, ipedHigh);

static const char* inputsEnum[] = {"ribbon", "nim", 0};

static const char* typeEnum[] = {"adc", "qdc", "tdc"};

//////////////////////////////////////////////////////////////////////
/////////////////// Canonical class/object operations ////////////////
//////////////////////////////////////////////////////////////////////

/*! 

   Construction is largely a no-op.   The configuration pointer
   is set by onAttach which is called by the base class constructor.
   It's nice to have some easy methods for a change.

*/
C785::C785() :
  m_pConfiguration(0)
 {}

C785::~C785() {}



///////////////////////////////////////////////////////////////////////
//////////////////////// object operations ////////////////////////////
///////////////////////////////////////////////////////////////////////

/*!
   This function requests the module to add itself as a 
   member of a CBLT readout chain.  CBLT readout chain elements have the
   following attributes:
   - A base address used to read the chain as a whole.
   - A position for each module in the chain as defined by the
     C785::Position type.
   
  \note This function operates immediately on the hardware. 
        therefore, the base address must already have been configured
	in the module so that it can be located.  Typically this function
	would, in fact, be called after module initialization by a chain
        object.. e.g. the chain object would initialize all chain members
	and then invoke this function for each chain members to get them
	ready for action.

   \param controller CVMUSB&
          The VMUSB controller object used to interact with the VME bus
	  and therefore the module.
   \param mcstAddress uint32_t
          The address to be programmed into the module's MCST/CBLT 
	  address register (e.g. where the read will be directed.
   \param where C785::Position
          The position of the module in the chain.  This can be one of:
	  C785::leftmost, C785::middle, or C785::rightmost.
     and determines how the module's MCST/CBLT control register will be 
     programmed.
*/
void
C785::addToChain(CVMUSB& controller,
		 uint32_t mcstAddress,
		 C785::Position where)
{
  // Get the base address of the module:

  uint32_t baseAddress = getIntegerParameter("-base");

  // Set the MCST/CBLT register:

  controller.vmeWrite16(baseAddress+McastAddr, initamod, 
			 static_cast<uint16_t>(mcstAddress >> 24));

  // Figure out the bits for the McastCtl register>

  uint16_t controlWord;
  switch (where) {
  case  leftmost:
    controlWord = CbltLeft;
    break;
  case rightmost:
    controlWord = CbltRight;
    break;
  case middle:
    controlWord = CbltMiddle;
    break;
  default:
    // Disable:

    controlWord = CbltDisabled;
  }
  controller.vmeWrite16(baseAddress+McastCtl, initamod, controlWord);
  
}

/////////////////////////////////////////////////////////////////////////
//////////////////////// overridable operations ////////////////////////
////////////////////////////////////////////////////////////////////////
/*!
   This is called when the configuration object is attached to us.
   what we need to do is create all of the parametrs and set their default
   values  The parameters that have default values are:

\verbatim
   Parameter                 Default
   -thresholds               list of 32 0's.
   -smallthresholds          false
   -ipl                      6 (interrupt 6 is expected by the rdo thread.
   -vector                   0x80
   -highwater                MEBDepth*3/4  (3/4 full event buffer).
   -fastclear                0
   -supressrange             true
   -supressunderthreshold    true
   -supressoverflow          true
   -timescale                600 (ns).  (775 only)
   -commonstop               false      (775 only)
   -iped                     180        (QDC's only).
   -type                     adc* | tdc | qdc  - Device type.
\endverbatim

   All others have no default values.   If, during initialization one of those
   is fetched an string exception of the sort:
   "Required configuration parameter $parameter_name was not supplied."

   \param configuration : CReadoutModule&
       The Readout module which will hold our configuration.
*/

void
C785::onAttach(XXUSB::CConfigurableObject& configuration)
{
  m_pConfiguration = &configuration;

  // Create the parameters... setting defaults as neeed.

  m_pConfiguration->addParameter("-base", XXUSB::CConfigurableObject::isInteger,
				 NULL, "0");
  m_pConfiguration->addParameter("-geo", XXUSB::CConfigurableObject::isInteger,
				 &GeoLimits);
  m_pConfiguration->addParameter("-thresholds", XXUSB::CConfigurableObject::isIntList,
				 &threshValidity,
		 "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
  m_pConfiguration->addParameter("-smallthresholds", XXUSB::CConfigurableObject::isBool,
				 NULL, "false");
  m_pConfiguration->addParameter("-ipl", XXUSB::CConfigurableObject::isInteger,
				 &iplRange, "0");
  m_pConfiguration->addParameter("-vector", XXUSB::CConfigurableObject::isInteger,
				 &vectorRange, "0x80");

  // The high water mark default is computed for cleanliness.

  char hwdefault[100];
  sprintf(hwdefault, "%d", MEBDepth*3/4);
  m_pConfiguration->addParameter("-highwater", XXUSB::CConfigurableObject::isInteger,
				 &hwRange, string(hwdefault));

  m_pConfiguration->addParameter("-fastclear", XXUSB::CConfigurableObject::isInteger,
				 &fcRange, "0");
  m_pConfiguration->addParameter("-supressrange", XXUSB::CConfigurableObject::isBool,
				 NULL, "true");
  m_pConfiguration->addParameter("-supressunderthreshold", XXUSB::CConfigurableObject::isBool,
				 NULL, "false");
  m_pConfiguration->addParameter("-supressoverflow", XXUSB::CConfigurableObject::isBool,
				 NULL, "false");
  m_pConfiguration->addParameter("-timescale", XXUSB::CConfigurableObject::isInteger,
				 &tsRange, "600");

  m_pConfiguration->addParameter("-iped", XXUSB::CConfigurableObject::isInteger,
				 &ipedRange, "180");

  m_pConfiguration->addParameter("-requiredata", XXUSB::CConfigurableObject::isBool,
				 NULL, "true");
  m_pConfiguration->addParameter("-commonstop", XXUSB::CConfigurableObject::isBool,
				 NULL, "false");

  // Issue 201 :

  m_pConfiguration->addEnumParameter("-inputs", inputsEnum, "ribbon");

  m_pConfiguration->addEnumParameter("-type", typeEnum, "adc");
  
}
/*!
    Initialize the module prior to data taking. We get the parameters
    from the configuration and
    - Ensure this module is a V785 by reading the module type.
    - Program the module according to the configuration.
    - Program the module to BERR when empty of events.

    Note that it will be up to the caller to ensure that our vector is used
    to trigger the list that will eventually be added to by addReadoutList.
    This is probably best done by having the vector/ipl set programmatically
    rather than by the user through his/her configuration file.

    \param controller : CVMUSB&
       Reference to a VM-USB controller object that will be used to program
       the module.  For the most part we'll do this programming with single
       shot operations, rather than a list as this is not performance critical.

    \throw string
        If the module is not a V785 as determined by reading it's module id
	from its configuration prom, then we will throw this exception
	the string will be of the form:
	"Module config-name at base base-address is not a V785."

  @todo - do we need to add a -type option to specify the type of module e.g. 'adc', 'tdc' 'qdc' or
  are the 'extran' initializations for all 'harmless'?
*/
void
C785::Initialize(CVMUSB& controller)
{
  unsigned base = getIntegerParameter("-base");
  auto type = m_pConfiguration->cget("-type");
  // Ok now we've established that we have a V785 we can program it.
  // programming access use the address modifier initamod:
  
  // first reset it to a nice power up configuration... this is done by
  // flipping the the SOFT.RESET bit in the bit set 1 register.
  // sett fig 4.12 in the V785 manual.

  controller.vmeWrite16(base+BSet1, initamod, (uint16_t)0x80);
  controller.vmeWrite16(base+BClear1, initamod, (uint16_t)0x80);

  // turn off multicast and set the mcast address to zero...
  // this latter because that tries to ensure there's not a collision between
  // The mcast and rotary base addresses.
  //
  controller.vmeWrite16(base+McastAddr, initamod, (uint16_t)0);
  controller.vmeWrite16(base+McastCtl, initamod, (uint16_t)0);

  // Set the GEOgraphical address of the module.

  uint16_t geo = getIntegerParameter("-geo");
  controller.vmeWrite16(base+GEO, initamod, geo);
  controller.vmeWrite16(base+BSet1, initamod, (uint16_t)0x80);
  controller.vmeWrite16(base+BClear1, initamod, (uint16_t)0x80);

  // Set the thresholds for the module:
  // As well as the meaning of the thresholds.

  vector<uint16_t> thresholds;
  getThresholds(thresholds);
  for (int chan = 0; chan < 32; chan++) {
    uint16_t threshold = thresholds[chan];
    controller.vmeWrite16(base+Thresholds+chan*sizeof(uint16_t), initamod, 
			  threshold);
  }
  if (getBoolParameter("-smallthresholds")) {
    controller.vmeWrite16(base+BSet2, initamod, (uint16_t)0x100);	// big thresholds.
  } 
  else {
    controller.vmeWrite16(base+BClear2, initamod, (uint16_t)0x100); // small thresholds.
  }
  
  // Set the interrupt characteristics of the module:

  uint16_t ipl     = getIntegerParameter("-ipl");
  uint16_t IVector = getIntegerParameter("-vector");
  uint16_t WhenIRQ = getIntegerParameter("-highwater");

  controller.vmeWrite16(base+IPL,       initamod, ipl);
  controller.vmeWrite16(base+Vector,    initamod, IVector);
  //
  // For now irq on first event:
  //
  //   controller.vmeWrite16(base+EventTrig, initamod, WhenIRQ);
  //
  controller.vmeWrite16(base+EventTrig, initamod, (uint16_t)1);

  // Set the fast clear window:

  uint16_t fclearValue = getIntegerParameter("-fastclear");
  controller.vmeWrite16(base+FastClear, initamod, fclearValue);

  // Set the supression.

  bool supressed = getBoolParameter("-supressrange");
  if (!supressed) {		// Set means disable checks.
    controller.vmeWrite16(base+BSet2, initamod, (uint16_t)0x18);
  }
  else {
    controller.vmeWrite16(base+BClear2, initamod, (uint16_t)0x18);
  }

	if (!supressed) {
		// Set the supression of under threshold.

		bool underthresholdsupressed = getBoolParameter("-supressunderthreshold");
		if (!underthresholdsupressed) {		// Set means disable checks.
			controller.vmeWrite16(base+BSet2, initamod, (uint16_t)0x10);
		}
		else {
			controller.vmeWrite16(base+BClear2, initamod, (uint16_t)0x10);
		}

		// Set the supression of overflow.

		bool overflowsupressed = getBoolParameter("-supressoverflow");
		if (!overflowsupressed) {		// Set means disable checks.
			controller.vmeWrite16(base+BSet2, initamod, (uint16_t)0x8);
		}
		else {
			controller.vmeWrite16(base+BClear2, initamod, (uint16_t)0x8);
		}
	}

  // If the user chooses to require data even if the module
  // is 'empty' take care of that too:

  bool requireData = getBoolParameter("-requiredata");
  if (requireData) {
    controller.vmeWrite16(base+BSet2, initamod, (uint16_t)0x1000);
  } 
  else {
    controller.vmeWrite16(base+BClear2, initamod, (uint16_t)0x1000);
  }

  // If the module is a 775 write the timerange register.

  if (type == "tdc") {
    int    range   = getIntegerParameter("-timescale");
    int    reg2Offset;
    //
    // Compute the register value (see 4.33 of the V775 manual).
    //
    float  nsRange = static_cast<float>(range);
    float  rRange  = 36040.0/(nsRange + 1.3333);
    controller.vmeWrite16(base+FSRange, initamod, static_cast<uint16_t>(rRange + 0.5));

    // Set common start/stop mode.

    bool commonStop = getBoolParameter("-commonstop");
    if (commonStop) {
      
      reg2Offset = BSet2;
    } 
    else {
      reg2Offset = BClear2;
    }
    controller.vmeWrite16(base + reg2Offset, initamod,
			  static_cast<uint16_t>(0x400));
  }
  // QDC specific settings:
  if (type == "qdc") { //(type == 792) || (type == 862) || (type == 965)) {
    int iped = getIntegerParameter("-iped");
    controller.vmeWrite16(base + Iped, initamod, static_cast<uint16_t>(iped));
  }

  // Finally, ensure that at the end of a readout we'll get a BERR, rather than
  // we'll just write control regiseter 1 the way we want it:
  // Bits:      CAEN Mnemonic   Meaning:
  // 4          BLNKKEND  (on)  Berr at end of event.
  // 0x10       PROG RESET(off) Front panel reset only clears MEB.
  // 0x20       BERR ENABLE(on) BERR after end of data rather than inv data.
  // 0x40       ALIGN64 (off)   Alighn output to 64 bit boundaries.
  // All this makes the mask:
  //    0x24
  //
  controller.vmeWrite16(base+Control1, initamod, (uint16_t)0x24);


 
}
/*!
   Add the module read to a readout list.  In this case we will
   add MaxLongwordsBuffered*2 reads in fifo mode to the list.
   The *2 is because we're still acquiring data when the read is
   happening...and we really want to drain the device
   \param list : CVMUSBReadoutList&
       Reference to the readout list to append to.

*/
void
C785::addReadoutList(CVMUSBReadoutList& list)
{
  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
  // only read 1 event.
  //
  for (int i =0; i < EventsPerRead; i++) {

    list.addFifoRead32(base,
		       readamod, static_cast<size_t>(MaxEventSize+16));

    // Clear event buffers:

    list.addWrite16(base + BSet2,
		    initamod, (uint16_t)4);
    list.addWrite16(base + BClear2,
		    initamod, (uint16_t)4);
  }
}



/////////////////////////////////////////////////////////////////////
//////////////////// Private utility functions //////////////////////
/////////////////////////////////////////////////////////////////////

// Return the value of an integer parameter.
// Parameters:
//    std::string name - name of the parameter.
// Returns:
//    value
// Throws a string exception (from cget) if there is no such parameter.
// caller is responsible for ensuring the parameter is an int.
//
unsigned int
C785::getIntegerParameter(string name)
{
  string sValue =  m_pConfiguration->cget(name);
  unsigned int    value  = strtoul(sValue.c_str(), NULL, 0);

  return value;
}
//  Return the value of a bool parameter.
// Parameters:
//    std::string name - name of the parameter.
// Returns:
//   true if the value is one of: true, yes, 1, on, enabled.
bool
C785::getBoolParameter(string name)
{
  string sValue = m_pConfiguration->cget(name);
  set<string> trueValues;
  trueValues.insert("true");
  trueValues.insert("yes");
  trueValues.insert("yes");
  trueValues.insert("1");
  trueValues.insert("on");
  trueValues.insert("enabled");


  return (trueValues.count(sValue) != 0);
}
// Get the values of the thresholds  The thresholds should be a list
// of integer values.. which we will stuff into a vector.
// Parameters:
//    vector<uint16_t>& thresholds  - a vector to which the thresholds will be appended.
// 
void
C785::getThresholds(vector<uint16_t>& thresholds)
{
  int    argc;
  const char **argv;
  string sValue = m_pConfiguration->cget("-thresholds");
  Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);

  assert(argc == 32);		// Validator should have done this.

  for(int i =0; i < 32; i++) {
    thresholds.push_back(static_cast<uint16_t>(strtol(argv[i], NULL, 0)));
  }
  

  Tcl_Free((char*)argv);

 
}
////////////////////////////////////////// Implement the 'adc' command //////////////////////////


/**
 * constructor
 * @param interp the interpreter on which to register.
 * @param parser refernces the parser object
 * 
 */
C785Command::C785Command(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "adc", parser) {}

/** 
 * destructor
 */
C785Command::~C785Command() {}


/**
 *  createDevice
 *      Create a new device bound into its containig module.
 * @param name - ignored name of the device.
 */
CReadoutModule* 
C785Command::createDevice(std::string name) {
  auto dev = new C785;
  auto result = new CReadoutModule;
  result->SetDriver(dev);

  return result;
}