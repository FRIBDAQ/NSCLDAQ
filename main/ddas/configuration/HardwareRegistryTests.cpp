/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
         Aaron Chester
         Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/

#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"
#include "HardwareRegistry.h"

using namespace std;
namespace HR = ::DAQ::DDAS::HardwareRegistry;

class HardwareRegistryTest : public CppUnit::TestFixture {
public:
  CPPUNIT_TEST_SUITE(HardwareRegistryTest);
  CPPUNIT_TEST(resetToDefaults_0);
  CPPUNIT_TEST(getSpecification_revb_100m_12b);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  /** @brief Reset the hardware registry. */
  void tearDown() { HR::resetToDefaults(); }

  /**
   * @brief Check that resetting the map to default values works.
   * @note This test should be kept as the very first test of them all
   * because if it does not pass, then all remaining tests are subject
   * to being dependent on the previous test.
   */
  void resetToDefaults_0() {
    HR::configureHardwareType(HR::RevB_100MHz_12Bit, {430, 23});
    HR::resetToDefaults();
    getSpecification_revb_100m_12b();
  }

  /** @brief Get 100 MSPS 12 bit Rev. B specification. */
  void getSpecification_revb_100m_12b() {
    auto spec = HR::getSpecification(HR::RevB_100MHz_12Bit);
    EQMSG("revb default rev", 11, spec.s_hdwrRevision);
    EQMSG("revb default adc freq", 100, spec.s_adcFrequency);
    EQMSG("revb default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 100 MSPS 12 bit Rev. C specification. */
  void getSpecification_0b() {
    auto spec = HR::getSpecification(HR::RevC_100MHz_12Bit);
    EQMSG("revc default rev", 12, spec.s_hdwrRevision);
    EQMSG("revc default adc freq", 100, spec.s_adcFrequency);
    EQMSG("revc default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 100 MSPS 12 bit Rev. D specification. */
  void getSpecification_0c() {
    auto spec = HR::getSpecification(HR::RevD_100MHz_12Bit);
    EQMSG("revd default rev", 13, spec.s_hdwrRevision);
    EQMSG("revd default adc freq", 100, spec.s_adcFrequency);
    EQMSG("revd default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 250 MSPS 12 bit Rev. F specification. */
  void getSpecification_3() {
    auto spec = HR::getSpecification(HR::RevF_250MHz_12Bit);
    EQMSG("revf_250_12bit default adc freq", 250, spec.s_adcFrequency);
    EQMSG("revf_250_12bit default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 250 MSPS 14 bit Rev. F specification. */
  void getSpecification_4() {
    auto spec = HR::getSpecification(HR::RevF_250MHz_14Bit);
    EQMSG("revf_250_14bit default adc freq", 250, spec.s_adcFrequency);
    EQMSG("revf_250_14bit default adc resolution", 14, spec.s_adcResolution);
  }

  /** @brief Get 250 MSPS 16 bit Rev. F specification. */
  void getSpecification_5() {
    auto spec = HR::getSpecification(HR::RevF_250MHz_16Bit);
    EQMSG("revf_250_16bit default adc freq", 250, spec.s_adcFrequency);
    EQMSG("revf_250_16bit default adc resolution", 16, spec.s_adcResolution);
  }

  /** @brief Get 500 MSPS 12 bit Rev. F specification. */
  void getSpecification_6() {
    auto spec = HR::getSpecification(HR::RevF_500MHz_12Bit);
    EQMSG("revf_500_12bit default adc freq", 500, spec.s_adcFrequency);
    EQMSG("revf_500_12bit default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 500 MSPS 14 bit Rev. F specification. */
  void getSpecification_7() {
    auto spec = HR::getSpecification(HR::RevF_500MHz_14Bit);
    EQMSG("revf_500_14bit default adc freq", 500, spec.s_adcFrequency);
    EQMSG("revf_500_14bit default adc resolution", 14, spec.s_adcResolution);
  }

  /** @brief Override and check registry configuration info. */
  void configureHardwareType_0() {
    HR::configureHardwareType(HR::RevB_100MHz_12Bit, {430, 23, 2});

    auto spec = HR::getSpecification(HR::RevB_100MHz_12Bit);
    EQMSG("after configure, adc freq", 430, spec.s_adcFrequency);
    EQMSG("after configure, adc resolution", 23, spec.s_adcResolution);
    EQMSG("after configure, hdwr revision", 2, spec.s_hdwrRevision);
  }

  /** @brief Compute 100 MSPS 12 bit Rev. D type. */
  void computeHardwareType_0() {
    EQMSG("Compute RevD", int(HR::RevD_100MHz_12Bit),
          HR::computeHardwareType(13, 100, 12));
  }

  /** @brief Compute 250 MSPS 12 bit Rev. F type. */
  void computeHardwareType_3() {
    EQMSG("Compute RevF_250MHz_12Bit", int(HR::RevF_250MHz_12Bit),
          HR::computeHardwareType(15, 250, 12));
  }

  /** @brief Compute 250 MSPS 14 bit Rev. F type. */
  void computeHardwareType_4() {
    EQMSG("Compute RevF_250MHz_14Bit", int(HR::RevF_250MHz_14Bit),
          HR::computeHardwareType(15, 250, 14));
  }

  /** @brief Compute 250 MSPS 16 bit Rev. F type. */
  void computeHardwareType_5() {
    EQMSG("Compute RevF_250MHz_16Bit", int(HR::RevF_250MHz_16Bit),
          HR::computeHardwareType(15, 250, 16));
  }

  /** @brief Compute 500 MSPS 12 bit Rev. F type. */
  void computeHardwareType_6() {
    EQMSG("Compute RevF_500MHz_12Bit", int(HR::RevF_500MHz_12Bit),
          HR::computeHardwareType(15, 500, 12));
  }

  /** @brief Compute 500 MSPS 14 bit Rev. F type. */
  void computeHardwareType_7() {
    EQMSG("Compute RevF_500MHz_14Bit", int(HR::RevF_500MHz_14Bit),
          HR::computeHardwareType(15, 500, 14));
  }

  /** @brief Compute unknown type. */
  void computeHardwareType_8() {
    EQMSG("Compute Unknown", int(HR::Unknown),
          HR::computeHardwareType(15, 1000, 12));
  }

  /** @brief Compute 100 MSPS 12 bit Rev. B type. */
  void computeHardwareType_9() {
    EQMSG("Compute RevB", int(HR::RevB_100MHz_12Bit),
          HR::computeHardwareType(11, 100, 12));
  }

  /** @brief Compute 100 MSPS 12 bit Rev. C type. */
  void computeHardwareType_10() {
    EQMSG("Compute RevC", int(HR::RevC_100MHz_12Bit),
          HR::computeHardwareType(12, 100, 12));
  }

  /** @brief Compute type for a generic new hardware. */
  void computeHardwareType_11() {
    int type = HR::createHardwareType(34, 343, 232, 42);
    int foundType = HR::computeHardwareType(34, 343, 232);
    EQMSG("new hardware type", type, foundType);
  }

  /** @brief Create a new hardware type. */
  void createHardwareType_0() {
    int type = HR::createHardwareType(34, 343, 232, 42);
    EQMSG("new hardware type", 100, type);
  }

  /** @brief Check for duplicate types. */
  void createHardwareType_1() {
    int type1 = HR::createHardwareType(34, 343, 232, 42);
    int type2 = HR::createHardwareType(34, 343, 232, 42);
    EQMSG("duplicate types don't happen", type1, type2);
  }
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION(HardwareRegistryTest);
