/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
         Aaron Chester
         Jeromy Tompkins
         Facility for Rare Isotope Beams
         Michigan State University
         East Lansing, MI 48824-1321
*/

#include <cppunit/extensions/HelperMacros.h>

#include <sstream>
#include <vector>
#include <string>

#include "Asserts.h"
#include "ModEvtFileParser.h"
#include "Configuration.h"

using namespace std;
using namespace ::DAQ::DDAS;

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

class ModEvtFileParserTest : public CppUnit::TestFixture
{

public:
    CPPUNIT_TEST_SUITE( ModEvtFileParserTest );
    CPPUNIT_TEST( parse_0 );
    CPPUNIT_TEST( parse_1 );
    CPPUNIT_TEST( parse_2 );
    CPPUNIT_TEST( parse_3 );
    CPPUNIT_TEST( parse_4 );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() {}

    /** @brief Create a sample settings file. */
    vector<string> createSampleFileContent()
	{
	    vector<string> linesOfFile;
	    linesOfFile.push_back("10");
	    linesOfFile.push_back("123");
	    linesOfFile.push_back("341");

	    return linesOfFile;
	}

    /** @brief Merge lines of settings file into a single string. */
    string mergeLines(const vector<string>& content)
	{
	    string mergedContent;
	    for (auto& line : content) {
		mergedContent += line + '\n';
	    }
	
	    return mergedContent;
	}

    /** @brief Create the stream from the sample settings file. */
    string createSampleStream()
	{
	    return mergeLines(createSampleFileContent());
	}

    /** @brief Check modevtfile length error if insufficient data. */
    void parse_0()
	{
	    ModEvtFileParser parser;
	    std::stringstream stream(createSampleStream());
	    Configuration config;
	    config.setNumberOfModules(5);
	    CPPUNIT_ASSERT_THROW_MESSAGE(
		"Insufficient data in modevtlen.txt produces error",
		parser.parse(stream, config), std::runtime_error
		);
	}

    /** @brief modevtlen file with enough data gives no error. */
    void parse_1()
	{
	    ModEvtFileParser parser;
	    std::stringstream stream(createSampleStream());
	    Configuration config;
	    config.setNumberOfModules(3);
	    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
		"modevtlen.txt with sufficient data produces no error",
		parser.parse(stream, config)
		);
	}
    
    /** @brief modevtlen file with more than enough data gives no error. */    
    void parse_2()
	{
	    ModEvtFileParser parser;
	    std::stringstream stream(createSampleStream());
	    Configuration config;
	    config.setNumberOfModules(2);
	    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
		"modevtlen.txt with more data than necessary produces no error",
		parser.parse(stream, config));
	}

    /** @brief Verify that event lengths are stored in the configuration. */
    void parse_3()
	{
	    ModEvtFileParser parser;
	    std::stringstream stream(createSampleStream());
	    Configuration config;
	    config.setNumberOfModules(2);
	    parser.parse(stream, config);
	    auto modEvtLen = config.getModuleEventLengths();
	    EQMSG(
		"After parsing successfully, module evt data should be stored",
		size_t(2), modEvtLen.size());
	    EQMSG(
		"module evt length should be stored for module #0",
		10, modEvtLen.at(0)
		);
	    EQMSG(
		"module evt length should be stored for module #1",
		123, modEvtLen.at(1)
		);
	}

    /** 
     * @brief Throw an error if the modevtlen is < 4 (default Pixie list-mode 
     * event size in 32-bit words). 
     */
    void parse_4()
	{
	    ModEvtFileParser parser;
	    std::string content("3"); // content of file is a single number
	    std::stringstream stream(content);
	    Configuration config;
	    config.setNumberOfModules(1);
	    bool threwException = false;
	    std::string message;
	    try {
		parser.parse(stream, config);
	    } catch (std::runtime_error& exc) {
		threwException = true;
		message = exc.what();
	    }
	    EQMSG(
		"Module event length less than 4 is an error",
		true, threwException
		);
	    std::string errmsg = "Failure while reading module event length ";
	    errmsg += "configuration file. Found event length "
		+ content + " less than 4.";
	    EQMSG("ModEvtLen less than 4 error message ", errmsg, message);
	}
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION(ModEvtFileParserTest);

