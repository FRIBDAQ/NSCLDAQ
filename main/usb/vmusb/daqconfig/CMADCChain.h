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
 * @file CMADCChain.h 
 * @brief Header for the Mesytec chain manager for Mesytec devices that can CBLT.
 * @note WHen compiled with mvlcgenerate MVLC_GENERATOR is defined making this
 * work in both VMUSB and MVLC.
 */
#ifndef __CMADCCHAIN_H
#define __CMADCCHAIN_H
#include <CReadoutHardware.h>
#include <CMesytecBase.h>
#include <stdint.h>
#include <string>
#include <list>
#ifdef MVLC_GENERATOR
#include <DeviceCommand.h>
#endif


class CReadouModule;
class CVMUSB;
class CVMUSBReadoutList;
class CMADC32;

#ifdef MVLC_GENERATOR
class TCLConfigParser;
namespace XXUSB { 
  class CConfigurableObject;
}
#endif

/*!
   This class implements a chain of MADC32's to be read out via CBLT.
   The module contains an ordered list of modules which must be provided
   from 'left to right' in the VME crate.
   The list has a CBLT and MCST address assigned to it and 
   the MCST is used to re-enable after readout as well as to synchronously clear
   the timestamp counters in the modules in the chain.
   
   Configuration Parameters are:

\verbatim
   Parameter         Value type/default  Meaning
   -cbltaddress      uint32 0            Chained block transfer base address.
   -mcastaddress     uint32 0            Multicast base addresss.
   -maxwordspermodule int 1024           Maximum number of words to read from each  module
                                         this will actually be rounded up to the next  event boundary.
   -modules          list of strings {}  Names of the MADC32 modules included in the chain.
\endverbatim


*/
class CMADCChain : public CReadoutHardware
{
  // internally used data types:

private:
  typedef std::list<CMesytecBase*>    ChainList;
  typedef ChainList::iterator    ChainListIterator;
  
  // Per object data:
private:
  ChainList         m_Chain;	// List of modules first must be leftmost, last must be right most.
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject*   m_pConfig;  // My configuration database.
  TCLConfigParser* m_pParser;
#else
  CReadoutModule*   m_pConfig;  // My configuration database.
#endif

public:
#ifdef MVLC_GENERATOR
  CMADCChain(TCLConfigParser* parser);
#else
  CMADCChain();
#endif
  virtual ~CMADCChain();
#ifndef MVLC_GENERATOR
private:
#endif
  CMADCChain(const CMADCChain& rhs);
  CMADCChain& operator=(const CMADCChain& rhs);

  // Disallowed canonicals:

private:
  int operator==(const CMADCChain& rhs) const;
  int operator!=(const CMADCChain& rhs) const;
public:


  // overridable oeprations on constructed objects:

public:
#ifdef MVLC_GENERATOR
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
#else
  virtual void onAttach(CReadoutModule& configuration);
#endif
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
#ifndef MVLC_GENERATOR
  virtual CReadoutHardware* clone() const;
#endif
  // utilities:

private:

  std::list<std::string> getModules();
  void                   namesToList(std::list<std::string> moduleNames); // populate m_Chain.
  static bool moduleChecker(std::string name, std::string value, void* arg);
  
};

#ifdef MVLC_GENERATOR
/**
 * @class CMADCChainCommand
 * 
 *   Command/generator that defines the "madcchain" command to create/config chains.
 */

class CMADCChainCommand : public DeviceCommand {
  TCLConfigParser* m_parser;                   // Needed to pass to chains.
public:
  CMADCChainCommand(CTCLInterpreter& interp, TCLConfigParser& parser) ;
  virtual ~CMADCChainCommand();
protected:
  CReadoutModule* createDevice(std::string name);
};
#endif
#endif
