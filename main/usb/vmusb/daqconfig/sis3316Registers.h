/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright MADCDELAY5.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef SIS3316REGISTERS_H
#define SIS3316REGISTERS_H
/**
 *  @file sis3316Registers.h
 *  @brief Register definitions and AM's for the SIS3316.
 */

#include <stdint.h>

// let's get in the habit of using namespaces:

namespace SIS3316 {
    namespace Registers {


const int AMSINGLE=0x0d;   // Single shot operations in Supervisory data.
const int AMBLOCK=0x0f;    // Block modes in A32 block transfer mode.


// FPGA interface registers - as a struct.

static const uint32_t FPGAOFFSET(0);    // Offset to this struct:
#pragma pack(push, 1)
struct FpgaRegisters {
    uint32_t  s_csr;
    uint32_t  s_fwid;
    uint32_t  s_irqconfig;
    uint32_t  s_irqcontrol;
    
    uint32_t  s_arbitrationcsr;
    uint32_t  s_bcstsetup;
    uint32_t  s_internalTest;
    uint32_t  s_hwversion;
    
    uint32_t  s_temperature;
    uint32_t  s_wire1eepromcsr;   // i2c for 3316-2 variant.
    uint32_t  s_serialno;
    uint32_t  s_internalxfrspeed;
    
    uint32_t  s_adcfpgabootcontroller;
    uint32_t  s_spiflashcsr;
    uint32_t  s_spiflashdata;
    uint32_t  s_externalvetogdr;
    
    uint32_t  s_adcclocki2c;
    uint32_t  s_mgt1clocki2c;
    uint32_t  s_mgt2clocki2c;
    uint32_t  s_ddr3clocki2c;

    uint32_t  s_adcclockdstcontrol;
    uint32_t  s_nimclockmult;
    uint32_t  s_fpbuscontrol;
    uint32_t  s_nimincsr;

    uint32_t  s_acqcsr;
    uint32_t  s_coinclutcsr;
    uint32_t  s_coinclutaddr;
    uint32_t  s_coinclutdata;

    uint32_t  s_lemocoselect;
    uint32_t  s_lemotoselect;
    uint32_t  s_lemouoselect;
    uint32_t  s_trfeedbackselect;

    uint32_t  s_adc1234dataxferctl;
    uint32_t  s_adc5678dataxferctl;
    uint32_t  s_adc9abcdataxferctl;
    uint32_t  s_adcdefgdataxferctl;   // g is what you get for numbering from 1 SIS.

    uint32_t  s_adc1234dataxfersr;
    uint32_t  s_adc5678dataxfersr;
    uint32_t  s_adc9abcdataxfersr;
    uint32_t  s_adcdefgdataxfersr;   // g is what you get for numbering from 1 SIS.

    uint32_t  s_vmeadcfpgadlinkstatus;
    uint32_t  s_adcfpgaspibusystatus;
    uint32_t  s_reserved1;
    uint32_t  s_plldlpcontrol;

    uint32_t  s_reserved2;
    uint32_t  s_reserved3;
    uint32_t  s_reserved4;
    uint32_t  s_prescalerdivider;
    uint32_t  s_prescalerlength;

    uint32_t  s_channelTriggerCounts[16];
    
    
}

// The key registers. Key registers are SIS-speak for registers that 
// do something if you write to them.. no matter what's written.

static const uint32_t  KEYOFFSETS(0x400);       // Offset to this struct:

struct keyRegisters {
    uint32_t s_registerReset;
    uint32_t s_userFunction;
    uint32_t s_reserved1;
    uint32_t s_reserved2;

    uint32_t s_armSampleLogic;
    uint32_t s_disarmSampleLogic;
    uint32_t s_trigger;
    uint32_t s_clearTimestamp;

    uint32_t s_armBank1;    // Any armed back is
    uint32_t s_armBank2;    // disarmed by this.
    uint32_t s_enableNIMBankSwap;
    uint32_t s_disablePescaleOutputDivider;

    uint32_t s_PPSLatchClear;
    uint32_t s_logicReset;
    uint32_t s_adcClockPLLReset;
    uint32_t s_reserved3;
}

// Each bank of 4 ADCs  has two register sets:  THe ADC FPGA registers and the 
// memory data FIFO.  The latter are just data soup but the former have structure:

// FPGA Register bases (we number from 0 rather than 1):

static const uint32_t ADCFPGABASES[4] = {0x1000, 0x2000, 0x3000, 0x4000};
static const uint32_t FIFOBASES[4] = {0x1000000, 0x2000000, 0x3000000, 0x4000000};

// Ther's a special set of offsets to read back the DC Offset value
// and SPI control registe from the adCFPGARegs:

static const uint32_t DACREADBACKS = {0X1108, 0X2108, 0X3108, 0X4108};
static const uint32_t ADCSPIREADBACKS = {0X110C, 0X210C, 0X310C, 0X410C};

// there are a few peradc. registers:

struct adcRegisters {
    uint32_t s_firtrgsetup_a;
    uint32_t s_threshold_a;
    uint32_t s_hethreshold_a;
    uint32_t s_reserved_1;
}

struct adcFPGARegisters {
    uint32_t s_inputTapDelay;
    uint32_t s_gainTerminationcontrol;
    uint32_t s_DCOffset;
    uint32_t s_SPIControl;
    
    uint32_t s_eventConfig;
    uint32_t s_channelHeaderId;
    uint32_t s_endThreshold;
    uint32_t s_triggerGateLength;

    uint32_t s_dataconfig;
    uint32_t s_pupconfig;
    uint32_t s_pretrigger;
    uint32_t s_averageconfig;

    uint32_t s_format;
    uint32_t s_MAWtestconfig;
    uint32_t s_internalTrigDelay;
    uint32_t s_internalGateLength;

    adcRegisters s_adcSetup[5];   // Per ADC and for the sum.

    uint32_t s_trgstatmode;
    uint32_t s_peakchargeconfig;
    uint32_t s_extendedbufferconfig;
   // uint32_t s_extendedEventconfig;

    uint32_t s_accumulatorGateconfig[3];

    uint32_t s_FIRenergySetup[4];
    uint32_t s_histogramSetup[4];
    uint32_t s_MAWStartIndexConfig[4];

    uint32_t s_test;
    uint32_t s_unused[3];

    uint32_t s_sampleClockPLLDrp;
    uint32_t s_adfpgaVersion;
    uint32_t s_adcfpgastatus;
    uint32_t s_offsetreadback;
    uint32_t s_spireadback;

    uint32_t s_sampleAddress[4];
    uint32_t s_prevbankSample[4];  // to offset 0x130.

    uint8_t s_unused[0xcb];]    // cb not cc because there's a byte.
                                // Ad offset 0x1fc
    uint32_t s_sampleClockPLLDrpRead;


}
// Below are bit defintions for registers described above.

// The CSR is a funky register.  Rather than setting bits to 
// set things; the top 16 bits clear operations and the bottom
// 32 bits set operations.  The bottom 32 bits read state.
// e.g bit 32 clears the operation of restarting the FFPGAs
// bit 16 starts rebooting the FPGAS and reads whether the FPGAs 
// are being rebooted.  Don't blame me, blame STRUCK:
// Blame me for the lazyness of not provideing e.g. CSR_READ_FPGA_BOOT
// use the set bit as documented above.
static const uint32_t CSR_CLEAR_FPGA_BOOT(0x80000000);
static const uint32_t CSR_SET_FPGA_BOOT(0x00008000);

static const uint32_t CSR_CLEAR_LED2_APPMODE (0x00400000);
static const uint32_t CSR_SET_LED2_APPMODE   (0x00000040);
static const uint32_t CSR_CLEAR_LED1_APPMODE (0x00200000);
static const uint32_t CSR_SET_LED1_APPMODE   (0x00000020);   // LED application mode.
static const uint32_t CSR_CLEAR_LEDU_APPMODE (0x00100000);
static const uint32_t CSR_SET_LEDU_APPMODE   (0x00000010);
static const uint32_t CSR_CLEAR_LED_APPMODE  (0x00080000);
static const uint32_t CSR_SET_LED_APPMODE    (0x00000008);

static const uint32_t CSR_CLEAR_LED2 (0x00040000);
static const uint32_t CSR_SET_LED2   (0x00000004);
static const uint32_t CSR_CLEAR_LED1 (0x00020000);
static const uint32_t CSR_CLEAR_LEDU (0x00010000);
static const uint32_t CSR_SET_LEDU   (0x00000001);

// The firmware register include ths model number
// as well as the firmware major and minor revision numbers.
// The major revision actually is the functionality of the
// firmware.

static const uint32_t FWID_MODULEID_MASK(0XFFFF0000);
static const uint32_t FWID_MODULEID_VALUE(0x33160000);   // reg and mask should give this.
static const uint32_t FWID_MAJOR_MASK(0x0000ff00);
static const uint32_t FWID_MINOR_MASK(0x000000ff);

// Possible major firware values after anding with the mask:

static const uint32_t FWID_STDNGAMMA(0X00002000);   // For sis 3316
static const uint32_t FWID_STDNGAMMA_2(0x00004000);  // For sis 3316-2.

// Interrupt configuration register bits... Note that these are plain old
// bit fields.  For each we provide a mask and shift.
// to read register & mask >> shift gives a value.
// to write:  (value <<shift) | (register & ~mask).

static const uint32_t IRQCFG_ROAK_MASK(0x1000);
static const uint32_t IRQCFG_ROAK_SHIFT(12);
static const uint32_t IRQCFG_ENA_MASK(0X800);
static const uint32_t IRQCFG_ENA_SHIFT(11);
static const uint32_t IRQCFG_IPL_MASK(0x700);
static const uint32_t IRQCFG_IPL_SHIFT(8);
static const uint32_t IRQCFG_VECTOR_MASK(0x00ff);
static const uint32_t IRQCFG_VECTOR_SHIFT(0);

// The interrupt control/status register is a mess, there are write
// bits and read bits and they differ.  e.g. bit 31 when written is
// "UPdate IRQ Pulse" when read is the status of IRQ source 7.
// sigh so we have IRQCTL_RD and IRQCTL_WR bit/shifts/fields etc.
// to distingquish.  Use an _RD_ bit to read and an WR bit to write.

                  // the write bits.

static const uint32_t IRQCTL_WR_UPDATEPULSE (0x80000000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ7_SRC(0x00800000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ6_SRC(0x00400000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ5_SRC(0x00200000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ4_SRC(0x00100000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ3_SRC(0x00080000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ2_SRC(0X00040000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ1_SRC(0x00020000);
static const uint32_t IRQCTL_WR_DISABLE_IRQ0_SRC(0X00010000);
static const uint32_t IRQCTL_WR_ENABLE_IRQ7_SRC(0x80);
static const uint32_t IRQCTL_WR_ENABLE_IRQ6_SRC(0x40);
static const uint32_t IRQCTL_WR_ENABLE_IRQ5_SRC(0x20);
static const uint32_t IRQCTL_WR_ENABLE_IRQ4_SRC(0x10);
static const uint32_t IRQCTL_WR_ENABLE_IRQ3_SRC(0x08);
static const uint32_t IRQCTL_WR_ENABLE_IRQ2_SRC(0x04);
static const uint32_t IRQCTL_WR_ENABLE_IRQ1_SRC(0x02);
static const uint32_t IRQCTL_WR_ENABLE_IRQ0_SRC(0x01);

                // the read bits.

static const uint32_t IRQCTL_RD_STATUS_IRQ7(0x80000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ6(0x40000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ5(0x20000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ4(0x10000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ3(0x08000000); // End address threshold level
static const uint32_t IRQCTL_RD_STATUS_IRQ2(0x04000000); // End address threshold edge
static const uint32_t IRQCTL_RD_STATUS_IRQ1(0x02000000);
static const uint32_t IRQCTL_RD_STATUS_IRQ0(0x01000000);

static const uint32_t IRQCTL_RD_STATUS_FLAG7(0x00800000);
static const uint32_t IRQCTL_RD_STATUS_FLAG6(0x00400000);
static const uint32_t IRQCTL_RD_STATUS_FLAG5(0x00200000);
static const uint32_t IRQCTL_RD_STATUS_FLAG4(0x00100000);
static const uint32_t IRQCTL_RD_STATUS_FLAG3(0x00080000);
static const uint32_t IRQCTL_RD_STATUS_FLAG2(0x00040000);
static const uint32_t IRQCTL_RD_STATUS_FLAG1(0x00020000);
static const uint32_t IRQCTL_RD_STATUS_FLAG0(0x00010000);

static const uint32_t IRQCTL_RD_VMEIRQSTATUS(0x8000);
static const uint32_t IRQCTL_RD_INTIRQSTATUS(0x4000);
static const uint32_t IRQCTL_RD_ENABLE_IRQ7_SRC(0x80);
static const uint32_t IRQCTL_RD_ENABLE_IRQ6_SRC(0x40);
static const uint32_t IRQCTL_RD_ENABLE_IRQ5_SRC(0x20);
static const uint32_t IRQCTL_RD_ENABLE_IRQ4_SRC(0x10);
static const uint32_t IRQCTL_RD_ENABLE_IRQ3_SRC(0x08);
static const uint32_t IRQCTL_RD_ENABLE_IRQ2_SRC(0x04);
static const uint32_t IRQCTL_RD_ENABLE_IRQ1_SRC(0x02);
static const uint32_t IRQCTL_RD_ENABLE_IRQ0_SRC(0x01);

// The Arbitration CSR. \
// For the most part this is done straightforwardly however
// a read of the KILL always gives a zero.

static const uint32_t ARB_KILL_REQ      (0x80000000);
static const uint32_t ARB_RD_OTHER_GRANT(0x00200000);
static const uint32_t ARB_RD_OWN_GRANT  (0x00100000);
static const uint32_t ARB_RD_OTHER_REQ  (0x00020000);
static const uint32_t ARB_RD_OWN_REQ    (0x00010000);
static const uint32_t ARB_REQUEST       (0x00000001);

// Bits in the broad cast setup register
// For a pleasant change, the read an write bits mean
// the same thing and are present for both


static const uint32_t BCST_ADDR_MASK(0xff000000);
static const uint32_t BCST_ADDR_SHIFT(24);
static const uint32_t BCST_ENA_MASTER(0X20);
static const uint32_t BCST_ENA_BCST(0X10);

// Hardware version register:

static const uint32_t HWVERS_IS_2(0x80);
static const uint32_t HWVERS_VERSION_MASK(0xf);
static const uint32_t HWVERS_VERSION_SHIFT(0);

// inline. to convert the temp registe value to Centigrade

static inline TEMP_TO_C(uint32_t value) {
    int16_t v(value & 0xffff);
    float result = v;
    result = result / 4.0;
    return result;
}
// Definitions for the one wire EEPROM. Among other things has
// the module serial number.

static const uint32_t WIRE1CSR_BUSY         (0x80000000);
static const uint32_t WIRE1CSR_SERIALVALID  (0x01000000);
static const uint32_t WIRE1CSR_SERIALNO_MASK(0x00ffff00);
static const uint32_t WIRE1CSR_SERIALNO_SHIFT(8);
static const uint32_t WIRE1CSR_REST_BUS(0x00000400);
static const uint32_t WIRE1CSR_WRITE   (0x00000200);
static const uint32_t WIRE1CSR_READ    (0x00000100);
static const uint32_t WIRE1CSR_DATA_MASK(0x000000ff);
static const uint32_t WIRE1CSR_DATA_SHIFT(0);

// Offsets to  data in the one wire eeeprom:

static const uint32_t WIRE1_SERIAL_LOW_OFFSET(0);
static const uint32_t WIRE1_SERIAL_HIGH_OFFSET(1);
static const uint32_t WIRE1_DHCPOPTION_OFFSET(2);

// To make things cofusing, foir the -2 variant, the
// onewire register is actually a n I2C control register.
// with a completely different bit layout:
// TO distinguish we'll prefix this as  I2C_ rather than WIRE1.
//
static const uint32_t I2C_BUSY(0X80000000);   //read
static const uint32_t I2C_DISABLE_TEMPAUTOREAD(0X02000000); // Write.
static const uint32_t I2C_TEMPAUTOREAD(0x02000000);         // Read.
static const uint32_t I2C_ENABLE_TEMPAUTOREAD(0x01000000);  // Write.
static const uint32_t I2C_READBYTE(0X00002000);             // WRITE
static const uint32_t I2C_WRITEBYTE(0X00001000);            // WRITE.
static const uint32_t I2C_STOP     (0X00000800);            // wRITE
static const uint32_t I2C_REPEAT_START(0X00000400);         // WRITE.
static const uint32_t I2C_START(0X00000200);                // WRITE.
static const uint32_t I2C_ACK_ON_READ(0X00000100);          // WRITE.
static const uint32_t I2C_ACK_RECEIVED(0X00000100);         // READ (*)
static const uint32_t I2C_DATA_MASK(0XF);
static const uint32_t I2C_DATA_SHIFT(0);

// * - The ack received bit is only valid fi the BUSY bit is not set.
//     further more it's documented as "Received Ack on write cycle"
//     so it may not be valid if AC_ON_READ was set with a READBYTE operation.


// yeah, yeah I know, there's all this crap about getting the serialn number
// and DHCP option from the EEPROM and, in fact, that's the only way to set that, however,
// this register allows those to be read without any of that rigmarole.
// There's also a note there that the MAC address for the module is 
// based on the serial number as:  00-00-56-31-6n-nn where n-nn is the serial
// number which can be 0-65535 -- but wait what is it if the serial number is
// 4096 or bigger when it will spill over into the top nybble of the 6n byte?
// Better hope they don't sell more than 4095 boards _sigh_
//
// The serialno register is readonly.
//
static const uint32_t SERNO_DHCP_MASK(0xff000000); // No docs here for what these bites
static const uint32_t SERNO_DHCP_SHIFT(24);        // are; maybe in the Ethernet manual.
static const uint32_t SERNO_512MBYTE(0x00800000);  // If set have 512 Mbytes memory.
static const uint32_t SERNO_INVALID(0x00010000);   // If set don't believe the serial #.
static const uint32_t SERNO_SERIAL_MASK(0x0000ffff);
static const uint32_t SERNO_SERIAL_SHIFT(0);

// The text around the internalxfrspeed has too many conditions on
// which modules can be set to what so I'm not going to document
// it and leave the module with whatever the hell power up does.


// Adc fpga boot control register.  There are only 2 write bits and
// status bits for each of the four FPGA.  Note the manual has a clear
// error labeling the error on boot bits.  I'm assuming these are in the
// same order as all the other status bits.

static const uint32_t ADCFPGABOOT_REBOOT(1);      // Write
static const  uint32_t ADCFPGABOOT_HALTBOOT(2);    // write.
                                            // rest are read
static const  uint32_t ADCFPGABOOT_FINISHED(0X01000000);
static const  uint32_t ADCFPGABOOT_DONE_4(0X00800000);
static const  uint32_t ADCFPGABOOT_DONE_3(0X00400000);
static const  uint32_t ADCFPGABOOT_DONE_2(0X00200000);
static const  uint32_t ADCFPGABOOT_DONE_1(0X00100000);

static const  uint32_t ADCFPGABOOT_ERROR_4(0X00080000); // misdocumented
static const  uint32_t ADCFPGABOOT_ERROR_3(0X00040000); // misdocumented
static const  uint32_t ADCFPGABOOT_ERROR_2(0X00020000); // misdocumented
static const  uint32_t ADCFPGABOOT_ERROR_1(0X00010000); // misdocumented

static const  uint32_t ADCFPGABOOT_BOOTING_4(0x00008000);
static const  uint32_t ADCFPGABOOT_BOOTING_3(0x00004000);
static const  uint32_t ADCFPGABOOT_BOOTING_2(0x00002000);
static const  uint32_t ADCFPGABOOT_BOOTING_1(0x00001000);

// Not going to document the SPI Flash control/data registers they're only
// used for firmware updates.

// The external veto/gate-delay register. Note that even though there
// are 16 bits for the gate/delay value.  Documentation says that values
// greater than 2044 are treated as 2044

static const uint32_t VETOGDG_ENABLE_FPBUS(0x80000000);
static const uint32_t VETOGDG_ENABLE_INTERNAL(0x40000000);
static const uint32_t VETOGDG_EXT_TRG_DEADTIME_MASK(0x3fff0000);
static const uint32_t VETOGDG_EXT_TRG_DEADTIME_SHIFT(16);
static const uint32_t VETOGDG_VETO_DELAY_MASK(0xffff);
static const uint32_t VETOGDG_VETO_DELAY_SHIFT(0);

// All of the clock i2c registers have the same layout.
// I hope to use the SIS library rather than having to figure
// the i2c crap out.  Below, however are the bytes for each
// of the clock speeds:

static const uint32_t CLOCK_250MHZ[6] = {
    0x20, 0xc2, 0xbc, 0x33, 0xe4, 0xf2
};
static const uint32_t CLOCK_125MHZ[6] = {
    0x21, 0xc2, 0xbc, 0x33, 0xe4, 0xf2
};

static const uint32_t CLOCK_I2C_BUSY(0X80000000);
static const uint32_t CLOCK_I2C_READ_PUTACK(0X2000);
static const uint32_t CLOCK_I2C_WRITE_GETACK(0X1000)
static const uint32_t CLOCK_I2C_STOP(0X0800);
static const uint32_t CLOCK_I2C_REPEAT_START(0X0400);
static const uint32_t CLOCK_I2C_START(0X0200);
static const uint32_t CLOCK_I2C_READ_ACK(0X100);
static const uint32_t CLOCK_I2C_DATA_MASK(0X00FF);
static const uint32_t CLOCK_I2C_DATA_SHIFT(0);

// The only thing the adcclockdistcontrol does is to enable/diable
// the clock distributiuon multiplexer.

static const uint32_t DISTCTCL_MUX_ENABLE_MASK(0x3);
static const uint32_t DISTCTCL_MUX_ENABLE_SHIFT(0);

// Values for the mux control bits:

static const uint32_t DISTCTCL_MUX_OSC(0);
static const uint32_t DISTCTCL_MUX_VXS(1);
static const uint32_t DISTCTCL_MUX_EXTFP(2);
static const uint32_t DISTCTCL_MUX_EXTNIM(3);

// The NIM clock multiplier is an SI 5325 chip its registers
// can be manipulated via the nimclockmult register which provides
// an SPI control register.  See well below for the 
// SI5325 register definitions.  Sure would be nice
// if the SIS library had some convenient functions for
// common apps but..

static const uint32_t NIMCLK_MULT_CMD_MASK(0xc0000000);
static const uint32_t NIMCLK_MULT_CMD_SHIFT(30);
static const uint32_t NIMCLK_MULT_RWBUSY(0x80000000);  // READ
static const uint32_t NIMCLK_MULT_RSTBUSY(0x40000000);  // READ
static const uint32_t NIMCLK_MULT_INT_C1B_STATUS(0X00010000); // READ.
static const uint32_t NIMCLK_MULT_INSTRUCTION_MASK(0X0000FF00);
static const uint32_t NIMCLK_MULT_INSTRUCTION_SHIFT(8);
static const uint32_t NIMCLK_MULT_ADR_DATA_MASK(0X000000FF);
static const uint32_t NIMCLK_MULT_ADR_DATA_SHIFT(0);


// FP Bus control register.
// All bits are read/write.

static const uint32_t FPCTL_CLK_OUT_NIM(0x20);  // if set sample out from NIM.
static const uint32_t FPCTL_CLK_OUT_ENA(0x10);  // If set output sample clock -> FP bus.
static const uint32_t FPCTL_STATUS_OUTENA(0x02);  // Enable status out -> FP
static const uint32_t FPCTL_CTL_OUTENA(0x01);    // Enable CTCL out -> FP.

// Nim input control status register.

static const uint32_t NIMICSR_UI(0x02000000);
static const uint32_t NIMICSR_EXTUI(0x01000000);
static const uint32_t NIMICSR_TI(0x00200000);
static const uint32_t NIMICSR_EXTTI(0x00100000);
static const uint32_t NIMICSR_CI(0x00020000);
static const uint32_t NIMICSR_EXTCI(0x00010000);
static const uint32_t NIMICSR_TIUI_COUNTER_ENA(0x00008000);
static const uint32_t NIMICSR_EXTTRG_DTLOGIC_ENA(0X00004000);
static const uint32_t NIMICSR_UI_PPS_ENA(0X00002000);
static const uint32_t NIMICSR_UI_VETO_ENA(0X00001000);
static const uint32_t NIMICSR_UI_FUNCTION(0X00000800);
static const uint32_t NIMICSR_UI_LEVEL(0X00000400);
static const uint32_t NIMICSR_UI_INVERT(0X00000200);
static const uint32_t NIMICSR_UI_TSCLEAR(0X0000100);
static const uint32_t NIMICSR_TI_FUNCTION(0X80);
static const uint32_t NIMICSR_TI_LEVEL(0X40);
static const uint32_t NIMICSR_TI_INVERT(0X20);
static const uint32_t NIMICSR_TI_TRGENA(0X10);
static const uint32_t NIMICSR_CI_FUNCTION(0X8);
static const uint32_t NIMICSR_CI_LEVEL(4);
static const uint32_t NIMICSR_CI_INVERT(2);
static const uint32_t NIMICSR_CI_ENABLE(1);


// Acquistiohn contro and status register. The
// top 16 bits of this register are read-onliy 
// the bottom 16 are read-write.

static const uint32_t ACQCSR_ADDR_THRESHOLD_13_16(0X80000000);
static const uint32_t ACQCSR_SAMPLE_BUSY_13_16(0X40000000);
static const uint32_t ACQCSR_ADDR_THRESHOLD_9_12(0X20000000);
static const uint32_t ACQCSR_SAMPLE_BUSY_9_12(0X10000000);
static const uint32_t ACQCSR_ADDR_THRESHOLD_5_8(0X08000000);
static const uint32_t ACQCSR_SAMPLE_BUSY_5_8(0X040000000);
static const uint32_t ACQCSR_ADDR_THRESHOLD_1_4(0X02000000);
static const uint32_t ACQCSR_SAMPLE_BUSY_1_4(0X01000000);

static const uint32_t ACQCSR_PPS_LATCH(0X00800000);
static const uint32_t ACQCSR_BSWA_NIM(0X00400000);
static const uint32_t ACQCSR_FPBUS_ADDDR_THRESHOLD(0X00200000);
static const uint32_t ACQCSR_FPBUS_SAMPLING(0X00100000);

static const uint32_t ACQCSR_ADDDR_THRESHOLD_OR(0X00080000);
static const uint32_t ACQCSR_SAMPLE_BUSY_OR(0X00040000);
static const uint32_t ACQCSR_BANK2_ARMED(0X00020000);
static const uint32_t ACQCSR_ARMED(0X00010000);

static const uint32_t ACQCSR_EXTTRG_DISABLE(0X00008000);
static const uint32_t ACQCSR_INTERNALTRG_TO_EXTTRG(0X00004000);
static const uint32_t ACQCSR_NIMUI_ISBANKSWAP(0X00002000);
static const uint32_t ACQCSR_NIMTI_ISBANKSWAP(0X00001000);

static const uint32_t ACQCSR_LOCALVETO_ISVETO(0X00000800);
static const uint32_t ACQCSR_EXT_TSTAMPCLR_ENA(0X00000400);
static const uint32_t ACQCSR_EXTTRG_IS_VETO(0X0000200);
static const uint32_t ACQCSR_EXTTRG_IS_TRG(0X00000100);

static const uint32_t ACQCSR_FPIN_SAMPLECONTROL_ENABLE(0X00000080);
static const uint32_t ACQCSR_FPIN_CTRL2_ENABLE(0X00000040);
static const uint32_t ACQCSR_FPIN_CTRL1_ISVETO(0X00000020);
static const uint32_t ACQCSR_FPIN_CTRL1_ISTRG(0X00000010);

static const uint32_t ACQCSR_SINGLE_BANK_MODE(0X00000001);  // Not implemented yet?


// Control/status of the coincidence lookup table. The coincidence lengths
// are (value+1)*sample_period.
// note that setting the clear bit will require a 525usec delay to complete.

static const uint32_t LUTCSR_CLEAR(0X80000000);  // NOTE DELAY 525usec after this.
static const uint32_t LUTCSR_COINC2_LENGTH_MASK(0XFF00);
static const uint32_t LUTCSR_COINC2_LENGTH_SHIFT(8);
static const uint32_t LUTCSR_COINC1_LENGTH_MASK(0X00FF);
static const uint32_t LUTCSR_COINC1_LENGTH_SHIFT(0);

//  The lookup table address register actually has two things:
// The masks of triggers we care about for the trigger pattern
// the address of a trigger pattern within the table.
// the address is actually a trigger pattern.
// One writes this register then writes the s_coinclutdata to load
// that address.  There are 65536 LUT entries, one for each possible
// trigger pattern.

static const uint32_t LUTADDR_CHANMASK_MASK(0XFFFF0000);
static const uint32_t LUTADDR_CHANMASK_SHIFT(16);
static const uint32_t LUTADDR_ADDR_MASK(0XFFFF);
static const uint32_t LUTADDR_ADDR_SHIFT(0);

// The data register provides, for each trigger, which edge we care about,
// The validation bits for each of the two triggers.
// Note that after a write to the s_coinclutdata register the address in the
// s_coinclutaddr register is incremented without changing the mask.

static const uint32_t LUTDATA_FALLING_MASK(0XFFFF0000);   // bits set are falling edge.
static const uint32_t LUTDATA_FALLING_SHIFT(16);
static const uint32_t LUTDATA_VALID2(0X2);
static const uint32_t LUTDATA_VALID1(0X2);

// s_lemocoselect:
// The CO NIMOUT function is selectable.  One can select CO_SAMPLE_CLK in which
// case, if I understand correctly, regardless of the other bits in this registrer,
// The sample clock is routed to CO, if that bit is not set, the OR of the other
// selectec conditions is routed to CO:

static const uint32_t CO_SET(0X04000000);      // always asserted.
static const uint32_t CO_SAMPLEBANK2(0X00400000);
static const uint32_t CO_SAMPLING_ARMED(0X00200000);
static const uint32_t CO_SWAP_WITH_NIM(0X00100000);
static const uint32_t CO_HETRG_13_16(0X00080000);
static const uint32_t CO_HETRG_9_12(0X00040000);
static const uint32_t CO_HEGTRG5_8(0X00020000);
static const uint32_t CO_HETRG_1_4(0X00010000);
static const uint32_t CO_SAMPLE_CLK(0X00000001);

// s_lemotoselect:
//   The or of the selected conditions is presented at the
// TO nim output.
//

static const uint32_t TO_PULSE(0x80000000); 
static const uint32_t TO_SET(0x40000000);
static const uint32_t TO_EXTTSCLEAR(0x10000000);

static const uint32_t TO_EXTVETO(0x08000000);
static const uint32_t TO_EXTTRG(0x04000000);
static const uint32_t TO_EXTTRG_STRETCHED(0X02000000);
static const uint32_t TO_LUT1_STRETCHED_COINC(0X01000000);

static const uint32_t TO_SAMPLE_BANK2(0x00400000);
static const uint32_t TO_SAMPLE_ARMED(0X00200000);
static const uint32_t TO_NIM_SWAPCTL(0X00100000);

static const uint32_t TO_SUM_TRG_13_16(0x00080000);
static const uint32_t TO_SUM_TRG_9_12(0X00040000);
static const uint32_t TO_SUM_TRG_5_8(0X00020000);
static const uint32_t TO_SUM_TRG_1_4(0X00010000);

   // Channel trigger bits -> TO

static const uint32_t TO_TRG_16(0X00008000);
static const uint32_t TO_TRG_15(0X00004000);
static const uint32_t TO_TRG_14(0X00002000);
static const uint32_t TO_TRG_13(0X00001000);
static const uint32_t TO_TRG_12(0X00000800);
static const uint32_t TO_TRG_11(0X00000400);
static const uint32_t TO_TRG_10(0X00000200);
static const uint32_t TO_TRG_9(0X00000100);
static const uint32_t TO_TRG_8(0X000080);
static const uint32_t TO_TRG_7(0X000040);
static const uint32_t TO_TRG_6(0X000020);
static const uint32_t TO_TRG_5(0X000010;);
static const uint32_t TO_TRG_4(0X00008);
static const uint32_t TO_TRG_3(0X00004);
static const uint32_t TO_TRG_2(0X00002);
static const uint32_t TO_TRG_1(0X00001);

//s_lemouoselect
//   Bits that define the output of UO.  The output is the OR of the
// conditions described in the register.

static const uint32_t UO_PULSE_3(0X80000000);
static const uint32_t UO_SET(0X40000000);
static const uint32_t UO_EXTTS_CLEAR(0x10000000);

static const uint32_t UO_EXT_VETO(0X08000000);
static const uint32_t UO_EXT_TRG(0X04000000);
static const uint32_t UO_PRESCALER(0X02000000);
static const uint32_t UO_LUT2_STRETCHED_COINC(0X01000000);

static const uint32_t UO_SAMPLE_BANK2(0X00400000);
static const uint32_t UO_SAMPLE_ARMED(0X00200000);
static const uint32_t UO_NIMSWAPCTL(0X00100000);

static const uint32_t UO_HETRG_13_16(0X00080000);
static const uint32_t UO_HETRG_9_12(0X00040000);
static const uint32_t UO_HETRG_5_8(0X00020000);
static const uint32_t UO_HETRG_1_4(0X00010000);

static const uint32_t UO_PULSE_12(0X00004000);
static const uint32_t UO_PULSE_8(0X00002000);
static const uint32_t UO_PULSE_4(0X00001000);


static const uint32_t UO_VETOED(0X00000200);
static const uint32_t UO_GATE(0x00000100);

static const uint32_t UO_EXT_TS_CLR_ASSERTED(0X00000080);
static const uint32_t UO_SAMPLING(0X00000010);

static const uint32_t UO_ADDR_THRESHOLD(0X8);
static const uint32_t UO_SAMPLE_BUSY(0X4);
static const uint32_t UO_LOGIC_DBL_ARMED(0X2);
static const uint32_t UO_LOGIC_SNGL_ARMED(0X1);


// s_trfeedbackselect
// Conditions with bits set are ORd and fed back to the
// trigger logic as the trigger feedback which can be used
// as an external trigger.


static const uint32_t TRFEEDBACK_LUT1(0x01000000); 

static const uint32_t TRFEEDBACK_SUM_STRETCHED_13_16(0x00080000);
static const uint32_t TRFEEDBACK_SUM_STRETCHED_9_12(0x00040000);
static const uint32_t TRFEEDBACK_SUM_STRETCHED_5_8(0x00020000);
static const uint32_t TRFEEDBACK_SUM_STRETCHED_1_4(0x00010000);

static const uint32_t TRFEEDBACK_INTERNAL_16(0X00008000);
static const uint32_t TRFEEDBACK_INTERNAL_15(0X00004000);
static const uint32_t TRFEEDBACK_INTERNAL_14(0X00002000);
static const uint32_t TRFEEDBACK_INTERNAL_13(0X00001000);

static const uint32_t TRFEEDBACK_INTERNAL_12(0X00000800);
static const uint32_t TRFEEDBACK_INTERNAL_11(0X00000400);
static const uint32_t TRFEEDBACK_INTERNAL_10(0X00000200);
static const uint32_t TRFEEDBACK_INTERNAL_9(0X00000100);

static const uint32_t TRFEEDBACK_INTERNAL_8(0X00000080);
static const uint32_t TRFEEDBACK_INTERNAL_7(0X00000040);
static const uint32_t TRFEEDBACK_INTERNAL_6(0X00000020);
static const uint32_t TRFEEDBACK_INTERNAL_5(0X00000010);

static const uint32_t TRFEEDBACK_INTERNAL_4(0X00000008);
static const uint32_t TRFEEDBACK_INTERNAL_3(0X00000004);
static const uint32_t TRFEEDBACK_INTERNAL_2(0X00000002);
static const uint32_t TRFEEDBACK_INTERNAL_1(0X00000001);

// The module has internal memory associated with each FADC.
// to read the module requires transferring data from that memory
// to the FIFO memory from which the VME can read it out.
//  This is done using the transfer control registers.  There is one
// for each of the groups of 4 channels (s_adcnnnndataqxferctl).
// Within each 4 channel gropu there are three memory spaces.
// space 0 is the first two channels of the group, space
// 1 is for the second two channels of the group and 
// space 3 transfers a set of statistics counters.
//  The bottom 27 bits represent the address within the space
//  at which
// the data transfer should start.

// Command values to or in:

static const uint32_t DATAXFERCTL_RESET(0x00000000);
static const uint32_t DATAXFERCTL_READ(0X80000000);
static const uint32_t DATAXFERCTL_WRITE(0xc0000000);

// Address spaces to or in:

static const uint32_t DATAXFERCTL_FIRSTPAIR(0X00000000);
static const uint32_t DATAXFERCTL_SECONDPAIR(0X10000000);
static const uint32_t DATAXFERCTL_STATISTICS(0X30000000);

static const uint32_t DATAXFERCTL_ADDRESS_MASK(0X0FFFFFFF);
static const uint32_t DATAXFERCTL_ADDRESS_SHIFT(0);

// So to read offset 0x10 for the first pair of digitizers one would write:
// DATAXFERCTL_READ | DATAXFERCTL_FIRSTPAIR | 0x10  
// not saying this is a reasonable thing to do.  It's just an example.


// Each group of four digitizers also has a transfer status register.
// s_adcnnnndataxfersr. These show the status of any transfer in progress
// from the ADC FPGA to the fifo.    Presumably the units of the counter
// field are 32 bit words as there are two fewer bits than the address.
//
// All have the same bits:

static const uint32_t DATAXFERSR_BUSY(0X80000000);
static const uint32_t DATAXFERSR_TOADC(0X40000000);
static const uint32_t DATAXFERSR_FIFO_ALMOST_FULL(0X10000000);

    // Not sure what the next two bits are about
    // I'll label them as described but they are bits not fields.

static const uint32_t DATAXFERSR_MAXPENDING(0X08000000);
static const uint32_t DATAXFERSR_NUMPENDING(0X04000000);
static const uint32_t DATAXFERSR_COUNTER_MASK(0X03FFFFFF);
static const uint32_t DATAXFERSR_COUNTER_SHIFT(0);


// s_vmeadcfpgalinkstatus
//   Provides status and the ability to clear latched
// error bits.  The Error bits are read/write.  Reading
// show status and writing clears the latched status.
// e.g. ADCLINKSTAT_FRAME_ERR_1 When read as 1 indicates
// a latched frame error for ADC FPGA1's data link.
// When that bit is written, the latched bit is cleared -- until
// the next occurance of a frame error.
// Note to make naming harder, there are, what I _think_ are
// instantaneous error flags as well.  Those are named
//   ADCLINKSTAT_error_name_NOW_n
// To distinguish them frolm the latched bits.

        // FPGA4
static const uint32_t ADCLINKSTAT_FRAME_ERR_4(0X80000000);
static const uint32_t ADCLINKSTAT_SOFT_ERR_4(0X40000000);
static const uint32_t ADCLINKSTAT_HARD_ERR_4(0X20000000);
static const uint32_t ADCLINKSTAT_LANE_UP_4(0X10000000);        //RO.
static const uint32_t ADCLINKSTAT_CHAN_UP_4(0X08000000);       //RO
static const uint32_t ADCLINKSTAT_FRAME_ERR_NOW_4(0X04000000); //ro
static const uint32_t ADCLINKSTAT_SOFT_ERR_NOW_4(0x02000000);  //ro
static const uint32_t ADCLINKSTAT_HARD_ERR_NOW_4(0x0100000);   // ro

    // FPGA3

static const uint32_t ADCLINKSTAT_FRAME_ERR_3(0X00800000);
static const uint32_t ADCLINKSTAT_SOFT_ERR_3(0X00400000);
static const uint32_t ADCLINKSTAT_HARD_ERR_3(0X00200000);
static const uint32_t ADCLINKSTAT_LANE_UP_3(0X00100000);        //RO.
static const uint32_t ADCLINKSTAT_CHAN_UP_3(0X00080000);       //RO
static const uint32_t ADCLINKSTAT_FRAME_ERR_NOW_3(0X00040000); //ro
static const uint32_t ADCLINKSTAT_SOFT_ERR_NOW_3(0x00020000);  //ro
static const uint32_t ADCLINKSTAT_HARD_ERR_NOW_3(0x00010000);   // ro

    // FPGA2:

static const uint32_t ADCLINKSTAT_FRAME_ERR_2(0X00008000);
static const uint32_t ADCLINKSTAT_SOFT_ERR_2(0X00004000);
static const uint32_t ADCLINKSTAT_HARD_ERR_2(0X00002000);
static const uint32_t ADCLINKSTAT_LANE_UP_2(0X000001000);        //RO.
static const uint32_t ADCLINKSTAT_CHAN_UP_2(0X00000800);       //RO
static const uint32_t ADCLINKSTAT_FRAME_ERR_NOW_2(0X00000400); //ro
static const uint32_t ADCLINKSTAT_SOFT_ERR_NOW_2(0x00000200);  //ro
static const uint32_t ADCLINKSTAT_HARD_ERR_NOW_2(0x00000100);   // ro

    // FPGA1

static const uint32_t ADCLINKSTAT_FRAME_ERR_1(0X80);
static const uint32_t ADCLINKSTAT_SOFT_ERR_1(0X40);
static const uint32_t ADCLINKSTAT_HARD_ERR_1(0X20);
static const uint32_t ADCLINKSTAT_LANE_UP_1(0X10);        //RO.
static const uint32_t ADCLINKSTAT_CHAN_UP_1(0X08);       //RO
static const uint32_t ADCLINKSTAT_FRAME_ERR_NOW_1(0X04); //ro
static const uint32_t ADCLINKSTAT_SOFT_ERR_NOW_1(0x02);  //ro
static const uint32_t ADCLINKSTAT_HARD_ERR_NOW_1(0x01);   // ro

// s_adcfpgaspibusystatus:
// Bits are readonly:

static const uint32_t ADCFGPGASPISTAT_BUSY_OR(0X80000000);
static const uint32_t ADCFGPGASPISTAT_BUSY_4(0X8);
static const uint32_t ADCFGPGASPISTAT_BUSY_3(0X4);
static const uint32_t ADCFGPGASPISTAT_BUSY_2(0X2);
static const uint32_t ADCFGPGASPISTAT_BUSY_1(0X1);

// s_plldlpcontrol -Control/status of the PLL chip for the
// sample clock.
// SOme bits are read only some are write only as shown
// in the comments below.
static const uint32_t PLLDRP_LOCKED(0X40000000);  // ro
static const uint32_t PLLDRP_CLR_RESET(0x20000000); // wo
static const uint32_t PLLDRP_RESET(0x02000000);    // rw
static const uint32_t PLLDRP_ENABLE(0x01000000);   // wo
static const uint32_t PLLDRP_WR_ENABLE(0x00800000); // wo
static const uint32_t PLLDRP_ADDRESS(0x007f0000);  // wo
static const uint32_t PLLDRP_DATA_MASK)0x0000ffff); // rw
static const uint32_t PLLDRP_DATA_SHIFT(0);

// The s_prescalerdivider is just 32 bit of data.   If 0,
// the prescale divider is disabled.

// s_prescalerlength is just 32 bits of length.
// the actual length is (value+1) sample clocks.

/////////////////////////////////////////////////////////////////////////////////////////////////
// Registers that control the ADC FPGAs -- there are four of those, one for each
// four channel ADC group.  See the adcFPGARegisters struct for the layout of the registers relative
// to each base address.

// s_inputTapDelay
//    I believe this is a write only register.  Supposedly when the sample clock source or freq
// has been changed one must issue a calibration tand then write the tap (calibration bit off?).
// THe meaning of the delay depends on the digitzer type:
//  for the 3316 it's value * 40s with a maxiumum value of 1/2 clock period.
// for the 3316-2 it's value * 78ps.  Presumably the same max value given the 1/2 sample bit.

static const uint32_t TAPDELAY_ADD_HALF_SAMPLE_DELAY(0X1000);
static const uint32_t TAPDELAY_CALIBRATE(0X800);    // not used on 3316-2 delay 20 sample clocks.
static const uint32_t TAPDELAY_CLR_LINKERRS(0X400);
static const uint32_t TAPDELAY_SELECT_3_4(0X200);     // Select second pair of channels.
static const uint32_t TAPDELAY_SELECT_1_2(0X100);     // Select5d first pair of channels.
static const uint32_t TAPDELAY_DELAY_MASK(0X0FF);     // 4 Bits for a 3316-2.
static const uint32_t TAPDELAY_DELAY_SHIFT(0);

// s_gainTerminationcontrol
//   Note that the gain values must be shifted into position for the appropriate channel
//   to be  meaningful and, on read, maked and shited right to be compared with values.
//
//  GTC_ standas for Gain,Termination Control below.

static const uint32_t GTC_1KOHM_4(0X04000000);   // If not set, termination is 50ohms.
static const uint32_t GTC_GAIN_4_MASK(0X03000000);
static const uint32_t GTC_GAIN_4_SHIFT(24);      // See gain/range values values below.

static const uint32_t GTC_1KOHM_3(0X040000);   // If not set, termination is 50ohms.
static const uint32_t GTC_GAIN_3_MASK(0X030000);
static const uint32_t GTC_GAIN_4_SHIFT(16);      // See gain/range values values below.

static const uint32_t GTC_1KOHM_2(0X0400);   // If not set, termination is 50ohms.
static const uint32_t GTC_GAIN_2_MASK(0X0300);
static const uint32_t GTC_GAIN_2_SHIFT(8);      // See gain/range values values below.

static const uint32_t GTC_1KOHM_1(0X04);   // If not set, termination is 50ohms.
static const uint32_t GTC_GAIN_1_MASK(0X03);
static const uint32_t GTC_GAIN_1_SHIFT(0);      // See gain/range values values below.

static const uint32_t GTC_RANGE_5V(0);
static const uint32_t GTC_RANGE_2V(1);
static const uint32_t GTC_RANGE_1_9V(2);    // Note 3 is also 1.9 V.

// s_DCOffset 
//    well the DAC register is needlessly  complex.  Rather than just having
//    a set of values you can write, there are these stupid command and mode
//    bits... really just 4 DCOffset registers you could write would
//    be so much simpler.

static const uint32_t DACCTL_MODE_MASK(0XE0000000);
static const uint32_t DACCTL_MODE_SHIFT(31);

static const uint32_t DACCTL_COMMAND_MASK(0x0F000000);
static const uint32_t DACCTL_COMMAND_SHIFT(24);

static const uint32_t DACCTL_ADDRESS_MASK(0X00F00000);
static const uint32_t DACCTL_ADDRESS_SHIFT(20);

static const uint32_t DACCTL_DATA_MASK(0X000FFFF0);   // sigh couldn't just make this the bottom 16 bits.
static const uint32_t DACCTL_DATA_SHIFT(4);

    // Mode field meanings;  must be shifted into position
    //  e.g. DACCTL_MODE_LDAC << DACCTL_MODE_SHIFT
    //

static const uint32_t DACCTL_MODE_WRITE(4);
static const uint32_t DACCTL_MODE_WRITE_WLOAD(5);
static const uint32_t DACCTL_LOAD(6);
static const uint32_t DACCTL_CLEAR(7);

    // command values must be shifted by DAQ_COMMAND_SHIFT

static const uint32_t DACCTL_CMD_WRITE_INPUT(0);
static const uint32_t DACCTL_CMD_UPDATE(1);
static const uint32_t DACCTL_CMD_WRITE_AND_UPDATE_ALL(2);
static const uint32_t DACCTL_CMD_WRITE_UPDATE(3);
static const uint32_t DACCTL_CMD_POWER_CYCLE(4);
static const uint32_t DACCTL_CMD_LD_CLEARCODE_REG(5);
static const uint32_t DACCTL_CMD_LDAC_REG(6);
static const uint32_t DACCTL_CMD_RESET(7);
static const uint32_t DACCTL_CMD_SETUP_DCEN(8);
static const uint32_t DACCTL_CMD_NOOP(9);

    // Adress registser values:

static const uint32_t DACCTL_ADDR_DACA(0); // For adc1.
static const uint32_t DACCTL_ADDR_DACB(1); // For adc2 etc .
static const uint32_t DACCTL_ADDR_DACC(2);
static const uint32_t DACCTL_ADDR_DACD(3);
static const uint32_t DACCTL_ADDR_ALL(0XF);  // Broadcast to all dacs.

    // THe following are full scale values for the ranges:
    // The DAC is subtracted from the inputs.
static const uint32_t DAC_RANGE_5V_MAX(0XFFFF);    // 0 - 5V
static const uint32_t DAC_RANGE_5V_MID(0X800);     // -2.5 - 2.5V
static const uint32_t DAC_RANGE_5V_MIN(0);         // -5V - 0

static const uint32_t DAC_RANGE_2V_MAX(5200);      // 0 - 2v
static const uint32_t DAC_RANGE_2V_MID(0X8000);    // -1 - 1v
static const uint32_t DAC_RANGE_2V_MIN(1300);      // -2 - 0v

// The DAC readback register is just the DAC value with no need to shift
// unused bits are defined as zero but:

static const uint32_t DAC_READBACK_MASK(0X0000FFFF);
static const uint32_t DAC_READBACK_SHIFT(0);

// s_SPIControl.
//    All bits are read/write.  Operations require 23usec.

static uint32_t ADCSPI_COMMAND_MASK(0xc0000000);
static uint32_t ADCSPI_COMMAND_SHIFT(30);

static uint32_t ADCSPI_ADCOUT_ENABLE(0X01000000);
static uint32_t ADCSPI_SEL34(0X00400000);
static uint32_t ADCSPI_ADDR_MASK(0X0001FF00);
static uint32_t ADCSPI_ADDR_SHIFT(8);
static uint32_t ADCSPI_DATA_MASK(0X0000000F);
static uint32_t ADCSPI_DATA_SHIFT(0);

    // command values must be positioned using ADCSPI_COMMAND_SHIFT

static uint32_t ADCSPI_CMD_WRITE(2);
static uint32_t ADCSPI_CMD_READ(3);

    // The readback register just has the data:

static uint32_t ADCSPI_READBACK_MASK(0X000000FF);
static uint32_t ADCSPI_READBACK_SHIFT(0);


// s_eventConfig.
//   Each adc in the group has 8 bits of control information.
//   we provide definitions of each of the bits and then
//   mask/shifts for each of the 4 ADCs in an FPGA group.

static const uint32_t ADCCFG_INVERT(1);
static const uint32_t ADCCFG_SUMTRG_ENABLE(2);
static const uint32_t ADCCFG_INTERNAL_TRG_ENABLE(4);
static const uint32_t ADCCFG_EXTERN_TRG_ENABLE(8);
static const uint32_t ADCCFG_INTERNAL_GATE1_ENABLE(0X10);
static const uint32_t ADCCFG_INTERNAL_GATE2_ENABLE(0X20);
static const uint32_t ADCCFG_EXTERNAL_GATE_ENABLE(0X40);
static const uint32_t ADCCFG_EXTERNAL_VETO_ENABLE(0X80);

static const uint32_t ADCCFG_CH1_MASK(0X000000FF);
static const uint32_t ADCCFG_CH1_SHIFT(0);

static const uint32_t ADCCFG_CH2_MASK)0X0000FF00);
static const uint32_t ADCCFG_CH2_SHIFT(8);

static const uint32_t ADCCFG_CH3_MASK(0X00FF0000);
static const uint32_t ADCCFG_CH3_SHIFT(16);

static const uint32_t ADCCFG_CH4_MASK(0XFF000000);
static const uint32_t ADCCFG_CH4R_MASK(24);

// For whatever reason the next documented register in the
// manual is thes_extendedEventconfig Same idea as above
// but only two meaningful bits:

static const uint32_t ADCEXTCFG_SAVE_RAWFIRST(0X10);
static const uint32_t ADCEXTCFG_INTERNAL_PUPFIRST(1);

static const uint32_t ADCEXTCFG_CH1_MASK(0X000000FF);
static const uint32_t ADCEXTCFG_CH1_SHIFT(0);

static const uint32_t ADCEXTCFG_CH2_MASK)0X0000FF00);
static const uint32_t ADCEXTCFG_CH2_SHIFT(8);

static const uint32_t ADCEXTCFG_CH3_MASK(0X00FF0000);
static const uint32_t ADCEXTCFG_CH3_SHIFT(16);

static const uint32_t ADCEXTCFG_CH4_MASK(0XFF000000);
static const uint32_t ADCEXTCFG_CH4R_MASK(24);

// s_chanelHeaderId
//   This is another typical piece of crap.
//   Sure, there are a bunch of bits. but really
//   you _must_ set bits 2-3 to the ADC group number
//   and bits, 0-1 are set to the channel within the group
//   What they should do is only give you what are now
//    bits 4-11 and set the bottom bits for you 
//    without giving you a chance to set the group bits 
//    incorrectly or claiming you can set the adc bits.
//   We'll try to capture that here:

static const uint32_t ID_FREE_MASK(0XFF000000);  // Set these to anything.
static const uint32_t ID_FREE_SHIFT(24);
static const uint32_t ID_MBGROUPNO_MASK(0X00C00000); // must be group #
static const uint32_t ID_MBGROUPNO_SHIFT(22);
    // Leave the channel bits out of it.

// s_endThreshold - limits data in conjunction with the top bit, and determines when
//   the address threshold flag is set in the appropriate register.

static const uint32_t ADDRTHRESH_SUPPRESSMORE(0x80000000); 
static const uint32_t ADDRTHRESH_END_MASK(0X0FFFFFFF);
static const uint32_t ADDRTHRESH_END_SHIFT(0);

// s_triggerGatelength length of the active trigger gate window.
// the length is 2+value sampling clocks.

static const uint32_t GATELEN_MASK(0XFFFFFFFF);
static const uint32_t GATELEN_SHIFT(0);

// s_dataconfig
//   configures the storage of the raw waveform data.
//   note that only an even number of samples (0 is even) can be stored
//   because the path to the memory stores two samples per transfer. Furthermore,
//   the start index must be even.
//   twere me I'd ignore those bottom bits and the manual claims that happens
//    but....(I think?!?).

static const uint32_t DATACFG_LENGTH_MASK(0XFFFF0000);
static const uint32_t DATACFG_LENGTH_SHIFT(16);
static const uint32_t DATACFG_START_ADDR_MASK(0X0000FFFF);
static const uint32_t DATACFG_START_ADDR_SHIFT(0);



// Si5325 clock mulitplier chip definitions.

// Here are the SPI instructions recognized by the 5325 from
// https://www.skyworksinc.com/-/media/Skyworks/SL/documents/public/reference-manuals/si53xx-reference-manual.pdf
// They are defined as 32 bit integers to support masking and shifting
// into SIS3316 registers without casting.
static const uint32_t SI5325_SPI_SET_ADDR(0X0);  // addr/data are the rdwr address.
static const uint32_t SI5325_SPI_WRITE(0X40);    // Write addr/data are te data.
static const uint32_t SI5325_SPI_WRADDR_INCR(0xc0);  // increment the write address by the data.
static const uint32_t SI5325_SPI_READ(0X80);     // read from the read address.
static const uint32_t SI5325_SPI_RDADDR_INCR(0xa0); // Increment the read address by the data.

// s_pupconfig
//   Specifies the length of the pileup and re-pileup windows.

static const uint32_t PUP_REPILEUP_WIN_MASK(0xffff0000);   // Even values only though.
static const uint32_t PUP_REPILEUP_WIN_SHIFT(16);
static const uint32_t PUP_PILEUP_WIN_MASK(0X0000FFFF);     //Even values only.
static const uint32_t PUP_PILEUP_WIN_SHIFT(0);


// s_pretrigger  - register that sets up the pretrigger delay.

static const uint32_t PRETRIG_ADDITIONAL_ENA(0X8000);   // add  "delay of FIR trigger P+G bit"
static const uint32_t PRETRIG_DELAY_MASK(0x3fff);       // note must be even.
static const uint32_t PRETRIG_DELAY_SHIFT(0);

// s_averageconfig - configures trace averaging to reduce the signal
// noise sent to the DPP. 
// In another rather crappy thing, 
// This register has compleetly different meaning depdngin on the
// model number:


    // SI3316-125MHz 16Bit meanings:

static const uint32_t AVG_MODE_MASK(0X70000000);
static const uint32_t AVG_MODE_SHIFT(28);
static const uint32_t AVG_PRETRIGGER_MASK(0X0FFF0000);
static const uint32_t AVG_PRETRIGGER_SHIFT(16);
static const uint32_t AVG_LENGTH_MASK(0X0000FFFF);   // Must be even.
static const uint32_t AVG_LENGTH_SHIFT(0);

    // Must be shifted in place via AVG_MODE_SHIFT.

static const uint32_t AVG_MODE_DISABLED(0);
static const uint32_t AVG_MODE_4(1);
static const uint32_t AVG_MODE_8(2);
static const uint32_t AVG_MODE_16(3);
static const uint32_t AVG_MODE_32(4);
static const uint32_t AVG_MODE_64(5);
static const uint32_t AVG_MODE_128(6);
static const uint32_t AVG_MODE_256(7);

    // But wait -- there's more.  For the SIS3316-2 250MHZ 14bit
    // module thsi register can configure averaging and decimation:
    // with 5 bits per channel in the group. defined:

static const uint32_t DECIMATION_ENABLE(0X10);  // If clr averaging.
static const uint32_t DECIMATION_MASK(0XF);
static const uint32_t DECIMATION_4(0);
static const uint32_t DECIMATION_8(1);
static const uint32_t DECIMATION_16(2);
static const uint32_t DECIMATION_32(3);
static const uint32_t DECIMATION_64(4);
static const uint32_t DECIMATION_128(5);
static const uint32_t DECIMATION_256(6);
static const uint32_t DECIMATION_512(7);
static const uint32_t DECIMATION_2(8);    // actually any value  with the 8s bit set.


    // Now shift counts for each of the channels:  apply to both
    // DECIMATION_ENABLE and whatever mode is used:

static const uint32_t DECIMATION_CH4_SHIFT(24);
static const uint32_t DECIMATION_CH3_SHIFT(16);
static const uint32_t DECIMATION_CH2_SHIFT(8);
static const uint32_t DECIMATION_CH1_SHIFT(0);

// s_format
//    Each channel in the group has 
//    7 bits defined below along with masks and
//    shifts for each of the four channels in the group. 
//     The bits determine what each
//     trigger saves in memory (and hence the FIFO).
///    note:  MAW:  Moving average window.

    // a note ABOUT EMAW_TESTBIT:
    // this is not an on/off bit:
    //  But determines what is selected from the processing
    //  of the moving average window:
    // If 0 the FIR trigger is selected.
    // if 1 the FIR energy is selcted.
static const uint32_t DFMT_STORE_COUNTER(0X40);
static const uint32_t DFMT_STORE_EMWAW_TESTBIT(0X20);
static const uint32_t DFMT_STORE_MAW_TEST_ENABLE(0X10);

static const uint32_t DFMT_STORE_MAW_ENERGIES(0X8);
static const uint32_t DFMT_STORE_MAW_TRIGGER(0X4);
static const uint32_t DFMT_STORE_ACCUMULATORS(0X2);
static const uint32_t DFMT_STORE_PEAKS(0X1);

    // Now the masks and shifts for each
    // channel in the group
    

static const uint32_t DFMT_CH4_MASK(0X7F000000);
static const uint32_t DFMT_CH4_SHIFT(24);
static const uint32_t DFMT_CH3_MASK(0X007F0000);
static const uint32_t DFMT_CH3_SHIFT(16);
static const uint32_t DFMT_CH2_MASK(0X00007F00);
static const uint32_t DFMT_CH2_SHIFT(8);
static const uint32_t DFMT_CH1_MASK(0X0000007F);
static const uint32_t DFMT_CH1_SHIFT(0);

//  s_MAWtestconfig
//   While the name implies  this is for testing,
//   It nonetheless seems important:
//   Values are common for all 4 channels in a group.
//   One can set pretrigger delays, and lengths of
//   the MAW test buffer.  These lengths must be
//   even;  that their bottom bits must be 0.

static const uint32_t MAWTESTCFG_PRETRIG_DELAY_MASK(0x01FF0000);
static const uint32_t MAWTESTCFG_PRETRIG_DELAY_SHIFT(16);
static const uint32_t MAWTESTCFG_LENGTH_MASK(0X0000FFFF);
static const uint32_t MAWTESTCFG_LENGTH_SHIFT(0);

// s_internalTrigDelay:
//   There are  4x8 bit fields that supply a delay which
//   is 2 clock cycles * the value:

static const uint32_t INTTRGDELAY_CH4_MASK(0XFF000000);
static const uint32_t INTTRGDELAY_CH4_SHIFT(24);
static const uint32_t INTTRGDELAY_CH3_MASK(0X00FF0000);
static const uint32_t INTTRGDELAY_CH3_SHIFT(16);
static const uint32_t INTTRGDELAY_CH2_MASK(0X0000FF00);
static const uint32_t INTTRGDELAY_CH2_SHIFT(8);
static const uint32_t INTTRGDELAY_CH1_MASK(0X000000FF);
static const uint32_t INTTRGDELAY_CH1_SHIFT(0);

// s_internalGateLength
//   There is a common internal gate length for all
//  4 internal gates as well as an internal coincidence
//  gate length.  Threa are also enable bits for
//  the twwo gates that can be fired by the 
//  internal triggers.
//  The gate length is 2x value ...ist hat true of the
//  coincidence widht?  doesn't say so but...?


static const uint32_t INTTRGGATE_G2ENA_CH4(0X08000000);
static const uint32_t INTTRGGATE_G2ENA_CH3(0X04000000);
static const uint32_t INTTRGGATE_G2ENA_CH2(0X02000000);
static const uint32_t INTTRGGATE_G2ENA_CH1(0X01000000);

static const uint32_t INTTRGGATE_G1ENA_CH4(0X00800000);
static const uint32_t INTTRGGATE_G1ENA_CH3(0X00400000);
static const uint32_t INTTRGGATE_G1ENA_CH2(0X00200000);
static const uint32_t INTTRGGATE_G1ENA_CH1(0X00100000);

static const uint32_t INTTRGGATE_LENGTH_MASK(0X0000FF00);
static const uint32_t INTTRGGATE_LENGTH_SHIFT(8);
static const uint32_t INTTRGGATE_COINC_MASK(0X000000FF);
static const uint32_t INTTRGGATE_COINC_SHIFT(0);

// Each group has a set of registers in the s_adcSetup
// array these registers are described below.
// note there's a fifth one which applies to the
// sum signal.


// s_firTRGsetup:
//    main use is the gap time and peaking time for the FIR trigger.
//    note the bottom bits of all fields are ignored making them
//    an even number of clocks(?).
static const uint32_t FIR_NIMOUTLEN_MASK(0XFF000000);
static const uint32_t FIR_NIMOUTLEN_SHIFT(0X24);
static const uint32_t FIR_GAPTIME_MASK(0X00FFF000)
static const uint32_t FIR_GAPTIME_SHIFT(12);
static const uint32_t FIR_PEAKTIME_MASK(0X00000FFF);
static const uint32_t FIR_PEAKTIME_SHIFT(0);

// s_threshold_a
//  Trigger thresholds for the FIR trigger.
//    There are so few CFD modes we just supply
//     them as values to or in:

static const uint32_t FIRTHRESH_TRGENABLE(0X80000000);
static const uint32_t FIRTHRESH_HESUPPRESS(0X40000000);
static const uint32_t FIRTHRESH_CFD_DISABLE(0X00000000);
static const uint32_t FIRTHRESH_CFD_ZEROCROSS(0X20000000);
static const uint32_t FIRTHRESH_CFD_50PCT(0X30000000);
static const uint32_t FIRTHRESH_THRESHOLD_MASK(00FFFFFFF);
static const uint32_t FIRTHRESH_THRESHOLD_SHIFT(0);

//  s_hethreshold_a
//    controls the high energy trigger operation:
//    This includes the thing presented to the trigger mux.
//    Note that there are only three states for that
//    again we don't bother with shifts and masks.
static const uint32_t FIRHETHRESH_BOTH_EDGES(0X80000000);
static const uint32_t FIRHETHRESH_STRETCHTOMUX(0X40000000);
static const uint32_t FIRHETHRESH_MUXROUTE_INTTRG(0x00000000;
static const uint32_t FIRHETHRESH_MUXROUTE_HETRG(0X10000000);
static const uint32_t FIRHETHRESH_MUXROUTE_PILEUP(0X20000000);
static const uint32_t FIRHETHRESH_THRESHOLD_MASK(0X0FFFFFFF);
static const uint32_t FIRHETHRESH_THRESHOLD_SHIFT(0);

// Now back to per group registers.

// s_trgstatmode -- how trigger statistics are read:
// If TRGSTAT_ONBANKSWITCH is set, then each bank switch
// latches the trigger statistics counter.
//
static const uint32_t TRGSTAT_ONBANKSWITCH(1);

// s_peakchargeconfig
//   In addition to capturing waveforms,
//   the digitizer can do waveform analysis extracting
//   a peak and charge from the trace data.  This register controls
// that.
// 
//  Note again I just enumerate the baseline averaging modes.
//   so they can be ored in directly without shifts/masks.
// Note as with many timing constants/sample counts,
// thebottom bit of the pregate delay is always 0.

static const unint32_t PQCONFIG_ENABLE(0X80000000);
static const unint32_t PQCONFIG_AVG_MASK(0X3000000); // For extraction
static const unint32_t PQCONFIG_AVG32(0X00000000);
static const unint32_t PQCONFIG_AVG64(0X01000000);
static const unint32_t PQCONFIG_AVG128(0X20000000);
static const unint32_t PQCONFIG_AVG256(0X30000000)

static const unint32_t PQCONFIG_PREGATE_DELAY_MASK(0X0FFF0000);
static const unint32_t PQCONFIG_PREGATE_DELAY_SHIFT(16);

// s_extendedbufferconfig 
//   Yet another way to specify # of samples.
//    Just a counter so we don't bother to define fields.


// s_accumulatorGateConfig
//
// Length and start sample of accumulator gates (Q integration?).
// Each group has three of these labeled in order Gate1, Gate2 and
// Gate8.

static const unint32_t ACQGATE_GATE_LENGTH_MASK(0X7FFF0000);
static const unint32_t ACQGATE_GATE_LENGTH_SHIFT(16);
static const unint32_t ACQGATE_GATE_START_MASK(0X0000FFFF);
static const unint32_t ACQGATE_GATE_START_SHIFT(0);

// s_FIREnergySetup
//   There is a register for each digitizer in the group.
//   This register sets up trapezoidal filter parameters
//   for the energy extraction:

static const uint32_t EFILT_TAU_TBLSEL_MASK(0XC0000000);
static const uint32_t EFILT_TAU_TBLSEL_SHIFT(30);
static const uint32_t EFILT_TAU_FACTOR_MASK(0X3F00000);
static const uint32_t EFILT_TAU_FACTOR_SHIFT(24);

static const uint32_t EFILT_EXTRAFILT_NONE(0X00000000);
static const uint32_t EFILT_EXTRAFILT_AVG4(0X00400000);
static const uint32_t EFILT_EXTRAFILT_AVG8(0X00800000);
static const uint32_t EFILT_EXTRAFILT_AVB16(0X00C00000);

static const uint32_t EFILT_GAPT_MASK(0X0003F000);
static const uint32_t EFILT_GAPT_SHIFT(12);

static const uint32_t EFILT_PEAKT_MASK(0X00000FFF);
static const uint32_t EFILT_PEAKT_SHIFT(0);

// s_histogramSetup -  the adc can histogram
// extracted energy data.   The energy can be scaled
// and offset  with the bin within the 64kbin histogram
// computed as value/scale - offset in that order of operations.
// This is an array of registers for each of the digitizers
// in a group.

static const uint32_t HIST_HITWRITE_DISABLE(0X80000000);
static const uint32_t HIST_CLEAR_WITH_TSCLEAR(0X040000000);
static const uint32_t HIST_SCALE_MASK(0X0FFF0000);
static const uint32_t HIST_SCALE_SHIFT(16);
static const uint32_t HIST_OFFSET_MASK(0X0000FF00);
static const uint32_t HIST_OFFSET_SHIFT(8);
static const uint32_t HIST_PUP_ENABLE(0X00000002); 
static const uint32_t HIST_ENABLE(0X00000001);

// s_MAWStartIndexConfig - One per ADC in the group (4 registers).
// From the manual:
//  With the value of the “MAW Test Buffer Start Index“, it is possible to delay the start of
// saving the Trigger or Energy trapezoid into the MAW Test buffer. This can be helpful to save
// (see) the tail of the Energy trapezoid in case that the sum (Pretiggerdelay + 2*Peaking time +
// Gap time) is greater than 2048.
//
// You can save the Energy value from a defined position (index) of the Energy trapezoidal
// rather than the maximum value of the trapezoidal with a non zero Energy Pickup Index value.
// With a zero Energy Pickup Index value the logic will save the maximum value.

static uint32_t MAWSTART_ENERGY_MASK(0XFFFF0000);
static uint32_t MAWSTART_ENERGY_SHIFT(16);
static uint32_t MAWSTART_START_MASK(0X0000FFFF);
static uint32_t MAWSTART_START_SHIFT(0);

// s_test - may have one useful bit after all:
//
// If this bit is true, then the internal gate 
// is set in the bottom bit of trace data.
// This is essentially a digital probel for the gate.
// I can see it being useful for debugging timing.
static unint32_t TEST_SAVE_INTERNAL_GATE(1);

// s_sampleClockPllDrp:
//   Each FGPA has an associated PLL to synthesize an input
//   clock into a sampling clock for its ADC.
// 
//  Handling this PLL Is another abomination.  If I understand
//  correctly when writing data to the PLL I write the data bits here
//  but, if I'm readingm data from the PLL, the read data bits are
// in s_sampleClockPLLDrpRead
//
// I've complained a lot about this module but that uhm... that's
// uhm....going pretty far.
// How reset works is also ugly...rather than just twinking a bit
// and the chip clears I need to set set the reste in one bit and then
// clear it in another.
// I'm trying to make this clear by calling the data bits WRITE_DATA
// as in the manual.

static uint32_t SAMPLEPLL_RESET_CLEAR(0X20000000);
static uint32_t SAMPLEPLL_RESET_SET(0X02000000);
static uint32_t SAMPLEPLL_ENABLE(0X01000000);
static uint32_t SAMPLEPLL_WRITE(0X00800000);
static uint32_t SAMPLEPLL_ADDR_MASK(0X00780000);
static uint32_t SAMPLEPLL_ADDR_SHIFT(16);
static uint32_t SAMPLEPLL_WRITE_DATA_MASK(0X0000FFFF);
static uint32_t SAMPLEPLL_WRITE_DATA_SHIFT(0);

// s_adcfpgaVersion

static uint32_t FPGAVSN_TYPE_MASK(0XFFFF0000);
static uint32_t FPGAVSN_TYPE_SHIFT(16);
static uint32_t FPGAVSN_VERSION_MASK(0X0000FF00);
static uint32_t FPGAVSN_VERSION_SHIFT(8);
static uint32_t FPGAVSN_REVISION_MASK(0X000000FF);
static uint32_t FPGAVSN_REVISION_SHIFT(0);
static uint32_t FPGAVSN_TYPE_125MHZ(0X0125);
static uint32_t FPGAVSN_TYPE_250MHZ(0X0250);

// s_adcfgpastatus:
//
// According to the manual:
//  holds the ADC FPGA Status of the Data Link between the ADC FPGA and the
//  VME FPGA , the Status of the both Memory Controller and the Status of the ADC-Clock
//  DCM.

static unit32_t FPGASTAT_CLOCKPLL_RESET(0X00200000);
static unit32_t FPGASTAT_CLOCKPLL_OK(0X00100000);

static unit32_t FPGASTAT_MEMORY2_OK(0X00020000);
static unit32_t FPGASTAT_MEMORY1_OK(0X00010000);
static unit32_t FPGASTAT_DLINK_SPEED(0X00000100);
static unit32_t FPGASTAT_VME_FRAME_ERR(0X00000080);  // Latched errors & status:
static unit32_t FPGASTAT_VME_SOFT_ERR(0X00000040);
static unit32_t FPGASTAT_VME_HARD_ERR(0X00000020);
static unit32_t FPGASTAT_VME_LANE_UP(0X00000010);   //These are continuously updating(?)
static unit32_t FPGASTAT_VME_CHAN_UP(0X00000008);
static unit32_t FPGASTAT_VME_FRAME(0x00000004);
static unit32_t FPGASTAT_VME_SOFT(0X00000002);
static unit32_t FPGASTAT_VME_HARD(0X00000001);

// s_sampleAddress is just that; no bits of interest.
// as is s_prevbankSample

// Finaly s_sampleClockPLLDrpRead - the read data and status from 
// s_sampleClockPllDrp
//

static unit32_t SAMPLE_PLL_READ_RESET(0X02000000);
static unit32_t SAMPLE_PLL_READ_DATA_MASK(0XFFFF);
static unit32_t SAMPLE_PLL_READ_DATA_SHIFT(0);



#pragma pack (pop)


    }
}

#endif