#ifndef MVLC_CCAENV1X90OPCODES_H
#define MVLC_CCAENV1X90OPCODES_H

/*!
   This file defines the microsequencer opcodes
   understood by the CAEN V1x90 family of TDC boards.
   The Opcode names are taken right from the manual so
   I cannot be held responsible for their cryptic
   naming coventions
*/

namespace CCAENV1x90Opcodes 
{

  // Can't use static or const in typedef.


  #define Opcode static const unsigned short


  // Acquisition mode:

  Opcode   TRG_MATCH           (0x0000);
  Opcode   CONT_STOR           (0x0100);
  Opcode   READ_ACQ_MOD        (0x0200);
  Opcode   SET_KEEP_TOKEN      (0x0300);
  Opcode   CLEAR_KEEP_TOKEN    (0x0400);
  Opcode   LOAD_DEF_CONFIG     (0x0500);
  Opcode   SAVE_USER_CONFIG    (0x0600);
  Opcode   LOAD_USER_CONFIG    (0x0700);
  Opcode   AUTOLOAD_USER_CONFIG (0x0800);
  Opcode   AUTOLOAD_DEF_CONFIG (0x0900);

  // Trigger setup:

  Opcode   SET_WIN_WIDTH       (0x1000);
  Opcode   SET_WIN_OFFS        (0x1100);
  Opcode   SET_SW_MARGIN       (0x1200);
  Opcode   SET_REJ_MARGIN      (0x1300);
  Opcode   EN_SUB_TRG          (0x1400);
  Opcode   DIS_SUB_TRG         (0x1500);
  Opcode   READ_TRG_CONF       (0x1600);

  // TDC Edge detection & resolution.

  Opcode   SET_DETECTION       (0x2200);
  Opcode   READ_DETECTION      (0x2300);
  Opcode   SET_TR_LEAD_LSB      (0x2400);
  Opcode   SET_PAIR_RES        (0x2500);
  Opcode   READ_RES            (0x2600);
  Opcode   SET_DEAD_TIME       (0x2800);
  Opcode   READ_DEAD_TIME      (0x2900);

  // TDC Readout-data structure:

  Opcode   EN_HEADER_TRAILER   (0x3000);
  Opcode   DIS_HEADER_TRAILER  (0x3100);
  Opcode   READ_HEADER_TRAILER (0x3200);
  Opcode   SET_EVENT_SIZE      (0x3300);
  Opcode   READ_EVENT_SIZE     (0x3400);
  Opcode   EN_ERROR_MARK       (0x3500);
  Opcode   DIS_ERROR_MARK      (0x3600);
  Opcode   EN_ERROR_BYPASS     (0x3700);
  Opcode   DIS_ERROR_BYPASS    (0x3800);
  Opcode   SET_ERROR_TYPES     (0x3900);
  Opcode   READ_ERROR_TYPES    (0x3a00);
  Opcode   SET_FIFO_SIZE       (0x3b00);
  Opcode   READ_FIFO_SIZE      (0x3c00);

  // Channel enable/disable control:

  Opcode   EN_CHANNEL          (0x4000);
  Opcode   DIS_CHANNEL         (0x4100);
  Opcode   EN_ALL_CH           (0x4200);
  Opcode   DIS_ALL_CH          (0x4300);
  Opcode   WRITE_EN_PATTERN    (0x4400);
  Opcode   READ_EN_PATTERN     (0x4500);
  Opcode   WRITE_EN_PATTERN32  (0x4600);
  Opcode   READ_EN_PATTERN32   (0x4700);

  // Adjustment opcodes.

  Opcode   SET_GLOB_OFFS       (0x5000);
  Opcode   READ_GLOB_OFFS      (0x5100);
  Opcode   SET_ADJUST_CH      (0x5200);
  Opcode   READ_ADJUST_CH      (0x5300);
  Opcode   SET_RC_ADJ          (0x5400);
  Opcode   READ_RC_ADJ         (0x5500);
  Opcode   SAVE_RC_ADJ         (0x5600);

  // Miscellaneous

  Opcode   READ_TDC_ID        (0x6000);
  Opcode   READ_MICRO_REV     (0x6100);
  Opcode   RESET_DLL_PLL      (0x6200);

  //  Advanced.

  Opcode   WRITE_SETUP_REG    (0x7000);
  Opcode   READ_SETUP_REG     (0x7100);
  Opcode   UPDATE_SETUP_REG   (0x7200);
  Opcode   DEFAULT_SETUP_REG  (0x7300);
  Opcode   READ_ERROR_STATUS  (0x7400);
  Opcode   READ_DLL_LOCK      (0x7500);
  Opcode   READ_STATUS_STREAM (0x7600);

  // Debug and test (don't use these unless you know what you are
  // doing.

  Opcode   WRITE_EEPROM       (0xc000);
  Opcode   READ_EEPROM        (0xc100);
  Opcode   REV_DATE_MICRO_FW  (0xc200);
  Opcode   WRITE_SPARE        (0xc300);
  Opcode   READ_SPARE         (0xc400);
  Opcode   ENABLE_TEST_MODE   (0xc500);
  Opcode   DISABLE_TEST_MODE  (0xc600);
  Opcode   SET_TDC_TEST_OUTPUT (0xc700);
  Opcode   SET_DLL_CLOCK      (0xc800);
  Opcode   READ_SETUP_SCANPATH (0xc900);

};



#endif
