/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Test the CMarker and CMarkerCommand objects.
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "TCLConfigParser.h"
#include "CMarker.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "CReadoutModule.h"
#include <string>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <TCLException.h>

// Test helper classes - in a namespace.

namespace markertests {
    class MyParser : public TCLConfigParser {  // don't forget to  init.!!!
    public:
        MyParser(const std::string& infile) :
            TCLConfigParser(infile) {}
    protected:
        void addExtensions() {
            addExtension(new CMarkerCommand(*getInterpreter(), *this));
        }
    };
}

class MarkerTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(MarkerTests);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(init_1);
    CPPUNIT_TEST(list_1);
    CPPUNIT_TEST(end_1);
    CPPUNIT_TEST_SUITE_END();

protected:
    void create_1();
    void init_1();
    void list_1();
    void end_1();
public:
    void setUp() {
        // Make a temp scipt file:

        char nameTemplate[100];
        strcpy(nameTemplate, "config.tclXXXXXX");
        m_fd = mkstemp(nameTemplate);
        m_filename = nameTemplate;

        m_parser = new markertests::MyParser(m_filename);
        m_parser->initialize();           // Define the commands.
    }
    void tearDown() {
        delete m_parser;
        close(m_fd);
        unlink(m_filename.c_str());
    }
private:
    int m_fd;                  // test script file.
    std::string m_filename;

    markertests::MyParser* m_parser;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MarkerTests);

////////////// Test the command creates what we need:

void
MarkerTests::create_1() {
    // The script:

    {
        std::ofstream script(m_filename);
        script << "marker create junk -value 0x12345\n";
    }
    (*m_parser)();

    auto device = m_parser->findDevice("junk");
    CPPUNIT_ASSERT(device);
    auto driver = device->getDriver();
    CPPUNIT_ASSERT(driver);     // there's a driver
    CMarker* pMarker = dynamic_cast<CMarker*>(driver);
    CPPUNIT_ASSERT(pMarker);    // and it's usable as a marker.
    CPPUNIT_ASSERT(device->getConfiguration());   // Should be a config.

}
// Test nothing gets done on initialization:

void
MarkerTests::init_1() {
    {
        std::ofstream script(m_filename);
        script << "marker create junk -value 0x12345\n";
    }
    (*m_parser)();

    auto device = m_parser->findDevice("junk");
    CVMUSB controller;
    device->Initialize(controller);   // Calls the dev's init.

    auto ops = controller.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(0), ops.size());

}
// The addReadoutList will add a write_marker command. 
void
MarkerTests::list_1() {
    {
        std::ofstream script(m_filename);
        script << "marker create junk -value 0x12345\n";
    }
    (*m_parser)();

    auto device = m_parser->findDevice("junk");
    CVMUSBReadoutList list;
    device->addReadoutList(list);

    auto ops = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("write_marker 0x12345"),
        ops.at(0)
    );
}
// Nothing is written on the end:

void
MarkerTests::end_1() {
    {
        std::ofstream script(m_filename);
        script << "marker create junk -value 0x12345\n";
    }
    (*m_parser)();

    auto device = m_parser->findDevice("junk");
    CVMUSB controller;
    device->onEndRun(controller);

    auto ops = controller.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(0), ops.size());
}