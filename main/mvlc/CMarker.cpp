/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321

@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for the driver that generates marker objects.
      
*/




#include "CMarker.h"
#include "DeviceCommand.h"
#include "CReadoutModule.h"
#include <XXUSBConfigurableObject.h>
#include <CVMUSBReadoutList.h>


#include <stdlib.h>

using namespace std;


static XXUSB::CConfigurableObject::limit valueLow(0);
static XXUSB::CConfigurableObject::limit valueHigh(0xffffffff);  // mvlc 32 bit markers
static XXUSB::CConfigurableObject::Limits valueLimits(valueLow, valueHigh);


///////////////////////////////////////////////////////////////////////////////////////
////////////////////////// Canonical class/object operations /////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

/** 
 * constructor:
 */
CMarker::CMarker() :
  m_pConfiguration(0)
{}

/**
 * destructor
 */
CMarker::~CMarker() {}

/////////////////////////////////////////////////////////////////////////////////
//////////////////////// Overridable operations /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*!
    Attach the module to its configuration by  storing the config reference and
    adding the -value parameter to the config.
    \param CReadoutModule& configuration

*/
void
CMarker::onAttach(XXUSB::CConfigurableObject& configuration)
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
  return m_pConfiguration->getUnsignedParameter(name);

}


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