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
@brief Test CVMUSBReadoutList class.
*/
#include "CVMUSBReadoutList.h"
#include "CVMUSB.h"
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include "CReadoutModule.h"
#define private public             // Wanna test wait micro.
#include "CV1x90.h"
#undef private

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

class C1x90Tests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(C1x90Tests);
    CPPUNIT_TEST(command_1);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(init_1);   // Default device type.
    CPPUNIT_TEST(init_2);   // -model v1190B
    CPPUNIT_TEST(init_3);   // -model v1290A
    CPPUNIT_TEST(init_4);   // -model v1290N
    CPPUNIT_TEST(init_5);   // Explicit V1190A
    CPPUNIT_TEST_SUITE_END();

protected:
    void command_1();
    void create_1();
    void init_1();
    void init_2();
    void init_3();
    void init_4();
    void init_5();
public:
    void setUp() {
        // Make a temp script file:

        char nameTemplate[100];
        strcpy(nameTemplate, "1x90.tclXXXXXX");
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
CPPUNIT_TEST_SUITE_REGISTRATION(C1x90Tests);

// tdc1x90 command is registered in the parser's interpreter.

void
C1x90Tests::command_1() {
    Tcl_Interp* pInterp = m_parser->getInterpreter()->getInterpreter();
    auto token = Tcl_FindCommand(pInterp, "tdc1x90", nullptr, TCL_GLOBAL_ONLY);
    CPPUNIT_ASSERT(token);
}


//  the command can create a module:

void
C1x90Tests::create_1() {
    {
        std::ofstream script(m_filename);
        script << "tdc1x90 create tdc -base 0x12340000\n";
        script << "stack create event -trigger nim1 -modules tdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CReadoutModule* pModule = m_parser->findDevice("tdc");
    CPPUNIT_ASSERT(pModule);
    CV1x90* pDev = dynamic_cast<CV1x90*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDev);

}
// Initialization is just testing the mnodel and suffix are gotten right .. the list is too involved.

void
C1x90Tests::init_1() {
    // Default is 1190 A:

    {
        std::ofstream script(m_filename);
        script << "tdc1x90 create tdc -base 0x12340000\n";
        script << "stack create event -trigger nim1 -modules tdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* pEvent = m_parser->getEventStack();
    CPPUNIT_ASSERT(pEvent);

    CVMUSB controller;
    pEvent->Initialize(controller);
    CReadoutModule* pModule = m_parser->findDevice("tdc");
    CPPUNIT_ASSERT(pModule);
    CV1x90* pDev = dynamic_cast<CV1x90*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDev);

    CPPUNIT_ASSERT_EQUAL(1190, pDev->m_Model);
    CPPUNIT_ASSERT_EQUAL('A', pDev->m_Suffix);
}

void
C1x90Tests::init_2() {
    // explicitly 1190 B:

    
    {
        std::ofstream script(m_filename);
        script << "tdc1x90 create tdc -base 0x12340000 -model v1190B\n";
        script << "stack create event -trigger nim1 -modules tdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* pEvent = m_parser->getEventStack();
    CPPUNIT_ASSERT(pEvent);

    CVMUSB controller;
    pEvent->Initialize(controller);
    CReadoutModule* pModule = m_parser->findDevice("tdc");
    CPPUNIT_ASSERT(pModule);
    CV1x90* pDev = dynamic_cast<CV1x90*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDev);

    CPPUNIT_ASSERT_EQUAL(1190, pDev->m_Model);
    CPPUNIT_ASSERT_EQUAL('B', pDev->m_Suffix);
}
void
C1x90Tests::init_3() {
    // -model 1290A
    {
        std::ofstream script(m_filename);
        script << "tdc1x90 create tdc -base 0x12340000 -model v1290A\n";
        script << "stack create event -trigger nim1 -modules tdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* pEvent = m_parser->getEventStack();
    CPPUNIT_ASSERT(pEvent);

    CVMUSB controller;
    pEvent->Initialize(controller);
    CReadoutModule* pModule = m_parser->findDevice("tdc");
    CPPUNIT_ASSERT(pModule);
    CV1x90* pDev = dynamic_cast<CV1x90*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDev);

    CPPUNIT_ASSERT_EQUAL(1290, pDev->m_Model);
    CPPUNIT_ASSERT_EQUAL('A', pDev->m_Suffix);
}

void
C1x90Tests::init_4() {
    // -model v1290N

    {
        std::ofstream script(m_filename);
        script << "tdc1x90 create tdc -base 0x12340000 -model v1290N\n";
        script << "stack create event -trigger nim1 -modules tdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* pEvent = m_parser->getEventStack();
    CPPUNIT_ASSERT(pEvent);

    CVMUSB controller;
    pEvent->Initialize(controller);
    CReadoutModule* pModule = m_parser->findDevice("tdc");
    CPPUNIT_ASSERT(pModule);
    CV1x90* pDev = dynamic_cast<CV1x90*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDev);

    CPPUNIT_ASSERT_EQUAL(1290, pDev->m_Model);
    CPPUNIT_ASSERT_EQUAL('N', pDev->m_Suffix);
}

void
C1x90Tests::init_5() {
    // -model v1190A explcitly.

    {
        std::ofstream script(m_filename);
        script << "tdc1x90 create tdc -base 0x12340000 -model v1190A\n";
        script << "stack create event -trigger nim1 -modules tdc\n";
    }
    CPPUNIT_ASSERT_NO_THROW(
        (*m_parser)()
    );

    CStack* pEvent = m_parser->getEventStack();
    CPPUNIT_ASSERT(pEvent);

    CVMUSB controller;
    pEvent->Initialize(controller);
    CReadoutModule* pModule = m_parser->findDevice("tdc");
    CPPUNIT_ASSERT(pModule);
    CV1x90* pDev = dynamic_cast<CV1x90*>(pModule->getDriver());
    CPPUNIT_ASSERT(pDev);

    CPPUNIT_ASSERT_EQUAL(1190, pDev->m_Model);
    CPPUNIT_ASSERT_EQUAL('A', pDev->m_Suffix);
}