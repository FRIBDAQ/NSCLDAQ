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
#include "CStack.h"

#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <CConfiguration.h>
#include <assert.h>
#include <tcl.h>
#include <Globals.h>
#include <stdlib.h>

#include <set>
#include <string>
#include <iostream>
using namespace std;


// The m_listOffset static variable is used to keep track of the offset at which 
// the multiple stacks can be loaded. This is used by loadStack.   The
// assumption when using loadStack is that loadStack is used to handle all stacks.

size_t CStack::m_listOffset(0);	// See however restStackOffset.
bool   CStack::m_incrementalScaler(true);


/// The stuff below is data the validators need:
/// We need to set up the enumeration validator dynamically as there's no simple way
/// to initialize a set.

// -ipl is in the range 1-7 (as 0 is disables the trigger).

static XXUSB::CConfigurableObject::limit iplLow(1);
static XXUSB::CConfigurableObject::limit iplHigh(7);
static XXUSB::CConfigurableObject::Limits iplRange(iplLow, iplHigh);

// -stack is in the range 2-7.

static XXUSB::CConfigurableObject::limit stackLow(2);
static XXUSB::CConfigurableObject::limit stackHigh(7);
static XXUSB::CConfigurableObject::Limits stackRange(stackLow, stackHigh);

// -vector is in the range 0x0 to 0x7f as there are only 8 bits...and I have to
//         mulitiply it by 2.

static XXUSB::CConfigurableObject::limit vectorLow(0);
static XXUSB::CConfigurableObject::limit vectorHigh(0xffff);    // 16bit vector (Bug #5879).
static XXUSB::CConfigurableObject::Limits vectorRange(vectorLow, vectorHigh);

// -delay is in the range 0 - 0xff  number of microseconds of delay 
//        between trigger and list start (to allow for ADC conversinos).
//
static XXUSB::CConfigurableObject::limit delayLow(0);
static XXUSB::CConfigurableObject::limit delayHigh(0xff);
static XXUSB::CConfigurableObject::Limits delayRange(delayLow, delayHigh);

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Canonicals /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
   Construct a stack. The work of preparing the configuration and attaching it
   is done in onAttach.
*/
CStack::CStack() :
  m_pConfiguration(0)
{}

/*!
  Copy constrution has the same deal except that we will clone the configuration
  object if it exists in the rhs.
 \param rhs const CStack&  refrence to the source that will be copyied.
*/
CStack::CStack(const CStack& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
 
  }
}



/*!
   Destruction is a no-op .. which, in the presence of copy construction
   can lead to memory leaks.
*/
CStack::~CStack()
{
  
}
/*!
   Assignment just returns itself...as copying the configuration can result in 
   an infinite recursion loop.
*/
CStack&
CStack::operator=(const CStack& rhs)
{
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////// CReadoutHardware interface implementation ////////////////
////////////////////////////////////////////////////////////////////////////////

/*!
   Called when a configuration is attached to the module.
   we need to define the set of configuration parameters we can support,
   and their validators
\verbatim
Option           value type               Value meaning
-trigger         enumeration              Defines the trigger source:
                                          nim1   - This will be stack 0 triggered by nim1.
					  scaler - This will be stack 1 triggered by timer
					  interrupt - this is some stack 2-n (see -stack)
                                                      triggered by an interrupt.
-period           integer                 Number of seconds between scaler triggers.  This
                                          is ignored if the trigger type is not a scaler stack.
-stack            integer                 Stack number.  This is ignored unless the -trigger
                                          option is interrupt.  The stack number will determine
                                          which interrupt list register will be programmed
                                          to trigger this list.
-delay            integer 0-0xff          Number of microseconds to delay between NIM 1
                                          and stack 0 start (allowance for conversion times
                                          of digitizers e.g.).
-vector           integer 0-0xffff         VME Interrupt status/ID that will be used to trigger this list.
                                          This is ignored if the trigger is not interrupt.
-ipl              integer 1-7             Interrupt priority level of the interrupt that will trigger
                                          this stack.  This will be ignored if the trigger is not 
					  interrupt.
-modules          stringlist              List of ADC, Scaler, Chain modules that will be read by
                                          this stack.
-incremental     boolean                 Only allowed for scaler triggered stacks.  If false,
                                          scaler ring items produced by this stack are marked
                                          non-incremental. If true (the default) scaler ring items
                                          are marked as incremental.
\endverbatim

  \param configuration : CReadoutModule&
      The configuration module and delegator that surrounds us.

*/
void
CStack::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  // set up the trigger option.

  static XXUSB::CConfigurableObject::isEnumParameter validTriggers;
  validTriggers.insert("nim1");
  validTriggers.insert("scaler");
  validTriggers.insert("interrupt");

  m_pConfiguration->addParameter("-trigger", 
				 XXUSB::CConfigurableObject::isEnum, &validTriggers,
				 "nim1");

  // All the remaining options except the -modules option are integers with and without
  // range checking.

  m_pConfiguration->addParameter("-period",
				 XXUSB::CConfigurableObject::isInteger, NULL, "2");
  m_pConfiguration->addParameter("-stack",
				 XXUSB::CConfigurableObject::isInteger, &stackRange, "2");
  m_pConfiguration->addParameter("-delay",
				 XXUSB::CConfigurableObject::isInteger, &delayRange, "0");
  m_pConfiguration->addParameter("-vector",
				 XXUSB::CConfigurableObject::isInteger, &vectorRange, "0");
  m_pConfiguration->addParameter("-ipl",
				 XXUSB::CConfigurableObject::isInteger, &iplRange, "6");
  m_pConfiguration->addParameter("-modules",
				CStack::moduleChecker, NULL, "");
  m_pConfiguration->addBooleanParameter("-incremental", "true");
  
}
/*!
  Initializes the stack prior to data taking, not to be confused with loading the
  stack. This does one-time initialization of the stack modules. We will iterate 
  through all modules read out by the stack, initializing them.
*/
void
CStack::Initialize(CVMUSB& controller)
{
  StackElements modules = getStackElements();
  StackElements::iterator p = modules.begin();

  // external try catch block to make sure that a single
  // failure stops the initialize process in its tracks. We don't
  // want to continue initializing.
  try {
    while(p != modules.end()) {
      CReadoutHardware* pModule = *p;                                // Wraps the hardware.

      pModule->Initialize(controller); 

      p++;
    }
  } catch (std::string& errmsg) {
    std::cout << "An error occurred during initialization! Reason: ";
    std::cout << errmsg << std::flush << std::endl;
    // rethrow to make this really fail 
    throw;
  } catch (std::exception& exc) {
    std::cout << "An error occurred during initialization! Reason: ";
    std::cout << exc.what() << std::flush << std::endl;
    // rethrow to make this really fail
    throw;
  } catch (...) {
    std::cout << "An error occurred during initialization! ";
    std::cout << "No reason is provided." << std::flush << std::endl;
    // rethrow to make this really fail 
    throw;
  }

  // If this module is stack triggered, set the m_incrementalScalers varaible.

  if(m_pConfiguration->cget("-trigger") == std::string("scaler")) {
    m_incrementalScaler = m_pConfiguration->getBoolParameter("-incremental");
  }
}

/*!
   Adds the modules in the stack to the readout list.  This can be done by
   an external 'force' setting up the stack manually, or it can/will be called
   by loadStack as it creates the list to load.
   \param list : CVMUSBReadoutList
       The USB Readout list that is being built by this object.

*/
void
CStack::addReadoutList(CVMUSBReadoutList& list)
{
  
#ifdef VMUSB_PROMPT_BUSY_WORKAROUND
  // Prompt busies in some VMUSB Firmwares only can be implemented
  // by stack delays.  See Issue #294

  // Delay only the event stack.
  if (getListNumber() == 0) {
    uint32_t delay = getIntegerParameter("-delay"); //usec
    delay = delay * 5;                              // 200nsec units.
    uint32_t delaysAdded = 0;
    while (delay > 0) {
      uint32_t dly  = delay > 255 ? 255 : delay;
      list.addDelay(dly);
      delay -= dly;
      delaysAdded++;
    }
  
  }

#endif
  StackElements modules = getStackElements();
  StackElements::iterator p = modules.begin();
  while (p != modules.end()) {
    CReadoutHardware* pModule = *p;
    pModule->addReadoutList(list);
    
    p++;
  }
#ifdef VMUSB_END_OF_EVENT_WORKAROUND
  if (getListNumber() == 0) {
    list.addMarker(0xffff);        // Forces end of event.
  
  }
#endif
}

/*!
   Executes the end of run operations of stack hardware. Unless hardware 
   implements their own custom onEndRun method, the default is a noop. 
*/
void
CStack::onEndRun(CVMUSB& controller)
{
  StackElements modules = getStackElements();
  StackElements::iterator p = modules.begin();
  while(p != modules.end()) {

    // try-catch within the loop because  we want to give all
    // modules a chance to perform end of run procedures independent
    // of the success of all other readout hardware. Note also that
    // we do not rethrow.
    try {
      CReadoutHardware* pModule = *p;                                // Wraps the hardware.
      pModule->onEndRun(controller); 

    } catch (std::string& errmsg) {
      std::cout << "An error occurred during end run procedures! Reason: ";
      std::cout << errmsg << std::flush << std::endl;
    } catch (std::exception& exc) {
      std::cout << "An error occurred during end run procedures! Reason: ";
      std::cout << exc.what() << std::flush << std::endl;
    } catch (...) {
      std::cout << "An error occurred during end run procedures! ";
      std::cout << "No reason is provided." << std::flush << std::endl;
    }

    p++;
  }
  
#ifdef VMUSB_IDLE_BUSY_WORKAROUND
  // Best to implement this here:  
  // there are many ways the O1 could have been defined...
  // the best thing we can do is read the device selection register and
  // see what the final state was.
  // If it's selected to by BUSY< then we reprogram it to be
  // end of event _and_ flip the invert bit.  
  // end of event should always be false when  idle so 
  // its inverse is asserted.

  uint32_t currentSelection = controller.readDeviceSource();
  if ((currentSelection & 0x7) == CVMUSB::DeviceSourceRegister::nimO1Busy) {
    bool inverted = (currentSelection & CVMUSB::DeviceSourceRegister::nimO1Invert) != 0;

    // new compute the new value of the register:

    currentSelection &= 0xffffffe0;             // Clear the field, invert and latch.
    currentSelection |= CVMUSB::DeviceSourceRegister::nimO1EndOfEvent;   // select end of event.
    if (!inverted) {
      currentSelection |= CVMUSB::DeviceSourceRegister::nimO1Invert;     // get the inversion sense right
    }                                                                    // for the new selection.
    controller.writeDeviceSource(currentSelection);
  }

#endif
}
/*!
  Clone virtualizes copy construction.

*/
CReadoutHardware*
CStack::clone() const
{
  return new CStack(*this);
}
////////////////////////////////////////////////////////////////////////////////
///////////////// Implementation of interfaces specific to CStack //////////////
////////////////////////////////////////////////////////////////////////////////


/*!
   Sets the stack offset (by default to 0).  The stack offset determines the
   offset in VM-USB memory where the next stack load by loadStack will occur.
   In normal use, the collection of CStack classes would manage the stack memory,
   and prior to loading all the stacks, the DAQ programs would just resetStackOffset();
   It is also possible for the DAQ software to externally manage a stack or two
   (assuming it takes care to avoid stack trigger collisions), in which case
   it might load a few stacks and then resetStackOffset(someOffset); 
   where someOffset is the next free word of stack memory after the
   externally managed stacks.

   This scheme, along with getOffset provides more flexibility than most DAQ
   systems want or need.

   There is an implicit assumption that the DAQ is managing exactly one VMUSB in all
   of this.

   \param to : size_t (defaults to 0)
        The stack offset at which the next load will occur.
*/
void
CStack::resetStackOffset(size_t to)
{
  m_listOffset = 0;
}
/*!
   Returns the current value of the stack offset. This value is the first free
   word of VMUSB stack memory.  There is an implicit assumption that the
   CStack collection in managing exactly one VMUSB module.

\return size_t
\retval Next free word of VMUSB memory.

*/
size_t
CStack::getOffset()
{
  return m_listOffset;
}
/*!
   Generates and loads the stack that reads out this trigger.
   This is done by creating a readout list, filling it in via addReadoutList,
   and loading it at the offset designated by m_listOffset using our list number.
   m_listOffset will be updated to reflect the number of stack lines that we have
   loaded.

   \param controller : CVMUSB&
     The VMUSB controller handle connected to the VM-USB that we will be managing.
*/
void
CStack::loadStack(CVMUSB& controller)
{
  uint8_t           listNumber;
  CVMUSBReadoutList readoutList;

  addReadoutList(readoutList);
  listNumber = getListNumber();



  

  // Load the list:... unless it has no elements!

  if (readoutList.size() > 0) {
    controller.loadList(listNumber,
			readoutList,
			m_listOffset);
    // The + 4 below is the size of the header each stack has according to Jan.
    // That is 2 longwords.
    
    m_listOffset += readoutList.size() * sizeof(uint32_t)/sizeof(uint16_t)+4 ; // Stack locs are words.

    // Adjust for any workarounds added:

    
  }
}
/*!
  To do this we need to get the stack number...
  - If it is 0 we don't need to do anything as these are automatically triggered
    by NIM1.
  - If it is 1 we need to get the -period parameter and program that as the
    scaler reaodout period (note that -period is in seconds, but we need to program
    1/2 second intervals).
  - If the stack number is 2-7, we need to use the -ipl, and -vector parameters
    to program the vector trigger register assigned to this list.

\param controller : CVMUSB&
    Reference to the VMUSB controller object connected to the VM-USB module.
*/
void
CStack::enableStack(CVMUSB& controller)
{
  uint8_t   listNumber  = getListNumber();

  // If zero, fetch the delay parameter and set it in the DAQ Settings register:

  if (listNumber == 0) {
#ifndef VMUSB_PROMPT_BUSY_WORKAROUND
    uint32_t delay        = getIntegerParameter("-delay");
    uint32_t daqsettings  = controller.readDAQSettings();
    daqsettings           = daqsettings & (~CVMUSB::DAQSettingsRegister::readoutTriggerDelayMask); // Clear bit field.
    daqsettings          |= (delay & CVMUSB::DAQSettingsRegister::readoutTriggerDelayMask);  // insert bit field.
    controller.writeDAQSettings(daqsettings); // Write the new register value.
#endif
    return;
  }


  // If 1 set up the scaler parameters:

  if (listNumber == 1) {
    Globals::scalerPeriod      = m_pConfiguration->getUnsignedParameter("-period");
    uint32_t scalerPeriod     = m_pConfiguration->getUnsignedParameter("-period") *2;
    uint32_t daqSettings      = controller.readDAQSettings();
    scalerPeriod  = (scalerPeriod << CVMUSB::DAQSettingsRegister::scalerReadoutPeriodShift)   &
                     CVMUSB::DAQSettingsRegister::scalerReadoutPeriodMask;   // Position the bit field
    daqSettings   = daqSettings & (~CVMUSB::DAQSettingsRegister::scalerReadoutPeriodMask); // Clear field
    daqSettings  |= scalerPeriod;                                    // Register value.
    controller.writeDAQSettings(daqSettings);

    return;
  }
  // If otherwise, the list is triggered by an interrupt given by 
  // -ipl and -vector.
  // We map the list to interrupt triggers as follows:
  // 2  - Low bits of ISV12   (which == 1)
  // 3  - High bits of ISV12. (which == 1)
  // 4  - Low Bits of ISV34   (which == 2)
  // 5  - High Bits of ISV34  (which == 2)
  // 6  - Low Bits of ISV56   (which == 3)
  // 7  - High bits of ISV56  (which == 3)
  //   ISV78 is not used.     (which == 4).
  //
  //
  int which     = (listNumber - 2)/2 + 1; // the ISV pair number.
  bool highHalf = (listNumber % 2) == 1;   // evens are in the low 1/2 odds the top  1/2.

  uint32_t isvValue = controller.readVector(which); // manipulate the current value.

  // Clear the 1/2 we will be filling in. (e.g. Keep the low 1/2 if highHalf is true).

  if (highHalf) {
    isvValue = isvValue & (CVMUSB::ISVRegister::AVectorMask | 
                           CVMUSB::ISVRegister::AIPLMask    | 
			   CVMUSB::ISVRegister::AStackIDMask);
  }
  else {
    isvValue = isvValue & (CVMUSB::ISVRegister::BVectorMask | 
			   CVMUSB::ISVRegister::BIPLMask    | 
			   CVMUSB::ISVRegister::BStackIDMask);
  }
  // Now build up the final value of the register from the parameters:

  uint32_t stackNumber = static_cast<uint32_t>(listNumber);
  uint32_t irq         = static_cast<uint32_t>(getIntegerParameter("-ipl"));
  uint32_t vectorNumber= static_cast<uint32_t>(getIntegerParameter("-vector"));

  if (highHalf) {
    isvValue |=  (stackNumber << CVMUSB::ISVRegister::BStackIDShift) | 
                 (irq         << CVMUSB::ISVRegister::BIPLShift)     | 
                 ((vectorNumber & 0xff)<< CVMUSB::ISVRegister::BVectorShift);
  }
  else {
    isvValue |=  (stackNumber << CVMUSB::ISVRegister::AStackIDShift)  | 
                 (irq         << CVMUSB::ISVRegister::AIPLShift)      | 
                 ((vectorNumber & 0xff) << CVMUSB::ISVRegister::AVectorShift);
  }
  // Write the vector back, which enables the trigger when data taking is turned on:

  controller.writeVector(which, isvValue);

  // Now deal with the top 8 bits of the vector:  These are packed four to a
  // register in USBVHIGH1 and USBVHIGH2.  Since we've allocated vector numbers
  // by stack we can compute the register and its shift count:
  
  which          = (((stackNumber - 2) / 4) == 0) ? CVMUSB::USBVHIGH1 : CVMUSB::USBVHIGH2;



  uint32_t shift = ((stackNumber -2) % 4) * 8;
  uint32_t mask  = 0xff << shift;
  
  uint32_t extValue = controller.readRegister(which);

  extValue &= ~mask;                   // Remove the old bits.
  extValue |= ((vectorNumber >> 8) << shift); // or in the new bits.



  controller.writeRegister(which, extValue);

  
  // Ensure the IPL's bit is not set in the interrupt mask:
  
  uint8_t irqmask = controller.readIrqMask();
  irqmask        &= ~(1 << irq);                // Clear our irq bit.
  
  
  irqmask        =  0;		// Enable all the damned interrupts.
  controller.writeIrqMask(irqmask);
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// Helper functions //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


/*
  Get an integer parameter.  The parameter is assumed to already have been 
validated by the configurator.
Parameters:
   std::string name     - Name of the parameter ot get.
Returns
   parameter value as an integer.

*/
unsigned int
CStack::getIntegerParameter(string name)
{
  string sValue =  m_pConfiguration->cget(name);
  unsigned int    value  = strtoul(sValue.c_str(), NULL, 0);

  return value;
}
/*
   Get the trigger type.  The trigger type is assumed to alread have been validated
   by the configurator.
Returns:
   Enumerated value that reflects the trigger value configured into the system.
*/
CStack::TriggerType
CStack::getTriggerType()
{
  string sValue = m_pConfiguration->cget("-trigger");

  if (sValue == "nim1")      return Nim1;
  if (sValue == "scaler")    return Scaler;
  if (sValue == "interrupt") return Interrupt;

  assert(0);			// Should not get here.
}

/*
   Gets the elements that make up our stack.  This is an stl list of pointers to
   modules that match the set of module names that were put in the stack via the
  -modules configuration parameter.  These modules were validated at configuration time.
   Since there's no way to destroy modules it's a really fatal error for them to not exist.
   To permit scaler stacks and the readout of scalers in response to other triggers,
   we will search for the modules in both the Adcs and Scalers list of the configuration.

Returns:
   A  list of pointers to the modules that should be included in the stack.

*/
CStack::StackElements
CStack::getStackElements()
{
  CConfiguration* pConfiguration   = Globals::pConfig;
  int             argc;
  const char**    argv;
  StackElements   result;
  string          sValue;

  // Split the list.. this must work because our validator ensured it:

  sValue = m_pConfiguration->cget("-modules");
  Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);
  assert(argc > 0);		// Can't have an empty module list.

  // Iterate searching for the modules first in the ADcs list and then in the Scalers list.
  // it's a fatal error to fail to find them.  Each module pointer is added to the result list.

  for (int i=0; i < argc; i++) {
    string name(argv[i]);
    CReadoutModule* pModule = pConfiguration->findAdc(name);
    if (!pModule) {
      pModule = pConfiguration->findScaler(name);
    }
    assert(pModule);		// Must exist in one or the other lists of modules.

    result.push_back(pModule->getHardwarePointer());
  }
  // Free the storage allocated by Split list and provide the list to the caller:

  Tcl_Free(reinterpret_cast<char*>(argv));
  return result;

}
/* Return the number of the list assigned to the stack.  This is done as follows:
   o If the list is triggered by nim1, the stack number is 0.
   o If the list is triggered by scaler the stack number 1.
   o If the list is triggered by interrupts, the stack number is defined by the -stack
     parameter.
 RETURNS:
    The stack number after all the figuring out described above.
*/
uint8_t
CStack::getListNumber()
{
  TriggerType trigger = getTriggerType();
  if (trigger == Nim1) {
    return 0;
  } else if (trigger == Scaler) {
    return 1;
  } else if (trigger == Interrupt) {
    return static_cast<uint8_t>(getIntegerParameter("-stack"));
  } else {
    throw std::string("Invalid trigger condition!!");
  }
}
/*
   Custom validator for the -modules switch.  This validator checks that
   - The proposed value is a valid Tcl list.
   - There is at least one element in the list.
   - The proposed value contains list elements that are known modules in either the
     ADC or Scaler lists of the configurator.
Parameters:
   string name          - The name of the configuration parameter (most likely -modules).
   string proposedValue - The new value proposed for the configuration parameter.
   void*  arg           - Unused argument to the validator from the application.
Returns:
   true   - The conditions described above are true.
   false  - Any of the above conditions failed.

*/
bool
CStack::moduleChecker(string name, string proposedValue, void* arg)
{
  int             argc;
  const char**    argv;
  int             status;
  CConfiguration* pConfiguration = Globals::pConfig;
  string          Name;
  CReadoutModule* pModule;


  // Break the proposed value up in to a list..and return false if the parameter is not a well
  // formed list.


  status = Tcl_SplitList(NULL, proposedValue.c_str(), &argc, &argv);
  if (status != TCL_OK) {
    return false;
  }
  // Iterate through the module names attempting to find them in the ADCs and SCALER lists.
  // if one is not found, return the argv storage and return false.

  for ( int i=0; i < argc; i++) {
    Name    = argv[i];
    pModule = pConfiguration->findAdc(Name);
    if (!pModule) {
      pModule = pConfiguration->findScaler(Name);
    }
    if (!pModule) {
      Tcl_Free(reinterpret_cast<char*>(argv));
      return false;
    }
  }
  // If we got this far all the modules validated, return the argv storage and 
  // a true value.

  Tcl_Free(reinterpret_cast<char*>(argv));
  return true;
}
