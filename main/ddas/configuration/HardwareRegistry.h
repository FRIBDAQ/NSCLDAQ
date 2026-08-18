/**
 * @file HardwareRegistry.h
 * @brief Defines a namespace used to store
 * information about known DDAS modules.
 */

#ifndef HARDWAREREGISTRY_H
#define HARDWAREREGISTRY_H

/** @namespace DAQ */
namespace DAQ {
/** @namespace DAQ::DDAS */
namespace DDAS {

/**
 * @addtogroup configuration libConfiguration.so
 * @{
 */

/**
 * @namespace DAQ::DDAS::HardwareRegistry
 * @brief The HardwareRegistry namespace is where information about
 * all known DDAS modules are stored.
 * @details
 * The information that the user can access in this namespace's
 * functions is not specific to a module but rather to a hardware type.
 * If the user wants to know basic information  about the ADC
 * resolution or frequency, this namespace will scratch that itch.
 * However, if the user is looking for information about the serial
 * number, this is not the place for that. In fact, there  currently
 * is no place for that.
 *
 * The HardwareRegistry is fairly simple. It is just a map of
 * HardwareRegistry::HardwareSpecifications that are keyed by
 * HardwareRegistry::HardwareType enumerations. The HardwareRegistry
 * provides a default state that provides the normal specifications
 * for each hardware type in use at the NSCL. If for some crazy reason,
 * something changes, the user can change the specifications associated
 * with each device type.
 *
 * There is also a function to determine the hardware type enumeration
 * value given the hardware revision, ACD frequency, and ADC resolution
 * called HardwareRegistry::computeHardwareType(). The reverse
 * operation is called
 * HardwareRegistry::getSpecification(HardwareType type).
 * Together these two functions are probably the most useful tools.
 */

namespace HardwareRegistry {
/**
 * @brief Generic hardware specs for hardware types.
 */
struct HardwareSpecification {
  int s_adcFrequency;  //!< Module ADC frequency in MSPS.
  int s_adcResolution; //!< Module ADC resolution (bit depth).
  int s_hdwrRevision;  //!< Module hardware revision.
  /** FPGA clock calibration in ns/tick. */
  double s_clockCalibration;
};

/**
 * @brief The HardwareType enum
 * @details
 * The HardwareType enumeration provides an identifier for each
 * type of hardware that might be found in the system. The user
 * can determine which hardware type they are dealing with by
 * calling the `Pixie16ReadModuleInfo()` methods to access the ADC
 * sampling frequency, the ADC resolution, and the hardware
 * revision. Together with those three pieces of information, it
 * is possible to determine the appropriate HardwareType.
 *
 * @todo (ASC 3/13/24): Create a method that can query a module
 * and compute the HardwareType.
 */
enum HardwareType {
  RevB_100MHz_12Bit = 1,
  RevC_100MHz_12Bit = 2,
  RevD_100MHz_12Bit = 3,
  RevF_250MHz_12Bit = 4,
  RevF_250MHz_14Bit = 5,
  RevF_250MHz_16Bit = 6,
  RevF_500MHz_12Bit = 7,
  RevF_500MHz_14Bit = 8,
  RevH_250MHz_14Bit = 9,
  Unknown = 0
};

void configureHardwareType(int type, const HardwareSpecification &spec);
/**
 * @brief Retrieve a reference to the current hdwr specification
 * for a hardware type.
 * @param type The enumerated hardware type.
 * @throw std::runtime_error If no specification exists for the
 *   hardware type provided.
 * @return Reference to a hardware specificiation.
 */
HardwareSpecification &getSpecification(int type);
void resetToDefaults();
/**
 * @brief Compute the hardware type from input specifications.
 * @param hdwrVersion Hardware revision.
 * @param adcFreq ADC sampling frequency.
 * @param adcRes  ADC bit resolution (e.g. 12, 14, etc.).
 * @return An enumerated hardware type.
 */
int computeHardwareType(int hdwrVersion, int adcFreq, int adcRes);
/**
 * @brief Create an enumerated hardware type from input
 * specifications.
 * @param hdwrVersion Hardware revision.
 * @param adcFreq ADC sampling frequency.
 * @param adcRes  ADC bit resolution (e.g. 12, 14, etc.).
 * @param clockCalibration FPGA clock calibration in ns/clock tick.
 * @return An enumerated hardware type.
 */
int createHardwareType(int hdwrVersion, int adcFreq, int adcRes,
                       double clockCalibration);

} // namespace HardwareRegistry

/** @} */

} // namespace DDAS
} // namespace DAQ

/**
 * @brief Check if two HardwareSpecifications are the same.
 * @param lhs, rhs Left- and right-hand objects for comparison.
 * @return bool
 * @retval true  If lhs and rhs are equal.
 * @retval false Otherwise.
 */
bool operator==(const DAQ::DDAS::HardwareRegistry::HardwareSpecification &lhs,
                const DAQ::DDAS::HardwareRegistry::HardwareSpecification &rhs);

#endif // HARDWAREREGISTRY_H
