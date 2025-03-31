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
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>


class ReadoutListTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ReadoutListTests);
    // Test for construction.

    CPPUNIT_TEST(construction_1);

    // Test for marker

    CPPUNIT_TEST(marker_1);

    // Test for amods:

    CPPUNIT_TEST(amods);

    // Tests for write


    CPPUNIT_TEST(write_1);     //d32
    CPPUNIT_TEST(write_2);     //d16y
    CPPUNIT_TEST_SUITE_END();

protected:
    void construction_1();
    void marker_1();
    void amods();
    void write_1();
    void write_2();

public:
    void setUp() {}
    void tearDown() {}

};

CPPUNIT_TEST_SUITE_REGISTRATION(ReadoutListTests);


/* construct shoudl give an empty list: -- assuming dumpForMvlc works  */

void
ReadoutListTests::construction_1() {
    CVMUSBReadoutList list;
    CPPUNIT_ASSERT_EQUAL(size_t(0), list.dumpForMvlc().size());
}

/* Marker shoul produce write_marker 0xvalue_in_hex */

void
ReadoutListTests::marker_1() {
    CVMUSBReadoutList list;
    list.addMarker(32);  // 0x20

    auto mvlclist = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(std::string("write_marker 0x20"), mvlclist.at(0));
}

/* Ensure the address modifier values are right using https://www.vita.com/page-1855176 for values. */

void 
ReadoutListTests::amods() {
    // @todo  THese asserts are backwards, the second parameter should be first.


    ///  Extended addressing (32 bit):

    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a32UserData, uint8_t(0x09));
    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a32UserProgram, uint8_t(0x0a));
    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a32UserBlock, uint8_t(0x0b));

    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a32PrivData, uint8_t(0x0d));
    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a32PrivProgram, uint8_t(0x0e));
    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a32PrivBlock, uint8_t(0x0f));

    // Short IO space (16  bit):

    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a16User, uint8_t(0x29));
    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a16Priv, uint8_t(0x2d));

    // Standard addresssing (24 bit)

    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a24UserData, uint8_t(0x39));
    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a24UserProgram, uint8_t(0x3a));
    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a24UserBlock, uint8_t(0x3b));

    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a24PrivData, uint8_t(0x3d));
    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a24PrivProgram, uint8_t(0x3e));
    CPPUNIT_ASSERT_EQUAL(CVMUSBReadoutList::a24PrivBlock, uint8_t(0x3f));

}

/* test the writes */
void
ReadoutListTests::write_1() {
    CVMUSBReadoutList list;
    list.addWrite32(0x12340000, CVMUSBReadoutList::a32UserData, 0x12345678);

    auto mvlclist = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x9 d32 0x12340000 0x12345678"),
        mvlclist[0]
    );
}
void 
ReadoutListTests::write_2() {
    CVMUSBReadoutList list;
    list.addWrite16(0x124300, CVMUSBReadoutList::a24UserData, 0x1234);

    auto mvlclist = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_write 0x39 d16 0x124300 0x1234"),
        mvlclist[0]
    );
    
}