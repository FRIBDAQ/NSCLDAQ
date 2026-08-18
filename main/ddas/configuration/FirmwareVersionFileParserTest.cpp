/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/

#include <cppunit/extensions/HelperMacros.h>

#include <sstream>
#include <string>

#include "Asserts.h"
#define private public
#include "Configuration.h"
#include "FirmwareVersionFileParser.h"
#undef private

using namespace std;
using namespace ::DAQ::DDAS;

class FirmwareVersionFileParserTest : public CppUnit::TestFixture {
public:
  CPPUNIT_TEST_SUITE(FirmwareVersionFileParserTest);

  // Rev. B/C/D 100 MHz 12 bit:

  CPPUNIT_TEST(parse_revbcd_100m_12b_sys);
  CPPUNIT_TEST(parse_revbcd_100m_12b_fippi);
  CPPUNIT_TEST(parse_revbcd_100m_12b_ldr);
  CPPUNIT_TEST(parse_revbcd_100m_12b_var);

  // Rev. F 250 MHz 12/14/16 bit:

  CPPUNIT_TEST(parse_revf_250m_12b_sys);
  CPPUNIT_TEST(parse_revf_250m_12b_fippi);
  CPPUNIT_TEST(parse_revf_250m_12b_dsp);
  CPPUNIT_TEST(parse_revf_250m_12b_var);

  CPPUNIT_TEST(parse_revf_250m_14b_sys);
  CPPUNIT_TEST(parse_revf_250m_14b_fippi);
  CPPUNIT_TEST(parse_revf_250m_14b_dsp);
  CPPUNIT_TEST(parse_revf_250m_14b_var);

  CPPUNIT_TEST(parse_revf_250m_16b_sys);
  CPPUNIT_TEST(parse_revf_250m_16b_fippi);
  CPPUNIT_TEST(parse_revf_250m_16b_dsp);
  CPPUNIT_TEST(parse_revf_250m_16b_var);

  // Rev. F 500 MHz 12/14 bit:

  CPPUNIT_TEST(parse_revf_500m_12b_sys);
  CPPUNIT_TEST(parse_revf_500m_12b_fippi);
  CPPUNIT_TEST(parse_revf_500m_12b_dsp);
  CPPUNIT_TEST(parse_revf_500m_12b_var);

  CPPUNIT_TEST(parse_revf_500m_14b_sys);
  CPPUNIT_TEST(parse_revf_500m_14b_fippi);
  CPPUNIT_TEST(parse_revf_500m_14b_dsp);
  CPPUNIT_TEST(parse_revf_500m_14b_var);

  // Rev. H 250 MHz 14 bit:

  CPPUNIT_TEST(parse_revh_250m_14b_sys);
  CPPUNIT_TEST(parse_revh_250m_14b_fippi);
  CPPUNIT_TEST(parse_revh_250m_14b_dsp);
  CPPUNIT_TEST(parse_revh_250m_14b_var);

  CPPUNIT_TEST_SUITE_END();

  Configuration m_config;

public:
  /** @brief Set a configuration and input stream. */
  void setUp() {
    FirmwareVersionFileParser parser;
    m_config = Configuration();
    stringstream stream(merge_lines(create_sample_file_content()));
    parser.parse(stream, m_config.m_fwMap);
  }
  void tearDown() {}

  /** @brief Create the sample file from the FW file for this install. */
  vector<string> create_sample_file_content() {
    vector<string> linesOfFile;
    string line;

    // TOP_SRC_DIR is set in the Makefile using -DTOP_SRC_DIR="@top_srcdir@"
    std::ifstream file(TOP_SRC_DIR "/ddas/readout/DDASFirmwareVersions.txt.in",
                       std::ios::in);

    while (1) {
      getline(file, line);
      if (!file.good())
        break;
      linesOfFile.push_back(line);
    }

    return linesOfFile;
  }

  /** @brief Create merged lines from sample firmware file. */
  string merge_lines(const vector<string> &content) {
    string mergedContent;
    for (auto &line : content) {
      mergedContent += line + '\n';
    }

    return mergedContent;
  }

  /** @brief Create the sample stream. */
  string create_sample_stream() {
    return merge_lines(create_sample_file_content());
  }

  //////////////////////////////////////////////////////////////////////////////
  // Rev. B/C/D 100 MSPS 12 bit:
  //

  void parse_revbcd_100m_12b_sys() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
    EQMSG("RevB common firmware is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/sys.bin"),
          fwConfig.s_ComFPGAConfigFile);
    fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
    EQMSG("RevC common firmware is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/sys.bin"),
          fwConfig.s_ComFPGAConfigFile);
    fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
    EQMSG("RevD common firmware is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/sys.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_revbcd_100m_12b_fippi() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
    EQMSG(
        "RevB fippi firmware file is set up appropriately",
        string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/fippi.bin"),
        fwConfig.s_SPFPGAConfigFile);
    fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
    EQMSG(
        "RevC fippi firmware file is set up appropriately",
        string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/fippi.bin"),
        fwConfig.s_SPFPGAConfigFile);
    fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
    EQMSG(
        "RevD fippi firmware file is set up appropriately",
        string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/fippi.bin"),
        fwConfig.s_SPFPGAConfigFile);
  }

  void parse_revbcd_100m_12b_ldr() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
    EQMSG("RevB dsp code file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/dsp.ldr"),
          fwConfig.s_DSPCodeFile);
    fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
    EQMSG("RevC dsp code file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/dsp.ldr"),
          fwConfig.s_DSPCodeFile);
    fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
    EQMSG("RevD dsp code file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/dsp.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_revbcd_100m_12b_var() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
    EQMSG("RevBCD dsp var file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/dsp.var"),
          fwConfig.s_DSPVarFile);
    fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
    EQMSG("RevC dsp var file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/dsp.var"),
          fwConfig.s_DSPVarFile);
    fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
    EQMSG("RevD dsp var file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_13-100-12_general_1.0.0/dsp.var"),
          fwConfig.s_DSPVarFile);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Rev. F 250 MSPS 12 bit:
  //

  void parse_revf_250m_12b_sys() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
    EQMSG("RevF_250MHz_12Bit common firmware is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-250-12_general_1.0.2/sys.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_revf_250m_12b_fippi() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
    EQMSG(
        "RevF_250MHz_12Bit fippi firmware file is set up appropriately",
        string("@firmwaredir@/xia_pixie-16_15-250-12_general_1.0.2/fippi.bin"),
        fwConfig.s_SPFPGAConfigFile);
  }

  void parse_revf_250m_12b_dsp() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
    EQMSG("RevF_250MHz_12Bit dsp code file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-250-12_general_1.0.2/dsp.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_revf_250m_12b_var() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
    EQMSG("RevF_250MHz_12Bit dsp var file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-250-12_general_1.0.2/dsp.var"),
          fwConfig.s_DSPVarFile);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Rev. F 250 MSPS 14 bit:
  //

  void parse_revf_250m_14b_sys() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
    EQMSG("RevF_250MHz_14Bit common firmware is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-250-14_general_1.0.1/sys.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_revf_250m_14b_fippi() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
    EQMSG(
        "RevF_250MHz_14Bit fippi firmware file is set up appropriately",
        string("@firmwaredir@/xia_pixie-16_15-250-14_general_1.0.1/fippi.bin"),
        fwConfig.s_SPFPGAConfigFile);
  }

  void parse_revf_250m_14b_dsp() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
    EQMSG("RevF_250MHz_14Bit dsp code file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-250-14_general_1.0.1/dsp.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_revf_250m_14b_var() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
    EQMSG("RevF_250MHz_14Bit dsp var file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-250-14_general_1.0.1/dsp.var"),
          fwConfig.s_DSPVarFile);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Rev. F 250 MSPS 16 bit:
  //

  void parse_revf_250m_16b_sys() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
    EQMSG("RevF_250MHz_16Bit common firmware is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-250-16_general_1.1.0/sys.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_revf_250m_16b_fippi() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
    EQMSG(
        "RevF_250MHz_16Bit fippi firmware file is set up appropriately",
        string("@firmwaredir@/xia_pixie-16_15-250-16_general_1.1.0/fippi.bin"),
        fwConfig.s_SPFPGAConfigFile);
  }

  void parse_revf_250m_16b_dsp() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
    EQMSG("RevF_250MHz_16Bit dsp code file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-250-16_general_1.1.0/dsp.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_revf_250m_16b_var() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
    EQMSG("RevF_250MHz_16Bit dsp var file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-250-16_general_1.1.0/dsp.var"),
          fwConfig.s_DSPVarFile);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Rev. F 500 MSPS 12 bit:
  //

  void parse_revf_500m_12b_sys() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
    EQMSG("RevF_500MHz_12Bit common firmware is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-500-12_general_1.2.0/sys.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_revf_500m_12b_fippi() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
    EQMSG(
        "RevF_500MHz_12Bit fippi firmware file is set up appropriately",
        string("@firmwaredir@/xia_pixie-16_15-500-12_general_1.2.0/fippi.bin"),
        fwConfig.s_SPFPGAConfigFile);
  }

  void parse_revf_500m_12b_dsp() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
    EQMSG("RevF_500MHz_12Bit dsp code file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-500-12_general_1.2.0/dsp.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_revf_500m_12b_var() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
    EQMSG("RevF_500MHz_12Bit dsp var file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-500-12_general_1.2.0/dsp.var"),
          fwConfig.s_DSPVarFile);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Rev. F 500 MSPS 14 bit:
  //

  void parse_revf_500m_14b_sys() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
    EQMSG("RevF_500MHz_14Bit common firmware is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-500-14_general_1.0.1/sys.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_revf_500m_14b_fippi() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
    EQMSG(
        "RevF_500MHz_14Bit fippi firmware file is set up appropriately",
        string("@firmwaredir@/xia_pixie-16_15-500-14_general_1.0.1/fippi.bin"),
        fwConfig.s_SPFPGAConfigFile);
  }

  void parse_revf_500m_14b_dsp() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
    EQMSG("RevF_500MHz_14Bit dsp code file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-500-14_general_1.0.1/dsp.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_revf_500m_14b_var() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
    EQMSG("RevF_500MHz_14Bit dsp var file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_15-500-14_general_1.0.1/dsp.var"),
          fwConfig.s_DSPVarFile);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Rev. H 250 MSPS 14 bit:
  //

  void parse_revh_250m_14b_sys() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevH_250MHz_14Bit);
    EQMSG("RevH_250MHz_14Bit common firmware is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_17-250-14_general_1.0.1/sys.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_revh_250m_14b_fippi() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevH_250MHz_14Bit);
    EQMSG(
        "RevH_250MHz_14Bit fippi firmware file is set up appropriately",
        string("@firmwaredir@/xia_pixie-16_17-250-14_general_1.0.1/fippi.bin"),
        fwConfig.s_SPFPGAConfigFile);
  }

  void parse_revh_250m_14b_dsp() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevH_250MHz_14Bit);
    EQMSG("RevH_250MHz_14Bit dsp code file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_17-250-14_general_1.0.1/dsp.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_revh_250m_14b_var() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevH_250MHz_14Bit);
    EQMSG("RevH_250MHz_14Bit dsp var file is set up appropriately",
          string("@firmwaredir@/xia_pixie-16_17-250-14_general_1.0.1/dsp.var"),
          fwConfig.s_DSPVarFile);
  }
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION(FirmwareVersionFileParserTest);
