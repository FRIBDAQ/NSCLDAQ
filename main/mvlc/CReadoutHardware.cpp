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
@brief Implementation for the base class for device driver classes.
*/
#include "CReadoutHardware.h"
#include <XXUSBConfigurableObject.h>


/**
 * constructor
 */
CReadoutHardware::CReadoutHardware()  :
    m_pConfiguration(nullptr)
{}
/**
 *  destructor
 *    We own the configuration so delete it:
 */
CReadoutHardware::~CReadoutHardware()  {
    delete m_pConfiguration;
}

/**
 * setConfiguration
 *   Replace the configuration object.  We assume ownership of the 
 * configuration and assume it was created via 'new'
 * 
 * @param pConfig - pointer to the new configuration object.
 */
void
CReadoutHardware::setConfiguration(XXUSB::CConfigurableObject* pConfig) {
    delete m_pConfiguration;
    m_pConfiguration = pConfig;
}
/**
 *  onAttach
 *     Public interface used to set a new configuration and define its parameters.
 * This base class just doesw a setConfiguration to save it so parameters can be retrieved.
 * 
 * @param configuration - References the device configuration.
 */
void
CReadoutHardware::onAttach(XXUSB::CConfigurableObject& configuration) {
    setConfiguration(&configuration);
}
/**
 * Initialize - perfrom any operations needed to initialize the devie in accordance
 * with its conrfiguration.  In general, only write operations are supported here because
 * we are genertei9ng saved VME operations to be put in the MVLC configuration.
 * 
 * @param controller - actually an object that records the operations needed to initialize
 *  the modules.
 */
void
CReadoutHardware::Initialize(CVMUSB& controller) {
    // Default; do nothing.
}
/**
 *  addReadoutList
 *     Records the operations that are needed to read out the module during data taking
 * for a trigger.alignas
 * @param list - records the list of operations issued by this method.
 */
void
CReadoutHardware::addReadoutList(CVMUSBReadoutList& list) {

}
/**
 * onEndRun
 *    Called to suply operations that are performed when the run ends.
 * @param controller - object that will memorize the operations perfromed.
 */
void
CReadoutHardware::onEndRun(CVMUSB& interface) {}