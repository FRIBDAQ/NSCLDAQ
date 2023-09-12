/**
 * @file HardwareRegistry.cpp
 * @brief Implement functions in the namespace used to store the DDAS 
 * hardware information.
 */

#include "HardwareRegistry.h"

#include <algorithm>
#include <tuple>
#include <map>
#include <string>
#include <stdexcept>

namespace HR = DAQ::DDAS::HardwareRegistry;
/** @typedef Registry
 * @brief Map of hardware specifications (MSPS, bit depth, revision, 
 * calibration) keyed by the hardware type.
 */
using Registry = std::map<int, HR::HardwareSpecification>;

// static registry
static Registry* gpRegistry = nullptr;
static int sDefaultFirstAvailableUserType = 100;
static int sNextAvailableUserType = sDefaultFirstAvailableUserType;

//////////////////////////////////////////////////////////////////////////////
// Static utility methods

// Set up the registry with default parameters
static void
setUpRegistry(Registry& registry) {
    // {freq, bits, hdwr rev, calib}
    registry[HR::RevB_100MHz_12Bit] = {100, 12, 11, 10. }; 
    registry[HR::RevC_100MHz_12Bit] = {100, 12, 12, 10. };
    registry[HR::RevD_100MHz_12Bit] = {100, 12, 13, 10. };
    registry[HR::RevF_100MHz_14Bit] = {100, 14, 15, 10. };
    registry[HR::RevF_100MHz_16Bit] = {100, 16, 15, 10. };
    registry[HR::RevF_250MHz_12Bit] = {250, 12, 15, 8. };
    registry[HR::RevF_250MHz_14Bit] = {250, 14, 15, 8. };
    registry[HR::RevF_250MHz_16Bit] = {250, 16, 15, 8. };
    registry[HR::RevF_500MHz_12Bit] = {500, 12, 15, 10. };
    registry[HR::RevF_500MHz_14Bit] = {500, 14, 15, 10. };
    registry[HR::RevF_500MHz_16Bit] = {500, 16, 15, 10. };
}

// Create a new registry and set default values
static Registry*
createRegistry()
{
    gpRegistry = new Registry;
    setUpRegistry(*gpRegistry);
    return gpRegistry;
}

// Avoid static initialization order fiasco by using a construct on first
// use idiom
static Registry&
getRegistry()
{
    if (gpRegistry == nullptr) {
        return *createRegistry();
    } else {
        return *gpRegistry;
    }
}

//////////////////////////////////////////////////////////////////////////////

/**
 * @brief Check if two HardwareSpecifications are the same.
 *
 * Two HardwareSpecifications are equal to one another iff the ADC frequency, 
 * ADC resolution, and revision number are the same.
 *
 * @param lhs  Left hand side specs.
 * @param rhs  Right hand side specs.
 *
 * @return bool
 * @retval true   If lhs and rhs are equal.
 * @retval false  Otherwise.
 */
bool
operator==(
    const HR::HardwareSpecification& lhs, const HR::HardwareSpecification& rhs
    )
{
    return ((lhs.s_adcFrequency == rhs.s_adcFrequency)
            && (lhs.s_adcResolution == rhs.s_adcResolution)
            && (lhs.s_hdwrRevision == rhs.s_hdwrRevision));
}    

/*!
 * \brief Configure the specifications associated with a hardware type.
 *
 * This method replaces whatever specification prexisted that was associated 
 * with the hardware type.
 * 
 * \param type  The enumerated hardware type.
 * \param spec  A specification to assign.
 */
void
DAQ::DDAS::HardwareRegistry::configureHardwareType(
    int type, const DAQ::DDAS::HardwareRegistry::HardwareSpecification &spec
    )
{
    getRegistry()[type] = spec;
}

/*!
 * \brief Retrieve a reference to the current hdwr specification for a 
 * hardware type.
 *
 * \param type  The enumerated hardware type.
 *
 * \throws std::runtime_error  If no specification exists for the hardware 
 *   type provided.
 *
 * \return HardwareSpecification&  Reference to a hardware specificiation.
 */
DAQ::DDAS::HardwareRegistry::HardwareSpecification&
DAQ::DDAS::HardwareRegistry::getSpecification(int type)
{
    Registry& registry = getRegistry();

    auto pFound = registry.find(type);
    if (pFound == registry.end()) {
	std::string errmsg("HardwareRegistry::getSpecification() ");
	errmsg += "Failed to locate specification for provided ";
	errmsg += "hardware type.";
	throw std::runtime_error(errmsg);
    }

    return pFound->second;
}

/*!
 * \brief Reset the contents of the registry to the default state
 */
void
DAQ::DDAS::HardwareRegistry::resetToDefaults()
{
    Registry& registry = getRegistry();
    registry.clear();
    sNextAvailableUserType = sDefaultFirstAvailableUserType;
    setUpRegistry(registry);
}


/*!
 * \brief Lookup a hardware type enumeration given info about a module
 *
 * \param hdwrVersion  Hardware revision.
 * \param adcFreq      ADC sampling frequency.
 * \param adcRes       ADC resolution (e.g. 12, 14, etc.).
 *
 * \return int  An enumerated hardware type.
 */
int
DAQ::DDAS::HardwareRegistry::computeHardwareType(
    int hdwrVersion, int adcFreq, int adcRes
    )
{
    HardwareSpecification spec = {adcFreq, adcRes, hdwrVersion};
    Registry& registry = getRegistry();
    auto res = std::find_if(
	registry.begin(), registry.end(),
	[&spec](const Registry::value_type& element)
	    {
		return (element.second == spec);
	    }
	);

    if (res != registry.end()) {
	return res->first;
    } else {
	return Unknown;
    }
}

/*!
 * \brief Create an enumerated hardware type from input specifications.
 *
 * \param hdwrVersion       Hardware revision.
 * \param adcFreq           ADC sampling frequency.
 * \param adcRes            ADC resolution (e.g. 12, 14, etc.).
 * \param clockCalibration  FPGA clock calibration in ns/clock tick.
 *
 * \return int  An enumerated hardware type.
 */   
int
DAQ::DDAS::HardwareRegistry::createHardwareType(
    int hdwrVersion, int adcFreq, int adcRes, double clockCalibration
    )
{
    Registry& registry = getRegistry();
    int type = computeHardwareType(hdwrVersion, adcFreq, adcRes);
		
    if (type == Unknown) {
	registry[sNextAvailableUserType] = {
	    adcFreq, adcRes, hdwrVersion, clockCalibration
	};
	type = sNextAvailableUserType++;
    }
		
    return type;
}
