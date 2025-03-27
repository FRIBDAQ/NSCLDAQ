/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Test TCLConfigParser class.
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#define private public
#include "TCLConfigParser.h"           // Open up internals to tests.
#undef private
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <memory>
#include <TCLException.h>
class TCLConfigTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TCLConfigTests);
    // construction tests
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    // Simple parese tests.
    CPPUNIT_TEST(parse_1);
    CPPUNIT_TEST(parse_2);
    CPPUNIT_TEST(parse_2);
    CPPUNIT_TEST_SUITE_END();
protected:
    void construct_1();
    void construct_2();

    void parse_1();
    void parse_2();
    void parse_3();
public: 
    void setUp() {
        // Make a temp scipt file:

        char nameTemplate[100];
        strcpy(nameTemplate, "configXXXXXX.tcl");
        m_fd = mkstemp(nameTemplate);
        m_filename = nameTemplate;

    }
    void tearDown() {
        // CLose and kill that file off.
        close(m_fd);
        unlink(m_filename.c_str());
    }

    int         m_fd;
    std::string m_filename;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TCLConfigTests);


//////////////////////// Construction tests /////////////////////////////////

/** Construction with file that exists works. */

void TCLConfigTests::construct_1() {
    TCLConfigParser parser(m_filename);
    CPPUNIT_ASSERT(parser.m_pInterp != nullptr);
    CPPUNIT_ASSERT_EQUAL(m_filename, parser.m_daqconfigFile);
    CPPUNIT_ASSERT_EQUAL(size_t(0), parser.m_commandExtensions.size());
    CPPUNIT_ASSERT(parser.m_pEventStack == nullptr);
    CPPUNIT_ASSERT(parser.m_pScalerStack == nullptr);
    CPPUNIT_ASSERT_EQUAL(size_t(0), parser.m_modules.size());
}

/* Construction does not check existence of the script. */

void TCLConfigTests::construct_2() {
    std::unique_ptr<TCLConfigParser> p;    // Automate destruction.
    std::string filename("dfjkdkdkfdjf");
    CPPUNIT_ASSERT_NO_THROW(
        p.reset(new TCLConfigParser(filename))
    );    // No such file.
}

////////////////////////// parse tests.. //////////////////////////////////////

/*  Processing a  valid Tcl file will work. */
void TCLConfigTests::parse_1() {
    // Make the file:

    {
        std::ofstream file(m_filename);
        file << "set a b\n";       // Valid tcl.
    }
    lseek(m_fd, SEEK_SET, 0);      // Rewind the file ...just in case.

    TCLConfigParser parser(m_filename);
    CPPUNIT_ASSERT_NO_THROW(
        parser()
    );
}

// process invalid tcl makes a CTCLException.

void TCLConfigTests::parse_2() {
    {
        std::ofstream file(m_filename);
        file << "et a b\n";       // no such command.
    }
    lseek(m_fd, SEEK_SET, 0);      // Rewind the file ...just in case.
    TCLConfigParser parser(m_filename);
    CPPUNIT_ASSERT_THROW(
        parser(),
        CTCLException
    );

}

// bad filename also throws:

void TCLConfigTests::parse_3() {
    TCLConfigParser parser(std::string("junk no such fiel"));
    CPPUNIT_ASSERT_THROW(
        parser(),
        CTCLException
    );
}