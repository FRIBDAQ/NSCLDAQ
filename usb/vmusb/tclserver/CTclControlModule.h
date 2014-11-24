/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CTCLCONTROLMODULE_H
#define __CTCLCONTROLMODULE_H


/**
 * @file CTclControlModule.h
 * @brief Class definition for a wrapper for Tcl based control modules.
 */

#ifndef __CCONTROLHARDWARE_H
#include "CControlHardware.h"	/* base class. */
#endif

class CTCLObject;
class CTCLInterpreter;

/**
 * @class CTclControlModule
 *
 *  This class allows one to define control modules that are implemented in Tcl.
 *  Tcl control modules must be command ensembles that implement the subcommands:
 *  Initialize, Update, Set, Get, addMonitorList processMonitorList and getMonitoredData
 *
 *  Typically this is all done by implementing either a snit::type or an incrTcl class
 *  Creating and configuring an instance of that entity and wrapping that entity in
 *  a CTclControlModule.  The wrapping is done by providing the command ensemble base command
 *  to the -ensemble configuration option.  e.g:
 * \verbatim
 *    snit::type mydriver {
 *    ...
 *    };                      #Assume this snit::type implements the appropriate methods.
 *    set instance [mydriver %AUTO%]
 *    Module create tcl mydriver
 *    Module config mydriver -ensemble $instance
 *
 * \endverbatim
 *
 *  @note The VMUSB and VMUSBReadoutList objects passed to the Tcl driver are the same 
 *  swig wrappers thatn are used for readout drivers.
 */
class CTclControlModule :  public CControlHardware
{
public:
  CTclControlModule();
  
public:
  virtual void onAttach(CControlModule& configuration);
  virtual void Initialize(CVMUSB& vme);	                     //!< Optional initialization.
  virtual std::string Update(CVMUSB& vme);
  virtual std::string Set(CVMUSB& vme, 
			  std::string parameter, 
			  std::string value);
  virtual std::string Get(CVMUSB& vme, 
			  std::string parameter);
  virtual void clone(const CControlHardware& rhs);

  // Interface to support monitoring.

  virtual void addMonitorList(CVMUSBReadoutList& vmeList);     //!< add items to the monitor list.
  virtual void* processMonitorList(void* pData, size_t remaining);
  virtual std::string getMonitoredData();

private:
  static std::string swigPointer(void* pObject, std::string type);
  static CTCLObject* marshallData(CTCLInterpreter* pInterp, void* pData, size_t nBytes);
};
#endif
