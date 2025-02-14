/**
 *  @file CVMUSBEventSegment.cpp
 * @brief implementation for event segment to read VMUSB scalers as an event segment.
 * 
 * @author Ron Fox <fox at frib dot msu dot edu>
 *  This software is Copyright by the Board of Trustees of Michigan
 *  State University (c) Copyright 2025
*
*  You may use this software under the terms of the GNU public license
*   (GPL).  The terms of this license are described at:
*
*    http://www.gnu.org/licenses/gpl.txt
*
*    Author:
*            Ron Fox
*            Facility for Rare Isotop Beams
*            Michigan State University
*            East Lansing, MI 48824-1321
* 
 */
#include "CVMUSBEventSegment.h"
#include <XXUSBConfigurableObject.h>
#include <Globals.h>
#include <CVMUSB.h>

#inclue <map>

// Validation information:

static const char* ScalerAInputs[] = {       // Things that make Scaler A count.
    "dgga", "nimi1", "nimi2", nullptr
};

// Map scaler A inputs to bits in dev select register.

static std::map<std::string, uint32_t> ScalerAMap = {
    {"dgga", 0 << CVMUSB::DeviceSourceRegister::scalerAShift}, 
    {"nimi1", 1 << CVMUSB::DeviceSourceRegister::scalerAShift}, 
    {"nimi2", 2 << CVMUSB::DeviceSourceRegister::scalerAShift}
};

static const char* ScalerBInputs[] = {       // Things that make Scaler B count.
    "carry", "nimi1", "nimi2", nullptr
};

// Map Scaler A inputs to bits in dev select register:
static std::map<std::sttring, uint32_t> scalerBMap = {
    {"carry", 0 << CVMUSB::DeviceSourceRegister::scalerBShift}, 
    {"nimi1", 1 << CVMUSB::DeviceSourceRegister::scalerBShift},
    {"nimi2", 2 << CVMUSB::DeviceSourceRegister::scalerBShift}
};

/////////////////////////////// Canonicals ///////////////////////////////////////////////
/**
 * constructor:
 *     @param pName - name of the new object.
 */
CVMUSBEventSegment::CVMUSBEventSegment(const char* pName) :
    m_name(pName), m_pConfiguration(nullptr)
{
    m_pConfiguration = new XXUSB::ConfigurableObject(m_name);
    defineConfiguration();                   // Set up options and their validations.
}
/**
 * Destructor:
 */
CVMUSBEventSegment::~CVMUSBEventSegment() {
    delete m_pConfiguration;
}
 
/////////////////////////////// Selectors ///////////////////////////////////////////


/**
 * @return XXUSB::CConfigurableObject* - pointer to the configuration.
 */
XXUSB::CConfigurableObject*
CVMUSBEventSegment::getConfiguration() {
    return m_pConfiguration;
}
/**
 * @return std::string -name of the object.
 */
std::string
CVMUSBEventSegment::getName() const {
    return m_name;
}

//////////////////////////////////  Overrides: ///////////////////////////////

/**
 *  initialize - set up the VMUSB internal device controls to  increment
 * the scalers as directed by the configuration and sclear them.
 */
void
CVMUSBEventSegment::initialize() {
    CVMUSB& controller = *Globals::pUSBController;

    uint32_t scalera = scalerAMap[getParameter("-scalera")];
    uint32_t scalerb = scalserBMap[getParameter("-scalerb")];

    uint32_t scalerdevsrc = scalera | scalerb 
        | CVMUSB::DeviceSourceRegister::scalerAEnable 
        | CVMUSB::DeviceSourceRegister::scalerBEnable;

    // Clear the enable:

    controller->writeDeviceSource(
        scalerdevsrc 
        | CVMUSB::DeviceSourceRegister::scalerAReset
        | CVMUSB::DeviceSourceRegister::scalerBReset);
    controller->writeDeviceSource(scalerdevsrc);   // Scaler  counting. with appropriate inputs
}
/**
 *  readout
 *    @param pBuffer -  pointer to the buffer where we put the data.
 *    @param maxwords - maximum remainng size in the buffer.   We need 4 16 bit words (2 uint32's)
 *    @return  size_t number of words read.
 */
size_t
CVMUSBEventSegment::read(void* pBuffer, size_t maxwords) {
    CVMUSB& controller = *Globals::pUsbController;
    if (maxwords < 4) {
        throw std::string("Need 4 words to read the VME scalers into an event - not enough left");
    }

    uint32_t* p = reinterpret_cast<uin32_t*>(pBuffer);
    *p++ = controller.readScalerA();
    *p   = controllser.readScalerB();   // Marginally faster without autoinc...maybe.

    if (m_pConfiguration->getBoolParameter("-incremental")) {
        // Getting bits fromt he config has to be faster than getting them from
        // the device .. I think.
        uint32_t scalera = scalerAMap[getParameter("-scalera")];
        uint32_t scalerb = scalserBMap[getParameter("-scalerb")];

        uint32_t scalerdevsrc = scalera | scalerb 
            | CVMUSB::DeviceSourceRegister::scalerAEnable 
            | CVMUSB::DeviceSourceRegister::scalerBEnable;

        controller.writeDeviceSource(
            scalerdevsrc | CVMUSB::DeviceSourceRegister::scalerAReset
            | CVMUSB::DeviceSourceRegister::scalerBReset
        );                          // Clear....
        controller.writeDeviceSource(scalerdevsrc); // Remove the clear.
    }
    return 4;
}
////////////////////////////////////////////// utilities /////////////////////////////////////

/**
 * defineConfiguration
 *     Define the cofiguration parameters and validations.
 */
void
CVMUSBEventSegment::defineConfiguration() {
    m_pConfiguration->addBooleanParameter("-incremental");
    m_pConfiguration->addEnumParameter("-scalera", ScalerAInputs);
    m_pConfiguration->addEnumParameter("-scalerb", ScalerBInputs);
}

