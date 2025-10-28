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
/**
 * @file CMADCChain.cpp
 * @brief Implementation of Mesytec CBLT chains for VMUSB and MVLC
 * @note MVLC_GENERATOR is defined if compiling for mvlcgenerate.
 */
#include <config.h>
#include <CMADCChain.h>
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <CMADC32.h>
#include <CConfiguration.h>
#ifndef MVLC_GENERATOR
#include <Globals.h>
#endif
#include <tcl.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <MADC32Registers.h>

#include <iostream>
#include <unistd.h>

#ifdef MVLC_GENERATOR
#include <TCLConfigParser.h>
#include <XXUSBConfigurableObject.h>
#endif

using namespace std;

static XXUSB::CConfigurableObject::limit Zero(0);
static XXUSB::CConfigurableObject::limit MaxWords(1024); // should fit in a VM-USB buffer with 12modules.
static XXUSB::CConfigurableObject::Limits WordLimit(Zero, MaxWords);

////////////////////////////////////////////////////////////////////////////////////////////
// Canonical method implementatinos.
////////////////////////////////////////////////////////////////////////////////////////////

/*!
  Constructino is a noop unless this is MVLC in which case the configuation
  parser has to be saved to check modules.
*/
#ifdef MVLC_GENERATOR
CMADCChain::CMADCChain(TCLConfigParser* parser) :
  m_pConfig(0),
  m_pParser(parser)
{
}
#else
CMADCChain::CMADCChain() :
  m_pConfig(0)
{
}
#endif

/*!
  Configurations get destroyed by themseves so:
*/
CMADCChain::~CMADCChain()
{
}
#ifndef MVLC_GENERATOR
/*!
   Copy construction requires configuration duplication as well as duplicatino of the
   chain list.
*/
CMADCChain::CMADCChain(const CMADCChain& rhs) :
  m_Chain(rhs.m_Chain),
  m_pConfig(0)
{
  if (rhs.m_pConfig) {
    m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
  }
}
/*!
   Assignment:
*/
CMADCChain&
CMADCChain::operator=(const CMADCChain& rhs)
{
  return *this;			// not really implemented or needed.
}
#endif
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
#ifdef MVLC_GENERATOR
CMADCChain::onAttach(XXUSB::CConfigurableObject& configuration)
#else
CMADCChain::onAttach(CReadoutModule& configuration)
#endif
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
#ifdef MVLC_GENERATOR
  configuration.addParameter("-modules",
			     CMADCChain::moduleChecker,
			     m_pParser, "");    
#else
  configuration.addParameter("-modules",
			     CMADCChain::moduleChecker,
			     NULL, "");
#endif
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
#ifndef MVLC_GENERATOR
/*!
 * Virtual copy constructor.
 */
CReadoutHardware*
CMADCChain::clone() const
{
  return new CMADCChain(*this);
}
#endif
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
  // These two are for Tcl_SplitList

  int    argc;
  const char** argv;
  list   <string>   Result;	                       // This is what we'll return.

  string sValue  = m_pConfig->cget("-modules"); // String valued parameter...
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
/*
 * Takes a list of module names and turns them into a list of pointers to the modules.
 * The pointers are stored in m_Chain.
 */
void
CMADCChain::namesToList(list<string> moduleNames)
{
  m_Chain.clear();
#ifdef MVLC_GENERATOR
   
  for (auto name : moduleNames) {
    CReadoutModule* pModule = m_pParser->findDevice(name);
    CMesytecBase* pAdc = dynamic_cast<CMesytecBase*>(pModule->getDriver());
    assert(pAdc);    // Validation ensured this.
    m_Chain.push_back(pAdc);
  }
#else
  CConfiguration*   pModules = Globals::pConfig;
  


  list<string>::iterator pName = moduleNames.begin();
  while(pName != moduleNames.end()) {
    string name = *pName;
    CReadoutModule* pModule = pModules->findAdc(name);
    CMesytecBase*        pAdc    = dynamic_cast<CMesytecBase*>(pModule->getHardwarePointer());
    assert(pAdc);
    m_Chain.push_back(pAdc);

    pName++;
  }
#endif
}
/**
 * Checks that a module list consists of valid Mesytec modules.
 */
bool
CMADCChain::moduleChecker(string name, string value, void* arg)
{
#ifdef MVLC_GENERATOR
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
#else
  // value should be a properly formatted list of strings.
  // each string should be the name of an Mesytec module.

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
    if (!pModule || !dynamic_cast<CMesytecBase*>(pModule->getHardwarePointer())) { // Not found, or not a C785.
      Tcl_Free((char*) argv);
      return false;
    } 
  }
  // Control only gets here if all validations passed
  
  Tcl_Free((char*)argv);
#endif
  return true;		       
}

#ifdef MVLC_GENERATOR
/////////////////////////////// Implementation of the CMADCChainCommand 

/**
 *  constructor
 *    We need to save the parser to pass to instances of CMADCChain we create:
 */

CMADCChainCommand::CMADCChainCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "madcchain", parser), m_parser(&parser) {}

  /**
   * destructor
   */

CMADCChainCommand::~ CMADCChainCommand() {}

   /**
    *  create the module wrapping a device:
    */
CReadoutModule*
CMADCChainCommand::createDevice(std::string name) {
  CReadoutModule* result = new CReadoutModule;
  result->SetDriver(new CMADCChain(m_parser));

  return result;
}

#endif