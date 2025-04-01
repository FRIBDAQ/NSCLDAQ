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
    CPPUNIT_TEST(write_2);     //d16

    // Tests for reads:

    CPPUNIT_TEST(read_1);   //d32
    CPPUNIT_TEST(read_2);   //d16

    // Block/fifo reads:

    CPPUNIT_TEST(blockread_1);
    CPPUNIT_TEST(blockread_2);

    // Block count for reads with count from a module field.

    CPPUNIT_TEST(blockcount_1);    // 16 bit.
    CPPUNIT_TEST(blockcount_2);    // 32 bit.
    CPPUNIT_TEST(blockcount_3);    // BLockcount read.
    CPPUNIT_TEST(blockcount_4);     // Fifo block count read.

    // software_delay test

    CPPUNIT_TEST(delay_1);

    CPPUNIT_TEST_SUITE_END();

protected:
    void construction_1();
    void marker_1();
    void amods();
    void write_1();
    void write_2();
    void read_1();
    void read_2();
    void blockread_1();
    void blockread_2();
    void blockcount_1();
    void blockcount_2();
    void blockcount_3();
    void blockcount_4();
    void delay_1();

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
        mvlclist.at(0)
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
        mvlclist.at(0)
    );
    
}

/* test the reads*/

void
ReadoutListTests::read_1() {
    CVMUSBReadoutList list;
    list.addRead32(0x123400, CVMUSBReadoutList::a24PrivData);  // amod 0xd

    auto mvlclist = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_read_mem 0x3d d32 0x123400"),
        mvlclist.at(0)
    );
}

void 
ReadoutListTests::read_2() {
    CVMUSBReadoutList list;
    list.addRead16(0x11223344, CVMUSBReadoutList::a32PrivData);

    auto mvlclist = list.dumpForMvlc();

    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_read_mem 0xd d16 0x11223344"),
        mvlclist.at(0)
    );
}
/*  BLock /FIFO reads */
void
ReadoutListTests::blockread_1() {
    CVMUSBReadoutList list;
    list.addBlockRead32(0x66660000, CVMUSBReadoutList::a32UserBlock, 100);

    auto mvlclist = list.dumpForMvlc();

    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read_mem 0xb 100 0x66660000"),
        mvlclist.at(0)
    );
}
void
ReadoutListTests::blockread_2() {
    CVMUSBReadoutList list;
    list.addFifoRead32(0x55550000, CVMUSBReadoutList::a32PrivBlock, 100);

    auto mvlclist = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_block_read 0xf 100 0x55550000"),
        mvlclist.at(0)
    );
}

// Read the block count - these makes 2 stack lines.

void
ReadoutListTests::blockcount_1() {   // 16 bit read of the count.
    CVMUSBReadoutList list;
    list.addBlockCountRead16(0x11110000, 0xff00, CVMUSBReadoutList::a32UserData);
    auto mvlclist = list.dumpForMvlc();

    CPPUNIT_ASSERT_EQUAL(size_t(2), mvlclist.size());
    auto read_op = mvlclist.at(0);
    auto mask_op = mvlclist.at(1);

    CPPUNIT_ASSERT_EQUAL(
        std::string("read_to_accu 0x9 d16 0x11110000"),
        read_op
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("mask_shift_accu 0xff00 24"),
        mask_op
    );
}

void
ReadoutListTests::blockcount_2() {     // 32 bit read of the count:

    CVMUSBReadoutList list;
    list.addBlockCountRead32(0x11110000, 0xff00, CVMUSBReadoutList::a32UserData);
    auto mvlclist = list.dumpForMvlc();

    CPPUNIT_ASSERT_EQUAL(size_t(2), mvlclist.size());
    auto read_op = mvlclist.at(0);
    auto mask_op = mvlclist.at(1);

    CPPUNIT_ASSERT_EQUAL(
        std::string("read_to_accu 0x9 d32 0x11110000"),
        read_op
    );
    CPPUNIT_ASSERT_EQUAL(
        std::string("mask_shift_accu 0xff00 24"),
        mask_op
    );

}
// addMaskedCountBlockRead32 generates the same thing as a 
// read32.

void
ReadoutListTests::blockcount_3() {
    // Note we don't bother to issue the addBlockCountRead since we're not going
    // to actually do the read.

    CVMUSBReadoutList list;
    list.addMaskedCountBlockRead32(0x12430000, CVMUSBReadoutList::a32PrivBlock);

    auto mvlclist = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_read_mem 0xf d32 0x12430000"),
        mvlclist.at(0)
    );
}
// Now for a fifo... same result but a vme_read not a vme_read_mem

void 
ReadoutListTests::blockcount_4() {
    CVMUSBReadoutList list;
    list.addMaskedCountFifoRead32(0x12430000, CVMUSBReadoutList::a32PrivBlock);
    
    auto mvlclist = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("vme_read 0xf d32 0x12430000"),
        mvlclist.at(0)
    );
}
// Stack delay operations 

void
ReadoutListTests::delay_1() {
    CVMUSBReadoutList list;
    list.addDelay(12);        // ms _sigh_.

    auto mvlclist = list.dumpForMvlc();
    CPPUNIT_ASSERT_EQUAL(size_t(1), mvlclist.size());
    CPPUNIT_ASSERT_EQUAL(
        std::string("software_delay 12"),
        mvlclist.at(0)
    );
}