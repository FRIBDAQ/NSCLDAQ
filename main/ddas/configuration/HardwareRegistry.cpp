/**
 * @file HardwareRegistry.cpp
 * @brief Implement functions in the namespace used to store the DDAS
 * hardware information.
 */

#include "HardwareRegistry.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>

namespace HR = DAQ::DDAS::HardwareRegistry;

/**
 * @typedef Registry
 * @brief Map of hardware specifications (MSPS, bit depth, revision,
 * calibration) keyed by the hardware type.
 */
using Registry = std::map<int, HR::HardwareSpecification>;

// Static registry
static Registry *gpRegistry = nullptr;
static int sDefaultFirstAvailableUserType = 100;
static int sNextAvailableUserType = sDefaultFirstAvailableUserType;

//////////////////////////////////////////////////////////////////////////////
// Static utility methods

// Set up the registry with default parameters
static void setUpRegistry(Registry &registry) {
  // {freq, bits, hdwr rev, calib}
  registry[HR::RevB_100MHz_12Bit] = {100, 12, 11, 10.};
  registry[HR::RevC_100MHz_12Bit] = {100, 12, 12, 10.};
  registry[HR::RevD_100MHz_12Bit] = {100, 12, 13, 10.};
  registry[HR::RevF_250MHz_12Bit] = {250, 12, 15, 8.};
  registry[HR::RevF_250MHz_14Bit] = {250, 14, 15, 8.};
  registry[HR::RevF_250MHz_16Bit] = {250, 16, 15, 8.};
  registry[HR::RevF_500MHz_12Bit] = {500, 12, 15, 10.};
  registry[HR::RevF_500MHz_14Bit] = {500, 14, 15, 10.};
  registry[HR::RevH_250MHz_14Bit] = {250, 14, 17, 8.};
}

// Create a new registry and set default values
static Registry *createRegistry() {
  gpRegistry = new Registry;
  setUpRegistry(*gpRegistry);
  return gpRegistry;
}

// Avoid static initialization order fiasco by construct-on-first-use idiom
static Registry &getRegistry() {
  if (gpRegistry == nullptr) {
    return *createRegistry();
  } else {
    return *gpRegistry;
  }
}

void DAQ::DDAS::HardwareRegistry::configureHardwareType(
    int type, const DAQ::DDAS::HardwareRegistry::HardwareSpecification &spec) {
  getRegistry()[type] = spec;
}

DAQ::DDAS::HardwareRegistry::HardwareSpecification &
DAQ::DDAS::HardwareRegistry::getSpecification(int type) {
  Registry &registry = getRegistry();

  auto pFound = registry.find(type);
  if (pFound == registry.end()) {
    std::string errmsg("HardwareRegistry::getSpecification() ");
    errmsg += "Failed to locate specification for provided ";
    errmsg += "hardware type.";
    throw std::runtime_error(errmsg);
  }

  return pFound->second;
}

void DAQ::DDAS::HardwareRegistry::resetToDefaults() {
  Registry &registry = getRegistry();
  registry.clear();
  sNextAvailableUserType = sDefaultFirstAvailableUserType;
  setUpRegistry(registry);
}

int DAQ::DDAS::HardwareRegistry::computeHardwareType(int hdwrVersion,
                                                     int adcFreq, int adcRes) {
  HardwareSpecification spec = {adcFreq, adcRes, hdwrVersion};
  Registry &registry = getRegistry();
  auto res = std::find_if(registry.begin(), registry.end(),
                          [&spec](const Registry::value_type &element) {
                            return (element.second == spec);
                          });

  if (res != registry.end()) {
    return res->first;
  } else {
    return Unknown;
  }
}

int DAQ::DDAS::HardwareRegistry::createHardwareType(int hdwrVersion,
                                                    int adcFreq, int adcRes,
                                                    double clockCalibration) {
  Registry &registry = getRegistry();
  int type = computeHardwareType(hdwrVersion, adcFreq, adcRes);

  if (type == Unknown) {
    registry[sNextAvailableUserType] = {adcFreq, adcRes, hdwrVersion,
                                        clockCalibration};
    type = sNextAvailableUserType++;
  }

  return type;
}

//////////////////////////////////////////////////////////////////////////////

bool operator==(const HR::HardwareSpecification &lhs,
                const HR::HardwareSpecification &rhs) {
  return ((lhs.s_adcFrequency == rhs.s_adcFrequency) &&
          (lhs.s_adcResolution == rhs.s_adcResolution) &&
          (lhs.s_hdwrRevision == rhs.s_hdwrRevision));
}
