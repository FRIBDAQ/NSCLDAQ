/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321

    Minimal tests fo the sis3804 driver.
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include "CReadoutModule.h"
#include "C3804.h"

#include "MVLCConfigParser.h"
#include "CStack.h"
#include <TCLInterpreter.h>
#include <TCLException.h>
#include <tcl.h>
#include <string>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <string.h>

class sis3804tests : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(sis3804tests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(read_1);
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void create_1();
    void read_1();
public:
    void setUp() {
        // Make a temp script file:

        char nameTemplate[100];
        strcpy(nameTemplate, "sis3804.tclXXXXXX");
        m_fd = mkstemp(nameTemplate);
        m_filename = nameTemplate;

        m_parser = new MVLCConfigParser(m_filename);
        m_parser->initialize();                   // Register extensions.
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
CPPUNIT_TEST_SUITE_REGISTRATION(sis3804tests);

void sis3804tests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "sis3804", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}

// can make an object via script:

void sis3804tests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "sis3804 create scaler -base 0x12000000\n";
        script << "stack create scalers -trigger scaler -modules scaler\n"; 
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );                             // no errors on script.

    auto module = m_parser->findDevice("scaler");
    CPPUNIT_ASSERT(module);       // Module exists.
    C3804* dev = dynamic_cast<C3804*>(module->getDriver());
    CPPUNIT_ASSERT(dev);         // has the right driver.
}

// there will be 9 operations in read and the last one is a read of the last shadow
// register:

void sis3804tests::read_1() {
    {
        std::ofstream script(m_filename);
        script << "sis3804 create scaler -base 0x12000000\n";
        script << "stack create scalers -trigger scaler -modules scaler\n"; 
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );                             // no errors on script.

    CVMUSBReadoutList list;
    m_parser->getScalerStack()->addReadoutList(list);

    auto ops = list.dumpForMvlc();

    CPPUNIT_ASSERT_EQUAL(size_t(8), ops.size());
    auto lastRead = ops.back();
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_read_mem 0x9 d32 0x1200021c"),
        lastRead
    );
}
