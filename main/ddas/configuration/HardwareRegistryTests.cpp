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

  CPPUNIT_TEST(reset_to_defaults);

  CPPUNIT_TEST(get_specification_revb_100m_12b);
  CPPUNIT_TEST(get_specification_revc_100m_12b);
  CPPUNIT_TEST(get_specification_revd_100m_12b);
  CPPUNIT_TEST(get_specification_revf_250m_12b);
  CPPUNIT_TEST(get_specification_revf_250m_14b);
  CPPUNIT_TEST(get_specification_revf_250m_16b);
  CPPUNIT_TEST(get_specification_revf_500m_12b);
  CPPUNIT_TEST(get_specification_revf_500m_14b);
  CPPUNIT_TEST(get_specification_revh_250m_14b);

  CPPUNIT_TEST(override_revb_100m_12b);

  CPPUNIT_TEST(compute_revb_100m_12b);
  CPPUNIT_TEST(compute_revc_100m_12b);
  CPPUNIT_TEST(compute_revd_100m_12b);
  CPPUNIT_TEST(compute_revf_250m_12b);
  CPPUNIT_TEST(compute_revf_250m_14b);
  CPPUNIT_TEST(compute_revf_250m_16b);
  CPPUNIT_TEST(compute_revf_500m_12b);
  CPPUNIT_TEST(compute_revf_500m_14b);
  CPPUNIT_TEST(compute_revh_250m_14b);

  CPPUNIT_TEST(create_new);
  CPPUNIT_TEST(compute_unknown);

  CPPUNIT_TEST(create_duplicate);

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
  void reset_to_defaults() {
    HR::configureHardwareType(HR::RevB_100MHz_12Bit, {430, 23});
    HR::resetToDefaults();
    get_specification_revb_100m_12b();
  }

  /** @brief Get 100 MSPS 12 bit Rev. B specification. */
  void get_specification_revb_100m_12b() {
    auto spec = HR::getSpecification(HR::RevB_100MHz_12Bit);
    EQMSG("RevB_100MHz_12Bit default rev", 11, spec.s_hdwrRevision);
    EQMSG("RevB_100MHz_12Bit default adc freq", 100, spec.s_adcFrequency);
    EQMSG("RevB_100MHz_12Bit default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 100 MSPS 12 bit Rev. C specification. */
  void get_specification_revc_100m_12b() {
    auto spec = HR::getSpecification(HR::RevC_100MHz_12Bit);
    EQMSG("RevC_100MHz_12Bit default rev", 12, spec.s_hdwrRevision);
    EQMSG("RevC_100MHz_12Bit default adc freq", 100, spec.s_adcFrequency);
    EQMSG("RevC_100MHz_12Bit default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 100 MSPS 12 bit Rev. D specification. */
  void get_specification_revd_100m_12b() {
    auto spec = HR::getSpecification(HR::RevD_100MHz_12Bit);
    EQMSG("RevD_100MHz_12Bit default rev", 13, spec.s_hdwrRevision);
    EQMSG("RevD_100MHz_12Bit default adc freq", 100, spec.s_adcFrequency);
    EQMSG("RevD_100MHz_12Bit default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 250 MSPS 12 bit Rev. F specification. */
  void get_specification_revf_250m_12b() {
    auto spec = HR::getSpecification(HR::RevF_250MHz_12Bit);
    EQMSG("RevF_250MHz_12Bit default rev", 15, spec.s_hdwrRevision);
    EQMSG("RevF_250MHz_12Bit default adc freq", 250, spec.s_adcFrequency);
    EQMSG("RevF_250MHz_12Bit default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 250 MSPS 14 bit Rev. F specification. */
  void get_specification_revf_250m_14b() {
    auto spec = HR::getSpecification(HR::RevF_250MHz_14Bit);
    EQMSG("RevF_250MHz_14Bit default rev", 15, spec.s_hdwrRevision);
    EQMSG("RevF_250MHz_14Bit default adc freq", 250, spec.s_adcFrequency);
    EQMSG("RevF_250MHz_14Bit default adc resolution", 14, spec.s_adcResolution);
  }

  /** @brief Get 250 MSPS 16 bit Rev. F specification. */
  void get_specification_revf_250m_16b() {
    auto spec = HR::getSpecification(HR::RevF_250MHz_16Bit);
    EQMSG("RevF_250MHz_16Bit default rev", 15, spec.s_hdwrRevision);
    EQMSG("RevF_250MHz_16Bit default adc freq", 250, spec.s_adcFrequency);
    EQMSG("RevF_250MHz_16Bit default adc resolution", 16, spec.s_adcResolution);
  }

  /** @brief Get 500 MSPS 12 bit Rev. F specification. */
  void get_specification_revf_500m_12b() {
    auto spec = HR::getSpecification(HR::RevF_500MHz_12Bit);
    EQMSG("RevF_500MHz_12Bit default rev", 15, spec.s_hdwrRevision);
    EQMSG("RevF_500MHz_12Bit default adc freq", 500, spec.s_adcFrequency);
    EQMSG("RevF_500MHz_12Bit default adc resolution", 12, spec.s_adcResolution);
  }

  /** @brief Get 500 MSPS 14 bit Rev. F specification. */
  void get_specification_revf_500m_14b() {
    auto spec = HR::getSpecification(HR::RevF_500MHz_14Bit);
    EQMSG("RevF_500MHz_14Bit default rev", 15, spec.s_hdwrRevision);
    EQMSG("RevF_500MHz_14Bit default adc freq", 500, spec.s_adcFrequency);
    EQMSG("RevF_500MHz_14Bit default adc resolution", 14, spec.s_adcResolution);
  }

  /** @brief Get 250 MSPS 14 bit Rev. H speficifation */
  void get_specification_revh_250m_14b() {
    auto spec = HR::getSpecification(HR::RevH_250MHz_14Bit);
    EQMSG("RevH_250MHz_14Bit default rev", 17, spec.s_hdwrRevision);
    EQMSG("RevH_250MHz_14Bit default adc freq", 250, spec.s_adcFrequency);
    EQMSG("RevH_250MHz_14Bit default adc resolution", 14, spec.s_adcResolution);
  }

  /** @brief Override and check registry configuration info. */
  void override_revb_100m_12b() {
    HR::configureHardwareType(HR::RevB_100MHz_12Bit, {430, 23, 2});
    auto spec = HR::getSpecification(HR::RevB_100MHz_12Bit);
    EQMSG("After configure, adc freq", 430, spec.s_adcFrequency);
    EQMSG("After configure, adc resolution", 23, spec.s_adcResolution);
    EQMSG("After configure, hdwr revision", 2, spec.s_hdwrRevision);
  }

  /** @brief Compute 100 MSPS 12 bit Rev. B type. */
  void compute_revb_100m_12b() {
    EQMSG("Compute RevB_100MHz_12Bit", int(HR::RevB_100MHz_12Bit),
          HR::computeHardwareType(11, 100, 12));
  }

  /** @brief Compute 100 MSPS 12 bit Rev. C type. */
  void compute_revc_100m_12b() {
    EQMSG("Compute RevC_100MHz_12Bit", int(HR::RevC_100MHz_12Bit),
          HR::computeHardwareType(12, 100, 12));
  }

  /** @brief Compute 100 MSPS 12 bit Rev. D type. */
  void compute_revd_100m_12b() {
    EQMSG("Compute RevD_100MHz_12Bit", int(HR::RevD_100MHz_12Bit),
          HR::computeHardwareType(13, 100, 12));
  }

  /** @brief Compute 250 MSPS 12 bit Rev. F type. */
  void compute_revf_250m_12b() {
    EQMSG("Compute RevF_250MHz_12Bit", int(HR::RevF_250MHz_12Bit),
          HR::computeHardwareType(15, 250, 12));
  }

  /** @brief Compute 250 MSPS 14 bit Rev. F type. */
  void compute_revf_250m_14b() {
    EQMSG("Compute RevF_250MHz_14Bit", int(HR::RevF_250MHz_14Bit),
          HR::computeHardwareType(15, 250, 14));
  }

  /** @brief Compute 250 MSPS 16 bit Rev. F type. */
  void compute_revf_250m_16b() {
    EQMSG("Compute RevF_250MHz_16Bit", int(HR::RevF_250MHz_16Bit),
          HR::computeHardwareType(15, 250, 16));
  }

  /** @brief Compute 500 MSPS 12 bit Rev. F type. */
  void compute_revf_500m_12b() {
    EQMSG("Compute RevF_500MHz_12Bit", int(HR::RevF_500MHz_12Bit),
          HR::computeHardwareType(15, 500, 12));
  }

  /** @brief Compute 500 MSPS 14 bit Rev. F type. */
  void compute_revf_500m_14b() {
    EQMSG("Compute RevF_500MHz_14Bit", int(HR::RevF_500MHz_14Bit),
          HR::computeHardwareType(15, 500, 14));
  }

  /** @brief Compute 250 MSPS 14 bit Rev. H type. */
  void compute_revh_250m_14b() {
    EQMSG("Compute RevH_250MHz_14Bit", int(HR::RevH_250MHz_14Bit),
          HR::computeHardwareType(17, 250, 14));
  }

  /** @brief Compute unknown type. */
  void compute_unknown() {
    EQMSG("Compute Unknown", int(HR::Unknown),
          HR::computeHardwareType(15, 1000, 12));
  }

  /**
   * @brief Create a new hardware type.
   * @note Default first available user type is controlled by the static
   * variable HR::sDefaultFirstAvailableUserType = 100.
   */
  void create_new() {
    int type = HR::createHardwareType(34, 343, 232, 42);
    EQMSG("new hardware type", 100, type);
  }

  /** @brief Compute type for a generic new hardware. */
  void compute_new_generic() {
    int type = HR::createHardwareType(34, 343, 232, 42);
    int foundType = HR::computeHardwareType(34, 343, 232);
    EQMSG("new hardware type", type, foundType);
  }

  /** @brief Check for duplicate types. */
  void create_duplicate() {
    int type1 = HR::createHardwareType(34, 343, 232, 42);
    int type2 = HR::createHardwareType(34, 343, 232, 42);
    EQMSG("duplicate types don't happen", type1, type2);
  }
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION(HardwareRegistryTest);
