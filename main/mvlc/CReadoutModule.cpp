/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Implementation for the packaging of device support and configuration.
*/

#include "CReadoutModule.h"
#include <XXUSBConfigurableObject.h>
#include "CReadoutHardware.h"


/////////////////////////////// canonicals ///////////////////////////////////////////

/**
 * constructor
 *    For now just initializes the member data to null.
 * Attach and SetDriver will do the rest.
 */

 CReadoutModule::CReadoutModule() :
    m_pConfiguration(0), m_pDeviceSupport(0) {
        
}
/**
 * destructor
 *    The configuration and driver must have been dynamically created and 
 * we now have ownershp of them so:
 */
CReadoutModule::~CReadoutModule() {
    delete m_pConfiguration;
    delete m_pDeviceSupport;
}

////////////////////////////// public methods. ////////////////////////////////////////

/** 
 * Attach
 *    Attach a new configuration to the system.  For now we just delete any old m_pConfiguration
 * and set the new one but:alignas
 * 
 * @param config - pointer to the new configuration - must have been 'newed' into existence.
 */
void
CReadoutModule::Attach(XXUSB::CConfigurableObject* config) {
    delete m_pConfiguration;
    m_pConfiguration = config;

    if (m_pDeviceSupport) m_pDeviceSupport->onAttach(*config);
}
/**
 * SetDriver
 *    Sets the device driver part of out package. 
 * 
 *  @param pDriver - pointer to adriver instance.

 */
void
CReadoutModule::SetDriver(CReadoutHardware* pDriver) {
    delete m_pDeviceSupport;
    m_pDeviceSupport = pDriver;

    if (m_pConfiguration) m_pDeviceSupport->onAttach(*m_pConfiguration);
}
/**
 * getConfiguration 
 *    Return a pointer to our config.
 */
XXUSB::CConfigurableObject*
CReadoutModule::getConfiguration() {
    return m_pConfiguration;
}
