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
  CPPUNIT_TEST(parse_crate_id);
  CPPUNIT_TEST(parse_num_modules);
  CPPUNIT_TEST(parse_slot_map);
  CPPUNIT_TEST(parse_settings_path);
  CPPUNIT_TEST(parse_bad_slot_map);
  CPPUNIT_TEST(parse_bad_settings_file_no_ext);
  CPPUNIT_TEST(parse_bad_config_line);
  CPPUNIT_TEST_SUITE_END();

  vector<string> m_cfgFileContent;

public:
  /** @brief Create a sample configuration file. */
  void setUp() { m_cfgFileContent = create_sample_file_content(); }
  void tearDown() {}

  /** @brief Defines the contents of the sample configuration file. */
  vector<string> create_sample_file_content() {
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
  string merge_lines(const vector<string> &content) {
    string mergedContent;
    for (auto &line : content) {
      mergedContent += line + '\n';
    }

    return mergedContent;
  }

  /** @brief Create a sample input configuration file input stream. */
  string create_sample_stream() {
    return merge_lines(create_sample_file_content());
  }

  /** @brief Verify reading correct crate ID value. */
  void parse_crate_id() {
    ConfigurationParser parser;
    std::stringstream stream(create_sample_stream());
    Configuration config;
    parser.parse(stream, config);
    EQMSG("Crate id is parsed correctly", 0, config.getCrateId());
  }

  /** @brief Verify reading correct number of modules. */
  void parse_num_modules() {
    ConfigurationParser parser;
    std::stringstream stream(create_sample_stream());
    Configuration config;
    parser.parse(stream, config);
    EQMSG("Number of modules is parsed correctly", size_t(3),
          config.getNumberOfModules());
  }

  /** @brief Verify reading correct slot map. */
  void parse_slot_map() {
    ConfigurationParser parser;
    std::stringstream stream(create_sample_stream());
    Configuration config;
    parser.parse(stream, config);
    EQMSG("Slot mapping is parsed correctly", vector<unsigned short>({2, 3, 4}),
          config.getSlotMap());
  }

  /** @brief Verify reading correct settings file path. */
  void parse_settings_path() {
    ConfigurationParser parser;
    std::stringstream stream(create_sample_stream());
    Configuration config;
    parser.parse(stream, config);
    EQMSG("Path to set file is parsed correctly",
          string("/path/to/my/settings/file.json"),
          config.getSettingsFilePath());
  }

  /** @brief Bad slot map results in an error. */
  void parse_bad_slot_map() {
    ConfigurationParser parser;
    auto lines = create_sample_file_content();

    // Expect 4 modules but only provide slots for 3:
    lines.at(1) = "4";

    Configuration config;
    std::string message;
    bool threwException = false;
    stringstream stream(merge_lines(lines));
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
  void parse_bad_settings_file_no_ext() {
    ConfigurationParser parser;
    Configuration config;
    auto lines = create_sample_file_content();
    // Bad settings file name (no extension):
    lines.at(5) = "/path/to/settings/file";
    stringstream stream(merge_lines(lines));
    // It's a failure if this _does_not_ throw:
    CPPUNIT_ASSERT_THROW(parser.parse(stream, config), std::runtime_error);
  }

  /** @brief Non-whitespace line following the settings file is an error. */
  void parse_bad_config_line() {
    ConfigurationParser parser;
    auto lines = create_sample_file_content();

    // Replace a comment after the DSPParFile with a bad one:
    std::string badLine = "invalid line contains non-whitespace chars";
    lines.at(6) = badLine;

    Configuration config;
    std::string message;
    bool threwException = false;
    stringstream stream(merge_lines(lines));
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
