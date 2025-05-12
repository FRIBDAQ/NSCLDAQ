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
#include "CCAENChain.h"
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <C785.h>
#include <CConfiguration.h>
#include <Globals.h>
#include <tcl.h>
#include <assert.h>
#include <stdlib.h>

using namespace std;

static const unsigned int LongwordsPerModule(36); // Maximum # longwords/module.

// If cblt/mcst address is not of the form 0x00xx0000 use a32.

static const uint8_t  cbltamod32(CVMUSBReadoutList::a32UserBlock);
static const uint8_t   mcstamod32(CVMUSBReadoutList::a32UserData);

// If cblt/mcst address is of the form 0x00xx0000 use a24:

static const uint8_t cbltamod24(CVMUSBReadoutList::a24UserBlock);
static const uint8_t mcstamod24(CVMUSBReadoutList::a24UserData);

static const uint32_t BSET2(0x1032); // Offset to the bit set 2 register.
static const uint32_t BCLR2(0x1034); // Offset t the bit clear 2 register.
static const uint16_t CLEAR_DATA(0x4); // The clear data bit in the above.

/////////////////////////////////////////////////////////////////
/////////////// Canonical class/object implementations /////////
////////////////////////////////////////////////////////////////

/*!
  Construction is largely a no-op as all the action happens
  at attach time.
*/
CCAENChain::CCAENChain() :
  m_pConfiguration(0)
{
}
/*!
  Copy construction requires duplication of the configuration as
  well as the list.
*/
CCAENChain::CCAENChain(const CCAENChain& rhs) :
  m_pConfiguration(0)
{
  if(rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
  m_Chain = rhs.m_Chain; 
}
/*! Destroy the module.  This is pretty much a no-op as configurations
    take care of their own destruction:
*/
CCAENChain::~CCAENChain()
{
}

/*!
   Assignment for now is not really good
*/
CCAENChain&
CCAENChain::operator=(const CCAENChain& rhs)
{
  return *this;
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
CCAENChain::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  m_pConfiguration->addParameter("-base", XXUSB::CConfigurableObject::isInteger,
				 NULL,"0");

  m_pConfiguration->addParameter("-modules", CCAENChain::moduleChecker,
				 NULL, "");
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
  CConfiguration*  pModules    = Globals::pConfig;	// This has the full set of modules.
  list<string>     moduleNames = getModules();  // From my configuration.

  // Now iterate through all the modules, finding them, initializing them and
  // counting them.  We know that each module is a valid C785 because they were
  // validated as such when added to our list.

  list<string>::iterator pName   = moduleNames.begin();
  bool                   isFirst = true;
  while (pName != moduleNames.end()) {
    CReadoutModule*  pModule = pModules->findAdc(*pName); // Locate the module.
    C785*            pAdc    = dynamic_cast<C785*>(pModule->getHardwarePointer()); // Should never fail so...
    assert(pAdc);

    pAdc->Initialize(controller);
    pName++;			// This is end() if pAdc is the last module.
    m_moduleCount++;            // Count the module.

    // To add to the chain we need to know the module position.
    // if isFirst is true, it's the first module (we'll reset that below).
    // if pName == end() it's the last module..
    // if neither it's middle.
    // Can't be both because our validator would have caught that.

    C785::Position where = C785::middle; // Most common case.
    if (isFirst) {
      where   = C785::leftmost;
      isFirst = false;
    }
    if (pName == moduleNames.end()) {
      where   = C785::rightmost;
    }
    pAdc->addToChain(controller, m_baseAddress, where);

  }
  //By this time all of the modules have had their MCST/CBLT address set.
  // and enabled.  We're going to do an MCST data reset to the chain.
  // this will ensure that all devices have got a simultaneously zeroed
  // event counter and the event buffers are simultaneously cleared.
  // this is needed to ensure event coherency in the event that triggers
  // are going into the system prior to startup.

  // FIgure out which amod to use:

  uint8_t mcstamod;
  if (m_baseAddress & 0xff000000) {
    mcstamod = mcstamod32;                  // A32.
  } else {
    mcstamod = mcstamod24;                  // A24.
  }

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

  // figure out the address modifiers:

  uint8_t cbltamod;
  uint8_t mcstamod;

  if (m_baseAddress & 0xff000000) {
    // a32:
    cbltamod = cbltamod32;
    mcstamod = mcstamod32;
  } else {
    // a24

    cbltamod = cbltamod24;
    mcstamod = mcstamod24;
  }

  list.addBlockRead32(m_baseAddress, cbltamod, transferCount); // Thwack the 'that was easy (TM)' button.
  list.addWrite16(m_baseAddress + BSET2, mcstamod, CLEAR_DATA);
  list.addWrite16(m_baseAddress + BCLR2, mcstamod, CLEAR_DATA);

}

/*!
   Need to be able to clone ourselves.. and that's just a new -> copy construction.. but
   this is essentially a 'virtualized' copy constructor.
   \return CReadoutHardware*
   \retval pointer to a dynamically allocated copy of *this.
*/
CReadoutHardware*
CCAENChain::clone() const
{
  return new CCAENChain(*this);
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

*/
list<string>
CCAENChain::getModules()
{
  // These two are for Tcl_SplitList

  int    argc;
  const char** argv;
  list   <string>   Result;	                       // This is what we'll return.

  string sValue  = m_pConfiguration->cget("-modules"); // String valued parameter...
  Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);   // List has already been validated.

  assert(argc > 1);		                       // Validator should have ensured this.

  // Store the names in our result list

  for (int i=0; i < argc; i++) {
    Result.push_back(string(argv[i]));
  }

  // Free the argv Tcl_SplitList dynamically allocated and return Result:

  Tcl_Free((char*)argv);
  return Result;
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
   void*  arg     - Arbitrary parameter that can be passed to validators by the
                    configurator (we don't use it).
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
  int     argc;			// Returns from Tcl_SplitList
  const char**  argv;

  // Validate the list and get the elements in argv:

  int status = Tcl_SplitList(NULL, value.c_str(), &argc, &argv);
  if (status != TCL_OK) {
    return false;		// not a valid tcl list
  }

  // Check that we have enough elements to make a chain:

  if (argc < 2) {		// Note that zero length lists can be legal!
    Tcl_Free((char*)argv);
    return false;
  }
  // Now iterate through the list elements and that each is a known C785.
  // dynamic_cast is used to ensure the C785-edness of CReadoutModule
  // pointers as it uses RTTI to ensure the cast is truly compatible.

  CConfiguration* pConfiguration = Globals::pConfig;


  for (int i = 0; i < argc; i++) {
    string moduleName        = argv[i];
    CReadoutModule* pModule = pConfiguration->findAdc(moduleName);
    if (!pModule || !dynamic_cast<C785*>(pModule->getHardwarePointer())) { // Not found, or not a C785.
      Tcl_Free((char*) argv);
      return false;
    } 
  }
  // Control only gets here if all validations passed

  Tcl_Free((char*)argv);
  return true;		       
}
