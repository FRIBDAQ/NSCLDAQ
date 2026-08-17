/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
         Aaron Chester
         Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/

#include <cppunit/extensions/HelperMacros.h>

#include <sstream>
#include <string>
#include <vector>

#include <DebugUtils.h>

#include "Asserts.h"
#include "Configuration.h"
#include "ConfigurationParser.h"

using namespace std;
using namespace ::DAQ::DDAS;
namespace HR = ::DAQ::DDAS::HardwareRegistry;

// A test suite
class ConfigurationParserTest : public CppUnit::TestFixture {
public:
  CPPUNIT_TEST_SUITE(ConfigurationParserTest);
  CPPUNIT_TEST(parseCrateID);
  CPPUNIT_TEST(parseNumModules);
  CPPUNIT_TEST(parseSlotMap);
  CPPUNIT_TEST(parseSettingsPath);
  CPPUNIT_TEST(parseBadSlotMap);
  CPPUNIT_TEST(parseBadSettingsFileNoExt);
  CPPUNIT_TEST(parseBadConfigLine);
  CPPUNIT_TEST_SUITE_END();

  vector<string> m_cfgFileContent;

public:
  /** @brief Create a sample configuration file. */
  void setUp() { m_cfgFileContent = createSampleFileContent(); }
  void tearDown() {}

  /** @brief Defines the contents of the sample configuration file. */
  vector<string> createSampleFileContent() {
    vector<string> linesOfFile;
    linesOfFile.push_back("0 # crate id");
    linesOfFile.push_back("3 # number of modules");
    linesOfFile.push_back("2 # slot for mod 0");
    linesOfFile.push_back("3");
    linesOfFile.push_back("4");
    linesOfFile.push_back("/path/to/my/settings/file.json");
    linesOfFile.push_back(""); // Whitespace is OK

    return linesOfFile;
  }

  /** @brief Merge the lines of the sample configuration file. */
  string mergeLines(const vector<string> &content) {
    string mergedContent;
    for (auto &line : content) {
      mergedContent += line + '\n';
    }

    return mergedContent;
  }

  /** @brief Create a sample input configuration file input stream. */
  string createSampleStream() { return mergeLines(createSampleFileContent()); }

  /** @brief Verify reading correct crate ID value. */
  void parseCrateID() {
    ConfigurationParser parser;
    std::stringstream stream(createSampleStream());
    Configuration config;
    parser.parse(stream, config);
    EQMSG("Crate id is parsed correctly", 0, config.getCrateId());
  }

  /** @brief Verify reading correct number of modules. */
  void parseNumModules() {
    ConfigurationParser parser;
    std::stringstream stream(createSampleStream());
    Configuration config;
    parser.parse(stream, config);
    EQMSG("Number of modules is parsed correctly", size_t(3),
          config.getNumberOfModules());
  }

  /** @brief Verify reading correct slot map. */
  void parseSlotMap() {
    ConfigurationParser parser;
    std::stringstream stream(createSampleStream());
    Configuration config;
    parser.parse(stream, config);
    EQMSG("Slot mapping is parsed correctly", vector<unsigned short>({2, 3, 4}),
          config.getSlotMap());
  }

  /** @brief Verify reading correct settings file path. */
  void parseSettingsPath() {
    ConfigurationParser parser;
    std::stringstream stream(createSampleStream());
    Configuration config;
    parser.parse(stream, config);
    EQMSG("Path to set file is parsed correctly",
          string("/path/to/my/settings/file.json"),
          config.getSettingsFilePath());
  }

  /** @brief Bad slot map results in an error. */
  void parseBadSlotMap() {
    ConfigurationParser parser;
    auto lines = createSampleFileContent();

    // Expect 4 modules but only provide slots for 3:
    lines.at(1) = "4";

    Configuration config;
    std::string message;
    bool threwException = false;
    stringstream stream(mergeLines(lines));
    try {
      parser.parse(stream, config);
    } catch (std::exception &exc) {
      threwException = true;
      message = exc.what();
    }
    ASSERTMSG("Failure should occur if insufficient slot mapping data exists",
              threwException);
    std::string errmsg = "Unable to parse a slot number from: "
                         "/path/to/my/settings/file.json";
    EQMSG("Error message should be informative", message, errmsg);
  }

  /** @brief Settings file with no extension results in an error. */
  void parseBadSettingsFileNoExt() {
    ConfigurationParser parser;
    Configuration config;
    auto lines = createSampleFileContent();
    // Bad settings file name (no extension):
    lines.at(5) = "/path/to/settings/file";
    stringstream stream(mergeLines(lines));
    // It's a failure if this _does_not_ throw:
    CPPUNIT_ASSERT_THROW(parser.parse(stream, config), std::runtime_error);
  }

  /** @brief Non-whitespace line following the settings file is an error. */
  void parseBadConfigLine() {
    ConfigurationParser parser;
    auto lines = createSampleFileContent();

    // Replace a comment after the DSPParFile with a bad one:
    std::string badLine = "invalid line contains non-whitespace chars";
    lines.at(6) = badLine;

    Configuration config;
    std::string message;
    bool threwException = false;
    stringstream stream(mergeLines(lines));
    try {
      parser.parse(stream, config);
    } catch (std::exception &exc) {
      threwException = true;
      message = exc.what();
    }
    ASSERTMSG("Failure should occur if an invalid config file line exists",
              threwException);
    std::string errmsg = "Unable to parse line '" + badLine + "'";
    EQMSG("Error message should be informative", message, errmsg);
  }
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION(ConfigurationParserTest);
