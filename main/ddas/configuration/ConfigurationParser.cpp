/**
 * @file ConfigurationParser.cpp
 * @brief Define a class to parse the cfgPixie16.txt file.
 */

#include "ConfigurationParser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <vector>

#include <config.h>

#include "Configuration.h"
#include "FirmwareVersionFileParser.h"

// Number of characters to skip when parsing a line. Maximum allowed length of
// any comment added by a user.
#define FILENAME_STR_MAXLEN 256

///
// Local trim functions
//

/** @brief Trim from beginning. */
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

/** @brief Trim from end. */
static inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace)))
              .base(),
          s.end());
  return s;
}

/** @brief Trim from both ends. */
static inline std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

/**
 * @details
 * Parses the configuration file line by line, extracting slot information,
 * optional per-module firmware and DSP settings files, and the (default)
 * per-crate DSP settings file. DSP settings files must have the file extension
 * .set for XIA API 2 and either .set or .json for XIA API 3. Exits on any
 * error processing the configuration file.
 */
void DAQ::DDAS::ConfigurationParser::parse(std::istream &input,
                                           DAQ::DDAS::Configuration &config) {
  int CrateNum;
  std::vector<unsigned short> PXISlotMap;
  int NumModules;

  char temp[FILENAME_STR_MAXLEN];
  std::string line;
  std::string DSPParFile;

  // Maps capture nosuch better than arrays...
  std::map<int, FirmwareMap> perModuleFirmware;
  std::map<int, std::string> perModuleSetfiles;

  // Extract the first value and skip the next FILENAME_STR_MAXLEN
  // characters from this line.
  input >> CrateNum;
  input.getline(temp, FILENAME_STR_MAXLEN);
  input >> NumModules;
  input.getline(temp, FILENAME_STR_MAXLEN);
  PXISlotMap.resize(NumModules);
  for (int i = 0; i < NumModules; i++) {
    auto slotInfo = parseSlotLine(input);
    PXISlotMap[i] = std::get<0>(slotInfo);
    std::string perModuleMap = std::get<1>(slotInfo);
    if (!perModuleMap.empty()) {
      std::ifstream fwMapStream(perModuleMap);
      FirmwareMap aMap;
      FirmwareVersionFileParser fwparser;
      fwparser.parse(fwMapStream, aMap);
      perModuleFirmware[i] = aMap;
      std::string perModuleSetfile = std::get<2>(slotInfo);
      if (!perModuleSetfile.empty()) {
        perModuleSetfiles[i] = perModuleSetfile;
      }
    }
  }
  input >> DSPParFile;
  input.getline(temp, FILENAME_STR_MAXLEN);

  // Check the file extension:

  size_t pos = DSPParFile.find_last_of('.');
  if (pos != std::string::npos) {
    std::string ext = DSPParFile.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (ext != "set" && ext != "json") {
      std::stringstream errmsg;
      errmsg << "The DSP settings file " << DSPParFile << " read from"
             << " cfgPixie16.txt must have the extension 'set' or"
             << " 'json' but read '" << ext << "'.";
      throw std::runtime_error(errmsg.str());
    }
  } else {
    std::stringstream errmsg;
    errmsg << "The DSP settings file " << DSPParFile << " read from"
           << " cfgPixie16.txt must have the extension 'set' or 'json'.";
    throw std::runtime_error(errmsg.str());
  }

  // After the settings file, only whitespace is allowed:

  /**
   * @todo (ASC 3/13/24): Whitespace-tolerant comments could be allowed
   * using this trim-and-inspect framework i.e. check that first character
   * of trimmed line is a comment character. For now, anything besides
   * whitespace will throw when parsing the configuration file.
   */
  while (getline(input, line)) {
    trim(line); // Modifies line
    if (!line.empty()) {
      std::string msg("Unable to parse line '");
      msg += line + "'";
      throw std::runtime_error(msg);
    }
  }

  config.setCrateId(CrateNum);
  config.setNumberOfModules(NumModules);
  config.setSlotMap(PXISlotMap);
  config.setSettingsFilePath(DSPParFile);

  // Set the per module firmware maps:
  for (auto const &p : perModuleFirmware) {
    config.setModuleFirmwareMap(p.first, p.second);
  }

  // Set the per module DSP Parameter maps:
  for (auto const &p : perModuleSetfiles) {
    config.setModuleSettingsFilePath(p.first, p.second);
  }
}

/**
 * @details
 * Slot lines consist of a mandatory slot number, and optional substitute
 * firmware mapping file and an optional settings file for that module.
 * Care must be taken since any populated field (other than the slot number)
 * might actually be a comment. Requirements:
 *  - Filenames cannot have spaces in their paths.
 *  - Files must be readable by the user.
 *  - Comments denoted by '#' must be spaced from the last file e.g.:
 @verbatim
 1 FirmwareMap.txt# This is an error but,
 2 FirmwareMap.txt  # This is ok,
 3 FirmwareMap.txt setfile.json # As is this.
 @endverbatim
 */
DAQ::DDAS::ConfigurationParser::SlotSpecification
DAQ::DDAS::ConfigurationParser::parseSlotLine(std::istream &input) {
  std::string line;
  std::getline(input, line);

  if (!input.good()) { // Maybe eof?
    std::string errmsg = "Unable to read a line from the input file "
                         "when parsing a slot line";
    throw std::runtime_error(errmsg);
  }
  std::stringstream lineStream(line);

  int slot;
  std::string firmwareMap;
  std::string setFile;

  // Do the slot separately so that we can indicate we can't parse:
  if (!(lineStream >> slot)) {
    std::string errmsg("Unable to parse a slot number from: ");
    errmsg += line;
    throw std::runtime_error(errmsg);
  }

  // Now the files:
  lineStream >> firmwareMap >> setFile;

  // Handle leading #'s which imply a comment.
  if (firmwareMap[0] == '#') { // Both are comments.
    firmwareMap.clear();
    setFile.clear();
  } else if (setFile[0] == '#') { // Setfile is a comment
    setFile.clear();
  }

  // Check readability of any files:
  if (firmwareMap != "") {
    if (access(firmwareMap.c_str(), R_OK)) {
      std::string msg("Unable to read firmware mapping file ");
      msg += firmwareMap;
      msg += " from ";
      msg += line;
      throw std::runtime_error(msg);
    }
    if (setFile != "") {
      if (access(setFile.c_str(), R_OK)) {
        std::string msg("Unable to read DSP Parameter file ");
        msg += setFile;
        msg += " from ";
        msg += line;
        throw std::runtime_error(msg);
      }
    }
  }

  // Ok everything is good so:

  return std::make_tuple(slot, firmwareMap, setFile);
}
