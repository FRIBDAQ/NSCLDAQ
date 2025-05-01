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

#include "CCAENChain.h"
#include "CReadoutModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "C785.h"
#include "TCLConfigParser.h"
#include <XXUSBConfigurableObject.h>
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <tcl.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>

using namespace std;

static const unsigned int LongwordsPerModule(36); // Maximum # longwords/module.

static const uint8_t  cbltamod(CVMUSBReadoutList::a32UserBlock);
static const uint8_t   mcstamod(CVMUSBReadoutList::a32UserData);

static const uint32_t BSET2(0x1032); // Offset to the bit set 2 register.
static const uint32_t BCLR2(0x1034); // Offset t the bit clear 2 register.
static const uint16_t CLEAR_DATA(0x4); // The clear data bit in the above.

/////////////////////////////////////////////////////////////////
/////////////// Canonical class/object implementations /////////
////////////////////////////////////////////////////////////////

/*!
  constructor:
     @param parser  points to the configuration parser.  We use it to 
      validate the modules in the -modules list.
*/
CCAENChain::CCAENChain(TCLConfigParser* parser) :
  m_pConfiguration(0),
  m_pParser(parser),
  m_moduleCount(0),
  m_baseAddress(0)
{}

/*! Destroy the module.  This is pretty much a no-op as configurations
    take care of their own destruction:
*/
CCAENChain::~CCAENChain()
{
}


///////////////////////////////////////////////////////////////////
///////////// overridable object operations ///////////////////////
///////////////////////////////////////////////////////////////////

/*!
   Called to attach the configuration object to us.
   We have to define the following parameters, none of which have defaults:
   -base, -modules.  The -base checker will just be the standard list checker,
   while we will supply a custom checker for the module list, that will
   ensure the module list is a valid list of strings, and that each item
   in the list identifies a module that is a C785 object, as only those
   are allowed to be in a chain.
*/
void
CCAENChain::onAttach(XXUSB::CConfigurableObject& configuration)
{
  m_pConfiguration = &configuration;

  m_pConfiguration->addParameter("-base", XXUSB::CConfigurableObject::isInteger,
				 NULL,"0");

  m_pConfiguration->addParameter("-modules", CCAENChain::moduleChecker,
				 m_pParser, "");
}

/*!
    Initialize prior to data taking.  We must:
    - Determine our base address
    - Determine, and locate our list of modules.
    - Initialize each module.
    - Tell each module in the list that it will be using
      our base address as it's CBLT/MCST address by invoking
      it's addToChain member function.
    - Save our base address for later (m_baseAddress)
    - Save our count of modules so that we know how to 
      generate our CBLT readout list when addReadoutList is called.
      (m_moduleCount).

      \param controller CVMUSB&
                 Reference to the controller that connects us to the 
                 VM-USB module.  This is really just passed to each
                 module for its own initialization and mcst setting.

*/
void
CCAENChain::Initialize(CVMUSB& controller)
{
  m_baseAddress                = getCBLTAddress();
  m_moduleCount                = 0;
  list<string>     moduleNames = getModules();  // From my configuration.

  // Now iterate through all the modules, finding them, initializing them and
  // counting them.  We know that each module is a valid C785 because they were
  // validated as such when added to our list.

  
  for (auto name : moduleNames) {
    CReadoutModule*  pModule = m_pParser->findDevice(name); // Locate the module.
    C785*            pAdc    = dynamic_cast<C785*>(pModule->getDriver()); // Should never fail so...
    assert(pAdc);

    pAdc->Initialize(controller);
    m_moduleCount++;            // Count the module.

    // To add to the chain we need to know the module position.
    // if isFirst is true, it's the first module (we'll reset that below).
    // if pName == end() it's the last module..
    // if neither it's middle.
    // Can't be both because our validator would have caught that.

    C785::Position where = C785::middle; // Most common case.
    if (name == moduleNames.front()) {
      where   = C785::leftmost;     // Actually the first element (leftmost).
    }
    if (name == moduleNames.back()) {
      where   = C785::rightmost;    // Actually the last element (rightmost).
    }
    pAdc->addToChain(controller, m_baseAddress, where);

  }
  //By this time all of the modules have had their MCST/CBLT address set.
  // and enabled.  We're going to do an MCST data reset to the chain.
  // this will ensure that all devices have got a simultaneously zeroed
  // event counter and the event buffers are simultaneously cleared.
  // this is needed to ensure event coherency in the event that triggers
  // are going into the system prior to startup.

  controller.vmeWrite16(m_baseAddress + BSET2, mcstamod, CLEAR_DATA);
  controller.vmeWrite16(m_baseAddress + BCLR2, mcstamod, CLEAR_DATA);
 
}
/*!
   Add our read to the readout list.  This is pretty simple:
   - In order to avoid going off the end of the buffer region of memory
     for long chains, we'll add a block fifo style read (no increment of address)
   - The base addresss for the block read is saved at m_baseAddress.
   - The number of transfers to set up for (we'll typically be terminated by BERR)
     is m_moduleCount * LongwordsPerModule.
   - The address modifier of the block transfer will be user block transfer mode which,
     when initiated at the CBLT transfer address pushes the module to do block transfers.

     \param list - CVMUSBReadoutList&
                  The VM-USB list to which to add the module.
*/
void
CCAENChain::addReadoutList(CVMUSBReadoutList& list)
{
  m_baseAddress               = getCBLTAddress();
  m_moduleCount               = getModules().size(); // Assume they're all good 

  unsigned int transferCount  = m_moduleCount * LongwordsPerModule;

  list.addBlockRead32(m_baseAddress, cbltamod, transferCount); // Thwack the 'that was easy (TM)' button.
  list.addWrite16(m_baseAddress + BSET2, mcstamod, CLEAR_DATA);
  list.addWrite16(m_baseAddress + BCLR2, mcstamod, CLEAR_DATA);

}


////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// utilities /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/*
      Access our configuration to return the block transfer address 
      that was configured into us.

      Returns: numeric value of the -base configuration parameter.
*/
uint32_t
CCAENChain::getCBLTAddress()
{
  string sValue = m_pConfiguration->cget("-base"); // String representing our value.
  uint32_t value= strtoul(sValue.c_str(), NULL, 0); // Convert to a number.

  return value;
}
/*
   Return the list of module names that were configured with the -modules
  configuration option. Returned as an STL list<string>, the list is returned
  in the order specified when -modules was configured.
  @todo - change the return type to vector<string>   as that's what we get from getList
*/
list<string>
CCAENChain::getModules()
{
  // Keep the return type because I'm too lazy:

  auto modules = m_pConfiguration->getList("-modules");    // vector.
  std::list<std::string> result(modules.begin(), modules.end());   // list from iterators.

  return result;     
  
}

/*  Custom validator for the -modules configuration parameter.
    The following validations will be performed:
    - The configuration string is a valid Tcl list.
    - The Tcl list has more than 1 element (can't have a single module chain).
    - The Tcl list consists of strings that are all the names of valid C785 modules
      known to the configuration.
PARAMETERS:
   string name    - Name of the parameter we validate (must be -modules).
   string value   - Proposed new value for the parameter.
   void*  arg     - Actually a pointer to the config parser..
RETURNS:
   true   - All these conditions have been met.
   false  - At least one of the above conditions was not met.
NOTE:
   A purist will note that we could use the list validator to validate the list 
   structure rather than doing that ourselves (including the requirement that there
   be a minimum number of 2 elements in the list), however since we're going to need
   the list values anyway to check the module name validity, there's nothing to be
   gained by factoring that out of our code.
*/
bool
CCAENChain::moduleChecker(string name, string value, void* arg)
{
  TCLConfigParser* parser = reinterpret_cast<TCLConfigParser*>(arg); 
  CTCLObject moduleList;
  auto interp = parser->getInterpreter();
  moduleList.Bind(interp);
  
  std::vector<CTCLObject> modules;
  try {
    moduleList = value;
    modules = moduleList.getListElements();
  }
  catch (...) {
    return false;                        // Not a valid list format
  }

  for (auto module : modules) {
    module.Bind(interp);
    std::string moduleName = std::string(module);
    auto pModule = parser->findDevice(moduleName);

    // It's bad if the module can't be found _or_ if the driver isn't a C785 object:

    if (!pModule || !dynamic_cast<C785*>(pModule->getDriver())) {
      return false;
    }
  }

  

  return true;		       
}
////////////////////////// Implement the CAENChainCommand:

/**
 *  constructor
 *     Construct the base class with the command name "caenchain" and save the parser
 * object for device construction.
 * 
 */
CAENChainCommand::CAENChainCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "caenchain", parser), 
  m_parser(&parser) 
  {}

/** destructor 
 *    We don't own the parser so we dont' delete it.
*/
CAENChainCommand::~CAENChainCommand() {}

/**
 * createDevice
 *     Create a new Readout module that encapsulates a CCAENChain instance.
 * Ownership passes to the caller.
 */
CReadoutModule*
CAENChainCommand::createDevice(std::string  name) {
  auto result = new CReadoutModule;
  result->SetDriver(new CCAENChain(m_parser));

  return result;
}