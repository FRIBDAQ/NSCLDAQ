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
    CPPUNIT_TEST_SUITE_END();

protected:
    void construction_1();
    void marker_1();
    void amods();

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

}
