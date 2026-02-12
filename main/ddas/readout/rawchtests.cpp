#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cstdint>
#include <cstring>

#include "Asserts.h"
#include "RawChannel.h"
#include "testcommon.h"

class rawchTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(rawchTest);
  CPPUNIT_TEST(construct_1);
  CPPUNIT_TEST(construct_2);
  CPPUNIT_TEST(construct_3);

  CPPUNIT_TEST(copyin_1);
  CPPUNIT_TEST(setdata_1);

  CPPUNIT_TEST(settime_1);
  CPPUNIT_TEST(settime_2);
  CPPUNIT_TEST(settime_3);

  CPPUNIT_TEST(setlength_1);
  CPPUNIT_TEST(setlength_2);

  CPPUNIT_TEST(setchan_1);

  CPPUNIT_TEST(validate_1);
  CPPUNIT_TEST(validate_2);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}

protected:
  void construct_1();
  void construct_2();
  void construct_3();

  void copyin_1();
  void setdata_1();

  void settime_1();
  void settime_2();
  void settime_3();

  void setlength_1();
  void setlength_2();

  void setchan_1();

  void validate_1();
  void validate_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(rawchTest);

/** @brief Default constructor zeroes a bunch of fields. */
void rawchTest::construct_1() {
  DDASReadout::RawChannel ch;
  EQ(uint32_t(0), ch.s_moduleType);
  EQ(double(0.0), ch.s_time);
  EQ(0, ch.s_chanid);
  EQ(false, ch.s_ownData);
  EQ(0, ch.s_ownDataSize);
  EQ(0, ch.s_channelLength);
  EQ((uint32_t *)(nullptr), ch.s_data);
}

/** @brief Construct with locally owned storage. */
void rawchTest::construct_2() {
  DDASReadout::RawChannel ch(100); // Local owned storage.
  EQ(uint32_t(0), ch.s_moduleType);
  EQ(double(0.0), ch.s_time);
  EQ(0, ch.s_chanid);
  EQ(true, ch.s_ownData);
  EQ(100, ch.s_ownDataSize);
  EQ(0, ch.s_channelLength);
  ASSERT(ch.s_data);
}

/** @brief Construct with user owned storage (support zero copy). */
void rawchTest::construct_3() {
  uint32_t data[100];
  DDASReadout::RawChannel ch(100, data); // Zero-copy storage.

  EQ(uint32_t(0), ch.s_moduleType);
  EQ(double(0.0), ch.s_time);
  EQ(0, ch.s_chanid);
  EQ(false, ch.s_ownData);
  EQ(100, ch.s_ownDataSize);
  EQ(100, ch.s_channelLength); // b/c it's already assumed a hit.
  EQ((uint32_t *)(data), ch.s_data);
}

/** @brief Copy in a hit. */
void rawchTest::copyin_1() {
  uint32_t data[4];
  makeHit(data, 1, 2, 3, 0x12345, 100);
  DDASReadout::RawChannel ch;
  ch.copyInData(4, data);

  EQ(true, ch.s_ownData);
  EQ(4, ch.s_ownDataSize);
  EQ(4, ch.s_channelLength);
  EQ(0, memcmp(data, ch.s_data, sizeof(data)));
}

/** @breif Zero-copy in a hit. */
void rawchTest::setdata_1() {
  uint32_t data[4];
  makeHit(data, 1, 2, 3, 0x12345, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  EQ(0, ch.s_ownDataSize); // I don't own any data!
  EQ(4, ch.s_channelLength);
  EQ((uint32_t *)(data), ch.s_data);
}

/** @brief Set the time without a clock calibration. */
void rawchTest::settime_1() {
  uint32_t data[4];
  makeHit(data, 1, 2, 3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetTime();
  EQ(double(12345678), ch.s_time);
}

/** @brief Set a time with clock calibration parameter. */
void rawchTest::settime_2() {
  uint32_t data[4];
  makeHit(data, 1, 2, 3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetTime(2.0);
  EQ(double(12345678 * 2), ch.s_time);
}

/** @brief Set time from an external timestamp and calibration parameter. */
void rawchTest::settime_3() {
  uint32_t data[6];
  makeHit(data, 1, 2, 3, 12345678, 100);
  data[4] = 0x54321;
  data[5] = 0x1234;

  // Fix up data[0] as well (yikes!). We made the hit with an event and
  // header size of 4 (see makeHit in testcommon.cpp) but now we have two
  // extra words so:

  data[0] = (6 << 17) | (6 << 12) | (1 << 8) | (2 << 4) | 3;

  DDASReadout::RawChannel ch;
  ch.setData(6, data);
  ch.SetTime(2.0, true);

  EQ(double(0x123400054321 * 2), ch.s_time);
}

/** @brief Can set the channel length. */
void rawchTest::setlength_1() {
  uint32_t data[4];
  makeHit(data, 1, 2, 3, 12345678, 100); // Length is 4.
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetLength();
  EQ(4, ch.s_channelLength);
}

/** Changing the eventlength field in the hit changes the channel length. */
void rawchTest::setlength_2() {
  uint32_t data[8];
  makeHit(data, 1, 2, 3, 12345678, 100);
  data[0] = (data[0] & 0x80010000) | (8 << 17); // Event length 4 -> 8
  DDASReadout::RawChannel ch;
  ch.setData(8, data);
  ch.SetLength();
  EQ(8, ch.s_channelLength);
}

/** @brief Extract the channel ID. */
void rawchTest::setchan_1() {
  uint32_t data[4];
  makeHit(data, 1, 2, 3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetChannel();
  EQ(3, ch.s_chanid);
}

/** @brief Check valid event length. */
void rawchTest::validate_1() {
  uint32_t data[4];
  makeHit(data, 1, 2, 3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(4, data);
  ch.SetLength();
  EQ(0, ch.Validate(4));
}

/** @brief Fail on invalid length. */
void rawchTest::validate_2() {
  uint32_t data[8];
  makeHit(data, 1, 2, 3, 12345678, 100);
  DDASReadout::RawChannel ch;
  ch.setData(8, data);
  ch.SetLength();
  EQ(1, ch.Validate(8)); // Size field says 4!
}
