/*
 *   This software is Copyright by the Board of Trustees of Michigan
 *   State University (c) Copyright 2016.
 *
 *   You may use this software under the terms of the GNU public license
 *  (GPL).  The terms of this license are described at:
 *
 *    http://www.gnu.org/licenses/gpl.txt
 *
 *    Author:
 *          Aaron Chester
 *          FRIB
 *          Michigan State University
 *          East Lansing, MI 48824-1321
 */

#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"

#include "Configuration.h"
#include "HardwareRegistry.h"
#include "SystemBooter.h"
#include "config_pixie16api.h"

using namespace std;
namespace HR = ::DAQ::DDAS::HardwareRegistry;
using namespace ::DAQ::DDAS;

/** @brief Tests for the ModEvtFileParser class */
class SystemBooterTest : public CppUnit::TestFixture {
public:
  CPPUNIT_TEST_SUITE(SystemBooterTest);

  CPPUNIT_TEST(check_default_verbosity);
  CPPUNIT_TEST(check_default_mode);

  CPPUNIT_TEST(set_verbosity);
  CPPUNIT_TEST(set_boot_mode);

  CPPUNIT_TEST(offline_boot);

  CPPUNIT_TEST_SUITE_END();

  SystemBooter m_booter;

public:
  void setUp() {}
  void tearDown() {
    // Exit offline mode if we booted into it. If modNum = 99 >= number of
    // modules in the system, exit all of them. We play it safe:
    Pixie16ExitSystem(99);
  }

  /////////////////////////////////////////////////////////////////////////////

  /** @brief Checks default verbosity for the system booter. */
  void check_default_verbosity() {
    CPPUNIT_ASSERT_EQUAL(true, m_booter.isVerbose());
  }

  /** @brief Checks default mode for the system booter. */
  void check_default_mode() {
    CPPUNIT_ASSERT_EQUAL((unsigned short)0, m_booter.getOfflineMode());
  }

  /////////////////////////////////////////////////////////////////////////////

  /** @brief Tests setting verbosity for the system booter. */
  void set_verbosity() {
    m_booter.setVerbose(false);
    CPPUNIT_ASSERT_EQUAL(false, m_booter.isVerbose());
  }

  /** @brief Tests setting offline mode for the system booter. */
  void set_boot_mode() {
    m_booter.setOfflineMode(1);
    CPPUNIT_ASSERT_EQUAL((unsigned short)1, m_booter.getOfflineMode());
  }

  /////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Tests offline boot functionality and verifies the offline channel
   * map. Shouldn't throw.
   * @details
   * Since the offline boot loads some offline-mode (fake) firmware, we don't
   * set any of the normal configuration stuff outside of the channel map. The
   * SDK says that the first 3 modules are 16-channel boards and the 4th has 32
   * channels, so we'll verify that. Tests in the Configuration class provide
   * coverage for per-module firmware and settings maps, etc.
   */
  void offline_boot() {
    m_booter.setVerbose(true);
    m_booter.setOfflineMode(1);

    Configuration config;
    m_booter.boot(config, SystemBooter::FullBoot);

    auto channelMap = config.getChannelMap();
    CPPUNIT_ASSERT_EQUAL((size_t)4, channelMap.size());
    CPPUNIT_ASSERT_EQUAL((unsigned short)16, channelMap[0]);
    CPPUNIT_ASSERT_EQUAL((unsigned short)16, channelMap[1]);
    CPPUNIT_ASSERT_EQUAL((unsigned short)16, channelMap[2]);
    CPPUNIT_ASSERT_EQUAL((unsigned short)32, channelMap[3]);
  }
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION(SystemBooterTest);
