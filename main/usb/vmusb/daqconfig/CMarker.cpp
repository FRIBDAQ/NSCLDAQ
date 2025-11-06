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
 * @file CMarker.cpp
 * @brief Implementation for marker implementation for VMUSB and MVLC
 * @note MVLC_GENERATOR is defined when compiling for mvlcgenerate
 * @note Markers in the MVLC are 32 bits wide while in the VMUSB 16 bits
 */


#include <config.h>

#include <CMarker.h>

#include <CReadoutModule.h>
#include <CVMUSBReadoutList.h>
#include <stdlib.h>
#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
#endif

using namespace std;


static XXUSB::CConfigurableObject::limit valueLow(0);
#ifdef MVLC_GENERATOR
static XXUSB::CConfigurableObject::limit valueHigh(0xfffffff);
#else
static XXUSB::CConfigurableObject::limit valueHigh(0xffff);
#endif
static XXUSB::CConfigurableObject::Limits valueLimits(valueLow, valueHigh);


///////////////////////////////////////////////////////////////////////////////////////
////////////////////////// Canonical class/object operations /////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

CMarker::CMarker() :
  m_pConfiguration(0)
{}



CMarker::~CMarker() {}

#ifndef MVLC_GENERATOR
CMarker::CMarker(const CMarker& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}
CMarker&
CMarker::operator=(const CMarker& rhs)
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
CMarker::onAttach(XXUSB::CConfigurableObject& configuration)
#else
CMarker::onAttach(CReadoutModule& configuration)
#endif
{
  m_pConfiguration = &configuration;

  m_pConfiguration->addParameter(
    "-value", XXUSB::CConfigurableObject::isInteger, &valueLimits, "0"
  );

}
/*!
   The device does not need to be initialized.
*/
void 
CMarker::Initialize(CVMUSB& controller)
{}

/*!
    The module is added to the readout list by fetching the value parameter as a 16 bit integer
    and adding the marker command to the readout list.
*/
void
CMarker::addReadoutList(CVMUSBReadoutList& list)
{
  unsigned int value = getIntegerParameter("-value");

  list.addMarker(value);
}
#ifndef MVLC_GENERATOR
/*!
  Virtual constructor:

*/
CReadoutHardware* 
CMarker::clone() const
{
  return new CMarker(*this);
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
CMarker::getIntegerParameter(string name)
{
  string sValue =  m_pConfiguration->cget(name);
  unsigned int    value  = strtoul(sValue.c_str(), NULL, 0);

  return value;
}


#ifdef MVLC_GENERATOR

////////////////////// Implement the marker command class.   ///////////////

/**
 *  constructor:
 *    All the work is done in the base class constructor.
 */
CMarkerCommand::CMarkerCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "marker", parser) {}

/**
 * destructor:
 */
CMarkerCommand::~CMarkerCommand() {}


/** createDevice
 *    Responsible for creating a new CMarker object:
 * 
 * @param name (unused) - name of the marker object.
 * @return CReadoutModule* - a readout module with a CMarker driver object installed.
 * 
 */
CReadoutModule*
CMarkerCommand::createDevice(std::string name) {
  CReadoutModule* result = new CReadoutModule;
  result->SetDriver(new CMarker);

  return result;
}
#endif
