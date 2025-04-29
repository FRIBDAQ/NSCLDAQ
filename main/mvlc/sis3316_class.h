/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_class.h                                              */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH/TF                                            */
/*  date:                 26.07.2012                                       */
/*  last modification:    23.07.2024                                       */
/*    - add support for SIS3316-2 module variant                           */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Struck Innovative Systeme GmbH                                         */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*                                                                         */
/*  https://www.struck.de                                                  */
/*                                                                         */
/*  (c) 2021-2024                                                          */
/*                                                                         */
/***************************************************************************/

#ifndef MVLC_SIS3316_CLASS_
#define MVLC_SIS3316_CLASS_


//#include "project_system_define.h" //define LINUX or WIN

// Just define the stuff we need defined:

#define LINUX

#include "sis3316.h"

#ifdef LINUX
#include "vme_interface_class.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#endif

#ifdef WIN
#include <Windows.h>

#include "vme_interface_class.h"

#include <iostream>
#include <cstdio>

void usleep(unsigned int uint_usec);

using namespace std;
#endif

namespace SIS {
	namespace ADC {
		namespace SIS3316 {
			enum sensor {
				VOLTAGE_D1V5,
				VOLTAGE_C2V5,
				VOLTAGE_D3V3,
				VOLTAGE_MGT1V0,
				VOLTAGE_MGT1V2,
				VOLTAGE_VME_VCCINT,
				VOLTAGE_VME_VCCAUX,
				VOLTAGE_VME_VCCBRAM,
				VOLTAGE_ADC1_VCCINT,
				VOLTAGE_ADC1_VCCAUX,
				VOLTAGE_ADC1_VCCBRAM,
				VOLTAGE_ADC2_VCCINT,
				VOLTAGE_ADC2_VCCAUX,
				VOLTAGE_ADC2_VCCBRAM,
				VOLTAGE_ADC3_VCCINT,
				VOLTAGE_ADC3_VCCAUX,
				VOLTAGE_ADC3_VCCBRAM,
				VOLTAGE_ADC4_VCCINT,
				VOLTAGE_ADC4_VCCAUX,
				VOLTAGE_ADC4_VCCBRAM,
				TEMP_TOP,
				TEMP_MID,
				TEMP_BOT,
				TEMP_PWR_U90,
				TEMP_PWR_U91,
				TEMP_PWR_U92,
				TEMP_VME_CORE,
				TEMP_ADC1_CORE,
				TEMP_ADC2_CORE,
				TEMP_ADC3_CORE,
				TEMP_ADC4_CORE,
			};
			enum DeviceVariant {
				TYPE_SIS3316,
				TYPE_SIS3316_2,
			};
			enum SampleRate { // SI570 Oscilator
				SAMPLERATE_250MSPS,
				SAMPLERATE_227MSPS,
				SAMPLERATE_208MSPS,
				SAMPLERATE_179MSPS,
				SAMPLERATE_167MSPS,
				SAMPLERATE_139MSPS,
				SAMPLERATE_125MSPS,
				SAMPLERATE_119MSPS,
				SAMPLERATE_114MSPS,
				SAMPLERATE_104MSPS,
				SAMPLERATE_100MSPS,
				SAMPLERATE_83MSPS,
				SAMPLERATE_71MSPS,
				SAMPLERATE_62M5SPS,
				SAMPLERATE_50MSPS,
				SAMPLERATE_25MSPS,
				SAMPLERATE_12P5MSPS   // 12.5 MHz
			};
		}
	}
}

class sis3316_adc
{
private:
	vme_interface_class* i;
	unsigned int baseaddress;

	// FPGA bootflash access
	int FlashEnableCS(int chip);
	int FlashDisableCS(int chip);
	int FlashWriteEnable(void);
	int FlashWriteDisable(void);
	int FlashProgramPage(int address, char* data, int len);
	int FlashEraseBlock(int address);
	int FlashWriteSR1CR1(char sr, char cr);
	int FlashXfer(char in, char* out);

	// Peripheral I2C bus access
	int I2cStart(uint32_t base);
	int I2cStop(uint32_t base);
	int I2cWriteByte(uint32_t base, unsigned char data, char* ack);
	int I2cReadByte(uint32_t base, unsigned char* data, char ack);
	int I2cAddressSlave(uint32_t base, uint8_t adr, uint8_t dir, uint8_t *ack);
	int I2cTransmission(uint32_t base, uint8_t adr, uint8_t* outData, size_t outLen, uint8_t* inData, size_t inLen);
	int Si570FreezeDCO(uint32_t base);
	int Si570Divider(uint32_t base, unsigned char* data);
	int Si570UnfreezeDCO(uint32_t base);
	int Si570NewFreq(uint32_t base);
	int Si570ReadDivider(uint32_t base, unsigned char* data);
	int eeprom_busy(uint32_t base);

	// Onewire bus access
	int owReset(int* presence);
	int owRead(unsigned char* data);
	int owWrite(unsigned char data);
	int owEeReadPage(int page, unsigned char* data);
	int owEeWritePage(int page, unsigned char* data);
	int ow_id_ee(unsigned char* data);

	// DRP bus access
	int DrpPollBusy(uint32_t base);
	int DrpReadReg(uint32_t base, uint8_t adr, uint16_t* data);
	int DrpWriteReg(uint32_t base, uint8_t adr, uint16_t data);
	int DrpRmwReg(uint32_t base, uint8_t adr, uint16_t mask, uint16_t data);

	int si5325_clk_muliplier_write(unsigned int addr, unsigned int data);
	int si5325_clk_muliplier_read(unsigned int addr, unsigned int* data);
	int si5325_clk_muliplier_internal_calibration_cmd(void);

public:
	unsigned char freqSI570_calibrated_value_125MHz[6];
	unsigned char freqPreset62_5MHz[6];
	unsigned char freqPreset125MHz[6];
	unsigned char freqPreset250MHz[6];

	unsigned int serial_number;
	unsigned int vme_fpga_version;
	unsigned int adc_fpga_version;
	unsigned int adc_125MHz_flag;
	unsigned int device_variant;  // 0 -> SIS3316   ,  2 -> SIS3316-2

	unsigned int adc_dac_offset_ch_array[16];
	unsigned int adc_gain_termination_ch_array[16];
	unsigned int nim_output_selection_array[3];

public:
	sis3316_adc(vme_interface_class* crate, unsigned int baseaddress);
	~sis3316_adc(void);

	int register_read(UINT addr, UINT* data);
	int register_write(UINT addr, UINT data);

	int update_firmware(char* path, int offset, void (*cb)(int percentage));
	int verify_firmware(char* path, int offset, void (*cb)(int percentage));
	int FlashRead(int address, char* data, int len);
	int FlashReadStatus1(char* status);
	int FlashReadStatus2(char* status);
	int FlashEnableProg(void);
	int FlashDisableProg(void);
	int FlashGetId(char* id);

	int read_ee(int offset, int len, unsigned char* data);
	int write_ee(int offset, int len, unsigned char* data);
	int write_ow_dhcp_option(unsigned char* data);

	int readSensor(SIS::ADC::SIS3316::sensor channel, double *value);

	int get_SI570_oscillator_hs_div_and_n1_div_values(unsigned int enum_sample_rate, unsigned int* hs_div_val, unsigned int* n1_div_val,  double* double_get_frequency);
	int change_frequency_HSdiv_N1div(int osc, unsigned int hs_div_val, unsigned int n1_div_val);
	int get_frequency(int osc, unsigned char* values);
	int set_frequency(int osc, unsigned char* values);
	int set_external_clock_multiplier(unsigned int bw_sel, unsigned int n1_hs, unsigned int n1_clk1, unsigned int n1_clk2, unsigned int n2, unsigned int n3, unsigned int clkin1_mhz);
	int bypass_external_clock_multiplier(void);
	int get_status_external_clock_multiplier(unsigned int* status);

	int get_adc_fpga_iob_delay_value(unsigned int enum_sample_rate  ,unsigned int*iob_delay_value);
	int configure_adc_fpga_iob_delays(unsigned int iob_delay_value);
	int reset_adc_fpga_sample_clock_PLL(void);
	int reset_adc_fpga_and_DDR_memory(void);

	int write_channel_header_ID(unsigned int channel_header_id_reg_value);

	int poll_on_adc_dac_offset_busy(void);
	int write_all_adc_dac_offsets(void);
	int configure_all_adc_dac_offsets(void);
	int write_all_gain_termination_values(void);
	int write_nim_output_selection_values(void);

	int internal_sum_trigger_generation_setup(unsigned int uint_sum_trigger_threshold_reg, unsigned int uint_sum_he_trigger_threshold_reg, unsigned int uint_sum_trigger_setup_reg, unsigned int adc_fpga_group /* 0 to 3 */);
	int internal_trigger_generation_setup(unsigned int uint_trigger_threshold_reg, unsigned int uint_he_trigger_threshold_reg, unsigned int uint_trigger_setup_reg, unsigned int channel_no /* 0 to 15 */);

	int adc_spi_setup(void);
	int adc_spi_reg_enable_adc_outputs(void);
	int adc_spi_read(unsigned int adc_fpga_group, unsigned int adc_chip, unsigned int spi_addr, unsigned int* spi_data); // changed 2.12.2014
	int adc_spi_write(unsigned int adc_fpga_group, unsigned int adc_chip, unsigned int spi_addr, unsigned int spi_data); // changed 2.12.2014

	int read_DMA_Channel_PreviousBankDataBuffer(unsigned int bank2_read_flag, unsigned int channel_no /* 0 to 15 */,
		unsigned int max_read_nof_words, unsigned int* dma_got_no_of_words, unsigned int* uint_adc_buffer);

	int read_MBLT64_Channel_PreviousBankDataBuffer(unsigned int bank2_read_flag, unsigned int channel_no /* 0 to 15 */,
		unsigned int* dma_got_no_of_words, unsigned int* uint_adc_buffer);

	int read_Channel_PreviousSampleAddress(unsigned int bank2_read_flag, unsigned int channel_no /* 0 to 15 */, unsigned int* previous_sample_address);

	int read_Channel_EnergyHistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_adc_buffer);

	int read_Channel_StatisticCounter(unsigned int adc_fpga_no /* 0 to 3 */, unsigned int* uint_statistic_buffer); // new 27.08.2019

	int read_Channel_TofHistogramBuffer(unsigned int channel_no /* 0 to 15 */, int histogram_index, unsigned int lenght, unsigned int* uint_buffer);
	int read_Channel_ShapeHistogramBuffer(unsigned int channel_no /* 0 to 15 */, int histogram_index, unsigned int lenght, unsigned int* uint_buffer);
	int read_Channel_ChargeHistogramBuffer(unsigned int channel_no /* 0 to 15 */, int histogram_index, unsigned int lenght, unsigned int* uint_buffer);

	// requires the ADC-FPGA Firmware V_0201 (SIS3316-14bit-250MHz) or higher
	int write_Channel_PSD_LookupTable_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer);
	int read_Channel_PSD_LookupTable_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer);
	int read_Channel_PSD_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer);

	int read_Channel_TOF_Gamma_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer);
	int read_Channel_TOF_Neutron_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer);

	int read_Channel_PeakSum_Gamma_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer);
	int read_Channel_PeakSum_Neutron_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer);

	int read_Channel_Statistic_Buffer(unsigned int fpga_goup /* 0 to 3 */, unsigned int lenght, unsigned int* uint_buffer);
};

#endif
