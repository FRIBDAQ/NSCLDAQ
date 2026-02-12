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
  CPPUNIT_TEST(parse_0a);
  CPPUNIT_TEST(parse_0b);
  CPPUNIT_TEST(parse_0c);
  CPPUNIT_TEST(parse_1a);
  CPPUNIT_TEST(parse_1b);
  CPPUNIT_TEST(parse_1c);
  CPPUNIT_TEST(parse_2a);
  CPPUNIT_TEST(parse_2b);
  CPPUNIT_TEST(parse_2c);
  CPPUNIT_TEST(parse_3a);
  CPPUNIT_TEST(parse_3b);
  CPPUNIT_TEST(parse_3c);
  CPPUNIT_TEST(parse_4);
  CPPUNIT_TEST(parse_5);
  CPPUNIT_TEST(parse_6);
  CPPUNIT_TEST(parse_7);
  CPPUNIT_TEST(parse_8);
  CPPUNIT_TEST(parse_9);
  CPPUNIT_TEST(parse_10);
  CPPUNIT_TEST(parse_11);
  CPPUNIT_TEST(parse_12);
  CPPUNIT_TEST(parse_13);
  CPPUNIT_TEST(parse_14);
  CPPUNIT_TEST(parse_15);
  CPPUNIT_TEST(parse_16);
  CPPUNIT_TEST(parse_17);
  CPPUNIT_TEST(parse_18);
  CPPUNIT_TEST(parse_19);
  CPPUNIT_TEST(parse_20);
  CPPUNIT_TEST(parse_21);
  CPPUNIT_TEST(parse_22);
  CPPUNIT_TEST(parse_23);
  CPPUNIT_TEST(parse_24);
  CPPUNIT_TEST(parse_25);
  CPPUNIT_TEST(parse_26);
  CPPUNIT_TEST(parse_27);
  CPPUNIT_TEST(parse_28);
  CPPUNIT_TEST(parse_29);
  CPPUNIT_TEST(parse_30);
  CPPUNIT_TEST(parse_31);
  CPPUNIT_TEST_SUITE_END();

  Configuration m_config;

public:
  /** @brief Set a configuration and input stream. */
  void setUp() {
    FirmwareVersionFileParser parser;
    m_config = Configuration();
    stringstream stream(mergeLines(createSampleFileContent()));
    parser.parse(stream, m_config.m_fwMap);
  }
  void tearDown() {}

  /** @brief Create the sample file from the FW file for this install. */
  vector<string> createSampleFileContent() {
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
  string mergeLines(const vector<string> &content) {
    string mergedContent;
    for (auto &line : content) {
      mergedContent += line + '\n';
    }

    return mergedContent;
  }

  /** @brief Create the sample stream. */
  string createSampleStream() { return mergeLines(createSampleFileContent()); }

  /// @name RevBCDChecks
  ///@{
  /**
   * @brief Checks that the Rev. B/C/D firmware configurations are read
   * properly. All 100 MSPS 12 bit.
   */
  void parse_0a() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
    EQMSG("RevB common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_12b100m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_0b() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
    EQMSG("RevC common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_12b100m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_0c() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
    EQMSG("RevD common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_12b100m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_1a() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
    EQMSG("RevBCD fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_12b100m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }

  void parse_1b() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
    EQMSG("RevBCD fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_12b100m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }

  void parse_1c() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
    EQMSG("RevBCD fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_12b100m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }

  void parse_2a() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
    EQMSG("RevBCD dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_2b() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
    EQMSG("RevBCD dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_2c() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
    EQMSG("RevBCD dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_3a() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevB_100MHz_12Bit);
    EQMSG("RevBCD dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_3b() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevC_100MHz_12Bit);
    EQMSG("RevBCD dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_3c() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevD_100MHz_12Bit);
    EQMSG("RevBCD dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }
  ///@}

  /// @name RevFChecks
  ///@{
  /** @brief Checks that the Rev. F firmware configurations are read
   * properly.
   */
  /// @name 100MSPS14Bit
  ///@{
  /** @brief 100 MSPS, 14 bit. */
  void parse_4() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_14Bit);
    EQMSG("RevF_100MHz_14Bit common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_14b100m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_5() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_14Bit);
    EQMSG("RevF_100MHz_14Bit fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_14b100m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }
  void parse_6() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_14Bit);
    EQMSG("RevF_100MHz_14Bit dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_14b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_7() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_14Bit);
    EQMSG("RevF_100MHz_14Bit dsp var file is set up appropriately",
          string("@dspdir@/Pixie16_current_14b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }
  ///@}

  /// @name 100MSPS16Bit
  ///@{
  /** @brief 100 MSPS, 16 bit. */
  void parse_8() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_16Bit);
    EQMSG("RevF_100MHz_16Bit common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_16b100m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_9() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_16Bit);
    EQMSG("RevF_100MHz_16Bit fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_16b100m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }

  void parse_10() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_16Bit);
    EQMSG("RevF_100MHz_16Bit dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_16b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_11() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_100MHz_16Bit);
    EQMSG("RevF_100MHz_16Bit dsp var file is set up appropriately",
          string("@dspdir@/Pixie16_current_16b100m.ldr"),
          fwConfig.s_DSPCodeFile);
  }
  ///@}

  /// @name 250MSPS12Bit
  ///@{
  /** @brief 250 MSPS, 12 bit. */
  void parse_12() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
    EQMSG("RevF_250MHz_12Bit common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_12b250m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_13() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
    EQMSG("RevF_250MHz_12Bit fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_12b250m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }

  void parse_14() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
    EQMSG("RevF_250MHz_12Bit dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b250m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_15() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_12Bit);
    EQMSG("RevF_250MHz_12Bit dsp var file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b250m.ldr"),
          fwConfig.s_DSPCodeFile);
  }
  ///@}

  /// @name 250MSPS14Bit
  ///@{
  /** @brief 250 MSPS, 14 bit. */
  void parse_16() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
    EQMSG("RevF_250MHz_14Bit common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_14b250m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_17() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
    EQMSG("RevF_250MHz_14Bit fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_14b250m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }

  void parse_18() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
    EQMSG("RevF_250MHz_14Bit dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_14b250m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_19() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_14Bit);
    EQMSG("RevF_250MHz_14Bit dsp var file is set up appropriately",
          string("@dspdir@/Pixie16_current_14b250m.ldr"),
          fwConfig.s_DSPCodeFile);
  }
  ///@}

  /// @name 250MSPS16Bit
  ///@{
  /** @brief 250 MSPS, 16 bit. */
  void parse_20() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
    EQMSG("RevF_250MHz_16Bit common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_16b250m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_21() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
    EQMSG("RevF_250MHz_16Bit fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_16b250m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }

  void parse_22() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
    EQMSG("RevF_250MHz_16Bit dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_16b250m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_23() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_250MHz_16Bit);
    EQMSG("RevF_250MHz_16Bit dsp var file is set up appropriately",
          string("@dspdir@/Pixie16_current_16b250m.ldr"),
          fwConfig.s_DSPCodeFile);
  }
  ///@}

  /// @name 500MSPS12Bit
  ///@{
  /** @brief 500 MSPS, 12 bit. */
  void parse_24() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
    EQMSG("RevF_500MHz_12Bit common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_12b500m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_25() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
    EQMSG("RevF_500MHz_12Bit fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_12b500m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }

  void parse_26() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
    EQMSG("RevF_500MHz_12Bit dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b500m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_27() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_12Bit);
    EQMSG("RevF_500MHz_12Bit dsp var file is set up appropriately",
          string("@dspdir@/Pixie16_current_12b500m.ldr"),
          fwConfig.s_DSPCodeFile);
  }
  ///@}

  /// @name 500MSPS14Bit
  ///@{
  /** @brief 500 MSPS, 14 bit. */
  void parse_28() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
    EQMSG("RevF_500MHz_14Bit common firmware is set up appropriately",
          string("@firmwaredir@/syspixie16_current_14b500m.bin"),
          fwConfig.s_ComFPGAConfigFile);
  }

  void parse_29() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
    EQMSG("RevF_500MHz_14Bit fippi firmware file is set up appropriately",
          string("@firmwaredir@/fippixie16_current_14b500m.bin"),
          fwConfig.s_SPFPGAConfigFile);
  }

  void parse_30() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
    EQMSG("RevF_500MHz_14Bit dsp code file is set up appropriately",
          string("@dspdir@/Pixie16_current_14b500m.ldr"),
          fwConfig.s_DSPCodeFile);
  }

  void parse_31() {
    FirmwareConfiguration fwConfig =
        m_config.getFirmwareConfiguration(HardwareRegistry::RevF_500MHz_14Bit);
    EQMSG("RevF_500MHz_16Bit dsp var file is set up appropriately",
          string("@dspdir@/Pixie16_current_14b500m.ldr"),
          fwConfig.s_DSPCodeFile);
  }
  ///@}

  ///@}
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION(FirmwareVersionFileParserTest);
