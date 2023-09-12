/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  NewParseTests.cpp
 *  @brief: Test new cfgPixie16.txt features in daqdev/DDAS#106
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <tuple>

#define private public
#include "ConfigurationParser.h"
#include "Configuration.h"
#undef private

static const char* FirmwareFileTemplate="FirmwareXXXXXX";
static const char* DSPParameterFileTemplate="DSPParametersXXXXXX";
static const char* fwOverrideFileContents =  "\
[Rev0xD-12Bit-100MSPS]                    \n\
a                            \n\
b                            \n\
c                           \n\
d                           \n\
2.1 \n\
[Rev0xF-16Bit-250MSPS] \n\
e   \n\
f   \n\
g \n\
h \n\
23.1 \n\
[Rev0xF-14Bit-500MSPS] \n\
i \n\
j  \n\
k  \n\
l   \n\
232.1  \n\
\n\
[Rev24-145Bit-23234MSPS]   \n\
 m  \n\
     n  \n\
o   \n\
p   \n\
2323.1";

/**
 * Create the firmware override file and return its filename.
 */
static std::string makeFirmwareFile()
{
    char filename[100];
    strncpy(filename, FirmwareFileTemplate, sizeof(filename));
    int fd = mkstemp(filename);
    
    if (fd < 0) {
        CPPUNIT_FAIL("Failed to make a temp file.");
    }
    ssize_t n = write(fd, fwOverrideFileContents, strlen(fwOverrideFileContents));
    if (n < strlen(fwOverrideFileContents)) {
        CPPUNIT_FAIL("Failed naive write of firmware file contents.");
        close(fd);
    }
    close(fd);
    
    return filename;
}
/**
 * Like makeFirmwareFile but make an empty .set file.
 */
static std::string makeDSPParametersFile ()
{
    char filename[1000];
    strncpy(filename, DSPParameterFileTemplate, sizeof(filename));
    int fd = mkstemp(filename);
    close (fd);                     // Zero length file is fine.
    
    return filename;
}

class NewParseTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NewParseTest);
    
    // Tests of the ability to parse extended slot lines.
    
    CPPUNIT_TEST(good_1);
    CPPUNIT_TEST(good_2);
    CPPUNIT_TEST(good_3);
    CPPUNIT_TEST(good_4);
    CPPUNIT_TEST(good_5);
    CPPUNIT_TEST(good_6);
    
    CPPUNIT_TEST(bad_1);
    CPPUNIT_TEST(bad_2);
    
    // Tests of extended capabilities of ConfigurationParser::Parse
    // To provide per module firmware maps and DSPparameter files
    
    CPPUNIT_TEST(permodule_1);
    CPPUNIT_TEST(permodule_2);
    CPPUNIT_TEST(permodule_3);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_FwFile;
    std::string m_setfile;
    DAQ::DDAS::ConfigurationParser* m_parser;
public:
    void setUp() {
        m_FwFile = makeFirmwareFile();
        m_setfile = makeDSPParametersFile();
        m_parser = new DAQ::DDAS::ConfigurationParser;
    }
    void tearDown() {
        unlink(m_FwFile.c_str());
        unlink(m_setfile.c_str());
        delete m_parser;
    }
protected:
    void good_1();
    void good_2();
    void good_3();
    void good_4();
    void good_5();
    void good_6();
    
    void bad_1();
    void bad_2();
    
    void permodule_1();
    void permodule_2();
    void permodule_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(NewParseTest);

// Success with a line that's only the slot.

void NewParseTest::good_1()
{
    std::stringstream line("2\n");
    auto result = m_parser->parseSlotLine(line);
    EQ(2, std::get<0>(result));
    EQ(std::string(""), std::get<1>(result));
    EQ(std::string(""), std::get<2>(result));
}

// Slot + comment

void NewParseTest::good_2()
{
    std::stringstream line("2 # this is a comment.\n");
    auto result = m_parser->parseSlotLine(line);
    EQ(2, std::get<0>(result));
    EQ(std::string(""), std::get<1>(result));
    EQ(std::string(""), std::get<2>(result));
}

// slot + firmware map file.

void NewParseTest::good_3()
{
    std::string l("2 ");
    l += m_FwFile;
    l += "\n";
    std::stringstream line(l);
    
    auto result = m_parser->parseSlotLine(line);
    EQ(2, std::get<0>(result));
    EQ(m_FwFile, std::get<1>(result));
    EQ(std::string(""), std::get<2>(result));
}
void NewParseTest::good_4()
{
    std::string l("2 ");
    l += m_FwFile;
    l += " # some comment\n";
    std::stringstream line(l);
    
    auto result = m_parser->parseSlotLine(line);
    EQ(2, std::get<0>(result));
    EQ(m_FwFile, std::get<1>(result));
    EQ(std::string(""), std::get<2>(result));
}

void NewParseTest::good_5()
{
    std::string l("2 ");
    l += m_FwFile;
    l +=  "  ";
    l += m_setfile;
    l += "\n";
    
    std::stringstream line(l);
    
    auto result = m_parser->parseSlotLine(line);
    EQ(2, std::get<0>(result));
    EQ(m_FwFile, std::get<1>(result));
    EQ(m_setfile, std::get<2>(result));
}

void NewParseTest::good_6()
{
    std::string l("2 ");
    l += m_FwFile;
    l +=  "  ";
    l += m_setfile;
    l += " # This is a comment.\n";
    
    std::stringstream line(l);
    
    auto result = m_parser->parseSlotLine(line);
    EQ(2, std::get<0>(result));
    EQ(m_FwFile, std::get<1>(result));
    EQ(m_setfile, std::get<2>(result));
}


// Bad parameters file.

void NewParseTest::bad_1()
{
    std::string l("2 ");
    l += m_FwFile;
    l +=  "  ";
    l += "/this/file/does/not/exist";
    l += " # This is a comment.\n";
    
    std::stringstream  line(l);
    
    CPPUNIT_ASSERT_THROW(
        m_parser->parseSlotLine(line),
        std::runtime_error
	);
    
    
}
/**
 * Bad firmware override file
 */
void NewParseTest::bad_2()
{
    std::string l("2 ");
    l += "/some/file/that/does/not/exist/";
    l +=  "  ";
    l += m_setfile;
    l += " # This is a comment.\n";
    
    std::stringstream  line(l);
    
    CPPUNIT_ASSERT_THROW(
        m_parser->parseSlotLine(line),
        std::runtime_error
	);
}
//////////////////////////////////////////////////////////////////////////////

/**
 *   String file for cfgpixie with no overrides.
 */
void NewParseTest::permodule_1()
{
    std::string cfgPixie = "\
1\n\
3\n\
2\n\
3\n\
4\n\
";
    cfgPixie += m_setfile;        // Add the set file.
    cfgPixie +=  "\n";
    DAQ::DDAS::Configuration config;          // empty.
    std::stringstream configStream(cfgPixie);
    m_parser->parse(configStream, config);
    
    // The per module maps of the resulting configuration should be empty:
    
    ASSERT(config.m_moduleFirmwareMaps.empty());
    ASSERT(config.m_moduleSetFileMap.empty());
}
/**
 * String file where module 3 has a firmware override:
 */
void NewParseTest::permodule_2()
{
    std::string cfgPixie = "\
1\n\
3\n\
2\n\
3 ";
    cfgPixie += m_FwFile;
    cfgPixie += "\n\
4\n\
";
    cfgPixie += m_setfile;        // Add the set file.
    cfgPixie +=  "\n";
    DAQ::DDAS::Configuration config;          // empty.
    std::stringstream configStream(cfgPixie);
    m_parser->parse(configStream, config);
    
    ASSERT(!config.m_moduleFirmwareMaps.empty());
    ASSERT(
        config.m_moduleFirmwareMaps.find(1) !=
        config.m_moduleFirmwareMaps.end()
	);
}
// Can add a per module .set file:

void NewParseTest::permodule_3()
{
    std::string cfgPixie = "\
1\n\
3\n\
2\n\
3 ";
    cfgPixie += m_FwFile;
    cfgPixie += " ";
    cfgPixie += m_setfile;
    cfgPixie += "\n\
4\n\
";
    cfgPixie += m_setfile;        // Add the set file.
    cfgPixie +=  "\n";
    DAQ::DDAS::Configuration config;          // empty.
    std::stringstream configStream(cfgPixie);
    m_parser->parse(configStream, config);
    
    ASSERT(!config.m_moduleFirmwareMaps.empty());
    ASSERT(
        config.m_moduleFirmwareMaps.find(1) !=
        config.m_moduleFirmwareMaps.end()
	);
    
    ASSERT(!config.m_moduleSetFileMap.empty());
    ASSERT(config.m_moduleSetFileMap.find(1) != config.m_moduleSetFileMap.end());
}
