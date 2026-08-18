/**
 * @file FirmwareVersionFileParser.cpp
 * @brief Implementation of the class used to parse DDASFirmwareVersion.txt.
 */

#include "FirmwareVersionFileParser.h"

#include <iostream>
#include <string>

namespace {
// The total number of hardware types we expect at the NSCL
const int TOTAL_PIXIE16_VARIANTS = 9;
} // namespace

/**
 * @details
 * Regular expression matching
 @verbatim "(^\[Rev([xXa-fA-F0-9]+)-(\d+)Bit-(\d+)MSPS\]$)" @endverbatim
 * to extract the firmware, bit depth, and module MSPS.
 */
DAQ::DDAS::FirmwareVersionFileParser::FirmwareVersionFileParser()
    : m_matchExpr(R"(^\[Rev([xXa-fA-F0-9]+)-(\d+)Bit-(\d+)MSPS\]$)") {}

/**
 * @details
 * Any firmware configurations that were stored in the configuration object
 * before this will be overwritten with new content.
 */
void DAQ::DDAS::FirmwareVersionFileParser::parse(
    std::istream &input, DAQ::DDAS::FirmwareMap &config) {
  FirmwareConfiguration empty;

  // These will overwrite any existing firmware configurations with
  // an empty configuration.
  config[HardwareRegistry::RevB_100MHz_12Bit] = empty;
  config[HardwareRegistry::RevC_100MHz_12Bit] = empty;
  config[HardwareRegistry::RevD_100MHz_12Bit] = empty;
  config[HardwareRegistry::RevF_250MHz_12Bit] = empty;
  config[HardwareRegistry::RevF_250MHz_14Bit] = empty;
  config[HardwareRegistry::RevF_250MHz_16Bit] = empty;
  config[HardwareRegistry::RevF_500MHz_12Bit] = empty;
  config[HardwareRegistry::RevF_500MHz_14Bit] = empty;
  config[HardwareRegistry::RevH_250MHz_14Bit] = empty;

  // Read input file with code provided by XIA using XIA defined
  // formatted file
  for (std::string line; std::getline(input, line, '\n');) {
    if (std::regex_match(line, m_matchExpr)) {
      std::smatch color_match;
      std::regex_search(line, color_match, m_matchExpr);
      int revision = std::stoi(
          std::string(color_match[1].first, color_match[1].second), 0, 0);
      int adcRes =
          std::stoi(std::string(color_match[2].first, color_match[2].second));
      int adcFreq =
          std::stoi(std::string(color_match[3].first, color_match[3].second));
      int calibration;

      /** @todo (ASC 3/13/24): CFWFileParseError for exceptions. */

      FirmwareConfiguration fwConfig;
      std::string msg("DDASFirmwareVersionFile.txt is incomplete! ");

      input >> fwConfig.s_ComFPGAConfigFile;
      if (!input.good()) {
        msg += "Failed to read ComFPGAConfigFile from ";
        msg += fwConfig.s_ComFPGAConfigFile;
        throw std::runtime_error(msg);
      }

      input >> fwConfig.s_SPFPGAConfigFile;
      if (!input.good()) {
        msg += "Failed to read SPFPGAConfigFile from ";
        msg += fwConfig.s_SPFPGAConfigFile;
        throw std::runtime_error(msg);
      }

      input >> fwConfig.s_DSPCodeFile;
      if (!input.good()) {
        msg += "Failed to read DSPCodeFile from ";
        msg += fwConfig.s_DSPCodeFile;
        throw std::runtime_error(msg);
      }

      input >> fwConfig.s_DSPVarFile;
      if (!input.good()) {
        msg += "Failed to read DSPVarFile from ";
        msg += fwConfig.s_DSPVarFile;
        throw std::runtime_error(msg);
      }

      input >> calibration;
      if (!input.good()) {
        msg += "Failed to read clock calibration from ";
        msg += calibration;
        throw std::runtime_error(msg);
      }

      int type = HardwareRegistry::createHardwareType(revision, adcFreq, adcRes,
                                                      calibration);
      config[type] = fwConfig;
    }
  }
}
