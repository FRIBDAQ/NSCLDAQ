/**
 * @brief Header for the CSIS3316 class that provides support for the SIS3316
 * @author Ron Fox <fox@frib.msu.edu>
 * 
 * Facility for Rare Isotope Beams
 * 640 S. Shaw Lane
 * East Lansing, MI 48824-1321
 * (c) Copyright 2025 Board of Trustees of Michigan State University 
 * 
 *  You may use this software under the terms of the GNU public license
 *   (GPL).  The terms of this license are described at:
 *
 *   http://www.gnu.org/licenses/gpl.txt
 * 
 * 
 */

#include "CSIS3316.h"
#include "CReadoutModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "sis3316_adc.h"
#include <sis_vmusb_interface.h>
#include "sis3316.h"                   // Register definitions.

// Parameter constraints:

// CLock sources:

static const char* ClockSources[] = {
    "fp", "250MHz","125Mhz", "50MHz", "25Mhz", "12.5MHz", NULL
};
static XXUSB::COnfigurableObjrect::isEnumParameter
    ClockSourceValues(XXUSB::CConfigurableObject::makeEnumSet(ClockSources));

static XXUSB::CConfigurableObject::limit Zero(0);    // Shared low limit for many things.
static XXUSB::CConfigurableObject::limit MaxSamples(65535);
static XXUSB::CConfigurableObject::limit MaxId(4095);
static XXUSB::CConfigurableObject::limit MaxPretrigger(65535);


/**
 *  constructor:
 *      JUst needs to initialize the pointers to null; and create the bus object.
 */
CSIS3316::CSIS3316() :
    m_pConfiguration(nullptr), m_pModule(nullptr), m_pVmeBus(nullptr)
{
    m_pVmeBus = new sis_vmusb_interface;         // By now there's a VMUSB object.
}
/**
 *  destructor:
 */
CSIS3316::~CSIS3316() {
    // Just need to delete the dynamic stuff:

    delete m_pModule;
    delete m_pVmeBus;
    delete m_pConfiguration;
}
/**
 *  Copy constructor = might actually be illegal we'll see.
 * 
 * @param rhs - the object being copied.
 */
CSIS3316::CSIS3316(const CSIS3316& rhs) :
    m_pConfiguration(nullptr), m_pModule(nullptr), m_pVmeBus(nullptr)
{
    if (rhs.m_pConfiguration) {
        m_pConfiguration = new CReadoutModule(*rhs.m_pConfiguration);
    }
    if (rhs.m_pVmeBus) {
        m_pVmeBus = new sis_vmusb_interface(*rhs.m_pVmeBus);
    }
    if (m_pModule) {
        m_pModule = new  sis3316_adc(*rhs.m_pModule);
    }
}

/**
 * operator= 
 *   @param rhs - the object being assigned to this.
 *   @return CSIS3316& - referenced to this.
 */
CSIS3316&
CSIS3316::operator=(const CSIS3316& rhs) {
    if (this != &rhs) {            // Else noop.,
        m_pConfiguration = rhs.m_pConfiguration ?
            new CReadoutModule(*rhs.m_pConfiguration) :
            nullptr;
        m_pVmeBus = rhs.m_pVmeBusj ?
            new sis_vmusb_inhterface(*rhs.m_pVmeBus) :
            nullptr;
        m_pModule = rhs.m_pModule ?
            new sis3316_adc(*rhs.m_podule) :
            nullptr;
    }

    return *this;
}

/**
 * onAttach:
 *    Provides an empty configuration to the object.  We save it for later
 * and define out configuration options into  it so we can be appropriately configured.
 * 
 * @param configuration - referencdes out configuration object.
 * 
 * 
 */
void
CSIS3316::onAttach(CReadoutModule& configuration) {

}