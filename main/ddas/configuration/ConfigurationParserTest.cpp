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

#include "Asserts.h"

#include "ConfigurationParser.h"
#include "Configuration.h"

#include <sstream>
#include <vector>
#include <string>

using namespace std;
using namespace ::DAQ::DDAS;

namespace HR = ::DAQ::DDAS::HardwareRegistry;

template<class T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec)
{
    stream << "(";
    for (auto& number : vec) {
        stream << number << " ";
    }
    stream << ")";

    return stream;
}

// A test suite
class ConfigurationParserTest : public CppUnit::TestFixture
{

public:
    CPPUNIT_TEST_SUITE(ConfigurationParserTest);
    CPPUNIT_TEST(parseCrateID);
    CPPUNIT_TEST(parseNumModules);
    CPPUNIT_TEST(parseSlotMap);
    CPPUNIT_TEST(parseSettingsPath);
    CPPUNIT_TEST(parseBadSlotMap);
    CPPUNIT_TEST_SUITE_END();

    vector<string> m_cfgFileContent;

public:
    void setUp() {
	m_cfgFileContent = createSampleFileContent();
    }
    void tearDown() {}
    
    vector<string> createSampleFileContent() {
        vector<string> linesOfFile;
        linesOfFile.push_back("0 # crate id");
        linesOfFile.push_back("3 # number of modules");
        linesOfFile.push_back("2 # slot for mod 0");
        linesOfFile.push_back("3");
        linesOfFile.push_back("4");
        linesOfFile.push_back("/path/to/my/settings/file.set");

        return linesOfFile;
    }
    
    string mergeLines(const vector<string>& content) {
	string mergedContent;
	for (auto& line : content) {
	    mergedContent += line + '\n';
	}
	
	return mergedContent;
    }
    
    string createSampleStream() {
	return mergeLines(createSampleFileContent());
    }

    void parseCrateID() {
	ConfigurationParser parser;
	std::stringstream stream(createSampleStream());
	Configuration config;
	parser.parse(stream, config);
	EQMSG("Crate id is parsed correctly", 0, config.getCrateId());
    }

    void parseNumModules() {
	ConfigurationParser parser;
	std::stringstream stream(createSampleStream());
	Configuration config;
	parser.parse(stream, config);
	EQMSG(
	    "Number of modules is parsed correctly",
	    size_t(3),
	    config.getNumberOfModules()
	    );
    }

    void parseSlotMap() {
        ConfigurationParser parser;
        std::stringstream stream(createSampleStream());
        Configuration config;
        parser.parse(stream, config);
        EQMSG(
	    "Slot mapping is parsed correctly",
	    vector<unsigned short>({2,3,4}),
	    config.getSlotMap()
	    );
    }

    void parseSettingsPath() {
	ConfigurationParser parser;
	std::stringstream stream(createSampleStream());
	Configuration config;
	parser.parse(stream, config);
	EQMSG(
	    "Path to set file is parsed correctly",
	    string("/path/to/my/settings/file.set"),
	    config.getSettingsFilePath()
	    );
    }

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
	} catch (std::exception& exc) {
	    threwException = true;
	    message = exc.what();
	}
	ASSERTMSG(
	    "Failure should occur if insufficient slot mapping data exists",
	    threwException
	    );
	std::string errmsg = "Unable to parse a slot number from: " \
	    "/path/to/my/settings/file.set";
	EQMSG("Error message should be informative", message, errmsg);
    }
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION(ConfigurationParserTest);

