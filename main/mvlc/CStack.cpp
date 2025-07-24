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

#include "CStack.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "TCLConfigParser.h"
#include "CReadoutModule.h"
#include <XXUSBConfigurableObject.h>
#include <assert.h>
#include <tcl.h>
#include <stdlib.h>
#include <set>
#include <string>
#include <iostream>
using namespace std;




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
  
   @param parser - the configuration parser which we need to validate module names.
*/
CStack::CStack(TCLConfigParser* pParser) :
  m_pConfiguration(0), m_pParser(pParser)
{}



/*!
   Destruction is a no-op .. which, in the presence of copy construction
   can lead to memory leaks.
*/
CStack::~CStack()
{
  
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
CStack::onAttach(XXUSB::CConfigurableObject& configuration)
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
				CStack::moduleChecker, this, "");
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


  // external try catch block to make sure that a single
  // failure stops the initialize process in its tracks. We don't
  // want to continue initializing.
  try {
    for (auto p :  modules) {
      
      p->Initialize(controller); 

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
  // If necessary, prepend the delay to the stack
  // 

  int usecDelay = getIntegerParameter("-delay");
  if (usecDelay > 0)  {
    int cyclesDelay =  usecDelay * 5;   // Prepend delay wants useconds.
    list.addDelay(cyclesDelay);
  }
  
  StackElements modules = getStackElements();
  
  for (auto p : modules) {

    p->addReadoutList(list);
    
  }
}

/*!
   Executes the end of run operations of stack hardware. Unless hardware 
   implements their own custom onEndRun method, the default is a noop. 
*/
void
CStack::onEndRun(CVMUSB& controller)
{
  StackElements modules = getStackElements();
  
  for (auto p : modules) {

    // try-catch within the loop because  we want to give all
    // modules a chance to perform end of run procedures independent
    // of the success of all other readout hardware. Note also that
    // we do not rethrow.
    try {
      
      p->onEndRun(controller); 

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
  return m_pConfiguration->getIntegerParameter(name);
} 
/*
   Get the trigger type.  The trigger type is assumed to alread have been validated
   by the configurator.
Returns:
   Enumerated value that reflects the trigger value configured into the system.
*/
CStack::TriggerType
CStack::getTriggerType() const
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
  StackElements result;
  int argc;
  const char** argv;
  // Split the list.. this must work because our validator ensured it:

  string sValue = m_pConfiguration->cget("-modules");
  Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);
  

  // Iterate searching for the modules first in the ADcs list and then in the Scalers list.
  // it's a fatal error to fail to find them.  Each module pointer is added to the result list.

  for (int i=0; i < argc; i++) {
    string name(argv[i]);
    CReadoutModule* pModule = m_pParser->findDevice(name);
    assert(pModule);		// Must exist because of our validator.

    result.push_back(pModule);
  }
  // Free the storage allocated by Split list and provide the list to the caller:

  Tcl_Free(reinterpret_cast<char*>(argv));
  return result;

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
  CStack*         me = reinterpret_cast<CStack*>(arg);
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
    
    if (!me->m_pParser->findDevice(Name)) {
      Tcl_Free(reinterpret_cast<char*>(argv));
      return false;
    }
  }
  // If we got this far all the modules validated, return the argv storage and 
  // a true value.

  Tcl_Free(reinterpret_cast<char*>(argv));
  return true;
}

//////////////////////////////// The stack command /////////////////////////////////


/** 
 * constructor
 *     @param interp - references the interpreter we get registerd on.
 *     @param parser - Referneces the parser that will parse the configuration file.
 *   
 * @note the command is hardwared to "stack"
 */
CStackCommand::CStackCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "stack", parser) {}

/** 
 * destructor
 */
CStackCommand::~CStackCommand() {}

/**
 *  createDevice
 *     Create a device module and bind a CStack in as the driver
 * @param name - name of the device.
 * @return CReadoutModule* - pointer to the module we created.  The caller must bind in the
 *         configuration object.
 */
CReadoutModule* 
CStackCommand::createDevice(std::string name) {
  auto result = new CReadoutModule;
  result->SetDriver(new CStack(&getParser()));

  return result;
}
