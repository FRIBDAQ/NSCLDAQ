/**
 * @file ConfigurationParser.h
 * @brief Define a class to parse the contents of the cfgPixie16.txt file.
 */

#ifndef CONFIGURATIONPARSER_H
#define CONFIGURATIONPARSER_H

#include <iosfwd>
#include <regex>
#include <tuple>

/** @namespace DAQ */
namespace DAQ {
/** @namespace DAQ::DDAS */
namespace DDAS {

class Configuration;
class FirmwareConfiguration;

/**
 * @addtogroup configuration libConfiguration.so
 * @{
 */

/**
 * @class ConfigurationParser ConfigurationParser.h
 * @brief A class to parse the contents of the cfgPixie16.txt file.
 * @details
 * This file is pretty basic. It contains information about the slot
 * map, crate id, and settings file path. It has the following form:
 *
 @verbatim
 CRATE_ID
 NUM_MODULES
 SLOT_MODULE_0   [Per-module-firmware-map [per-module-set-file]]
 SLOT_MODULE_1   [Per-module-firmware-map [per-module-set-file]]
 ...
 SLOT_MODULE_N-1
 PATH_TO_SETTINGS_FILE
 @endverbatim
 *
 * where CRATE_ID is a non-negative number, NUM_MODULES is a positive
 * number,  SLOT_MODULE_# is a number greater than or equal to 2, and
 * PATH_TO_SETTINGS_FILE is a legitimate path. In the top section, the
 * parser will ignore up to 256 characters following the leftmost
 * integer or string found on each line. Because of this, it is
 * customary to add notes on each of these lines. All notes should
 * be prepended with a '#':
 *
 @verbatim
 1    # crate id
 2    # number of modules
 2    # slot of first module
 3    # slot of second module
 /path/to/setfile.json # another comment
 @endverbatim
 *
 * Note the structure shown above reflects changes for issue
 * daqdev/DDAS#106. Each slot specification can have an optional one
 * or two fields: The first optional field is a per-slot firmware map
 * file and the second an optional per-slot DSP settings file
 * (since optional firmwares may require settings files of a different
 * format). :
 @verbatim
 1    # crate id
 5    # number of modules
 2    # slot of first module
 3    FirmwareMap.txt # Use FW defined in this map
 4    FirmwareMap.txt alternate_setfile.json # FW map and settings
 5    # Default FW and settings
 6
 /path/to/setfile.json # another comment
 @endverbatim
 * The ConfigurationParser can be used in the following fashion:
 * @code
 * using namespace DAQ::DDAS;
 * Configuration config;
 * ConfigurationParser parser;
 * std::ifstream configFile("cfgPixie16.txt", std::ios::in);
 * parser.parse(configFile, config);
 * @endcode
 *
 */
class ConfigurationParser {
public:
  /**
   * @typedef SlotSpecification
   * @brief Data returned when parsing a slot.
   * @details
   * daqdev/DDAS#106. This typedef defines the data that can be
   * returned when parsing a slot line. The int is the slot number.
   * The first string is the optional firmware map (empty string if
   * not given) and the  last the optional .set file specification
   * (empty if not given).
   */
  typedef std::tuple<int, std::string, std::string> SlotSpecification;

public:
  /**
   * @brief Parse the contents of the cfgPixie16.txt file.
   * @param input The input stream associated with the cfgPixie16
   *   content (likely an std::ifstream)
   * @param config A configuration to store the parsed data.
   * @throw std::runtime_error If the DSP parameter file has an
   *   invalid file extension.
   * @throw std::runtime_error If the configuration file contains
   *   anything other than whitespace after reading the settings file.
   */
  void parse(std::istream &input, Configuration &config);
  /**
   * @brief Parses a slot line.
   * @param input Input stream from which the line is parsed.
   * @throw std::runtime_error If there are errors processing this
   *   line, e.g. the slot cannot be decoded or a file is not
   *   readable.
   * @return Tuple containing the slot number and and file paths.
   *   The filepaths will be empty strings if omitted.
   */
  SlotSpecification parseSlotLine(std::istream &input);
};

/** @} */

} // namespace DDAS
} // namespace DAQ

#endif // CONFIGURATIONPARSER_H
