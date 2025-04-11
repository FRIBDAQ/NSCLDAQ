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
@brief Test (minimally) The C3820 class and command generator.
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "C3820.h"
#include "MVLCConfigParser.h"
#include <TCLInterpreter.h>
#include <tcl.h>
#include <string>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>

class SIS3820Tests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(SIS3820Tests);
    CPPUNIT_TEST(command_1);   // sis3820 command was made.
    CPPUNIT_TEST(read_1);      // read not in timestamp mode.
    CPPUNIT_TEST(read_2);      // Read in timestamp mode.
    CPPUNIT_TEST_SUITE_END();
protected:
    void command_1();
    void read_1();
    void read_2();
public:
    void setUp() {
        // Make a temp scipt file:

        char nameTemplate[100];
        strcpy(nameTemplate, "sis3820.tclXXXXXX");
        m_fd = mkstemp(nameTemplate);
        m_filename = nameTemplate;

        m_parser = new MVLCConfigParser(m_filename);
    }
    void tearDown() {
        delete m_parser;
        close(m_fd);
        unlink(m_filename.c_str());
    }
private:
    int m_fd;
    std::string m_filename;
    TCLConfigParser* m_parser;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SIS3820Tests);

void
SIS3820Tests::command_1() {

}

void
SIS3820Tests::read_1() {

}

void
SIS3820Tests::read_2() {

}