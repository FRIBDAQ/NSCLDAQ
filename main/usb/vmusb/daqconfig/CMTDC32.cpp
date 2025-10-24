/**
#******************************************************************************
#
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#***************************************************************************//**
# 



##
# @file MTDC32.cpp
# @brief Support for the Mesytec MTDC32  (implementation)
# @author Ron Fox (rfoxkendo@gmail.com)
*/


#include <CMTDC32.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <MADC32Registers.h>
#include <CReadoutModule.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <errno.h> 
#include <string.h>
#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
#endif

/* Parameter constraint data structures: */


// Values for the -datalen

static const char* dataLenValues[] = {
    "8", "16", "32", "64", NULL
};
static const uint16_t dataLenRegisterValues[] = {
    0, 1, 2, 3    
};

// Values for -multievent

static const char* multiEventModes[] = {
    "off", "on", "limited", NULL
};
static const uint16_t multiEventModeRegisterValues[] = {
    0, 1, 3                                       // Yes 2 is not used.
};

// Values for -marktype:

static const char* markTypes[] = {
    "eventcount", "timestamp", "extended-timestamp", NULL
};
static const uint16_t markTypeRegisterValues[] = {
    0, 1, 3
};

// values for -resolution

static const char* resolutionValues[] = {
    "500ps", "250ps", "125ps", "62.5ps", "31.3ps", "15.6ps",
    "7.8ps", "3.9ps", NULL
};
static const uint16_t resolutionRegisterValues[] = {
    9, 8, 7, 6, 5, 4, 3, 2    
};


// -format values:

static const char* formatValues[] = {
    "standard", "fulltime", NULL
};
static const uint16_t formatRegisterValues[] = {
    0, 1
};

// -edge

static const char* edgeValues[] = {
    "rising", "falling", NULL
};
static const uint16_t edgeRegisterValue[] = {
    0, 1
};

// -busy:

static const char* busyValues[] = {
    "bothbanks", "cbusoutput", "bufferfull", "abovethreshold", NULL
};
static const uint16_t busyRegisterValues[] = {
    0, 3, 4, 8
};

// -timingsource

static const char* timingSources[] = {
    "vme", "external", NULL
};
static const uint16_t timingSourceValues[] = {
    0, 1
};

// -bank0triggersource

static const char* bank0TriggerSources[] = {
    "Tr0", "Tr1",
    "Ch0", "Ch1", "Ch2", "Ch3", "Ch4", "Ch5", "Ch6", "Ch7",
    "Ch8", "Ch9", "Ch10", "Ch11", "Ch12", "Ch13", "Ch14", "Ch15",
    "Ch16", "Ch17", "Ch18", "Ch19", "Ch20", "Ch21", "Ch22", "Ch23",
    "Ch24", "Ch25", "Ch26", "Ch27","Ch28", "Ch29", "Ch30", "Ch31",
    "Bank0","Bank1", NULL
};
static const uint16_t bank0TriggerSrcRegisterValues[] = {
    0x0001, 0x0002,
    0x0080, 0x0084, 0x0088, 0x008c, 0x0090, 0x0094, 0x0098, 0x009c,   // Ch 7 is last one.
    0x00a0, 0x00a4, 0x00a8, 0x00ac, 0x00b0, 0x00b4, 0x00b8, 0x00bc,   // Ch 15...
    0x00c0, 0x00c4, 0x00c8, 0x00cc, 0x00d0, 0x00d4, 0x00d8, 0x00dc,   // Ch 23...
    0x00e0, 0x00e4, 0x00e8, 0x00ec, 0x00f0, 0x00f4, 0x00f8, 0x00fc,   // ch 31.
    0x0100, 0x2000                                                    // Any bank 0..1.
};
static const char** bank1TriggerSources             = bank0TriggerSources;
static const uint16_t* bank1TriggerSrcRegisterValues = bank0TriggerSrcRegisterValues;

#ifndef MVLC_GENERATOR
// for debugging :

static void dumpRegisters(CVMUSB& controller, uint32_t base);
#endif
/**
 * constructor
 *   initialize the configuration member to null since it does not get
 *   set until onAttach
 */
CMTDC32::CMTDC32() :
    m_pConfiguration(0)
{}

/**
 * destructor - nothing to do
 */
CMTDC32::~CMTDC32() {}
#ifndef MVLC_GENERATOR
/**
 * copy construction, if rhs has a configuration copy it into this
 *
 * @param rhs - the object we are initialized from.
 */
CMTDC32::CMTDC32(const CMTDC32& rhs) :
   m_pConfiguration(0)
{
    if (rhs.m_pConfiguration) {
        m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
    }
}


/**
 * assignment
 *    same as copy construction, however we need to delet our
 *    config.
 *
 *   @param rhs - the guy being assigned to us.
 *   @return *this
 */
CMTDC32&
CMTDC32::operator=(const CMTDC32& rhs)
{
    if (this != &rhs) {
        delete m_pConfiguration;       // Delete of 0 is harmless.
        if (rhs.m_pConfiguration) {
            m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
        } else {
            m_pConfiguration = 0;
        }
    }
    return *this;
}
#endif
/*---------------------------------------------------------------------------
 * Implementations of the CReadout hardware interface:
 *
 */

/**
 * onAttach
 *    Called to attach a hardware configuration to an object.
 *    *  If one already exists it is deleted.
 *    *  The new one is saved at m_pConfriguration.
 *    *  THe new configuration is loaded with configuration parameter
 *       definitions.
 *
 *  @param configuration - reference to our new configuration object
 */
void
#ifdef MVLC_GENERATOR
CMTDC32::onAttach(XXUSB::CConfigurableObject& configuration)
#else
CMTDC32::onAttach(CReadoutModule& configuration)
#endif
{
    
    // Take care of the existing configuraiton and
    // replace it with configuration
    
    delete m_pConfiguration;                   // Deleting 0 is ok.
    m_pConfiguration = &configuration;
   
    // Define the configurable parameters... the default values are the default
    // register values for these.
    
    m_pConfiguration->addIntegerParameter("-base");
    m_pConfiguration->addIntegerParameter("-initdelay", 0, 0xff, MADCDELAY);
    m_pConfiguration->addIntegerParameter("-id", 0,255, 0);
    m_pConfiguration->addIntegerParameter("-ipl", 0,7,0);
    m_pConfiguration->addIntegerParameter("-vector", 0,255, 0);
    m_pConfiguration->addIntegerParameter("-irqthreshold", 0, 0x7fff, 1);
    m_pConfiguration->addIntegerParameter("-maxtransfers", 0, 0x7fff, 1);
    
    m_pConfiguration->addEnumParameter("-datalen", dataLenValues, "32");
    m_pConfiguration->addEnumParameter(
        "-multievent", multiEventModes, "off"
    );
    m_pConfiguration->addBooleanParameter("-skipberr", false);
    m_pConfiguration->addBooleanParameter("-countevents", false);
    m_pConfiguration->addEnumParameter(
        "-marktype", markTypes, "timestamp"
    );
    m_pConfiguration->addBooleanParameter("-joinedbanks", true);
    m_pConfiguration->addEnumParameter(
        "-resolution", resolutionValues, "3.9ps"
    );
    m_pConfiguration->addEnumParameter(
        "-format", formatValues, "standard"
    );
    m_pConfiguration->addIntegerParameter("-bank0winstart", 0, 65535, 16*1024-16);
    m_pConfiguration->addIntegerParameter("-bank1winstart", 0, 65535, 16*1024-16);
    m_pConfiguration->addIntegerParameter("-bank0winwidth", 0, 16*1024, 32);
    m_pConfiguration->addIntegerParameter("-bank1winwidth", 0, 16*1024, 32);
    m_pConfiguration->addEnumParameter("-bank0triggersource", bank0TriggerSources, "Tr0");
    m_pConfiguration->addEnumParameter("-bank1triggersource", bank1TriggerSources, "Tr1");
    m_pConfiguration->addBooleanParameter("-bank0firsthit", false);
    m_pConfiguration->addBooleanParameter("-bank1firsthit", false);
    m_pConfiguration->addEnumParameter(
        "-edge", edgeValues, "falling"
    );
    m_pConfiguration->addBooleanParameter("-tr0terminated", false);
    m_pConfiguration->addBooleanParameter("-tr1terminated", false);
    m_pConfiguration->addBooleanParameter("-resetterminated", false);
    m_pConfiguration->addBooleanParameter("-ecltrig1isoscillator", false);
    m_pConfiguration->addBooleanParameter("-trigfromecl", false);
    m_pConfiguration->addBooleanParameter("-nimtrig1isoscillator", false);
    m_pConfiguration->addEnumParameter(
        "-busy", busyValues, "bothbanks"
    );
    m_pConfiguration->addBooleanParameter("-pulseron", false);
    m_pConfiguration->addIntegerParameter("-pulserpattern", 0);
    m_pConfiguration->addIntegerParameter(
        "-bank0threshold", 0, 255, 105
    );
    m_pConfiguration->addIntegerParameter(
        "-bank1threshold", 0, 255, 105
    );
    m_pConfiguration->addEnumParameter(
        "-timingsource", timingSources, "vme"
    );
    m_pConfiguration->addIntegerParameter("-tsdivisor", 1, 65535, 1);  // avoid 65536 special case.
    
    m_pConfiguration->addBooleanParameter("-tstamp");
    m_pConfiguration->addIntegerParameter("-multlow0", 0, 255, 0);
    m_pConfiguration->addIntegerParameter("-multhi0", 0, 255, 255);
    m_pConfiguration->addIntegerParameter("-multlow1", 0, 255, 0);
    m_pConfiguration->addIntegerParameter("-multhi1", 0, 255, 255);
}
/**
 * Initialize
 *    Initialize the module for data taking according to the
 *    configuration parameter values.
 *
 *   @param controller - reference to the VM-USB controller object
 *                       which can perform operations on our behalf.
 */
void
CMTDC32::Initialize(CVMUSB& controller)
{
  bool joinedbanks = m_pConfiguration->getBoolParameter("-joinedbanks");
    // Reset the device and wait for it to settle.

  //  std::cerr << std::hex;

    uint32_t base = m_pConfiguration->getIntegerParameter("-base");
    controller.vmeWrite16(base+Reset, initamod, static_cast<uint16_t>(0));
    //    std::cerr << "MTDC Reset\n";
    sleep(1);
    
    // turn off data acquisition and reset any data stuck in the module fifos:

    //    std::cerr << StartAcq << " " << 0 << std::endl;

    controller.vmeWrite16(base+StartAcq, initamod, static_cast<uint16_t>(0));

    //    std::cerr << ReadoutReset << " " << 0;

    controller.vmeWrite16(base+ReadoutReset, initamod, static_cast<uint16_t>(0));
    
    // A lot of actions are needed to initialize the device. Therefore these are all
    // built into a list.
    
    CVMUSBReadoutList list;

    // Unipolar channel discriminator levels:

    addWrite(list, base+MTDCBank0InputThr,
	     m_pConfiguration->getIntegerParameter("-bank0threshold"));
    if (!joinedbanks) {
      addWrite(list, base+MTDCBank1InputThr,
	       m_pConfiguration->getIntegerParameter("-bank1threshold"));
    }

    // Set up the operation mode registers
    
    addWrite(list, base+BankOperation, static_cast<uint16_t>(joinedbanks ? 0 : 1));
    addWrite(list, base+Resolution,
	     resolutionRegisterValues[m_pConfiguration->getEnumParameter("-resolution", resolutionValues)]);

    addWrite(list, base+OutputFormat,
        formatRegisterValues[m_pConfiguration->getEnumParameter("-format", formatValues)]);

    addWrite(list, base+MTDCBank0TrigSource,
        bank0TriggerSrcRegisterValues[m_pConfiguration->getEnumParameter("-bank0triggersource", bank0TriggerSources)]);
    if (!joinedbanks) {
      addWrite(list, base+MTDCBank1TrigSource,
	       bank1TriggerSrcRegisterValues[m_pConfiguration->getEnumParameter("-bank1triggersource", bank1TriggerSources)]);
    }

    
    uint16_t firstHit = 0;
    if (m_pConfiguration->getBoolParameter("-bank0firsthit")) {
        firstHit |= 1;
    }
    if (m_pConfiguration->getBoolParameter("-bank1firsthit")) {
        firstHit |= 2;
    }
    addWrite(list, base+MTDCFirstHitOnly, firstHit);
    
    // Program inputs and outputs - Assumes jumpers are for differential inputs.

    if (m_pConfiguration->cget("-edge") == std::string("falling")) {
        addWrite(list, base+MTDCEdgeSelect, 0);
    } else {
        addWrite(list, base+MTDCEdgeSelect, 3);
    }

    
   addWrite(list, base+ECLTermination, getTermination());

   addWrite(list, base+ECLGate1OrTiming, static_cast<uint16_t>(
        m_pConfiguration->getBoolParameter("-ecltrig1isoscillator") ? 1 : 0));
    addWrite(list, base+MTDCTriggerSelect, static_cast<uint16_t>(
        m_pConfiguration->getBoolParameter("-trigfromecl") ? 1 : 0
    ));
    
    addWrite(list, base+NIMGate1OrTiming, static_cast<uint16_t>(
        m_pConfiguration->getBoolParameter("-nimtrig1isoscillator" )? 1 : 0
    ));
    addWrite(list, base+NIMBusyFunction,
        busyRegisterValues[m_pConfiguration->getEnumParameter("-busy", busyValues)]
    );
    
    // Support the pulser for test purposes.
    
    if (m_pConfiguration->getBoolParameter("-pulseron")) {
        addWrite(list, base+TestPulser, 1);
        addWrite(
            list, base+MTDCPulserPattern,
            m_pConfiguration->getIntegerParameter("-pulserpattern")
        );
    } else {
        addWrite(list, base+TestPulser, 0);
    }

    
    // The multiplicity requirements:

    addWrite(list, base+MTDCBank0HighLimit, m_pConfiguration->getIntegerParameter("-multhi0"));
    addWrite(list, base+MTDCBank0LowLimit,  m_pConfiguration->getIntegerParameter("-multlow0"));
    if (!joinedbanks) {
      addWrite(list, base+MTDCBank1HighLimit, m_pConfiguration->getIntegerParameter("-multhi1"));
      addWrite(list, base+MTDCBank1LowLimit,  m_pConfiguration->getIntegerParameter("-multlow1"));
    }


    // Set the trigger matching windows:

    for (int i =0; i < 5; i++) {
      addWrite(list, base+MTDCBank0WinStart, m_pConfiguration->getIntegerParameter("-bank0winstart"));
      addWrite(list, base+MTDCBank0WinWidth, m_pConfiguration->getIntegerParameter("-bank0winwidth"));
      if (!joinedbanks) {
	addWrite(list, base+MTDCBank1WinStart, m_pConfiguration->getIntegerParameter("-bank1winstart"));
	addWrite(list, base+MTDCBank1WinWidth, m_pConfiguration->getIntegerParameter("-bank1winwidth"));
      }
    }
    // Program the counters.  Note that timestamps will come from the
    // Chain via the broadcast so that all modules are simultaneously
    // cleared.

    if(m_pConfiguration->cget("-timingsource") == std::string("vme")) {
        addWrite(list, base+TimingSource, 0);
    } else {
        addWrite(list, base+TimingSource, 1);
    }
    addWrite(list, base+TimingDivisor, m_pConfiguration->getIntegerParameter("-tsdivisor"));

    
    // Module id register:
    
    addWrite(list, base+ModuleId, m_pConfiguration->getIntegerParameter("-id"));
    
    // Interrupt control registers (IPL, vector and threshold etc).
    
    
    addWrite(list, base+Vector, m_pConfiguration->getIntegerParameter("-vector"));
    addWrite(list, base+IrqThreshold, m_pConfiguration->getIntegerParameter("-irqthreshold"));
    addWrite(list, base+MaxTransfer, m_pConfiguration->getIntegerParameter("-maxtransfers"));
    addWrite(list, base+Ipl, m_pConfiguration->getIntegerParameter("-ipl"));
    
    // Set up the way the module will handle the FIFO/event buffer:
    
    addWrite(list, base+DataFormat,
	     dataLenRegisterValues[m_pConfiguration->getEnumParameter("-datalen", dataLenValues)]);
    addWrite(list, base+MultiEvent, computeMultiEventRegister());
    addWrite(list, base+MarkType,
        markTypeRegisterValues[m_pConfiguration->getEnumParameter("-marktype", markTypes)]);
    
        
    // Finally, reset the readout again and start daq:
    
    addWrite(list, base+ReadoutReset, 1);
    addWrite(list, base+InitFifo, 0);
    //  addWrite(list, base+StartAcq, 1);   // Lists must not end in a delay so:

    list.addWrite16(base+StartAcq, initamod, 1);
    
    // Run the list:

    size_t readSize;
    uint8_t rdBuffer[128];

    int status = controller.executeList(list, &rdBuffer, sizeof(rdBuffer), &readSize);
#ifndef MVLC_GENERATOR
    if (status < 0) {
      std::cerr << "MTDC initialization list failed " << status << std::endl;
      std::cerr << strerror(errno) << std::endl;
      throw std::string("Init failed - list execution failed");
    }
    
#endif
    
}
/**
 * addReadoutList
 *
 * Add the instructions needed to read out the single module (note that in CBLT mode
 * the chain actually does this sort of thing)>
 *
 * @param list - vmusb readout list being built up:
 */
void
CMTDC32::addReadoutList(CVMUSBReadoutList& list)
{
  // Need the base:

  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");

  list.addFifoRead32(base + eventBuffer, readamod, (size_t)1024); // really 256 is maximum in single event mode I think.
  list.addWrite16(base + ReadoutReset, initamod, (uint16_t)1);
  //  list.addDelay(5);
    
}
/**
 * setChainAddresses
 *    Called by the chain to insert this module into a CBLT readout with common
 *    CBLT base address and MCST address.
 *
 *   @param controller - The controller object.
 *   @param position   - indicator of the position of the module in chain (begining, middle, end)
 *   @param cbltBase   - Base address for CBLT transfers.
 *   @param mcstBase   - Base address for multicast writes.
 */
void
CMTDC32::setChainAddresses(CVMUSB& controller, CMesytecBase::ChainPosition position,
                           uint32_t cbltBase, uint32_t mcastBase)
{                                                                 
  uint32_t base = m_pConfiguration->getIntegerParameter("-base");

  std::cerr << "Position: " << position << std::endl;
  std::cerr << std::hex << base << std::dec << std::endl;
  // Compute the value of the control register..though we will first program
  // the addresses then the control register:

  uint16_t controlRegister = MCSTENB | CBLTENB; // This much is invariant.
  switch (position) {
  case first:
    controlRegister |= FIRSTENB | LASTDIS;
    std::cerr << "First\n";
    break;
  case middle:
    controlRegister |= FIRSTDIS | LASTDIS;
    std::cerr << "Middle\n";
    break;
  case last:
    controlRegister |= FIRSTDIS | LASTENB;
    std::cerr << "Last\n";
    break;
  }
  std::cerr << "Setting mtdc chain address with " << std::hex << controlRegister << std::dec << std::endl;

  // program the registers, note that the address registers take only the top 8 bits.

  controller.vmeWrite16(base + CbltAddress, initamod, (uint16_t)(cbltBase >> 24));
  controller.vmeWrite16(base + McstAddress, initamod, (uint16_t)(mcastBase >> 24));
  controller.vmeWrite16(base + CbltMcstControl, initamod, (uint16_t)(controlRegister));    
}

/**
 *  initCBLTReadout
 *
 *  Initialize the readout for CBLT transfer (called by chain).
 *    @param controller - the controller object.
 *    @param cbltAddress - the chain block/broadcast address.
 *    @param wordsPerModule - Maximum number of words that can be read by this mod
 */
void
CMTDC32::initCBLTReadout(CVMUSB& controller,
			 uint32_t cbltAddress,
			 int wordsPermodule)
{
  // We need our timing source
  // IRQThreshold
  // VECTOR
  // IPL
  // Timestamp on/off

  // Assumptions:  Internal oscillator reset if using timestamp
  //               ..else no reset.
  //               most modulep arameters are already set up.


  int irqThreshold   = m_pConfiguration->getIntegerParameter("-irqthreshold");
  int vector         = m_pConfiguration->getIntegerParameter("-vector");
  int ipl            = m_pConfiguration->getIntegerParameter("-ipl");
  std::string markType = m_pConfiguration->cget("-marktype");
  bool timestamping  = (markType == "timestamp") || (markType == "extended-timestamp");
  
  // Stop acquistiion
  // ..and clear buffer memory:
  controller.vmeWrite16(cbltAddress + StartAcq, initamod, (uint16_t)0);
  controller.vmeWrite16(cbltAddress + InitFifo, initamod, (uint16_t)0);

  // Set stamping


  // Note the generic configuration already set the correct marktype.

  if(timestamping) {
    // Oscillator sources are assumed to already be set.
    // Reset the timer:

    //    controller.vmeWrite16(cbltAddress + MarkType,       initamod, (uint16_t)1); // Show timestamp, not event count.
    controller.vmeWrite16(cbltAddress + TimestampReset, initamod, (uint16_t)3); // reset all counter.
  }
  else {
    // controller.vmeWrite16(cbltAddress + MarkType,       initamod, (uint16_t)0); // Use Eventcounter.
    controller.vmeWrite16(cbltAddress + EventCounterReset, initamod, (uint16_t)0); // Reset al event counters.
  }
  // Set multievent mode
  
  //  controller.vmeWrite16(cbltAddress + MultiEvent, initamod, (uint16_t)3);      // Multi event mode 3.
  controller.vmeWrite16(cbltAddress + IrqThreshold, initamod, (uint16_t)irqThreshold);
  controller.vmeWrite16(cbltAddress + MaxTransfer, initamod,  (uint16_t)wordsPermodule);

  // Set the IRQ

  controller.vmeWrite16(cbltAddress + Vector, initamod, (uint16_t)vector);
  controller.vmeWrite16(cbltAddress + Ipl,    initamod, (uint16_t)ipl);
  controller.vmeWrite16(cbltAddress + IrqThreshold, initamod, (uint16_t)irqThreshold);

  // Init the buffer and start data taking.

  controller.vmeWrite16(cbltAddress + InitFifo, initamod, (uint16_t)0);
  controller.vmeWrite16(cbltAddress + ReadoutReset, initamod, (uint16_t)0);
  controller.vmeWrite16(cbltAddress + StartAcq , initamod, (uint16_t)1);
}
#ifndef MVLC_GENERATOR
/**
 * clone 
 *  Clones this object returning a new dynamically allocated copy.
 */
CReadoutHardware*
CMTDC32::clone() const {
  return new CMTDC32(*this);
}
#endif
/*-----------------------------------------------------------------------------------------
 * Private utilities:
 */

/**
 * addWrite
 *    The MTDC (or at least the MADC) required delays between operations in a stack.
 *    This function adds a write to a stack and the delay following it:
 *
 *    @param[inout] list    - Reference to the list that will be modified.
 *    @param[in]    address - VME address to write to initamod will always be used.
 *    @param[in]    value   - uint16_t word to write.
 */
void
CMTDC32::addWrite(CVMUSBReadoutList& list, uint32_t address, uint16_t value)
{
  int delay = m_pConfiguration->getIntegerParameter("-initdelay");
  
    list.addWrite16(address, initamod, value);
    // NO longer delaying; not needed but I'm too lazy to remove 
    // addWrite everywhere it's used.
    // list.addDelay(delay);
}
/**
 * computeMultiEventRegister
 *   Use the configuration setup to determine the value of the multi event register.
 *
 *   Uses:  -multievent, -skipberr, and -countevents.
 *
 * @return uint16_t - register value.
 */
uint16_t
CMTDC32::computeMultiEventRegister()
{
    uint16_t result = multiEventModeRegisterValues[
        m_pConfiguration->getEnumParameter("-multievent", multiEventModes)
				      ];
    if(m_pConfiguration->getBoolParameter("-skipberr")) {
        result |= 4;                    // Skip berr flag.
    }
    if(m_pConfiguration->getBoolParameter("-countevents")) {
        result |= 8;                         // Count events  not words.
    }
    
    return result;
}
/**
 * getTermination
 *   Compute the termination register value from the value of -tr0terminated,
 *   -tr1terminated, and -resetterminated.
 *
 *   @return uint16_t value to program into register 0x6062.
 */
uint16_t
CMTDC32::getTermination()
{
    uint16_t result(0);
    if(m_pConfiguration->getBoolParameter("-tr0terminated")) result |= 1;
    if(m_pConfiguration->getBoolParameter("-tr1terminated")) result |= 2;
    if(m_pConfiguration->getBoolParameter("-resetterminated")) result |=4;
    
    return result;
}

//--------------------------------------------------------------------
#ifndef MVLC_GENERATOR
/**
 * dumpRegisters
 *   Dumps the register values to a file named:  MTDC_base  where
 *   base is the base address of the module.
 *   The registers dumped are driven by the static data structure below.
 *
 * @param controller Controller used to read the module.
 * @param base       MOdule base address.
 */

// Vector of pairs of register names and values:



static std::vector<std::pair<std::string, uint32_t> > registersToDump = {
{"AddressSource",        0x6000},
{"Address",              0x6002},
{"ModuleId",             0x6004},
{"FirmwareRev",          0x600e},

{"Ipl",                  0x6010},
{"Vector",               0x6012},
{"IrqTest",              0x6014},
{"IrqReset",             0x6016},
{"IrqThreshold",         0x6018},
{"MaxTransfer",          0x601a},
{"WithdrawIrqOnEmpty",   0x601c},

{"CbltMcstControl",      0x6020},
{"CbltAddress",          0x6022},
{"McstAddress",          0x6024},


{"DataFormat",           0x6032},
{"ReadoutReset",         0x6034},
{"MultiEvent",           0x6036},
{"MarkType",             0x6038},

{"BankOperation",        0x6040},
{"Resolution",           0x6042},
{"OutputFormat",         0x6044},

{"MTDCBank0WinStart",    0x6050},

{"MTDCBank1WinStart",    0x6052},

{"MTDCBank0WinWidth",    0x6054},

{"MTDCBank1WinWidth",    0x6056},

{"MTDCBank0TrigSource",  0x6058},
{"MTDCBank1TrigSource",  0x605a},
{"MTDCFirstHitOnly",     0x605c},

{"MTDCEdgeSelect",       0x6060},

{"ECLTermination",       0x6062},
{"ECLGate1OrTiming",     0x6064},   // Set timing source too.
{"ECLFCOrTimeReset",     0x6066},


{"MTDCTriggerSelect",   0x6068},


{"NIMGate1OrTiming",     0x606a},   // Set timing source too.
{"NIMFCOrTimeReset",     0x606c},
{"NIMBusyFunction",      0x606e},

{"TestPulser",           0x6070}, // In order to ensure it's off !
{"MTDCPulserPattern",    0x6072},
{"MTDCBank0InputThr",    0x6078},
{"MTDCBank1InputThr",    0x607a},

{"MTDCBank0HighLimit",   0x60b0},
{"MTDCBank0LowLimit",    0x60b2},
{"MTDCBank1HighLimit",     0x60b4},
{"MTDCBank1LowLimit",    0x60b6}
  
};

static void
dumpRegisters(CVMUSB& controller, uint32_t base)
{
  // Construct the name of and open the output file.

  std::stringstream filename;
  filename  << "MTDC_" << std::hex << base << ".registers";
  std::ofstream f(filename.str().c_str(), std::ios_base::out); 
  f << std::hex;                        // All numeric  data in hex.
  
  // Iterate through the registersToDump array dumping Name (Offset) = Value to file
  // for each entry.

  for (auto i =0; i < registersToDump.size(); i++) {
    std::string name = registersToDump[i].first;
    uint32_t    offset = registersToDump[i].second;
    uint32_t    addr = base  + offset;
    uint16_t    data;
    controller.vmeRead16(addr, initamod, &data);
    f << name << " (" << offset << ") = " << data << std::endl;
    
  }

    
    

  // The file closes on object construction

  
}
#endif

#ifdef MVLC_GENERATOR
/////////////////////// Implement the MTDCCommand class:

/**
 *  constructor
 */
MTDCCommand::MTDCCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
    DeviceCommand(interp, "mtdc", parser) {

}
/**
 *  destructor:
 */
MTDCCommand::~MTDCCommand() {}


/**
 * creatDevice - make the module wrapping an MTDC32 object.
 */
CReadoutModule*
MTDCCommand::createDevice(std::string name) {
    CReadoutModule* result = new CReadoutModule;
    result->SetDriver(new CMTDC32);

    return result;
}
#endif