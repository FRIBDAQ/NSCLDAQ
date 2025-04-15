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

#include <CMADCChain.h>
#include <CReadoutModule.h>
#include <XXUSBConfigurableObject.h>
#include <TCLConfigParser.h>
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <CMADC32.h>
#include <tcl.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "MADC32Registers.h"

#include <iostream>
#include <unistd.h>
#include <vector>
using namespace std;

static XXUSB::CConfigurableObject::limit Zero(0);
static XXUSB::CConfigurableObject::limit MaxWords(1024); // should fit in a VM-USB buffer with 12modules.
static XXUSB::CConfigurableObject::Limits WordLimit(Zero, MaxWords);

////////////////////////////////////////////////////////////////////////////////////////////
// Canonical method implementatinos.
////////////////////////////////////////////////////////////////////////////////////////////

/*!
  Constructino is a noop.
*/
CMADCChain::CMADCChain(TCLConfigParser* parser) :
  m_pConfig(0),
  m_pParser(parser)
{
}

/*!
  Configurations get destroyed by themseves so:
*/
CMADCChain::~CMADCChain()
{
}

///////////////////////////////////////////////////////////////////////////
// Implementation of the CReadoutHardware interface.
///////////////////////////////////////////////////////////////////////////

/*!
   Called to attach the configuration object to us.
   We have to define the parameter switches.
   The only slightly whacky thing is that we define a 
   special validity checker for the -modules switch to ensure
   that we only are handed
   - defined module names.
   - modules that are in fact Mesytec modules.
   @param configuration - The configuration object to attach to us.
*/
void
CMADCChain::onAttach(XXUSB::CConfigurableObject& configuration)
{
  m_pConfig  = &configuration;
  configuration.addParameter("-cbltaddress",
			     XXUSB::CConfigurableObject::isInteger,
			     NULL, "0");
  configuration.addParameter("-mcastaddress",
			     XXUSB::CConfigurableObject::isInteger,
			     NULL, "0");
  configuration.addParameter("-maxwordspermodule",
			     XXUSB::CConfigurableObject::isInteger,
			     &WordLimit, "512");
  configuration.addParameter("-modules",
			     CMADCChain::moduleChecker,
			     m_pParser, "");
}
/*!
  Initialize prior to data taking.
  The modules must be programmed with their mcast/cblt address.
  We then use the MCAST adress to program maxwords/module
  We then synchronize startup by using mcast to:
  - Disabling data acquistion
  - Setting the maxwords/event.
  - Resetting the FIFO
  - Resetting the clocks
  - Resetting the readout
  - Starting the DAQ

  In theory this should ensure that all ADCS have synchronized clocks (if they have a common
clock source and divisor), and that no ADC can accept an event prior to any other adc.
   @param controller - reference to the VMUSB controller object.
*/
void
CMADCChain::Initialize(CVMUSB& controller)
{

  // Get meaningful values for all of the configuration parameters.

  uint32_t cbltAddress    = m_pConfig->getUnsignedParameter("-cbltaddress");
  uint32_t mcastAddress   = m_pConfig->getUnsignedParameter("-mcastaddress");
  int      wordsPerModule = m_pConfig->getIntegerParameter("-maxwordspermodule");
  list<string>  moduleNames = getModules();
  namesToList(moduleNames);

  // Visit each module initialize it  and program/enable it's mcast/cblt addresses:

  for (ChainListIterator i = m_Chain.begin(); i != m_Chain.end(); i++) {
    // Figure out the chain position value:

    CMADC32::ChainPosition pos = CMADC32::middle; // Most common case:
    ChainListIterator     next = i; next++;
    if(i == m_Chain.begin()) {
      cerr << "First in chain\n";
      pos = CMADC32::first;
    }
  
    if (next  == m_Chain.end()) {
      cerr << "Last In chain\n";
      pos = CMADC32::last;
    }
    (*i)->Initialize(controller);
    (*i)->setChainAddresses(controller, pos, cbltAddress, mcastAddress);

  }
  // Now we can prep the modules  via MCAST addressing.
  // This is done by having the first module in the chain use it's
  // timing settings, its multievent settings, our max transfer,
  // and its interrupt information
  //

  ChainListIterator first = m_Chain.begin();
  (*first)->initCBLTReadout(controller, mcastAddress, wordsPerModule);
  
}
/*!
  Create the readout list for the Mesytec chain  This will be a fifo block read at the
  cblt address with a count that is the largest event size * maxwords * num_modules + 1
  That should be sufficient to ensure there's a terminating 0xffffffff
  @param rdolist - CVMUSBReadoutList we are appending our instructions to.
*/
void
CMADCChain::addReadoutList(CVMUSBReadoutList& rdolist)
{
  uint32_t location = m_pConfig->getUnsignedParameter("-cbltaddress");
  uint32_t mcast    = m_pConfig->getUnsignedParameter("-mcastaddress");

  size_t   size     = m_pConfig->getIntegerParameter("-maxwordspermodule");
  list<string>  modnames;
  modnames = getModules();
  size              = size * 36 * (modnames.size() + 1);

  rdolist.addFifoRead32(location, cbltamod, size);

  // Broadcast readout_reswet:

  
  rdolist.addWrite16(mcast + ReadoutReset, initamod, (uint16_t)1);
}
/*!
 * Virtual copy constructor.
 */


/////////////////////////////////////////////////////////////////////////////////////////
// Utility functions (private).
/////////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns the list of module names in the -modules option.  This is returned as an
 * stl list<std::string> Validation is assumed to already have been done by moduleChecker.
 */
list<string>
CMADCChain::getModules()
{
  auto moduleVec = m_pConfig->getList("-modules");
  std::list<std::string> result(moduleVec.begin(), moduleVec.end());
  
  return result;
}
/*
 * Takes a list of module names and turns them into a list of pointers to the modules.
 * The pointers are stored in m_Chain.
 */
void
CMADCChain::namesToList(list<string> moduleNames)
{
  
  m_Chain.clear();
  for (auto name : moduleNames) {
    CReadoutModule* pModule = m_pParser->findDevice(name);
    CMesytecBase* pAdc = dynamic_cast<CMesytecBase*>(pModule->getDriver());
    assert(pAdc);    // Validation ensured this.
    m_Chain.push_back(pAdc);
  }

}
/**
 * Checks that a module list consists of valid Mesytec modules.
 */
bool
CMADCChain::moduleChecker(string name, string value, void* arg)
{
  // Arg is really the parser:

  TCLConfigParser* parser = reinterpret_cast<TCLConfigParser*>(arg);
  CTCLInterpreter* pInterp = parser->getInterpreter();
  
  // Use CTCLObject to produce a vector of object encapsulated names:

  CTCLObject valueObj;
  valueObj.Bind(pInterp);
  valueObj = value;
  auto valueVec = valueObj.getListElements();

  for (auto nameObj : valueVec) {
    nameObj.Bind(pInterp);
    std::string name = nameObj;
    auto pModule = parser->findDevice(name);
    if (!pModule || !dynamic_cast<CMesytecBase*>(pModule->getDriver())) {
      return false;
    }
  }
  // All modules are present and are Mesytec modules:P

  return true;

  
}

