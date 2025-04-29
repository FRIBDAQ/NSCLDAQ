/**
 *  @file sisvmusbTests.cpp
 *  @brief tests for sis_vmusb_interface.
 *  @author Ron Fox<rfoxkendo@gmail.com>
 * 
 *  
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
 */

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "CVMUSB.h"
#include "sis_vmusb_interface.h"    

#include <string>
#include <stdint.h>
class sisvmusbTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sisvmusbTests);
    CPPUNIT_TEST(mustopen_1);
    CPPUNIT_TEST(mustopen_2);
    CPPUNIT_TEST(open_1);
    CPPUNIT_TEST(close_1);

    CPPUNIT_TEST(write_1);
    CPPUNIT_TEST(write_2);
    CPPUNIT_TEST(write_3);
    CPPUNIT_TEST(write_4);
    CPPUNIT_TEST_SUITE_END();

protected:
    void mustopen_1();
    void mustopen_2();
    void open_1();
    void close_1();
    void write_1();
    void write_2();
    void write_3();
    void write_4();

    // I don't have setup/teardown requirements.
public:
    void setUp() {}
    void tearDown() {}
};

CPPUNIT_TEST_SUITE_REGISTRATION(sisvmusbTests);

void
sisvmusbTests::mustopen_1() {
    // If not open, get_vmeopen_messages fails.

    sis_vmusb_interface sis;
    unsigned int found; 
    char msg[100];
    int status = sis.get_vmeopen_messages(msg, &found);
    CPPUNIT_ASSERT_EQUAL(-1, status);
    CPPUNIT_ASSERT_EQUAL(unsigned(0), found);
}

void
sisvmusbTests::mustopen_2() {
    // Actual operations won't work if not open:

    sis_vmusb_interface sis;
    int status = sis.vme_A32D32_write(0, 0);
    CPPUNIT_ASSERT_EQUAL(-1, status);
}

void
sisvmusbTests::open_1() {
    // Open makes get_vmeopenMessages not return an error.

    sis_vmusb_interface sis;
    CVMUSB interface;
    int status = sis.vmeopen(&interface);
    CPPUNIT_ASSERT_EQUAL(0, status);

    unsigned found;
    char msg[100];
    status = sis.get_vmeopen_messages(msg, &found);
    CPPUNIT_ASSERT_EQUAL(0, status);
    CPPUNIT_ASSERT_EQUAL(unsigned(1), found);
}

void 
sisvmusbTests::close_1() {
    // Closing an open interface makes it unusable.

    sis_vmusb_interface sis;
    CVMUSB interface;
    CPPUNIT_ASSERT_EQUAL(0, sis.vmeopen(&interface));
    CPPUNIT_ASSERT_EQUAL(0, sis.vmeclose());

    unsigned found;
    char msg[100];
    CPPUNIT_ASSERT_EQUAL(-1, sis.get_vmeopen_messages(msg, &found));
    CPPUNIT_ASSERT_EQUAL(unsigned(0), found);

}
void
sisvmusbTests::write_1() {
    /* write should add the appropriate stuff to the interface's list: */

    CVMUSB interface;
    sis_vmusb_interface sis;
    sis.vmeopen(&interface);

    int status = sis.vme_A32D32_write(0x12340000, 0x4321);
    CPPUNIT_ASSERT_EQUAL(0, status);

    // Check the list:

    auto ops = interface.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(1), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340000 0x4321"),
        ops.at(0)
    );
}


void
sisvmusbTests::write_2() {
    CVMUSB interface;
    sis_vmusb_interface sis;
    sis.vmeopen(&interface);

    // A32DMA_D32write is just a loop:

    uint32_t data[] = {1, 2, 3, 4};
    unsigned written;
    int status = sis.vme_A32DMA_D32_write(0x12340000, data, 4, &written);

    CPPUNIT_ASSERT_EQUAL(0, status);
    CPPUNIT_ASSERT_EQUAL(unsigned(4), written);

    auto ops = interface.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(4), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340000 0x1"),
        ops.at(0)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340004 0x2"),
        ops.at(1)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340008 0x3"),
        ops.at(2)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x1234000c 0x4"),
        ops.at(3)
    );
    
}

void
sisvmusbTests::write_3() {
    // vme_A32BLT32_write is ub terms of vme_A32DMA_D32_write

    CVMUSB interface;
    sis_vmusb_interface sis;
    sis.vmeopen(&interface);

    uint32_t data[] = {1, 2, 3, 4};
    unsigned written;
    int status = sis.vme_A32BLT32_write(0x12340000, data, 4, &written);

    CPPUNIT_ASSERT_EQUAL(0, status);
    CPPUNIT_ASSERT_EQUAL(unsigned(4), written);

    auto ops = interface.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(4), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340000 0x1"),
        ops.at(0)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340004 0x2"),
        ops.at(1)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340008 0x3"),
        ops.at(2)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x1234000c 0x4"),
        ops.at(3)
    );
    
}

void
sisvmusbTests::write_4() {
    // Same as for vme_A32MBLT64_write

    CVMUSB interface;
    sis_vmusb_interface sis;
    sis.vmeopen(&interface);

    uint64_t data[] = {1, 2, 3, 4};
    unsigned written;
    int status = sis.vme_A32MBLT64_write(0x12340000, (uint*)data, 4, &written);

    CPPUNIT_ASSERT_EQUAL(0, status);
    CPPUNIT_ASSERT_EQUAL(unsigned(4), written);

    auto ops = interface.getRecordedOperations();
    CPPUNIT_ASSERT_EQUAL(size_t(8), ops.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340000 0x1"),
        ops.at(0)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340008 0x2"),
        ops.at(2)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340010 0x3"),
        ops.at(4)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340018 0x4"),
        ops.at(6)
    );

    // THe odd ops should be writing 0:


    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340004 0x0"),
        ops.at(1)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x1234000c 0x0"),
        ops.at(3)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340014 0x0"),
        ops.at(5)
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x1234001c 0x0"),
        ops.at(7)
    );
}