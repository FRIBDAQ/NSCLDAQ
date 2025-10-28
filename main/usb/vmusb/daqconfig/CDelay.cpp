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

// Implementation of a stack delay driver 


#include <config.h>
#include <tcl.h>

#include "CDelay.h"

#include <CReadoutHardware.h>
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
#else
#include <CUserCommand.h>
#endif
#include <CVMUSBReadoutList.h>
#include <stdlib.h>

static XXUSB::CConfigurableObject::limit valueLow(0);
#ifdef MVCL_GENERATOR
static XXUSB::CConfigurableObject::limit valueHigh(65536);   // 65 seconds should be more than long.
#else
static XXUSB::CConfigurableObject::limit valueHigh(255);
#endif
static XXUSB::CConfigurableObject::Limits valueLimits(valueLow, valueHigh);


///////////////////////////////////////////////////////////////////////////////////////
////////////////////////// Canonical class/object operations /////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

CDelay::CDelay() :
  m_pConfiguration(0)
{}

CDelay::~CDelay() {}

#ifndef MVLC_GENERATOR
CDelay::CDelay(const CDelay& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}


CDelay&
CDelay::operator=(const CDelay& rhs)
{
  return *this;
}
#endif
/////////////////////////////////////////////////////////////////////////////////
//////////////////////// Overridable operations /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*!
    Attach the module to its configuration by  storing the config reference and
    adding the -value parameter to the config.
    \param CReadoutModule& configuration

*/
void
#ifdef MVLC_GENERATOR
CDelay::onAttach(XXUSB::CConfigurableObject& configuration)
#else
CDelay::onAttach(CReadoutModule& configuration)
#endif
{
  m_pConfiguration = &configuration;

  m_pConfiguration->addParameter("-value", XXUSB::CConfigurableObject::isInteger, &valueLimits, "0");

}
/*!
   The device does not need to be initialized.
*/
void 
CDelay::Initialize(CVMUSB& controller)
{}

/*!
    The module is added to the readout list by fetching the value parameter as a 16 bit integer
    and adding the marker command to the readout list.
*/
void
CDelay::addReadoutList(CVMUSBReadoutList& list)
{
  unsigned int value = getIntegerParameter("-value");

  list.addDelay(static_cast<uint16_t>(value));
}
#ifndef MVLC_GENERATOR
/*!
  Virtual constructor:

*/
CReadoutHardware* 
CDelay::clone() const
{
  return new CDelay(*this);
}
#endif
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
CDelay::getIntegerParameter(std::string name)
{
  std::string sValue =  m_pConfiguration->cget(name);
  unsigned int    value  = strtoul(sValue.c_str(), NULL, 0);

  return value;
}

#ifdef MVLC_GENERATOR

//////////////////////////// Implement CDelayCommand


/** constructor
 * @param interp - interpreter the delay command is registered on
 * @param parser - The parser into which modules will be registered.
 * 
 */
CDelayCommand::CDelayCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "delay", parser) {

}

/**
 *  destructor.
 */
CDelayCommand::~CDelayCommand() {}

/**
 *  createDevice
 *      Create the module that wraps the device instance.
 */
CReadoutModule*
CDelayCommand::createDevice(std::string name) {
  auto result = new CReadoutModule();
  result->SetDriver(new CDelay);

  return result;
}
#endif