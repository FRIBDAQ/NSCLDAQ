/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_class.cpp                                            */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH/TF                                            */
/*  date:                 26.07.2012                                       */
/*  last modification:    08.03.2024                                       */
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
/*  (c) 2019-2024                                                          */
/*                                                                         */
/***************************************************************************/

#include "sis3316_class.h"

#ifdef LINUX
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#ifdef WIN
#include <iostream>
#include <cstdio>

void usleep(unsigned int uint_usec);

using namespace std;
#endif

namespace SIS {
	namespace ADC {
		namespace SIS3316 {


			typedef struct flashProperties_st {
				uint32_t size;            // complete memory array
				uint32_t pagesize;        // page programming size
				uint32_t sectorsize;      // sector erase size
				uint32_t pageprogramtime; // maximum page programming timeout
				uint32_t sectorerasetime; // maximum sector erasure timeout
				uint32_t chiperasetime;   // maximum chip erasure timeout
				uint32_t remsID;          // backwards-compatible manufacturer and device ID
				uint32_t rcfiID;          // extended ID from the ID-CFI fields
			} flashProperties_t;

			// properties of the eeprom
			typedef struct eeprom_24xx_st {
				uint8_t adr;     // i2c bus address
				int offsLen;     // length of the offset address register
				size_t size;     // memory size of the eeprom
				size_t pageSize; // size of a programming page
			} eeprom_24xx_t;
		}
	}
}

// byte 1 command codes
#define FL_WRITE_ENABLE            0x06
#define FL_WRITE_ENABLE_VLT_SREG   0x50
#define FL_WRITE_DISABLE           0x04
#define FL_READ_SREG1              0x05
#define FL_READ_SREG2              0x35
#define FL_WRITE_REGS              0x01 // write SR1 (first 8 bits) and CR1 (second 8 bits)
#define FL_PAGE_PROGRAM            0x02
#define FL_PAGE_PROGRAM_4          0x12 // 4 byte addressing
#define FL_QUAD_PAGE_PROGRAM       0x32
#define FL_SECTOR_ERASE            0x20 // 4kB
#define FL_SECTOR_ERASE_4          0x21 // 4kB, 4 byte addressing
#define FL_BLOCK_ERASE_64          0xD8 // 64kB
#define FL_BLOCK_ERASE_64_4        0xDC // 64kB, 4 byte addressing
#define FL_CHIP_ERASE              0x60
#define FL_ERASE_PGM_SUSPEND       0x75
#define FL_ERASE_PGM_RESUME        0x7A
#define FL_POWER_DOWN              0xB9
#define FL_CONT_READ_RESET         0xFF
#define FL_READ_DATA               0x03
#define FL_READ_DATA_4             0x13 // 4 byte addressing
#define FL_FAST_READ               0x0B
#define FL_FAST_READ_DUAL          0x3B
#define FL_FAST_READ_QUAD          0x6B
#define FL_FAST_READ_DUAL_IO       0xBB
#define FL_FAST_READ_QUAD_IO       0xEB
#define FL_WORD_READ_QUAD_IO       0xE7
#define FL_OCTAL_WORD_READ_QUAD_IO 0xE3
#define FL_SET_BURST_WRAP          0x77
#define FL_DEVICE_ID               0xAB
#define FL_MANUFACTURER_ID         0x90
#define FL_MANUFACTURER_ID_DUAL    0x92
#define FL_MANUFACTURER_ID_QUAD    0x94
#define FL_JEDEC_ID                0x9F
#define FL_READ_UNIQUE_ID          0x4B
#define FL_READ_SFDP_REG           0x5A
#define FL_ERASE_SEC_REG           0x44
#define FL_PROGRAM_SEC_REG         0x42
#define FL_READ_SEC_REG            0x48
#define FL_READ_CONFIG_REG         0x35 // RDCR

// device size threshold to select 4-byte addressing
#define FL_4B_ADR_THRESHOLD 16777216 // device will require more than 24 address bits


// S25FL256xxx bootflash (3316-2)
static SIS::ADC::SIS3316::flashProperties_t spiFlashPropsS25FL256 = {
	33554432,   // size (byte)
	256,        // pagesize (byte)
	65536,		// sectorsize (byte)
	1,			// page program time (ms)
	5000,		// sector erase time (ms)
	70000,		// chip erase time (ms)
	0,          // remsID, set to 0, use the cfi-id instead
	0x190201,	// rfciID, 0x19 (256Mb), 0x02 (256Mb), 0x01 (Spansion/Infineon)
};

// W25Q64xxxxxx bootflash (3316)
static SIS::ADC::SIS3316::flashProperties_t spiFlashPropsW25Q64 = {
	8388608,    // size (byte)
	256,        // pagesize (byte)
	65536,		// sectorsize (byte)
	5,			// page program time (ms)
	3000,		// sector erase time (ms)
	120000,		// chip erase time (ms)
	0,          // remsID, set to 0, use the cfi-id instead
	0x1740EF,	// rfciID, 0x17 (64Mb), 0x40 (SPI), 0xEF (Winbond)
};

// 24AA025E48 (3316-2)
static SIS::ADC::SIS3316::eeprom_24xx_t i2cEeprom24AA025 = {
	0x50,	// i2c slave address
	1,		// 1 address byte required
	256,	// size of the memory array (bytes)
	8,		// programmin page size (and boundary)
};

#define EEPROM_BUSY_TIMEOUT 100 // 100 ms

#define SIS3316_ADC_FPGA_BOOT_CSR		0x30
#define SIS3316_SPI_FLASH_CSR			0x34
#define SIS3316_SPI_FLASH_DATA			0x38
//#define SIS3316_ADC_CLK_OSC_I2C_REG		0x40
//#define SIS3316_REG_I2C_SI570_MGT_BASE  0x44
//#define SIS3316_REG_I2C_SI570_VXS_BASE  0x48
//#define SIS3316_REG_I2C_SI570_DDR3_BASE 0x4C

//#define SIS3316_REG_I2C_SFP_BASE        0xB0

// FLASH_CSR bits in 3316-2 (Artix 7 flash spi macro)
#define A7_SPI_BUSY			31
#define A7_SPI_MUX			15
#define A7_SPI_CS			14
#define A7_SPI_PAGEWRITE_EN	13 // enable page write block mode
#define A7_SPI_EXCH			12
#define A7_SPI_PAGEWRITE	11 // write to page block fifo
#define A7_SPI_BLOCKREAD_EN 10 // enable page read mode
#define A7_SPI_BLOCKREAD	9  // read from page read fifo

// SPI_FLASH_CSR bits in 3316 (Spartan 6 spi macro)
#define ENABLE_SPI_PROG 0
#define CHIPSELECT_1 1
#define CHIPSELECT_2 2
#define FIFO_NOT_EMPTY 14
#define FLASH_LOGIC_BUSY 31

// I2C_CSR bits (Spartan 6 i2c macro)
#define I2C_BUSY		31
#define I2C_READ		13
#define I2C_WRITE		12
#define I2C_STOP		11
#define I2C_REP_START	10
#define I2C_START		9
#define I2C_ACK			8

#define I2C_DIR_READ  1
#define I2C_DIR_WRITE 0

#define OSC_ADR	0x55 // default factory i2c address for si570 oscillator devices



// bits
#define DRP_BUSY   31
#define DRP_RESET  25
#define DRP_ENABLE 24
#define DRP_WRCYC  23
#define DRP_ADR    16 // LSB of 7 bits
#define DRP_DATA   0  // LSB of 16 bits

static double xadc_calcTemp(uint16_t reg)
{
	// transfer function from UG480 Equation 2-6
	return ((reg >> 4) * 503.975) / 4096.0 - 273.15;
}

static double xadc_calcVoltage(uint16_t reg, double reference)
{
	// 12 bit xadc, MSB aligned
	return ((reg >> 4) / 4096.0) * reference;
}

static double divider_fact(double vadc, double rtop, double rbot)
{
	return vadc * (1.0 + (rtop / rbot));
}

static double ltm_diode_fact(double vdiode)
{
	return (-(1.2 - vdiode) / -2e-3) - 273.0;
}

#define SI5325_SPI_POLL_COUNTER_MAX 100
#define SI5325_SPI_CALIBRATION_READY_POLL_COUNTER_MAX 1000

/**************************************************************************************/
sis3316_adc::sis3316_adc(vme_interface_class* crate, unsigned int baseaddress)
{
	int rc;
	unsigned int i_ch;

	if (crate)
	{
		i = crate;
	}
	this->baseaddress = baseaddress;

	get_frequency(0, freqSI570_calibrated_value_125MHz);

	// determine the hardware variant
	uint32_t reg;
	register_read(SIS3316_HARDWARE_VERSION, &reg);
	device_variant = reg & 0x80 ? SIS::ADC::SIS3316::TYPE_SIS3316_2 : SIS::ADC::SIS3316::TYPE_SIS3316;

	// frequency presets setup
	freqPreset62_5MHz[0] = 0x23;
	freqPreset62_5MHz[1] = (0x3 << 6) + (freqSI570_calibrated_value_125MHz[1] & 0x3F);
	freqPreset62_5MHz[2] = freqSI570_calibrated_value_125MHz[2];
	freqPreset62_5MHz[3] = freqSI570_calibrated_value_125MHz[3];
	freqPreset62_5MHz[4] = freqSI570_calibrated_value_125MHz[4];
	freqPreset62_5MHz[5] = freqSI570_calibrated_value_125MHz[5];

	freqPreset125MHz[0] = 0x21;
	freqPreset125MHz[1] = (0x3 << 6) + (freqSI570_calibrated_value_125MHz[1] & 0x3F);
	freqPreset125MHz[2] = freqSI570_calibrated_value_125MHz[2];
	freqPreset125MHz[3] = freqSI570_calibrated_value_125MHz[3];
	freqPreset125MHz[4] = freqSI570_calibrated_value_125MHz[4];
	freqPreset125MHz[5] = freqSI570_calibrated_value_125MHz[5];

	freqPreset250MHz[0] = 0x20;
	freqPreset250MHz[1] = (0x3 << 6) + (freqSI570_calibrated_value_125MHz[1] & 0x3F);
	freqPreset250MHz[2] = freqSI570_calibrated_value_125MHz[2];
	freqPreset250MHz[3] = freqSI570_calibrated_value_125MHz[3];
	freqPreset250MHz[4] = freqSI570_calibrated_value_125MHz[4];
	freqPreset250MHz[5] = freqSI570_calibrated_value_125MHz[5];

	adc_125MHz_flag = 0;

	// ADC chip setup via SPI
	rc = adc_spi_setup();
	if (rc != 0)
	{
		printf("Error adc_spi_setup: return_code = 0x%08x \n", rc);
	}

	// write Header ID registers
	rc = write_channel_header_ID(baseaddress & 0xff000000);
	if (rc != 0)
	{
		printf("Error write_channel_header_ID: return_code = 0x%08x \n", rc);
	}

	// adc DAC offset configuration
	rc = configure_all_adc_dac_offsets();
	if (rc != 0)
	{
		printf("Error configure_all_adc_dac_offsets: return_code = 0x%08x \n", rc);
	}

	// adc DAC offset setup
	for (i_ch = 0; i_ch < 16; i_ch++)
	{
		adc_dac_offset_ch_array[i_ch] = 0x8000; // middle: 5V range -> -/+2.5V; 2V range -> -/+1V
	}
	rc = write_all_adc_dac_offsets();
	if (rc != 0)
	{
		printf("Error write_all_adc_dac_offsets: return_code = 0x%08x \n", rc);
	}

	// channel Gain/Termination setup
	for (i_ch = 0; i_ch < 16; i_ch++)
	{
		adc_gain_termination_ch_array[i_ch] = 0x0; // (5V Range and 50Ohm termination)
	}
	rc = write_all_gain_termination_values();
	if (rc != 0)
	{
		printf("Error write_all_gain_termination_values: return_code = 0x%08x \n", rc);
	}

	rc = register_read(SIS3316_SERIAL_NUMBER_REG, &serial_number);
	rc = register_read(SIS3316_MODID, &vme_fpga_version);
	rc = register_read(SIS3316_ADC_CH1_4_FIRMWARE_REG, &adc_fpga_version);

	// reset spi macro
	register_write(SIS3316_SPI_FLASH_CSR, (1 << A7_SPI_MUX | 1 << A7_SPI_BLOCKREAD_EN | 1 << A7_SPI_PAGEWRITE_EN | 1 << A7_SPI_CS) << 16);
}

/**************************************************************************************/
sis3316_adc::~sis3316_adc(void)
{
}

/**************************************************************************************/
int sis3316_adc::register_read(UINT addr, UINT* data)
{
	return i->vme_A32D32_read(baseaddress + addr, data);
}

/**************************************************************************************/
int sis3316_adc::register_write(UINT addr, UINT data)
{
	return i->vme_A32D32_write(baseaddress + addr, data);
}

/**************************************************************************************/
int sis3316_adc::update_firmware(char* path, int offset, void (*cb)(int percentage))
{
	int rc;
	FILE* fp;
	char* buf;
	int fileSize;
	int percent, percent_old;
	int written = 0;
	int pageProgramSize;
	unsigned int retry_counter = 0;
	unsigned int retry_flag = 0;

	if (path == NULL)
	{
		return -100;
	}

#ifdef LINUX
	fp = fopen(path, "rb");
	if (fp == NULL)
	{
		return -101;
	}
#endif

#ifdef WIN
	fopen_s(&fp, path, "rb");
	if (fp == NULL)
	{
		return -101;
	}
#endif

	// select flash type
	SIS::ADC::SIS3316::flashProperties_t* props = device_variant == SIS::ADC::SIS3316::TYPE_SIS3316 ? &spiFlashPropsW25Q64 : &spiFlashPropsS25FL256;

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	rewind(fp);

	buf = (char*)malloc(fileSize + 1024);
	if (buf == NULL)
	{
		printf("error malloc(fileSize) fileSize = %d \n", fileSize);
		return -102;
	}

	size_t frc = fread(buf, 1, fileSize, fp);
	if (frc != fileSize)
	{
		return -103;
	}

	fclose(fp);

	percent = percent_old = 0;
	if (cb)
	{
		(cb)(percent);
	}

	FlashEnableProg();

	while (written < fileSize)
	{
		rc = 0;
		// erase
		if ((written & (props->sectorsize - 1)) == 0)
		{
			rc = FlashEraseBlock((offset + written) & 0xFFFF0000);
			if (rc != 0)
			{
				printf("erase: rc = %d \n", rc);
			}
		}

		if (fileSize >= (written + props->pagesize))
		{
			pageProgramSize = props->pagesize;
		}
		else
		{
			pageProgramSize = fileSize - written;
		}

#define TRY_RETRY
#ifdef TRY_RETRY

		retry_counter = 0;
		do
		{
			retry_flag = 0;
			rc = FlashProgramPage(offset + written, buf + written, pageProgramSize);
			if (rc != 0)
			{
				usleep(10000); // 10ms

				retry_counter++;
				retry_flag = 0;
				if (retry_counter < 16)
				{
					retry_flag = 1;
				}
				if (retry_counter > 0)
				{
					printf("Info: retry_counter = %d   addr = 0x%08X\n", retry_counter, offset + written);
				}
			}
		} while (retry_flag == 1);

#endif
		if (rc == 0)
		{
			written += pageProgramSize;
		}
		else
		{
			printf("retry Erase/FlashProgramPage \n");
			usleep(10000000);
		}

		if (cb)
		{
			percent = written * 100 / fileSize;
			if (percent != percent_old)
			{
				(cb)(percent);
				percent_old = percent;
			}
		}
	}

	FlashDisableProg();

	free(buf);

	// enable flash quad mode
	rc = FlashWriteSR1CR1(0, 0x02);
	if (rc) {
		return rc;
	}

	return 0;
}

/****************************************************************************************************/
int sis3316_adc::verify_firmware(char* path, int offset, void (*cb)(int percentage))
{
	int rc;
	FILE* fp;
	char* buf;
	char* read_buf;
	int fileSize;
	int percent, percent_old;
	int read = 0;
	int pageProgramSize;

	int page_start_offset;
	int page_read_len;
	int i;

	unsigned int verify_bad_flag = 0;

	if (path == NULL)
	{
		return -100;
	}

#ifdef LINUX
	fp = fopen(path, "rb");
	if (fp == NULL)
	{
		return -101;
	}
#endif

#ifdef WIN
	fopen_s(&fp, path, "rb");
	if (fp == NULL)
	{
		return -101;
	}
#endif

	// select flash type
	SIS::ADC::SIS3316::flashProperties_t* props = device_variant == SIS::ADC::SIS3316::TYPE_SIS3316 ? &spiFlashPropsW25Q64 : &spiFlashPropsS25FL256;

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	rewind(fp);

	buf = (char*)malloc(fileSize);
	if (buf == NULL)
	{
		return -102;
	}

	size_t frc = fread(buf, 1, fileSize, fp);
	if (frc != fileSize)
	{
		return -103;
	}
	fclose(fp);

	read_buf = (char*)malloc(props->pagesize);
	if (read_buf == NULL)
	{
		return -102;
	}

	percent = percent_old = 0;
	if (cb)
	{
		(cb)(percent);
	}

	FlashEnableProg();

	while (read < fileSize)
	{
		rc = 0;
		if (fileSize >= (read + props->pagesize))
		{
			pageProgramSize = props->pagesize;
		}
		else
		{
			pageProgramSize = fileSize - read;
		}

		// to read all bytes takes 20 minutes with etherner !
		// instead of read all bytes of a page (256) read only the last 16 bytes of a page

		page_start_offset = 0;
		page_read_len = pageProgramSize;
		if (pageProgramSize == props->pagesize)
		{
			page_read_len = 16;
			page_start_offset = (props->pagesize - page_read_len);
		}

		rc = FlashRead(offset + read + page_start_offset, &read_buf[page_start_offset], page_read_len);
		if (rc != 0)
		{
			printf("FlashReadPage: rc = %d \n", rc);
		}

		for (i = page_start_offset; i < pageProgramSize; i++)
		{
#ifdef test_remove
			if (verify_bad_flag == 0)
			{
				printf("address =   0x%08X   file = 0x%02X   read = 0x%02X \n", read + i, (unsigned char)buf[read + i], (unsigned char)read_buf[i]);
			}
#endif

			if (buf[read + i] != read_buf[i])
			{
				if (verify_bad_flag == 0)
				{
					printf("error at address =   0x%08X   file = 0x%02X   read = 0x%02X \n", read + i, (unsigned char)buf[read + i], (unsigned char)read_buf[i]);
				}
				verify_bad_flag = 1;
			}
		}

		read += pageProgramSize;

		if (cb)
		{
			percent = read * 100 / fileSize;
			if (percent != percent_old)
			{
				(cb)(percent);
				percent_old = percent;
			}
		}
	}

	FlashDisableProg();

	free(buf);
	if (verify_bad_flag == 1)
	{
		return -1;
	}
	return 0;
}

/***************************************************************/
int sis3316_adc::FlashEnableProg()
{
	// switch the spi lines to the fpga-internal spi macro
	switch (device_variant) {
	case SIS::ADC::SIS3316::TYPE_SIS3316: {
		uint32_t reg;
		int rc = register_read(SIS3316_SPI_FLASH_CSR, &reg);
		if (rc) {
			return rc;
		}

		// which chip to select
		reg |= 1 << ENABLE_SPI_PROG;
		return register_write(SIS3316_SPI_FLASH_CSR, reg);

	}

	case SIS::ADC::SIS3316::TYPE_SIS3316_2:
		return register_write(SIS3316_SPI_FLASH_CSR, 1 << A7_SPI_MUX);

	default:
		// unknown device type, should probably return a well defined status (which i don't have as of now)
		return -10;
	}
}

/***************************************************************/
int sis3316_adc::FlashDisableProg()
{
	// switch the spi lines to the fpga-internal spi macro
	switch (device_variant) {
	case SIS::ADC::SIS3316::TYPE_SIS3316: {
		uint32_t reg;
		int rc = register_read(SIS3316_SPI_FLASH_CSR, &reg);
		if (rc) {
			return rc;
		}

		// which chip to select
		reg &= ~(1 << ENABLE_SPI_PROG);
		return register_write(SIS3316_SPI_FLASH_CSR, reg);

	}

	case SIS::ADC::SIS3316::TYPE_SIS3316_2:
		return register_write(SIS3316_SPI_FLASH_CSR, (1 << A7_SPI_MUX) << 16);

	default:
		// unknown device type, should probably return a well defined status (which i don't have as of now)
		return -10;
	}
}

/***************************************************************/
int sis3316_adc::FlashEnableCS(int chip)
{
	switch (device_variant) {
	case SIS::ADC::SIS3316::TYPE_SIS3316: {
		// argument check
		if (chip < 0 && chip > 1) {
			return -100;
		}
		// SIS3316 has the option to select one of two flash chips
		// and performs a rmw cycle.
		uint32_t reg;
		int rc = register_read(SIS3316_SPI_FLASH_CSR, &reg);
		if (rc) {
			return rc;
		}

		// which chip to select
		reg |= chip ? 1 << CHIPSELECT_2 : 1 << CHIPSELECT_1;
		return register_write(SIS3316_SPI_FLASH_CSR, reg);

	}

	case SIS::ADC::SIS3316::TYPE_SIS3316_2:
		// SIS3316-2 only has one flash chip and will ignore the 'chip' argument
		// and makes use of the j-k type control register bit.
		return register_write(SIS3316_SPI_FLASH_CSR, 1 << A7_SPI_CS);

	default:
		// unknown device type, should probably return a well defined status (which i don't have as of now)
		return -10;
	}
}

/***************************************************************/
int sis3316_adc::FlashDisableCS(int chip)
{
	switch (device_variant) {
	case SIS::ADC::SIS3316::TYPE_SIS3316: {
		// argument check
		if (chip < 0 && chip > 1) {
			return -100;
		}
		// SIS3316 has the option to select one of two flash chips
		// and performs a rmw cycle.
		uint32_t reg;
		int rc = register_read(SIS3316_SPI_FLASH_CSR, &reg);
		if (rc) {
			return rc;
		}

		// which chip to select
		reg &= chip ? ~(1 << CHIPSELECT_2) : ~(1 << CHIPSELECT_1);
		return register_write(SIS3316_SPI_FLASH_CSR, reg);

	}

	case SIS::ADC::SIS3316::TYPE_SIS3316_2:
		// SIS3316-2 only has one flash chip and will ignore the 'chip' argument
		// and makes use of the j-k type control register bit.
		return register_write(SIS3316_SPI_FLASH_CSR, (1 << A7_SPI_CS) << 16);

	default:
		// unknown device type, should probably return a well defined status (which i don't have as of now)
		return -10;
	}
}

/***************************************************************/
int sis3316_adc::FlashWriteEnable()
{
	int rc;

	rc = FlashEnableCS(0);
	if (rc)
	{
		return rc;
	}

	rc = FlashXfer(FL_WRITE_ENABLE, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		return rc;
	}

	rc = FlashDisableCS(0);
	if (rc)
	{
		return rc;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::FlashWriteDisable()
{
	int rc;

	rc = FlashEnableCS(0);
	if (rc)
	{
		return rc;
	}

	rc = FlashXfer(FL_WRITE_DISABLE, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		return rc;
	}

	rc = FlashDisableCS(0);
	if (rc)
	{
		return rc;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::FlashRead(int address, char* data, int len)
{
	int rc;
	char tmp;
	tmp = 0;
	if (data == NULL)
	{
		return -100;
	}

	// select flash type
	SIS::ADC::SIS3316::flashProperties_t* props = device_variant == SIS::ADC::SIS3316::TYPE_SIS3316 ? &spiFlashPropsW25Q64 : &spiFlashPropsS25FL256;
	bool require4byte = props->size >= FL_4B_ADR_THRESHOLD;

	// enable the spi mux to the internal spi master macro
	rc = FlashEnableProg();
	if (rc) {
		return rc;
	}

	rc = FlashEnableCS(0);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	// 4 byte devices require a different read command
	uint8_t readCmd = require4byte ? FL_READ_DATA_4 : FL_READ_DATA;
	rc = FlashXfer(readCmd, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	// and an additional address byte
	if (require4byte) {
		rc = FlashXfer((char)(address >> 24), NULL);
		if (rc)
		{
			FlashDisableCS(0);
			FlashDisableProg();
			return rc;
		}
	}

	rc = FlashXfer((char)(address >> 16), NULL);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}
	rc = FlashXfer((char)(address >> 8), NULL);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}
	rc = FlashXfer((char)address, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	char char_temp = 0;
	for (int i = 0; i < len; i++)
	{
		rc = FlashXfer(char_temp, &tmp);
		data[i] = tmp;
		if (rc)
		{
			FlashDisableCS(0);
			FlashDisableProg();
			return rc;
		}
	}

	rc = FlashDisableCS(0);
	if (rc)
	{
		FlashDisableProg();
		return rc;
	}

	rc = FlashDisableProg();
	if (rc) {
		return rc;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::FlashProgramPage(int address, char* data, int len)
{
	int rc;
	int rc_dma_error = 0;
	char tmp;
	UINT utmp;
	UINT putWords;

	if (data == NULL)
	{
		return -100;
	}

	rc = FlashWriteEnable();
	if (rc)
	{
		return rc;
	}

	// select flash type
	SIS::ADC::SIS3316::flashProperties_t* props = device_variant == SIS::ADC::SIS3316::TYPE_SIS3316 ? &spiFlashPropsW25Q64 : &spiFlashPropsS25FL256;
	bool require4byte = props->size >= FL_4B_ADR_THRESHOLD;
	UINT* dmabuf = (UINT*)malloc(props->pagesize * sizeof(UINT));

	// program command
	rc = FlashEnableCS(0);
	if (rc)
	{
		FlashDisableCS(0);
		free(dmabuf);
		return rc;
	}

	// 4 byte devices require a different page program command
	uint8_t pageCmd = require4byte ? FL_PAGE_PROGRAM_4 : FL_PAGE_PROGRAM;
	rc = FlashXfer(pageCmd, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		return rc;
	}

	// and an additional address byte
	if (require4byte) {
		rc = FlashXfer((char)(address >> 24), NULL);
		if (rc)
		{
			FlashDisableCS(0);
			return rc;
		}
	}

	rc = FlashXfer((char)(address >> 16), NULL);
	if (rc)
	{
		FlashDisableCS(0);
		free(dmabuf);
		return rc;
	}
	rc = FlashXfer((char)(address >> 8), NULL);
	if (rc)
	{
		FlashDisableCS(0);
		free(dmabuf);
		return rc;
	}
	rc = FlashXfer((char)address, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		free(dmabuf);
		return rc;
	}

	if (device_variant == SIS::ADC::SIS3316::TYPE_SIS3316_2) {
		// A7
		// preload the page to 0xFF, then overwrite with potentially partial page data
		for (uint32_t k = 0; k < props->pagesize; k++) {
			dmabuf[k] = (1 << A7_SPI_PAGEWRITE) | 0xFF;
		}
		for (int k = 0; k < len; k++)
		{
			dmabuf[k] = (1 << A7_SPI_PAGEWRITE) | (uint8_t)(data[k]);
		}

		// enable page block load mode (needs to be enabled first, the fifo is held in reset when the mode is not active)
		rc = register_write(SIS3316_SPI_FLASH_CSR, 1 << A7_SPI_PAGEWRITE_EN);
		if (rc) {
			FlashDisableCS(0);
			free(dmabuf);
			return rc;
		}

		// push the page data into the page block load fifo.
		// this uses DMA_D32 cycles to reduce latency on the interface link (will translate to many A32D32 single cycles on vme).
		rc = i->vme_A32DMA_D32FIFO_write(baseaddress + SIS3316_SPI_FLASH_CSR, dmabuf, props->pagesize, &putWords);
		if (rc || (putWords != props->pagesize)) {
			rc_dma_error = -101;
		}
		free(dmabuf);

		// wait for the logic to finish transferring the data
		UINT reg;
		do {
			rc = register_read(SIS3316_SPI_FLASH_CSR, &reg);
			if (rc) {
				FlashDisableCS(0);
				return rc;
			}
			usleep(10);
		} while (reg & (1 << A7_SPI_BUSY));

		// disable page block loading for the next cycle
		rc = register_write(SIS3316_SPI_FLASH_CSR, (1 << A7_SPI_PAGEWRITE_EN) << 16);
		if (rc) {
			FlashDisableCS(0);
			return rc;
		}
	}
	else {
		// S6
		// dma d32
		for (uint32_t k = 0; k < props->pagesize; k++)
		{
			dmabuf[k] = (uint8_t)(data[k]);
		}

		rc = i->vme_A32DMA_D32FIFO_write(baseaddress + SIS3316_SPI_FLASH_DATA, dmabuf, props->pagesize, &putWords);
		if (rc || (putWords != props->pagesize))
		{
			rc_dma_error = -101;
		}
		free(dmabuf);

		// busy polling
		do
		{
			rc = register_read(SIS3316_SPI_FLASH_CSR, &utmp);
			if (rc)
			{
				return rc;
			}
			utmp &= (1 << FLASH_LOGIC_BUSY ^ 1 << FIFO_NOT_EMPTY);
			usleep(1000); // testing
		} while (utmp != 0);
	}

	// single cycles
	rc = FlashDisableCS(0);
	if (rc)
	{
		return rc;
	}
	usleep(1000); // testing
	// busy polling
	do
	{
		rc = FlashReadStatus1(&tmp);
		if (rc)
		{
			return rc;
		}
		tmp &= 1;

		usleep(1000); // testing
	} while (tmp);

	if (rc_dma_error)
	{
		return -101;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::FlashEraseBlock(int address)
{
	int rc;
	char tmp;

	rc = FlashWriteEnable();
	if (rc)
	{
		return rc;
	}

	// select flash type
	SIS::ADC::SIS3316::flashProperties_t* props = device_variant == SIS::ADC::SIS3316::TYPE_SIS3316 ? &spiFlashPropsW25Q64 : &spiFlashPropsS25FL256;
	bool require4byte = props->size >= FL_4B_ADR_THRESHOLD;

	// erase command
	rc = FlashEnableCS(0);
	if (rc)
	{
		return rc;
	}

	// 4 byte devices require a different erase block command
	uint8_t eraseCmd = require4byte ? FL_BLOCK_ERASE_64_4 : FL_BLOCK_ERASE_64;
	rc = FlashXfer(eraseCmd, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		return rc;
	}

	// and an additional address byte
	if (require4byte) {
		rc = FlashXfer((char)(address >> 24), NULL);
		if (rc)
		{
			FlashDisableCS(0);
			return rc;
		}
	}

	rc = FlashXfer((char)(address >> 16), NULL);
	if (rc)
	{
		FlashDisableCS(0);
		return rc;
	}
	rc = FlashXfer((char)(address >> 8), NULL);
	if (rc)
	{
		FlashDisableCS(0);
		return rc;
	}
	rc = FlashXfer((char)address, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		return rc;
	}

	rc = FlashDisableCS(0);
	if (rc)
	{
		return rc;
	}
	usleep(1000); // testing

	// busy polling
	do
	{
		rc = FlashReadStatus1(&tmp);
		if (rc)
		{
			return rc;
		}
		tmp &= 1;

		usleep(1000); // testing
	} while (tmp);

	return 0;
}

/***************************************************************/
int sis3316_adc::FlashWriteSR1CR1(char sr, char cr)
{
	int rc;

	rc = FlashWriteEnable();
	if (rc) {
		return rc;
	}

	rc = FlashEnableCS(0);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	rc = FlashXfer(FL_WRITE_REGS, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}
	rc = FlashXfer(sr, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}
	rc = FlashXfer(cr, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	rc = FlashDisableCS(0);
	if (rc)
	{
		FlashDisableProg();
		return rc;
	}

	return rc;
}

/***************************************************************/
int sis3316_adc::FlashXfer(char in, char* out)
{
	UINT tmp;
	char ctmp;
	int rc;

	if (device_variant == SIS::ADC::SIS3316::TYPE_SIS3316_2) {
		// A7
		tmp = (1 << A7_SPI_EXCH) | (uint8_t)in;
		rc = register_write(SIS3316_SPI_FLASH_CSR, tmp);
		if (rc)
		{
			return rc;
		}

		do
		{
			rc = register_read(SIS3316_SPI_FLASH_CSR, &tmp);
			if (rc)
			{
				return rc;
			}
		} while (tmp & 1 << A7_SPI_BUSY);
	}
	else {
		// S6
		tmp = in;
		rc = register_write(SIS3316_SPI_FLASH_DATA, tmp);
		if (rc)
		{
			return rc;
		}

		do
		{
			rc = register_read(SIS3316_SPI_FLASH_CSR, &tmp);
			if (rc)
			{
				return rc;
			}

			tmp &= (1 << FLASH_LOGIC_BUSY ^ 1 << FIFO_NOT_EMPTY);
		} while (tmp);

		rc = register_read(SIS3316_SPI_FLASH_DATA, &tmp);
		if (rc)
		{
			return rc;
		}
	}

	ctmp = tmp & 0xFF;
	if (out)
	{
		*out = ctmp;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::FlashGetId(char* id)
{
	int rc;

	if (id == NULL)
	{
		return -100;
	}

	// enable the spi mux to the internal spi master macro
	rc = FlashEnableProg();
	if (rc) {
		return rc;
	}

	rc = FlashEnableCS(0);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	rc = FlashXfer((char)FL_JEDEC_ID, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}
	rc = FlashXfer(0, id);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}
	rc = FlashXfer(0, id + 1);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}
	rc = FlashXfer(0, id + 2);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	rc = FlashDisableCS(0);
	if (rc)
	{
		FlashDisableProg();
		return rc;
	}

	rc = FlashDisableProg();
	if (rc) {
		return rc;
	}

	return 0;
}

int sis3316_adc::FlashReadStatus1(char* status)
{
	int rc;

	if (status == NULL)
	{
		return -100;
	}

	rc = FlashEnableCS(0);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	rc = FlashXfer(FL_READ_SREG1, NULL);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}
	rc = FlashXfer(0, status);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	rc = FlashDisableCS(0);
	if (rc)
	{
		FlashDisableProg();
		return rc;
	}

	return 0;
}

int sis3316_adc::FlashReadStatus2(char* status)
{
	int rc;

	if (status == NULL)
	{
		return -100;
	}

	rc = FlashEnableCS(0);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	rc = FlashXfer((char)0x35, NULL); //
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}
	rc = FlashXfer(0, status);
	if (rc)
	{
		FlashDisableCS(0);
		FlashDisableProg();
		return rc;
	}

	rc = FlashDisableCS(0);
	if (rc)
	{
		FlashDisableProg();
		return rc;
	}

	return 0;
}

/********************************************************************************************************************************/
int sis3316_adc::I2cStart(uint32_t base)
{
	int rc;
	int i;
	UINT tmp;

	// start
	rc = register_write(base, 1 << I2C_START);
	if (rc)
	{
		return rc;
	}

	i = 0;
	do
	{
		// poll i2c fsm busy
		rc = register_read(base, &tmp);
		if (rc)
		{
			return rc;
		}
		i++;
	} while ((tmp & (1 << I2C_BUSY)) && (i < 1000));

	// register access problem
	if (i == 1000)
	{
		return -100;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::I2cStop(uint32_t base)
{
	int rc;
	int i;
	UINT tmp;

	// stop
	rc = register_write(base, 1 << I2C_STOP);
	if (rc)
	{
		return rc;
	}

	i = 0;
	do
	{
		// poll i2c fsm busy
		rc = register_read(base, &tmp);
		if (rc)
		{
			return rc;
		}
		i++;
	} while ((tmp & (1 << I2C_BUSY)) && (i < 1000));

	// register access problem
	if (i == 1000)
	{
		return -100;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::I2cWriteByte(uint32_t base, unsigned char data, char* ack)
{
	int rc;
	int i;
	UINT tmp;

	// write byte, receive ack
	rc = register_write(base, 1 << I2C_WRITE ^ data);
	if (rc)
	{
		return rc;
	}

	i = 0;
	do
	{
		// poll i2c fsm busy
		rc = register_read(base, &tmp);
		if (rc)
		{
			return rc;
		}
		i++;
	} while ((tmp & (1 << I2C_BUSY)) && (i < 1000));

	// register access problem
	if (i == 1000)
	{
		return -100;
	}

	// return ack value?
	if (ack)
	{
		// yup
		*ack = tmp & 1 << I2C_ACK ? 1 : 0;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::I2cReadByte(uint32_t base, unsigned char* data, char ack)
{
	int rc;
	int i;
	UINT tmp;
	unsigned char char_tmp;

	// read byte, put ack
	tmp = 1 << I2C_READ;
	tmp |= ack ? 1 << I2C_ACK : 0;
	rc = register_write(base, tmp);
	if (rc)
	{
		return rc;
	}

	i = 0;
	do
	{
		// poll i2c fsm busy
		rc = register_read(base, &tmp);
		if (rc)
		{
			return rc;
		}
		i++;
	} while ((tmp & (1 << I2C_BUSY)) && (i < 1000));

	// register access problem
	if (i == 1000)
	{
		return -100;
	}
	char_tmp = (unsigned char)(tmp & 0xff);
	*data = char_tmp;
	return 0;
}

/***************************************************************/
int sis3316_adc::I2cAddressSlave(uint32_t base, uint8_t adr, uint8_t dir, uint8_t* ack)
{
	uint8_t a = ((adr & 0x7F) << 1) | dir;
	return I2cWriteByte(base, a, (char *)ack);
}

/***************************************************************/
int sis3316_adc::I2cTransmission(uint32_t base, uint8_t adr, uint8_t* outData, size_t outLen, uint8_t* inData, size_t inLen)
{
	int rc;

	// start condition
	rc = I2cStart(base);
	if (rc) {
		I2cStop(base);
		return rc;
	}

	// slave address cycle
	// setup the direction bit based on the following data phase
	// if an out-phase follows, set WRITE direction, else set READ direction
	uint8_t ack, dir = (outLen > 0) ? I2C_DIR_WRITE : I2C_DIR_READ;
	rc = I2cAddressSlave(base, adr, dir, &ack);
	if (rc) {
		I2cStop(base);
		return rc;
	}
	// abort if slave is missing (NACK)
	if (!ack) {
		I2cStop(base);
		return -1;
	}

	// data write cycle (if needed)
	if (outLen > 0) {
		for (size_t i = 0; i < outLen; i++) {
			rc = I2cWriteByte(base, outData[i], (char *) & ack);
			if (rc) {
				I2cStop(base);
				return rc;
			}

			if (!ack) {
				I2cStop(base);
				return -1;
			}
		}
	}

	// data read cycle (if needed)
	if (inLen > 0) {
		// generate a REPEATSTART cycle if an out-phase preceeded the in-phase
		if (dir == I2C_DIR_WRITE) {
			// the hardware currently does not implement a repeatstart cycle.
			// simulate by using a stop/start cycle.
			// this should not create any problems, since there is only one
			// master on the bus anyways.
			/*
			rc = i2c_repeatstart(dev, base);
			if (API_ERR(rc)) {
				i2c_stop(dev, base);
				return rc;
			}
			*/
			rc = I2cStop(base);
			if (rc) {
				return rc;
			}
			rc = I2cStart(base);
			if (rc) {
				I2cStop(base);
				return rc;
			}
			// re-address slave
			rc = I2cAddressSlave(base, adr, I2C_DIR_READ, &ack);
			if (rc) {
				I2cStop(base);
				return rc;
			}
		}

		for (size_t i = 0; i < inLen; i++) {
			// read the data, NACK the last byte
			ack = i == (inLen - 1) ? 0 : 1;
			rc = I2cReadByte(base, &inData[i], ack);
			if (rc) {
				I2cStop(base);
				return rc;
			}
		}
	}

	// free bus
	rc = I2cStop(base);


	return rc;
}

/***************************************************************/
int sis3316_adc::eeprom_busy(uint32_t base)
{
	SIS::ADC::SIS3316::eeprom_24xx_t* ee = &i2cEeprom24AA025;

	uint8_t out = 0;
	// eeprom is busy when the address cycle is NACKed
	// dummy out byte needed to generate 0 R/W bit
	int rc = I2cTransmission(base, ee->adr, &out, 1, NULL, 0);

	return rc == -1;
}

/***************************************************************/
int sis3316_adc::Si570FreezeDCO(uint32_t base)
{
	int rc;
	char ack;

	// start
	rc = I2cStart(base);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	// address
	rc = I2cWriteByte(base, OSC_ADR << 1, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// register offset
	rc = I2cWriteByte(base, 0x89, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// write data
	rc = I2cWriteByte(base, 0x10, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// stop
	rc = I2cStop(base);
	if (rc)
	{
		return rc;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::Si570ReadDivider(uint32_t base, unsigned char* data)
{
	int rc;
	char ack;
	int i;

	// start
	rc = I2cStart(base);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	// address
	rc = I2cWriteByte(base, OSC_ADR << 1, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// register offset
	rc = I2cWriteByte(base, 0x0D, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	rc = I2cStart(base);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	// address + 1
	rc = I2cWriteByte(base, (OSC_ADR << 1) + 1, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// read data
	for (i = 0; i < 6; i++)
	{
		ack = 1;
		if (i == 5)
		{
			ack = 0;
		}
		rc = I2cReadByte(base, &data[i], ack);
		if (rc)
		{
			I2cStop(base);
			return rc;
		}
	}

	// stop
	rc = I2cStop(base);
	if (rc)
	{
		return rc;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::Si570Divider(uint32_t base, unsigned char* data)
{
	int rc;
	char ack;
	int i;

	// start
	rc = I2cStart(base);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	// address
	rc = I2cWriteByte(base, OSC_ADR << 1, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// register offset
	rc = I2cWriteByte(base, 0x0D, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// write data
	for (i = 0; i < 2; i++)
	{
		rc = I2cWriteByte(base, data[i], &ack);
		if (rc)
		{
			I2cStop(base);
			return rc;
		}

		if (!ack)
		{
			I2cStop(base);
			return -101;
		}
	}

	// stop
	rc = I2cStop(base);
	if (rc)
	{
		return rc;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::Si570UnfreezeDCO(uint32_t base)
{
	int rc;
	char ack;

	// start
	rc = I2cStart(base);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	// address
	rc = I2cWriteByte(base, OSC_ADR << 1, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// register offset
	rc = I2cWriteByte(base, 0x89, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// write data
	rc = I2cWriteByte(base, 0x00, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// stop
	rc = I2cStop(base);
	if (rc)
	{
		return rc;
	}

	return 0;
}

/***************************************************************/
int sis3316_adc::Si570NewFreq(uint32_t base)
{
	int rc;
	char ack;
	// start
	rc = I2cStart(base);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	// address
	rc = I2cWriteByte(base, OSC_ADR << 1, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}
	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// register offset
	rc = I2cWriteByte(base, 0x87, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// write data
	rc = I2cWriteByte(base, 0x40, &ack);
	if (rc)
	{
		I2cStop(base);
		return rc;
	}

	if (!ack)
	{
		I2cStop(base);
		return -101;
	}

	// stop
	rc = I2cStop(base);
	if (rc)
	{
		return rc;
	}

	return 0;
}

/****************************************************************************************************/
int sis3316_adc::set_frequency(int osc, unsigned char* values)
{
	int rc;

	if (values == NULL)
	{
		return -100;
	}
	if (osc > 3 || osc < 0)
	{
		return -100;
	}

	uint32_t base = SIS3316_ADC_CLK_OSC_I2C_REG + 4 * osc;

	rc = Si570FreezeDCO(base);
	if (rc)
	{
		return rc;
	}

	rc = Si570Divider(base, values);
	if (rc)
	{
		return rc;
	}

	rc = Si570UnfreezeDCO(base);
	if (rc)
	{
		return rc;
	}

	rc = Si570NewFreq(base);
	if (rc)
	{
		return rc;
	}

	// min. 10ms wait
	usleep(15000); // 15 ms

	// DCM Reset
	rc = register_write(0x438, 0);
	if (rc)
	{
		return rc;
	}
	// DCM Reset -> the DCM/PLL of the ADC-FPGAs will be stable after max. 5ms
	//              or check the DCM OK bits (ADC FPGA Status registers bit 20)
	usleep(5000); // 5 ms

	return 0;
}

/****************************************************************************************************/
int sis3316_adc::owReset(int* presence)
{
	int rc;
	UINT data;

	rc = register_write(0x24, 1 << 10); // reset
	if (rc)
	{
		return rc;
	}

	do
	{
		rc = register_read(0x24, &data);
		if (rc)
		{
			return rc;
		}
	} while (data & 1 << 31); // while busy

	if (presence)
	{
		*presence = (data & 1 << 0) ? 0 : 1;
	}

	return 0;
}

/***********************************************************/
int sis3316_adc::owRead(unsigned char* data)
{
	int rc;
	UINT reg;

	rc = register_write(0x24, 1 << 8);
	if (rc)
	{
		return rc;
	}

	do
	{
		rc = register_read(0x24, &reg);
		if (rc)
		{
			return rc;
		}
	} while (reg & 1 << 31); // while busy

	if (data)
	{
		*data = reg & 0xFF;
	}

	return 0;
}

/***********************************************************/
int sis3316_adc::owWrite(unsigned char data)
{
	int rc;
	UINT reg;

	rc = register_write(0x24, 1 << 9 ^ data);
	if (rc)
	{
		return rc;
	}

	do
	{
		rc = register_read(0x24, &reg);
		if (rc)
		{
			return rc;
		}
	} while (reg & 1 << 31); // while busy

	return 0;
}

/***********************************************************/
int sis3316_adc::owEeReadPage(int page, unsigned char* data)
{
	int rc;
	int i;

	if (page >= 80)
	{
		return -1;
	}
	if (data == NULL)
	{
		return -2;
	}

	// presence
	rc = owReset(&i);
	if (rc)
	{
		return rc;
	}
	if (!i)
	{
		return -3;
	}

	// read page
	rc = owWrite(0xCC); // skip rom
	if (rc)
	{
		return rc;
	}

	rc = owWrite(0xF0); // read memory
	if (rc)
	{
		return rc;
	}

	rc = owWrite((page * 32) & 0xFF); // adr lo
	if (rc)
	{
		return rc;
	}

	rc = owWrite(((page * 32) >> 8) & 0xFF); // adr hi
	if (rc)
	{
		return rc;
	}

	for (i = 0; i < 32; i++)
	{
		rc = owRead(data + i); // data in
		if (rc)
		{
			owReset(NULL);
			return rc;
		}
	}

	this->owReset(NULL);

	return 0;
}

/***********************************************************/
int sis3316_adc::owEeWritePage(int page, unsigned char* data)
{
	int rc;
	int i;
	unsigned char aLo, aHi, esReg;
	unsigned char vfyData[32];

	if (page >= 80)
	{
		return -1;
	}
	if (data == NULL)
	{
		return -2;
	}

	// Step 1, copy page to eeprom internal scratchpad sram

	// presence
	rc = owReset(&i);
	if (rc)
	{
		return rc;
	}
	if (!i)
	{
		return -3;
	}

	// read page
	rc = owWrite(0xCC); // skip rom
	if (rc)
	{
		return rc;
	}

	rc = owWrite(0x0F); // write scratchpad
	if (rc)
	{
		return rc;
	}

	rc = owWrite((page * 32) & 0xFF); // adr lo
	if (rc)
	{
		return rc;
	}

	rc = owWrite(((page * 32) >> 8) & 0xFF); // adr hi
	if (rc)
	{
		return rc;
	}

	for (i = 0; i < 32; i++)
	{
		rc = owWrite(*(data + i)); // data in
		if (rc)
		{
			owReset(NULL);
			return rc;
		}
	}

	owReset(NULL);

	// Step 2, verify the scratchpad

	// presence
	rc = owReset(&i);
	if (rc)
	{
		return rc;
	}
	if (!i)
	{
		return -3;
	}

	// read page
	rc = owWrite(0xCC); // skip rom
	if (rc)
	{
		return rc;
	}

	rc = owWrite(0xAA); // read scratchpad
	if (rc)
	{
		return rc;
	}

	rc = owRead(&aLo); // adr lo
	if (rc)
	{
		return rc;
	}

	rc = owRead(&aHi); // adr hi
	if (rc)
	{
		return rc;
	}

	rc = owRead(&esReg); // es reg
	if (rc)
	{
		return rc;
	}

	for (i = 0; i < 32; i++)
	{
		rc = owRead(vfyData + i); // data in
		if (rc)
		{
			owReset(NULL);
			return rc;
		}
	}

	owRead(NULL); // crc16
	owRead(NULL); // crc16

	owReset(NULL);

	// Step 3, copy scratchpad to eeprom array

	// presence
	rc = owReset(&i);
	if (rc)
	{
		return rc;
	}
	if (!i)
	{
		return -3;
	}

	// read page
	rc = owWrite(0xCC); // skip rom
	if (rc)
	{
		return rc;
	}

	rc = owWrite(0x55); // copy scratchpad
	if (rc)
	{
		return rc;
	}

	rc = owWrite(aLo); // adr lo
	if (rc)
	{
		return rc;
	}

	rc = owWrite(aHi); // adr hi
	if (rc)
	{
		return rc;
	}

	rc = owWrite(esReg); // es reg
	if (rc)
	{
		return rc;
	}

	usleep(50000);

	owReset(NULL);

	return 0;
}

/***********************************************************/
int sis3316_adc::read_ee(int offset, int len, unsigned char* data)
{
	int rc;

	// argument sanity check
	if (data == NULL)
	{
		return -1;
	}
	if (len == 0)
	{
		return -3;
	}

	// SIS3316 type device uses a one-wire eeprom with 2560 bytes size.
	// SIS3316-2 type device uses an i2c eeprom with 256 bytes size (only the first 128 are writable).
	if (device_variant == SIS::ADC::SIS3316::TYPE_SIS3316) {
		int pageNum, localLen = len, pageOffs, localOffs = offset;
		int copyLen;
		unsigned char page[32];

		// specified argument check
		if (offset + len > 2560)
		{
			return -2;
		}

		// page loop
		// read
		while (localLen)
		{
			// page to start
			pageNum = localOffs / 32;
			// offset within first page
			pageOffs = localOffs % 32;

			// read
			rc = owEeReadPage(pageNum, page);
			if (rc)
			{
				return rc;
			}

			if ((pageOffs + localLen) > 32)
			{
				copyLen = 32 - pageOffs;
			}
			else
			{
				copyLen = localLen;
			}

			// copy back
			memcpy(data, page + pageOffs, copyLen);

			// adjust
			localLen -= copyLen;
			localOffs += copyLen;
			data += copyLen;
		}
	}
	else if (device_variant == SIS::ADC::SIS3316::TYPE_SIS3316_2) {
		SIS::ADC::SIS3316::eeprom_24xx_t* ee = &i2cEeprom24AA025;
		// argument sanity check
		if ((size_t)offset >= ee->size || (offset + len) > ee->size) {
			return -2;
		}

		// build i2c command
		uint8_t out[2];
		if (ee->offsLen == 1) {
			out[0] = (uint8_t)offset;
		}
		else {
			out[0] = (uint8_t)(offset >> 8);
			out[1] = offset & 0xFF;
		}
		rc = register_write(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x1000000); // disable autoread
		if (rc) {
			register_write(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x2000000); // enable autoread
			return rc;
		}
		usleep(10000); // 10ms

		// execute the bus transmission
		rc = I2cTransmission(SIS3316_REG_I2C_PERIPHERAL_BASE, ee->adr, out, ee->offsLen, data, len);
		register_write(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x2000000); // enable autoread
		return rc ;
	}

	return 0;
}

/***********************************************************/
int sis3316_adc::write_ee(int offset, int len, unsigned char* data)
{
	int rc;

	if (data == NULL)
	{
		return -1;
	}
	if (len == 0)
	{
		return -3;
	}

	// SIS3316 type device uses a one-wire eeprom with 256 bytes size.
	// SIS3316-2 type device uses an i2c eeprom with 256 bytes size (only the first 128 are writable).
	if (device_variant == SIS::ADC::SIS3316::TYPE_SIS3316) {
		int pageNum, localLen = len, pageOffs, localOffs = offset;
		int copyLen;
		unsigned char page[32];


		// specified argument check
		if (offset + len > 2560)
		{
			return -2;
		}

		// page loop
		// read-modify-write
		while (localLen)
		{
			// page to start
			pageNum = localOffs / 32;
			// offset within first page
			pageOffs = localOffs % 32;

			// read
			rc = owEeReadPage(pageNum, page);
			if (rc)
			{
				return rc;
			}

			if ((pageOffs + localLen) > 32)
			{
				copyLen = 32 - pageOffs;
			}
			else
			{
				copyLen = localLen;
			}

			// modify
			memcpy(page + pageOffs, data, copyLen);

			// write
			rc = owEeWritePage(pageNum, page);
			if (rc)
			{
				return 0;
			}

			// adjust
			localLen -= copyLen;
			localOffs += copyLen;
			data += copyLen;
		}
	}
	else if (device_variant == SIS::ADC::SIS3316::TYPE_SIS3316_2) {
		SIS::ADC::SIS3316::eeprom_24xx_t* ee = &i2cEeprom24AA025;

		if ((size_t)offset >= ee->size || (offset + len) > ee->size) {
			return -2;
		}

		// i2c command buffer (1 or 2 address bytes and up to pageSize data bytes)
		uint8_t* out = (uint8_t *)malloc(2 + ee->pageSize);
		if (out == NULL) {
			return -1;
		}

		rc = register_write(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x1000000); // disable autoread
		if (rc) {
			register_write(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x2000000); // enable autoread
			return rc;
		}
		usleep(10000); // 10ms


		size_t pageMask = ee->pageSize - 1;
		size_t written = 0;

		while (len) {
			// determine maxmium length for the current page based on the offset within the page
			size_t maxLen = ee->pageSize - (offset & pageMask);
			// clip to the remaining data size
			size_t currLen = len > maxLen ? maxLen : len;

			// build i2c command
			if (ee->offsLen == 1) {
				out[0] = (uint8_t)offset;
				memcpy(&out[1], &data[written], currLen);
			}
			else {
				out[0] = (uint8_t)(offset >> 8);
				out[1] = (uint8_t)(offset & 0xFF);
				memcpy(&out[2], &data[written], currLen);
			}

			// execute bus transmission
			rc = I2cTransmission(SIS3316_REG_I2C_PERIPHERAL_BASE, ee->adr, out,
				(size_t)(ee->offsLen + currLen), // add the address byte(s) to the write length
				NULL, 0);
			if (rc) {
				register_write(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x2000000); // enable autoread
				free(out);
				return rc;
			}

			// poll busy
			int loop = 0;
			while ((loop < EEPROM_BUSY_TIMEOUT) && eeprom_busy(SIS3316_REG_I2C_PERIPHERAL_BASE)) {
				usleep(1000);
				loop++;
			}
			if (loop >= EEPROM_BUSY_TIMEOUT) {
				register_write(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x2000000); // enable autoread
				free(out);
				return -5;
			}

			// adjust remaining data size
			len -= currLen;
			offset += (off_t)currLen;
			written += currLen;
		}

		register_write(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x2000000); // enable autoread
		free(out);
	}

	return 0;
}

/***********************************************************/
int sis3316_adc::ow_id_ee(unsigned char* data)
{
	int rc;
	int i;

	// presence
	rc = owReset(&i);
	if (rc)
	{
		return rc;
	}
	if (!i)
	{
		return -1;
	}

	// read rom cmd
	rc = owWrite(0x33);
	if (rc)
	{
		return rc;
	}

	// read data
	for (i = 0; i < 8; i++)
	{
		rc = owRead(data + i);
		if (rc)
		{
			return rc;
		}
	}

	return 0;
}

/***********************************************************/
int sis3316_adc::write_ow_dhcp_option(unsigned char* data)
{
	int rc;
	rc = write_ee(2, 1, data);
	if (rc)
	{
		return rc;
	}

	return 0;
}

/***********************************************************/
int sis3316_adc::DrpPollBusy(uint32_t base)
{
	int rc;
	bool busy = true;
	while (busy) {
		uint32_t reg;
		rc = register_read(base, &reg);
		if (rc) {
			return rc;
		}

		busy = reg & (1 << DRP_BUSY) ? true : false;
	}

	return rc;
}

/***********************************************************/
int sis3316_adc::DrpReadReg(uint32_t base, uint8_t adr, uint16_t* data)
{
	int rc;

	// generate a drp-read cycle at the given drp register address
	uint32_t reg = (uint32_t)adr << DRP_ADR | 1 << DRP_ENABLE;
	rc = register_write(base, reg);
	if (rc) {
		return rc;
	}

	// poll busy
	rc = DrpPollBusy(base);
	if (rc) {
		return rc;
	}

	// get read data from register
	rc = register_read(base, &reg);
	if (rc) {
		return rc;
	}

	*data = reg & 0xFFFF;

	return rc;
}

/***********************************************************/
int sis3316_adc::DrpWriteReg(uint32_t base, uint8_t adr, uint16_t data)
{
	int rc;

	// generate a drp-read cycle at the given drp register address
	uint32_t reg =
		(uint32_t)adr << DRP_ADR | 1 << DRP_ENABLE | 1 << DRP_WRCYC | (uint32_t)data << DRP_DATA;
	rc = register_write(base, reg);
	if (rc) {
		return rc;
	}

	// poll busy
	return DrpPollBusy(base);
}

/***********************************************************/
int sis3316_adc::DrpRmwReg(uint32_t base, uint8_t adr, uint16_t mask, uint16_t data)
{
	uint16_t reg;
	int rc;

	rc = DrpReadReg(base, adr, &reg);
	if (rc) {
		return rc;
	}

	reg &= ~mask;
	reg |= (data & mask);

	return DrpWriteReg(base, adr, reg);
}

/***********************************************************/
int sis3316_adc::readSensor(SIS::ADC::SIS3316::sensor channel, double* value) 
{
	// this feature is only supported on the SIS3316-2 module type
	if (device_variant != SIS::ADC::SIS3316::TYPE_SIS3316_2) {
		return -1;
	}

	int rc;
	uint8_t data[3];

	// split into different source busses based on the sensor channel
	switch (channel) {
		// I2C sensors on the peripheral i2c master
	case SIS::ADC::SIS3316::TEMP_TOP: {
		// out, read cmd
		data[0] = 0;
		rc = I2cTransmission(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x48, data, 1, data, 2);
		if (rc) {
			return rc;
		}
		// convert to floating point
		int16_t cnv = (int16_t)data[0] << 8 | data[1];
		*value = (double)cnv / 256.0;
	} break;

	case SIS::ADC::SIS3316::TEMP_MID: {
		// out, read cmd
		data[0] = 0;
		rc = I2cTransmission(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x49, data, 1, data, 2);
		if (rc) {
			return rc;
		}
		// convert to floating point
		int16_t cnv = (int16_t)data[0] << 8 | data[1];
		*value = (double)cnv / 256.0;
	} break;

	case SIS::ADC::SIS3316::TEMP_BOT: {
		// out, read cmd
		data[0] = 0;
		rc = I2cTransmission(SIS3316_REG_I2C_PERIPHERAL_BASE, 0x4A, data, 1, data, 2);
		if (rc) {
			return rc;
		}
		// convert to floating point
		int16_t cnv = (int16_t)data[0] << 8 | data[1];
		*value = (double)cnv / 256.0;
	} break;



#ifdef NOT_IMPLEMENTED_YET		
	// voltage and temp sensors on the vme xadc
	case SIS::ADC::SIS3316::TEMP_VME_CORE: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 0, &reg);
		if (rc) {
			return rc;
		}

		*value = xadc_calcTemp(reg);
	} break;

	case SIS::ADC::SIS3316::TEMP_PWR_U90: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 24, &reg);
		if (rc) {
			return rc;
		}

		*value = ltm_diode_fact(xadc_calcVoltage(reg, 1.0));
	} break;

	case SIS::ADC::SIS3316::TEMP_PWR_U91: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 16, &reg);
		if (rc) {
			return rc;
		}

		*value = ltm_diode_fact(xadc_calcVoltage(reg, 1.0));
	} break;

	case SIS::ADC::SIS3316::TEMP_PWR_U92: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 18, &reg);
		if (rc) {
			return rc;
		}

		*value = ltm_diode_fact(xadc_calcVoltage(reg, 1.0));
	} break;

	case SIS::ADC::SIS3316::VOLTAGE_VME_VCCINT: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 1, &reg);
		if (rc) {
			return rc;
		}

		*value = xadc_calcVoltage(reg, 3.0);
	} break;

	case SIS::ADC::SIS3316::VOLTAGE_VME_VCCBRAM: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 6, &reg);
		if (rc) {
			return rc;
		}

		*value = xadc_calcVoltage(reg, 3.0);
	} break;

	case SIS::ADC::SIS3316::VOLTAGE_VME_VCCAUX: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 2, &reg);
		if (rc) {
			return rc;
		}

		*value = xadc_calcVoltage(reg, 3.0);
	} break;

	case SIS::ADC::SIS3316::VOLTAGE_C2V5: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 19, &reg);
		if (rc) {
			return rc;
		}

		*value = divider_fact(xadc_calcVoltage(reg, 1.0), 2.4e3, 1e3);
	} break;

	case SIS::ADC::SIS3316::VOLTAGE_D1V5: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 17, &reg);
		if (rc) {
			return rc;
		}

		*value = divider_fact(xadc_calcVoltage(reg, 1.0), 1e3, 1e3);
	} break;

	case SIS::ADC::SIS3316::VOLTAGE_D3V3: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 25, &reg);
		if (rc) {
			return rc;
		}

		*value = divider_fact(xadc_calcVoltage(reg, 1.0), 4.7e3, 1e3);
	} break;

	case SIS::ADC::SIS3316::VOLTAGE_MGT1V0: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 26, &reg);
		if (rc) {
			return rc;
		}

		*value = divider_fact(xadc_calcVoltage(reg, 1.0), 4.7e3, 4.7e3);
	} break;

	case SIS::ADC::SIS3316::VOLTAGE_MGT1V2: {
		uint16_t reg;
		rc = DrpReadReg(SIS3316_REG_XADC_DRP_BASE, 27, &reg);
		if (rc) {
			return rc;
		}

		*value = divider_fact(xadc_calcVoltage(reg, 1.0), 4.7e3, 4.7e3);
	} break;

#endif

	default:
		return -1;
	}

	return 0;
}

/****************************************************************************************************/
/****************************************************************************************************/

/****************************************************************************************************/
/*                                                                                                  */
/*     change_frequency_HSdiv_N1div                                                                 */
/*     hs_div_val: allowed values are [4, 5, 6, 7, 9, 11]                                           */
/*     n1_div_val: allowed values are [2, 4, 6, .... 124, 126]                                      */
/*     Sample Frequence =  5 GHz / (hs_div_val * n1_div_val)                                        */
/*                                                                                                  */
/*     example:                                                                                     */
/*     hs_div_val = 5                                                                               */
/*     n1_div_val = 4                                                                               */
/*     Sample Frequence =  5 GHz / 20 = 250 MHz                                                     */
/*                                                                                                  */
/****************************************************************************************************/

int sis3316_adc::change_frequency_HSdiv_N1div(int osc, unsigned int hs_div_val, unsigned int n1_div_val)
{
	int rc;
	unsigned i;
	unsigned N1div;
	unsigned HSdiv;
	unsigned HSdiv_reg[6];
	unsigned HSdiv_val[6];
	unsigned char freqSI570_high_speed_rd_value[6];
	unsigned char freqSI570_high_speed_wr_value[6];

	if (osc > 3 || osc < 0)
	{
		return -100;
	}

	uint32_t base = SIS3316_ADC_CLK_OSC_I2C_REG + 4 * osc;

	HSdiv_reg[0] = 0;
	HSdiv_val[0] = 4;

	HSdiv_reg[1] = 1;
	HSdiv_val[1] = 5;

	HSdiv_reg[2] = 2;
	HSdiv_val[2] = 6;

	HSdiv_reg[3] = 3;
	HSdiv_val[3] = 7;

	HSdiv_reg[4] = 5;
	HSdiv_val[4] = 9;

	HSdiv_reg[5] = 7;
	HSdiv_val[5] = 11;

	HSdiv = 0xff;
	for (i = 0; i < 6; i++)
	{
		if (HSdiv_val[i] == hs_div_val)
		{
			HSdiv = HSdiv_reg[i];
		}
	}
	if (HSdiv > 11)
	{
		return -101;
	}

	// gt than 127 or odd then return
	if ((n1_div_val > 127) || ((n1_div_val & 0x1) == 1) || (n1_div_val == 0))
	{
		return -102;
	}
	N1div = n1_div_val - 1;

	rc = Si570ReadDivider(base, freqSI570_high_speed_rd_value);
	if (rc)
	{
		printf("Si570ReadDivider = %d \n", rc);
		return rc;
	}
	freqSI570_high_speed_wr_value[0] = ((HSdiv & 0x7) << 5) + ((N1div & 0x7c) >> 2);
	freqSI570_high_speed_wr_value[1] = ((N1div & 0x3) << 6) + (freqSI570_high_speed_rd_value[1] & 0x3F);
	freqSI570_high_speed_wr_value[2] = freqSI570_high_speed_rd_value[2];
	freqSI570_high_speed_wr_value[3] = freqSI570_high_speed_rd_value[3];
	freqSI570_high_speed_wr_value[4] = freqSI570_high_speed_rd_value[4];
	freqSI570_high_speed_wr_value[5] = freqSI570_high_speed_rd_value[5];

	rc = this->set_frequency(osc, freqSI570_high_speed_wr_value);
	if (rc)
	{
		printf("set_frequency = %d \n", rc);
		return rc;
	}

	return 0;
}

/****************************************************************************************************/
int sis3316_adc::get_frequency(int osc, unsigned char* values)
{
	int rc;

	if (values == NULL)
	{
		return -100;
	}
	if (osc > 3 || osc < 0)
	{
		return -100;
	}

	uint32_t base = SIS3316_ADC_CLK_OSC_I2C_REG + 4 * osc;

	rc = Si570ReadDivider(base, values);
	if (rc)
	{
		return rc;
	}

	return 0;
}

/****************************************************************************************************/

 
int sis3316_adc::get_SI570_oscillator_hs_div_and_n1_div_values(unsigned int enum_sample_rate, unsigned int* hs_div_val, unsigned int* n1_div_val, double* double_get_frequency)
{
	switch (enum_sample_rate) {
	case SIS::ADC::SIS3316::SAMPLERATE_250MSPS: 
		*n1_div_val = 4;
		*hs_div_val = 5;
		*double_get_frequency = 250000000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_227MSPS: // 227.273 MHz
		*n1_div_val = 2;
		*hs_div_val = 11;
		*double_get_frequency = 227273000.0;
		;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_208MSPS: // 208,333 MHz
		*n1_div_val = 4;
		*hs_div_val = 6;
		*double_get_frequency = 208333000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_179MSPS: // 178,571 MHz
		*n1_div_val = 4;
		*hs_div_val = 7;
		*double_get_frequency = 178571000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_167MSPS: // 166.667 MHz
		*n1_div_val = 6;
		*hs_div_val = 5;
		*double_get_frequency = 166667000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_139MSPS: // 138.889 MHz
		*n1_div_val = 6;
		*hs_div_val = 6;
		*double_get_frequency = 138889000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_125MSPS:
		*n1_div_val = 8;
		*hs_div_val = 5;
		*double_get_frequency = 125000000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_119MSPS: // 119.048 MHz
		*n1_div_val = 6;
		*hs_div_val = 7;
		*double_get_frequency = 119048000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_114MSPS: // 113.636 MHz
		*n1_div_val = 4;
		*hs_div_val = 11;
		*double_get_frequency = 113636000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_104MSPS: // 104.167 MHz
		*n1_div_val = 8;
		*hs_div_val = 6;
		*double_get_frequency = 104167000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_100MSPS:
		*n1_div_val = 10;
		*hs_div_val = 5;
		*double_get_frequency = 100000000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_83MSPS: // 83.333 MHz
		*n1_div_val = 12;
		*hs_div_val = 5;
		*double_get_frequency = 83333000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_71MSPS: // 71.429 MHz
		*n1_div_val = 14;
		*hs_div_val = 5;
		*double_get_frequency = 71429000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_62M5SPS: // 62.500 MHz
		*n1_div_val = 16;
		*hs_div_val = 5;
		*double_get_frequency = 62500000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_50MSPS:
		*n1_div_val = 20;
		*hs_div_val = 5;
		*double_get_frequency = 50000000.0;
		break;
	case SIS::ADC::SIS3316::SAMPLERATE_25MSPS:
		*n1_div_val = 40;
		*hs_div_val = 5;
		*double_get_frequency = 25000000.0;
		break;
	default:     // 125.000 MHz
		*n1_div_val = 8;
		*hs_div_val = 5;
		*double_get_frequency = 125000000.0;
		break;
	}
	return 0;
}

/****************************************************************************************************/
int sis3316_adc::get_adc_fpga_iob_delay_value(unsigned int enum_sample_rate, unsigned int* iob_delay_value)
{

	if (this->device_variant == SIS::ADC::SIS3316::TYPE_SIS3316) { // SIS3316


		if (this->adc_125MHz_flag == 0) { // 250 MHz
			*iob_delay_value = 0x1002; // SAMPLERATE_250MSPS
			switch (enum_sample_rate) {
				case SIS::ADC::SIS3316::SAMPLERATE_250MSPS: *iob_delay_value = 0x1002; break;
				case SIS::ADC::SIS3316::SAMPLERATE_227MSPS: *iob_delay_value = 0x101f; break;
				case SIS::ADC::SIS3316::SAMPLERATE_208MSPS: *iob_delay_value = 0x1035; break;
				case SIS::ADC::SIS3316::SAMPLERATE_179MSPS: *iob_delay_value = 0x12;   break;
				case SIS::ADC::SIS3316::SAMPLERATE_167MSPS: *iob_delay_value = 0x20;   break;
				case SIS::ADC::SIS3316::SAMPLERATE_139MSPS: *iob_delay_value = 0x35;   break;
				case SIS::ADC::SIS3316::SAMPLERATE_125MSPS: *iob_delay_value = 0x50;   break;
				case SIS::ADC::SIS3316::SAMPLERATE_119MSPS: *iob_delay_value = 0x60;   break;
				case SIS::ADC::SIS3316::SAMPLERATE_114MSPS: *iob_delay_value = 0x1010; break;
				case SIS::ADC::SIS3316::SAMPLERATE_104MSPS: *iob_delay_value = 0x1020; break;
				case SIS::ADC::SIS3316::SAMPLERATE_100MSPS: *iob_delay_value = 0x1020; break;
				case SIS::ADC::SIS3316::SAMPLERATE_83MSPS:  *iob_delay_value = 0x1030; break;
				case SIS::ADC::SIS3316::SAMPLERATE_71MSPS:  *iob_delay_value = 0x1060; break;
				case SIS::ADC::SIS3316::SAMPLERATE_62M5SPS: *iob_delay_value = 0x1060; break;
				case SIS::ADC::SIS3316::SAMPLERATE_50MSPS:  *iob_delay_value = 0x20;   break;
				case SIS::ADC::SIS3316::SAMPLERATE_25MSPS:  *iob_delay_value = 0x20;   break;
				default:     return -11;  // unknown or invalid sample_rate type
					break;
			}
		}
		else { // 125MHz
			*iob_delay_value = 0x1020; // SAMPLERATE_125MSPS
			switch (enum_sample_rate) {
				case SIS::ADC::SIS3316::SAMPLERATE_125MSPS:    *iob_delay_value = 0x1020;  break;
				case SIS::ADC::SIS3316::SAMPLERATE_119MSPS:    *iob_delay_value = 0x1020;  break;
				case SIS::ADC::SIS3316::SAMPLERATE_114MSPS:    *iob_delay_value = 0x1020;  break;
				case SIS::ADC::SIS3316::SAMPLERATE_104MSPS:    *iob_delay_value = 0x1030;  break;
				case SIS::ADC::SIS3316::SAMPLERATE_100MSPS:    *iob_delay_value = 0x1030;  break;
				case SIS::ADC::SIS3316::SAMPLERATE_83MSPS:     *iob_delay_value = 0x1040;  break;
				case SIS::ADC::SIS3316::SAMPLERATE_71MSPS:     *iob_delay_value = 0x1060;  break;
				case SIS::ADC::SIS3316::SAMPLERATE_62M5SPS:    *iob_delay_value = 0x20;    break;
				case SIS::ADC::SIS3316::SAMPLERATE_50MSPS:     *iob_delay_value = 0x30;    break;
				case SIS::ADC::SIS3316::SAMPLERATE_25MSPS:     *iob_delay_value = 0x30;    break;
				default: 	return -11;  // unknown or invalid sample_rate type
					break;
				
			}
		}

	}
	else { // SIS3316-2
		if (this->adc_125MHz_flag == 0) { // 250 MHz
			*iob_delay_value = 0x2; // SAMPLERATE_250MSPS
			switch (enum_sample_rate) {
			case SIS::ADC::SIS3316::SAMPLERATE_250MSPS:    *iob_delay_value = 0x2; break;
			case SIS::ADC::SIS3316::SAMPLERATE_227MSPS:    *iob_delay_value = 0x9; break;
			case SIS::ADC::SIS3316::SAMPLERATE_208MSPS:    *iob_delay_value = 0xA; break;
			case SIS::ADC::SIS3316::SAMPLERATE_179MSPS:    *iob_delay_value = 0x19; break;
			case SIS::ADC::SIS3316::SAMPLERATE_167MSPS:    *iob_delay_value = 0x1002; break;
			case SIS::ADC::SIS3316::SAMPLERATE_139MSPS:    *iob_delay_value = 0x100C; break;
			case SIS::ADC::SIS3316::SAMPLERATE_125MSPS:    *iob_delay_value = 0x1010; break;
			case SIS::ADC::SIS3316::SAMPLERATE_119MSPS:    *iob_delay_value = 0x1010; break;
			case SIS::ADC::SIS3316::SAMPLERATE_114MSPS:    *iob_delay_value = 0x1010; break;
			case SIS::ADC::SIS3316::SAMPLERATE_104MSPS:    *iob_delay_value = 0x1014; break;
			case SIS::ADC::SIS3316::SAMPLERATE_100MSPS:    *iob_delay_value = 0x1016; break;
			case SIS::ADC::SIS3316::SAMPLERATE_83MSPS:     *iob_delay_value = 0x0; break;
			case SIS::ADC::SIS3316::SAMPLERATE_71MSPS:     *iob_delay_value = 0xC; break;
			case SIS::ADC::SIS3316::SAMPLERATE_62M5SPS:    *iob_delay_value = 0x10; break;
			case SIS::ADC::SIS3316::SAMPLERATE_50MSPS:     *iob_delay_value = 0x10; break;
			case SIS::ADC::SIS3316::SAMPLERATE_25MSPS:     *iob_delay_value = 0x10; break;
			default:    return -11; break; // unknown or invalid sample_rate type
			}
		}
		else { // 125MHz
			*iob_delay_value = 0x1000; // SAMPLERATE_125MSPS
			switch (enum_sample_rate) {
			case SIS::ADC::SIS3316::SAMPLERATE_125MSPS:    *iob_delay_value = 0x1008; break;
			case SIS::ADC::SIS3316::SAMPLERATE_119MSPS:    *iob_delay_value = 0x1008; break;
			case SIS::ADC::SIS3316::SAMPLERATE_114MSPS:    *iob_delay_value = 0x1010; break;
			case SIS::ADC::SIS3316::SAMPLERATE_104MSPS:    *iob_delay_value = 0x1010; break;
			case SIS::ADC::SIS3316::SAMPLERATE_100MSPS:    *iob_delay_value = 0x1010; break;
			case SIS::ADC::SIS3316::SAMPLERATE_83MSPS:     *iob_delay_value = 0x1014; break;
			case SIS::ADC::SIS3316::SAMPLERATE_71MSPS:     *iob_delay_value = 0x101E; break;
			case SIS::ADC::SIS3316::SAMPLERATE_62M5SPS:    *iob_delay_value = 0x8; break;
			case SIS::ADC::SIS3316::SAMPLERATE_50MSPS:     *iob_delay_value = 0x10; break;
			case SIS::ADC::SIS3316::SAMPLERATE_25MSPS:     *iob_delay_value = 0x10; break;
			default:  	return -11; break;  // unknown or invalid sample_rate type
			}
		}

	}

#ifdef raus
	case SIS::ADC::SIS3316::TYPE_SIS3316_2:
		if (this->adc_125MHz_flag == 0) { // 250 MHz
			*iob_delay_value = 0x2; // SAMPLERATE_250MSPS
			switch (enum_sample_rate) {
			case SIS::ADC::SIS3316::SAMPLERATE_250MSPS: {   *iob_delay_value = 0x2; }
			case SIS::ADC::SIS3316::SAMPLERATE_227MSPS: {   *iob_delay_value = 0x9; }
			case SIS::ADC::SIS3316::SAMPLERATE_208MSPS: {   *iob_delay_value = 0xA; }
			case SIS::ADC::SIS3316::SAMPLERATE_179MSPS: {   *iob_delay_value = 0x19; }
			case SIS::ADC::SIS3316::SAMPLERATE_167MSPS: {   *iob_delay_value = 0x1002; }
			case SIS::ADC::SIS3316::SAMPLERATE_139MSPS: {   *iob_delay_value = 0x100C; }
			case SIS::ADC::SIS3316::SAMPLERATE_125MSPS: {   *iob_delay_value = 0x1010; }
			case SIS::ADC::SIS3316::SAMPLERATE_119MSPS: {   *iob_delay_value = 0x1010; }
			case SIS::ADC::SIS3316::SAMPLERATE_114MSPS: {   *iob_delay_value = 0x1010; }
			case SIS::ADC::SIS3316::SAMPLERATE_104MSPS: {   *iob_delay_value = 0x1014; }
			case SIS::ADC::SIS3316::SAMPLERATE_100MSPS: {   *iob_delay_value = 0x1016; }
			case SIS::ADC::SIS3316::SAMPLERATE_83MSPS:  {   *iob_delay_value = 0x0; }
			case SIS::ADC::SIS3316::SAMPLERATE_71MSPS:  {   *iob_delay_value = 0xC; }
			case SIS::ADC::SIS3316::SAMPLERATE_62M5SPS: {   *iob_delay_value = 0x10; }
			case SIS::ADC::SIS3316::SAMPLERATE_50MSPS:  {   *iob_delay_value = 0x10; }
			case SIS::ADC::SIS3316::SAMPLERATE_25MSPS:  {   *iob_delay_value = 0x10; }
			default: {   return -11; } // unknown or invalid sample_rate type
			}
		}
		else { // 125MHz
			*iob_delay_value = 0x1020; // SAMPLERATE_125MSPS
			switch (enum_sample_rate) {
			case SIS::ADC::SIS3316::SAMPLERATE_125MSPS: {   *iob_delay_value = 0x1020; }
			case SIS::ADC::SIS3316::SAMPLERATE_119MSPS: {   *iob_delay_value = 0x1020; }
			case SIS::ADC::SIS3316::SAMPLERATE_114MSPS: {   *iob_delay_value = 0x1020; }
			case SIS::ADC::SIS3316::SAMPLERATE_104MSPS: {   *iob_delay_value = 0x1030; }
			case SIS::ADC::SIS3316::SAMPLERATE_100MSPS: {   *iob_delay_value = 0x1030; }
			case SIS::ADC::SIS3316::SAMPLERATE_83MSPS: {   *iob_delay_value = 0x1040; }
			case SIS::ADC::SIS3316::SAMPLERATE_71MSPS: {   *iob_delay_value = 0x1060; }
			case SIS::ADC::SIS3316::SAMPLERATE_62M5SPS: {   *iob_delay_value = 0x20; }
			case SIS::ADC::SIS3316::SAMPLERATE_50MSPS: {   *iob_delay_value = 0x30; }
			case SIS::ADC::SIS3316::SAMPLERATE_25MSPS: {   *iob_delay_value = 0x30; }
			default: {	return -11; } // unknown or invalid sample_rate type
			}
		}

	}

	default: {
			*iob_delay_value = 0x1002; // SAMPLERATE_250MSPS
			// unknown device type, should probably return a well defined status (which i don't have as of now)
			return -10;
		}
	}
#endif


#ifdef raus

	unsigned int adc_125MHz_flag;
	unsigned int device_variant;  // 0 -> SIS3316   ,  2 -> SIS3316-2

#endif

	return 0;
}

/****************************************************************************************************/
int sis3316_adc::configure_adc_fpga_iob_delays(unsigned int iob_delay_value)
{
	int rc;

	rc = register_write(SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, 0xf00); // Calibrate IOB _delay Logic
	if (rc)
	{
		return rc;
	}
	rc = register_write(SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, 0xf00); // Calibrate IOB _delay Logic
	if (rc)
	{
		return rc;
	}
	rc = register_write(SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, 0xf00); // Calibrate IOB _delay Logic
	if (rc)
	{
		return rc;
	}
	rc = register_write(SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0xf00); // Calibrate IOB _delay Logic
	if (rc)
	{
		return rc;
	}
	usleep(10);
	rc = register_write(SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, 0x300 + iob_delay_value); // set IOB _delay Logic
	if (rc)
	{
		return rc;
	}
	rc = register_write(SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, 0x300 + iob_delay_value); // set IOB _delay Logic
	if (rc)
	{
		return rc;
	}
	rc = register_write(SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, 0x300 + iob_delay_value); // set IOB _delay Logic
	if (rc)
	{
		return rc;
	}
	rc = register_write(SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0x300 + iob_delay_value); // set IOB _delay Logic
	if (rc)
	{
		return rc;
	}
	usleep(100);

	return 0;
}

/****************************************************************************************************/
// Sample Clock DCM/PLL Reset on each SIS3316 FPGA ;
int sis3316_adc::reset_adc_fpga_sample_clock_PLL(void)
{
	int rc;
	rc = register_write(SIS3316_KEY_ADC_CLOCK_DCM_RESET, 0x0); //
	// min. 10ms wait or check if PLL is locked
	usleep(20000);

	return rc;
}
/****************************************************************************************************/
// Sample Clock DCM/PLL Reset on each SIS3316 FPGA ;
int sis3316_adc::reset_adc_fpga_and_DDR_memory(void)
{
	int rc;
	rc = register_write(SIS3316_KEY_ADC_FPGA_RESET, 0x0); //
	//
	usleep(20000);

	return rc;
}

/****************************************************************************************************/
int sis3316_adc::si5325_clk_muliplier_write(unsigned int addr, unsigned int data)
{
	unsigned int return_code;
	unsigned int write_data, read_data;
	unsigned int poll_counter;
	// write address
	write_data = 0x0000 + (addr & 0xff);																		 // write ADDR Instruction + register addr
	return_code = register_write(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, write_data); //
	if (return_code != 0)
	{
		return -1;
	}
	usleep(10000);

	poll_counter = 0;
	do
	{
		poll_counter++;
		register_read(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, &read_data); //
	} while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5325_SPI_POLL_COUNTER_MAX));
	if (poll_counter == SI5325_SPI_POLL_COUNTER_MAX)
	{
		return -2;
	}
	usleep(10000);

	// write data
	write_data = 0x4000 + (data & 0xff);																		 // write Instruction + data
	return_code = register_write(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, write_data); //
	if (return_code != 0)
	{
		return -1;
	}
	usleep(10000);

	poll_counter = 0;
	do
	{
		poll_counter++;
		register_read(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, &read_data); //
	} while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5325_SPI_POLL_COUNTER_MAX));
	if (poll_counter == SI5325_SPI_POLL_COUNTER_MAX)
	{
		return -2;
	}

	return 0;
}

/****************************************************************************************************/
int sis3316_adc::si5325_clk_muliplier_read(unsigned int addr, unsigned int* data)
{
	unsigned int return_code;
	unsigned int write_data, read_data;
	unsigned int poll_counter;
	// read address
	write_data = 0x0000 + (addr & 0xff);																		 // read ADDR Instruction + register addr
	return_code = register_write(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, write_data); //
	if (return_code != 0)
	{
		return -1;
	}

	poll_counter = 0;
	do
	{
		poll_counter++;
		register_read(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, &read_data); //
	} while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5325_SPI_POLL_COUNTER_MAX));
	if (poll_counter == SI5325_SPI_POLL_COUNTER_MAX)
	{
		return -2;
	}
	usleep(10000);

	// read data
	write_data = 0x8000;																						 // read Instruction + data
	return_code = register_write(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, write_data); //
	if (return_code != 0)
	{
		return -1;
	}
	usleep(10000);

	poll_counter = 0;
	do
	{
		poll_counter++;
		register_read(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, &read_data); //
	} while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5325_SPI_POLL_COUNTER_MAX));
	if (poll_counter == SI5325_SPI_POLL_COUNTER_MAX)
	{
		return -2;
	}
	//*data = (read_data & 0xff) ;
	*data = (read_data);
	return (0);
}

/****************************************************************************************************/
int sis3316_adc::si5325_clk_muliplier_internal_calibration_cmd(void)
{
	unsigned int return_code;
	unsigned int write_data, read_data;
	unsigned int poll_counter, cal_poll_counter;
	// write address
	write_data = 0x0000 + 136;																					 // write ADDR Instruction + register addr
	return_code = register_write(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, write_data); //
	if (return_code != 0)
	{
		return -1;
	}

	poll_counter = 0;
	do
	{
		poll_counter++;
		register_read(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, &read_data); //
	} while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5325_SPI_POLL_COUNTER_MAX));
	if (poll_counter == SI5325_SPI_POLL_COUNTER_MAX)
	{
		return -2;
	}

	// write data
	write_data = 0x4000 + 0x40;																					 // write Instruction + data
	return_code = register_write(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, write_data); //
	if (return_code != 0)
	{
		return -1;
	}

	poll_counter = 0;
	do
	{
		poll_counter++;
		register_read(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, &read_data); //
	} while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5325_SPI_POLL_COUNTER_MAX));
	if (poll_counter == SI5325_SPI_POLL_COUNTER_MAX)
	{
		return -2;
	}

	// poll until Calibration is ready
	cal_poll_counter = 0;
	do
	{
		cal_poll_counter++;
		// read data
		write_data = 0x8000;																						 // read Instruction + data
		return_code = register_write(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, write_data); //
		if (return_code != 0)
		{
			return -1;
		}

		poll_counter = 0;
		do
		{
			poll_counter++;
			register_read(SIS3316_NIM_CLK_MULTIPLIER_SPI_REG, &read_data); //
		} while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5325_SPI_POLL_COUNTER_MAX));
		if (poll_counter == SI5325_SPI_POLL_COUNTER_MAX)
		{
			return -2;
		}

	} while (((read_data & 0x40) == 0x40) && (cal_poll_counter < SI5325_SPI_CALIBRATION_READY_POLL_COUNTER_MAX));
	if (cal_poll_counter == SI5325_SPI_CALIBRATION_READY_POLL_COUNTER_MAX)
	{
		return -3;
	}

	return 0;
}

/****************************************************************************************************/
int sis3316_adc::get_status_external_clock_multiplier(unsigned int* status)
{
	int rc;
	unsigned int data;
	rc = si5325_clk_muliplier_read(128, &data); //
	*status = data & 0x1;
	rc = si5325_clk_muliplier_read(129, &data); //
	*status = *status + (data & 0x2);
	return rc;
}

/****************************************************************************************************/
int sis3316_adc::bypass_external_clock_multiplier(void)
{
	int rc;
	rc = si5325_clk_muliplier_write(0, 0x2);   // Bypass
	rc = si5325_clk_muliplier_write(11, 0x02); //  PowerDown clk2
	return rc;
}

/****************************************************************************************************/
int sis3316_adc::set_external_clock_multiplier(unsigned int bw_sel, unsigned int n1_hs, unsigned int n1_clk1, unsigned int n1_clk2, unsigned int n2, unsigned int n3, unsigned int clkin1_mhz)
{
	volatile unsigned int n1_val;
	volatile unsigned int n1_hs_val;
	volatile unsigned int n2_val;
	volatile unsigned int n3_val;

	// input frequency
	if ((clkin1_mhz < 10) || (clkin1_mhz > 250))
	{
		return -2;
	}
	// bw_sel : see DSPLLsinm for setting
	if (bw_sel > 15)
	{
		return -3;
	}
	// n1_hs
	if ((n1_hs < 4) || (n1_hs > 11))
	{
		return -4;
	}

	// n1_clk1
	if (n1_clk1 == 0)
	{
		return -5;
	}
	else
	{
		if ((((n1_clk1) & 0x1) == 1) && (n1_clk1 != 1))
		{ // odd but not 1
			return -5;
		}
		if ((n1_clk1 & 0xfff00000) != 0)
		{ // > 2**20
			return -5;
		}
	}

	// n1_clk2
	if (n1_clk2 == 0)
	{
		return -6;
	}
	else
	{
		if ((((n1_clk2) & 0x1) == 1) && (n1_clk2 != 1))
		{ // odd but not 1
			return -6;
		}
		if ((n1_clk2 & 0xfff00000) != 0)
		{ // > 2**20
			return -6;
		}
	}

	// n2
	if ((n2 < 32) || (n2 > 512))
	{
		return -7;
	}
	else
	{
		if ((n2 & 0x1) == 1)
		{ // odd
			return -7;
		}
	}

	// n3
	if (n3 == 0)
	{
		return -8;
	}
	else
	{
		if ((n3 & 0xfff80000) != 0)
		{ // > 2**19
			return -8;
		}
	}

	si5325_clk_muliplier_write(0, 0x0);	  // No Bypass
	si5325_clk_muliplier_write(11, 0x02); //  PowerDown clk2

	// N3 = 1
	n3_val = n3 - 1;
	si5325_clk_muliplier_write(43, ((n3_val >> 16) & 0x7)); //  N3 bits 18:16
	si5325_clk_muliplier_write(44, ((n3_val >> 8) & 0xff)); //  N3 bits 15:8
	si5325_clk_muliplier_write(45, (n3_val & 0xff));		//  N3 bits 7:0

	n2_val = n2;
	si5325_clk_muliplier_write(40, 0x00);					//    N2_LS bits 19:16
	si5325_clk_muliplier_write(41, ((n2_val >> 8) & 0xff)); //  N2_LS bits 15:8
	si5325_clk_muliplier_write(42, (n2_val & 0xff));		//  N2_LS bits 7:0

	n1_hs_val = n1_hs - 4;
	si5325_clk_muliplier_write(25, (n1_hs_val << 5)); //

	n1_val = n1_clk1 - 1;
	si5325_clk_muliplier_write(31, ((n1_val >> 16) & 0xf)); //  NC1_LS (low speed divider) bits 19:16
	si5325_clk_muliplier_write(32, ((n1_val >> 8) & 0xff)); //  NC1_LS (low speed divider) bits 15:8
	si5325_clk_muliplier_write(33, (n1_val & 0xff));		//  NC1_LS (low speed divider) bits 7:0

	n1_val = n1_clk2 - 1;
	si5325_clk_muliplier_write(34, ((n1_val >> 16) & 0xf)); //  NC2_LS (low speed divider) bits 19:16
	si5325_clk_muliplier_write(35, ((n1_val >> 8) & 0xff)); //  NC2_LS (low speed divider) bits 15:8
	si5325_clk_muliplier_write(36, (n1_val & 0xff));		//  NC2_LS (low speed divider) bits 7:0

	si5325_clk_muliplier_write(2, (bw_sel << 5)); // BWSEL_REG

	si5325_clk_muliplier_internal_calibration_cmd();

	return 0;
}

/************************************************************************************************************************************************/
// adc_fpga_group: 0,1,2,3
// adc_chip: 0 or 1
//				-1 : not all adc chips have the same chip ID
//				>0 : VME Error Code
int sis3316_adc::adc_spi_setup(void)
{
	int return_code;
	unsigned int adc_chip_id;
	unsigned int addr, data;
	unsigned i_adc_fpga_group;
	unsigned i_adc_chip;

	// disable ADC output
	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG + ((i_adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
		return_code = register_write(addr, 0x0); //
		if (return_code != 0)
		{
			return return_code;
		}
	}

	// dummy loop to access each adc chip one time after power up -- add 12.02.2015
	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		for (i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++)
		{
			this->adc_spi_read(i_adc_fpga_group, i_adc_chip, 1, &data);
		}
	}

	// reset
	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		for (i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++)
		{
			return_code = this->adc_spi_write(i_adc_fpga_group, i_adc_chip, 0x0, 0x24); // soft reset
		}
		usleep(10); // after reset
	}

	return_code = this->adc_spi_read(0, 0, 1, &adc_chip_id); // read chip Id from adc chips ch1/2

	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		for (i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++)
		{
			this->adc_spi_read(i_adc_fpga_group, i_adc_chip, 1, &data);
			// printf("i_adc_fpga_group = %d   i_adc_chip = %d    data = 0x%08x     adc_chip_id = 0x%08x     \n", i_adc_fpga_group, i_adc_chip, data, adc_chip_id);
			if (data != adc_chip_id)
			{
				printf("i_adc_fpga_group = %d   i_adc_chip = %d    data = 0x%08x     adc_chip_id = 0x%08x     \n", i_adc_fpga_group, i_adc_chip, data, adc_chip_id);
				return -1;
			}
		}
	}

	this->adc_125MHz_flag = 0;
	if ((adc_chip_id & 0xff) == 0x32)
	{
		this->adc_125MHz_flag = 1;
	}

	// reg 0x14 : Output mode
	if (this->adc_125MHz_flag == 0)
	{				 // 250 MHz chip AD9643
		data = 0x04; //  Output inverted (bit2 = 1)
	}
	else
	{				 // 125 MHz chip AD9268
		data = 0x40; // Output type LVDS (bit6 = 1), Output inverted (bit2 = 0) !
	}
	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		for (i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++)
		{
			this->adc_spi_write(i_adc_fpga_group, i_adc_chip, 0x14, data);
		}
	}

	// reg 0x18 : Reference Voltage / Input Span
	if (this->adc_125MHz_flag == 0)
	{				// 250 MHz chip AD9643
		data = 0x0; //  1.75V
	}
	else
	{ // 125 MHz chip AD9268
		// data = 0x8 ; 	//  1.75V
		data = 0xC0; //  2.0V
	}
	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		for (i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++)
		{
			this->adc_spi_write(i_adc_fpga_group, i_adc_chip, 0x18, data);
		}
	}

	// reg 0xff : register update
	data = 0x01; // update
	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		for (i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++)
		{
			this->adc_spi_write(i_adc_fpga_group, i_adc_chip, 0xff, data);
		}
	}

	// enable ADC output
	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG + ((i_adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
		return_code = register_write(addr, 0x1000000); //  set bit 24
		if (return_code != 0)
		{
			return return_code;
		}
	}

	return 0;
}

/*******************************************************************************/
int sis3316_adc::adc_spi_reg_enable_adc_outputs(void)
{
	int return_code;
	unsigned int addr;
	unsigned i_adc_fpga_group;

	// enable ADC output
	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG + ((i_adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
		return_code = register_write(addr, 0x1000000); //  set bit 24
		if (return_code != 0)
		{
			return return_code;
		}
	}

	return 0;
}

/*******************************************************************************/
// adc_fpga_group: 0,1,2,3
// adc_chip: 0 or 1
// return codes
//				-1 : invalid parameter
//				-2 : timeout
//				>0 : VME Error Code
// #define TEST_DEBUG
int sis3316_adc::adc_spi_read(unsigned int adc_fpga_group, unsigned int adc_chip, unsigned int spi_addr, unsigned int* spi_data)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int uint_adc_mux_select;
	unsigned int pollcounter;

	pollcounter = 1000;

	if (adc_fpga_group > 4)
	{
		return -1;
	}
	if (adc_chip > 2)
	{
		return -1;
	}
	if (spi_addr > 0x1fff)
	{
		return -1;
	}

	if (adc_chip == 0)
	{
		uint_adc_mux_select = 0; // adc chip ch1/ch2
	}
	else
	{
		uint_adc_mux_select = 0x400000; // adc chip ch3/ch4
	}

	// read register to get the information of bit 24 (adc output enabled)
	addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG + ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
	return_code = register_read(addr, &data); //
	if (return_code != 0)
	{
#ifdef TEST_DEBUG
		printf("adc_spi_read vme_A32D32_read 1: return_code = 0x%08x     \n", return_code);
#endif
		return return_code;
	}
	data = data & 0x01000000; // save bit 24

	data = data + 0xC0000000 + uint_adc_mux_select + ((spi_addr & 0x1fff) << 8);
	addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG + ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
	return_code = register_write(addr, data); //
#ifdef TEST_DEBUG
	if (return_code != 0)
	{
		printf("adc_spi_read vme_A32D32_write 1: return_code = 0x%08x     \n", return_code);
	}
#endif

	addr = SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG;
	do
	{																	   // the logic is appr. 20us busy
		return_code = register_read(addr, &data); //
#ifdef TEST_DEBUG
		if (return_code != 0)
		{
			printf("adc_spi_read vme_A32D32_read 2: return_code = 0x%08x     \n", return_code);
		}
#endif
		pollcounter--;
		//} while (((data & 0x80000000) == 0x80000000) && (pollcounter > 0) && (return_code == 0)); // VME FPGA Version 0x0006 and higher
	} while (((data & 0x0000000f) != 0x00000000) && (pollcounter > 0) && (return_code == 0)); // changed 2.12.2014,  VME FPGA Version 0x0005 and lower
#ifdef TEST_DEBUG
	printf("adc_spi_read pollcounter: pollcounter = 0x%08x     \n", pollcounter);
#endif

	if (return_code != 0)
	{
		return return_code;
	}
	if (pollcounter == 0)
	{
		return -2;
	}

	usleep(20); //

	// addr = SIS3316_ADC_CH1_4_SPI_READBACK_REG  ; // removed 21.01.2015
	addr = SIS3316_ADC_CH1_4_SPI_READBACK_REG + ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET); // changed 21.01.2015
	return_code = register_read(addr, &data);									//
#ifdef TEST_DEBUG
	if (return_code != 0)
	{
		printf("adc_spi_read vme_A32D32_read 3: return_code = 0x%08x     \n", return_code);
	}
#endif
	if (return_code != 0)
	{
		return return_code;
	}

	*spi_data = data & 0xff;
	return 0;
}

int sis3316_adc::adc_spi_write(unsigned int adc_fpga_group, unsigned int adc_chip, unsigned int spi_addr, unsigned int spi_data)
{
	volatile int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int uint_adc_mux_select;
	unsigned int pollcounter;
	pollcounter = 1000;

	if (adc_fpga_group > 4)
	{
		return -1;
	}
	if (adc_chip > 2)
	{
		return -1;
	}
	if (spi_addr > 0xffff)
	{
		return -1;
	}

	if (adc_chip == 0)
	{
		uint_adc_mux_select = 0; // adc chip ch1/ch2
	}
	else
	{
		uint_adc_mux_select = 0x400000; // adc chip ch3/ch4
	}

	// read register to get the information of bit 24 (adc output enabled)
	addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG + ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
	return_code = register_read(addr, &data); //
	if (return_code != 0)
	{
		return return_code;
	}
	data = data & 0x01000000; // save bit 24

	data = data + 0x80000000 + uint_adc_mux_select + ((spi_addr & 0xffff) << 8) + (spi_data & 0xff);
	addr = SIS3316_ADC_CH1_4_SPI_CTRL_REG + ((adc_fpga_group & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
	return_code = register_write(addr, data); //

	// usleep(1000) ; //

	addr = SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG;
	// printf("poll_on_spi_busy: pollcounter = 0x%08x    \n", pollcounter );
	do
	{																	   // the logic is appr. 20us busy
		return_code = register_read(addr, &data); //
		// printf("SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG = 0x%08x    \n", data);
		pollcounter--;
	} while (((data & 0x80000000) == 0x80000000) && (pollcounter > 0) && (return_code == 0)); // VME FPGA Version 0x0006 and higher
	//} while (((data & 0x0000000f) != 0x00000000) && (pollcounter > 0) && (return_code == 0)); // changed 2.12.2014,  VME FPGA Version 0x0005 and lower
	// printf("poll_on_spi_busy: pollcounter = 0x%08x (%d)   data = 0x%08x   return_code = 0x%08x \n", pollcounter, pollcounter, data, return_code);
	if (return_code != 0)
	{
		return return_code;
	}
	if (pollcounter == 0)
	{
		return -2;
	}
	return 0;
}

/******************************************************************************************************************************************************************************/
int sis3316_adc::write_channel_header_ID(unsigned int channel_header_id_reg_value)
{
	int return_code;
	unsigned int data;

	// Channel Header ID register
	data = channel_header_id_reg_value;
	return_code = register_write(SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG, data); //
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write(SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG, data + 0x400000); //
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write(SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG, data + 0x800000); //
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write(SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG, data + 0xC00000); //
	if (return_code != 0)
	{
		return return_code;
	}
	return 0;
}

/******************************************************************************************************************************************************************************/
int sis3316_adc::poll_on_adc_dac_offset_busy(void)
{
	volatile int return_code;
	unsigned int data;
	unsigned int poll_counter;

	poll_counter = 1000;
	do
	{
		poll_counter--;
		return_code = register_read(SIS3316_ADC_FPGA_SPI_BUSY_STATUS_REG, &data); //
	} while (((data & 0xf) != 0) && (poll_counter > 0) && (return_code == 0));
	// printf("Error poll_on_adc_dac_offset_busy: poll_counter = 0x%08x \n", poll_counter);
	if (return_code != 0)
	{
		return return_code;
	}
	if (poll_counter == 0)
	{
		return 0x900;
	}
	return 0;
}

int sis3316_adc::write_all_adc_dac_offsets(void)
{
	int return_code;
	unsigned int dac_offset;
	unsigned int i_adc_fpga_group;

	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		dac_offset = this->adc_dac_offset_ch_array[(i_adc_fpga_group * 4) + 0];
		return_code = register_write((i_adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0x80000000 + 0x2000000 + 0x000000 + ((dac_offset & 0xffff) << 4)); // clear error Latch bits
		return_code = this->poll_on_adc_dac_offset_busy();
		if (return_code != 0)
		{
			return return_code;
		}

		dac_offset = this->adc_dac_offset_ch_array[(i_adc_fpga_group * 4) + 1];
		return_code = register_write((i_adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0x80000000 + 0x2000000 + 0x100000 + ((dac_offset & 0xffff) << 4)); // clear error Latch bits
		return_code = this->poll_on_adc_dac_offset_busy();
		if (return_code != 0)
		{
			return return_code;
		}

		dac_offset = this->adc_dac_offset_ch_array[(i_adc_fpga_group * 4) + 2];
		return_code = register_write((i_adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0x80000000 + 0x2000000 + 0x200000 + ((dac_offset & 0xffff) << 4)); // clear error Latch bits
		return_code = this->poll_on_adc_dac_offset_busy();
		if (return_code != 0)
		{
			return return_code;
		}

		dac_offset = this->adc_dac_offset_ch_array[(i_adc_fpga_group * 4) + 3];
		return_code = register_write((i_adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0x80000000 + 0x2000000 + 0x300000 + ((dac_offset & 0xffff) << 4)); // clear error Latch bits
		return_code = this->poll_on_adc_dac_offset_busy();
		if (return_code != 0)
		{
			return return_code;
		}

		return_code = register_write((i_adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0xC0000000); // CMD: DAC LDAC (load)
		return_code = this->poll_on_adc_dac_offset_busy();
		if (return_code != 0)
		{
			return return_code;
		}
	}

	return 0;
}

int sis3316_adc::configure_all_adc_dac_offsets(void)
{
	int return_code;
	unsigned int dac_offset;
	unsigned int i_adc_fpga_group;

	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		dac_offset = this->adc_dac_offset_ch_array[(i_adc_fpga_group * 4) + 0];
		return_code = register_write((i_adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0x80000000 + 0x8000000 + 0xf00000 + 0x1); // Standalone mode, set Internal Reference
		if (return_code != 0)
		{
			return return_code;
		}
		return_code = this->poll_on_adc_dac_offset_busy();
		if (return_code != 0)
		{
			return return_code;
		}

		return_code = register_write((i_adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_DAC_OFFSET_CTRL_REG, 0xC0000000); // CMD: DAC LDAC (load)
		if (return_code != 0)
		{
			return return_code;
		}
		return_code = this->poll_on_adc_dac_offset_busy();
		if (return_code != 0)
		{
			return return_code;
		}
	}

	return 0;
}

/******************************************************************************************************************************************************************************/
int sis3316_adc::write_all_gain_termination_values(void)
{
	int return_code;
	unsigned int gain_termintion_reg_value;
	unsigned int i_adc_fpga_group;

	for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++)
	{
		gain_termintion_reg_value = (this->adc_gain_termination_ch_array[(i_adc_fpga_group * 4) + 0] & 0xff);																	// ch 1, 5, 9, 13
		gain_termintion_reg_value = gain_termintion_reg_value + ((this->adc_gain_termination_ch_array[(i_adc_fpga_group * 4) + 1] & 0xff) << 8);								// ch 2, 6, 10, 14
		gain_termintion_reg_value = gain_termintion_reg_value + ((this->adc_gain_termination_ch_array[(i_adc_fpga_group * 4) + 2] & 0xff) << 16);								// ch 3, 7, 11, 15
		gain_termintion_reg_value = gain_termintion_reg_value + ((this->adc_gain_termination_ch_array[(i_adc_fpga_group * 4) + 3] & 0xff) << 24);								// ch 4, 8, 12, 16
		return_code = register_write((i_adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_ANALOG_CTRL_REG, gain_termintion_reg_value); //
		if (return_code != 0)
		{
			return return_code;
		}
	}
	return 0;
}

/******************************************************************************************************************************************************************************/
int sis3316_adc::write_nim_output_selection_values(void)
{
	int return_code;
	return_code = register_write(SIS3316_LEMO_OUT_CO_SELECT_REG, nim_output_selection_array[0]); //
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write(SIS3316_LEMO_OUT_TO_SELECT_REG, nim_output_selection_array[1]); //
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write(SIS3316_LEMO_OUT_UO_SELECT_REG, nim_output_selection_array[2]); //
	if (return_code != 0)
	{
		return return_code;
	}
	return 0;
}

/******************************************************************************************************************************************************************************/
int sis3316_adc::internal_sum_trigger_generation_setup(unsigned int uint_sum_trigger_threshold_reg, unsigned int uint_sum_he_trigger_threshold_reg, unsigned int uint_sum_trigger_setup_reg, unsigned int adc_fpga_group /* 0 to 3 */)
{

	int return_code;
	if (adc_fpga_group > 3)
	{
		return 0x900;
	}
	return_code = register_write((adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_SETUP_REG, 0); // //  clear FIR Trigger Setup -> a following Setup will reset the logic !
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write((adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_THRESHOLD_REG, 0); // // disable all ch_sum
	if (return_code != 0)
	{
		return return_code;
	}

	return_code = register_write((adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG, uint_sum_he_trigger_threshold_reg); // //
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write((adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_THRESHOLD_REG, uint_sum_trigger_threshold_reg); // //
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write((adc_fpga_group * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_SETUP_REG, uint_sum_trigger_setup_reg); // //
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

int sis3316_adc::internal_trigger_generation_setup(unsigned int uint_trigger_threshold_reg, unsigned int uint_he_trigger_threshold_reg, unsigned int uint_trigger_setup_reg, unsigned int channel_no /* 0 to 15 */)
{

	int return_code;
	if (channel_no > 15)
	{
		return 0x900;
	}
	return_code = register_write((((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) + (SIS3316_ADC_CH1_FIR_TRIGGER_SETUP_REG + ((channel_no & 0x3) * 0x10)), 0); // //  clear FIR Trigger Setup -> a following Setup will reset the logic !
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write((((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) + (SIS3316_ADC_CH1_FIR_TRIGGER_THRESHOLD_REG + ((channel_no & 0x3) * 0x10)), 0); // // disable all ch_sum
	if (return_code != 0)
	{
		return return_code;
	}

	return_code = register_write((((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) + (SIS3316_ADC_CH1_FIR_HIGH_ENERGY_THRESHOLD_REG + ((channel_no & 0x3) * 0x10)), uint_he_trigger_threshold_reg); // //
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write((((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) + (SIS3316_ADC_CH1_FIR_TRIGGER_THRESHOLD_REG + ((channel_no & 0x3) * 0x10)), uint_trigger_threshold_reg); // //
	if (return_code != 0)
	{
		return return_code;
	}
	return_code = register_write((((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET) + (SIS3316_ADC_CH1_FIR_TRIGGER_SETUP_REG + ((channel_no & 0x3) * 0x10)), uint_trigger_setup_reg); // //
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/******************************************************************************************************************************************************************************/
int sis3316_adc::read_MBLT64_Channel_PreviousBankDataBuffer(unsigned int bank2_read_flag, unsigned int channel_no /* 0 to 15 */,
	unsigned int* dma_got_no_of_words, unsigned int* uint_adc_buffer)
{
	int return_code;
	unsigned int max_read_nof_words;
	max_read_nof_words = 0xffffff;
	return_code = this->read_DMA_Channel_PreviousBankDataBuffer(bank2_read_flag, channel_no, max_read_nof_words, dma_got_no_of_words, uint_adc_buffer);

	return return_code;
}

int sis3316_adc::read_DMA_Channel_PreviousBankDataBuffer(unsigned int bank2_read_flag, unsigned int channel_no /* 0 to 15 */,
	unsigned int max_read_nof_words, unsigned int* dma_got_no_of_words, unsigned int* uint_adc_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int previous_bank_addr_value;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	unsigned int max_poll_counter;

	// read previous Bank sample address
	// poll until it is valid.
	addr = SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG + ((channel_no & 0x3) * 4) + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
	max_poll_counter = 10000;
	*dma_got_no_of_words = 0;
	do
	{
		return_code = register_read(addr, &previous_bank_addr_value); //
		// return_code = i->vme_A32D32_read(this->baseaddress + addr, &previous_bank_addr_value); //  only if version VME _V0002
		if (return_code != 0)
		{
			printf("Error: vme_A32D32_read: return_code = 0x%08x   addr = 0x%08x \n", return_code, this->baseaddress + addr);
			return return_code;
		}
		max_poll_counter--;
		if (max_poll_counter == 0)
		{
			printf("Error: max_poll_counter = 0x%08x   \n", max_poll_counter);
			return 0x900;
		}
	} while (((previous_bank_addr_value & 0x1000000) >> 24) != bank2_read_flag); // previous Bank sample address is valid if bit 24 is equal bank2_read_flag

	if ((previous_bank_addr_value & 0xffffff) == 0)
	{ // no data sampled !
		*dma_got_no_of_words = 0;
		return 0;
	}

	// start Readout FSM
	if (bank2_read_flag == 1)
	{
		memory_bank_offset_addr = 0x01000000; // Bank2 offset
	}
	else
	{
		memory_bank_offset_addr = 0x00000000; // Bank1 offset
	}

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	// in case of using ETHERNET_UDP_INTERFACE it is possible to lose packets (observed with WIN7 using a "company-net", not observed with WIN7 and direct connection and not observed with LINUX)
	//
	unsigned int retry_counter = 0;
	unsigned int retry_flag = 0;

	do
	{
		addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
		data = 0x80000000 + memory_bank_offset_addr;
		return_code = register_write(addr, data);
		if (return_code != 0)
		{
			//	printf("Error: vme_A32D32_write: return_code = 0x%08x   addr = 0x%08x  data = 0x%08x \n", return_code,  this->baseaddress + addr, data);
			// return return_code;
		}
		else
		{
			// readout
			addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);

			req_nof_32bit_words = previous_bank_addr_value & 0xffffff;
			if (req_nof_32bit_words > max_read_nof_words)
			{
				req_nof_32bit_words = max_read_nof_words;
			}
			return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_adc_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
			if (return_code != 0)
			{
				// printf("Error: vme_A32MBLT64FIFO_read: return_code = 0x%08x   addr = 0x%08x  req_nof_32bit_words = 0x%08x \n", return_code,  this->baseaddress + addr, req_nof_32bit_words);
				// return return_code;
			}
			*dma_got_no_of_words = req_nof_32bit_words;
		}
		retry_flag = 0;
		if (return_code != 0)
		{
			retry_counter++;
			retry_flag = 0;
			if (retry_counter < 16)
			{
				retry_flag = 1;
			}
			if (retry_counter > 1)
			{
				printf("Info: retry_counter = %d \n", retry_counter);
			}
		}

		// reset FSM again
		addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
		data = 0x00000000; // Reset
		return_code = register_write(addr, data);
		if (return_code != 0)
		{
			// printf("Error: vme_A32D32_write: return_code = 0x%08x   addr = 0x%08x  data = 0x%08x \n", return_code,  this->baseaddress + addr, data);
			// return return_code;
		}

	} while (retry_flag == 1);

	if (retry_counter > 15)
	{
		return -1;
	}

	return 0;
}

/****************************************************************************************************/
int sis3316_adc::read_Channel_PreviousSampleAddress(unsigned int bank2_read_flag, unsigned int channel_no /* 0 to 15 */, unsigned int* previous_sample_address)
{
	int return_code;
	// unsigned int data ;
	unsigned int addr;
	unsigned int previous_bank_addr_value;
	// unsigned int req_nof_32bit_words ;
	// unsigned int got_nof_32bit_words ;
	// unsigned int memory_bank_offset_addr ;
	unsigned int max_poll_counter;

	// read previous Bank sample address
	// poll until it is valid.
	addr = SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG + ((channel_no & 0x3) * 4) + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_REG_OFFSET);
	max_poll_counter = 10000;
	*previous_sample_address = 0;
	do
	{
		return_code = register_read(addr, &previous_bank_addr_value); //
		if (return_code != 0)
		{
			return return_code;
		}
		max_poll_counter--;
		if (max_poll_counter == 0)
		{
			return 0x900;
		}
	} while (((previous_bank_addr_value & 0x1000000) >> 24) != bank2_read_flag); // previous Bank sample address is valid if bit 24 is equal bank2_read_flag
	*previous_sample_address = (previous_bank_addr_value & 0xffffff);

	return 0;
}

/****************************************************************************************************/
int sis3316_adc::read_Channel_EnergyHistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_adc_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	memory_bank_offset_addr = 0x00FF0000; // Histogram offset

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_adc_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_TofHistogramBuffer requires the ADC-FPGA Firmware V_0101 (SIS3316-14bit-250MHz)                                                */
int sis3316_adc::read_Channel_TofHistogramBuffer(unsigned int channel_no /* 0 to 15 */, int histogram_index, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	if (histogram_index == 0)
	{
		memory_bank_offset_addr = 0x00C00000; // Histogram offset Beam On
	}
	else
	{
		memory_bank_offset_addr = 0x00D00000; // Histogram offset Beam OFF
	}

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_ShapeHistogramBuffer requires the ADC-FPGA Firmware V_0101 (SIS3316-14bit-250MHz)                                              */
int sis3316_adc::read_Channel_ShapeHistogramBuffer(unsigned int channel_no /* 0 to 15 */, int histogram_index, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	if (histogram_index == 0)
	{
		memory_bank_offset_addr = 0x00E00000; // Histogram offset Beam On
	}
	else
	{
		memory_bank_offset_addr = 0x00E40000; // Histogram offset Beam OFF
	}

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	// printf("req_nof_32bit_words    = %d     \n", req_nof_32bit_words);

	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_ChargeHistogramBuffer requires the ADC-FPGA Firmware V_0101 (SIS3316-14bit-250MHz)                                             */
int sis3316_adc::read_Channel_ChargeHistogramBuffer(unsigned int channel_no /* 0 to 15 */, int histogram_index, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	if (histogram_index == 0)
	{
		memory_bank_offset_addr = 0x00F00000; // Histogram offset Beam On
	}
	else
	{
		memory_bank_offset_addr = 0x00F10000; // Histogram offset Beam OFF
	}

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

int sis3316_adc::read_Channel_StatisticCounter(unsigned int adc_fpga_no /* 0 to 3 */, unsigned int* uint_statistic_buffer)
{ // new 27.08.2019
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (adc_fpga_no * 4);
	data = 0x80000000 + 0x30000000; // Space Statistic Counter
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (adc_fpga_no * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = 24;
	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_statistic_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (adc_fpga_no * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
#ifdef WIN

void usleep(unsigned int uint_usec)
{
	unsigned int msec;
	if (uint_usec <= 1000)
	{
		msec = 1;
	}
	else
	{
		msec = (uint_usec + 999) / 1000;
	}
	Sleep(msec);
}
#endif

/************************************************************************************************************************************************/
/*  write_Channel_PSD_LookupTableHistogramBuffer requires the ADC-FPGA Firmware V_020x (SIS3316-14bit-250MHz)                                   */
int sis3316_adc::write_Channel_PSD_LookupTable_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	memory_bank_offset_addr = 0x00F40000; // Histogram PSD Lookup Table

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0xC0000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// write
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	// printf("req_nof_32bit_words    = %d     \n", req_nof_32bit_words);

	return_code = i->vme_A32BLT32FIFO_write(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xffffff), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_PSD_LookupTable_HistogramBuffer requires the ADC-FPGA Firmware V_0201 (SIS3316-14bit-250MHz)                                   */
int sis3316_adc::read_Channel_PSD_LookupTable_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	memory_bank_offset_addr = 0x00F40000; // Histogram PSD Lookup Table

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	// printf("req_nof_32bit_words    = %d     \n", req_nof_32bit_words);

	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_PSD_HistogramBuffer requires the ADC-FPGA Firmware V_0201 (SIS3316-14bit-250MHz)                                               */
int sis3316_adc::read_Channel_PSD_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	memory_bank_offset_addr = 0x00F00000; // Histogram PSD Lookup Table

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	// printf("req_nof_32bit_words    = %d     \n", req_nof_32bit_words);

	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_TOF_Gamma_HistogramBuffer requires the ADC-FPGA Firmware V_0201 (SIS3316-14bit-250MHz)                                         */
int sis3316_adc::read_Channel_TOF_Gamma_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	memory_bank_offset_addr = 0x00C00000; //  TOF Gamma Histogram

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	// printf("req_nof_32bit_words    = %d     \n", req_nof_32bit_words);

	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_TOF_Neutron_HistogramBuffer requires the ADC-FPGA Firmware V_0201 (SIS3316-14bit-250MHz)                                       */
int sis3316_adc::read_Channel_TOF_Neutron_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	memory_bank_offset_addr = 0x00D00000; //  TOF Neutron Histogram

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	// printf("req_nof_32bit_words    = %d     \n", req_nof_32bit_words);

	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_PeakSum_Gamma_HistogramBuffer requires the ADC-FPGA Firmware V_0201 (SIS3316-14bit-250MHz)                                     */
int sis3316_adc::read_Channel_PeakSum_Gamma_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	memory_bank_offset_addr = 0x00E00000; //  PeakSum Gamma Histogram

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	// printf("req_nof_32bit_words    = %d     \n", req_nof_32bit_words);

	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_PeakSum_Neutron_HistogramBuffer requires the ADC-FPGA Firmware V_0201 (SIS3316-14bit-250MHz)                                   */
int sis3316_adc::read_Channel_PeakSum_Neutron_HistogramBuffer(unsigned int channel_no /* 0 to 15 */, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	memory_bank_offset_addr = 0x00E10000; //  PeakSum Neutron Histogram

	if ((channel_no & 0x1) != 0x1)
	{																	// 0,1
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 1 , 3, ..... 15
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x02000000; // channel 2 , 4, ..... 16
	}

	if ((channel_no & 0x2) != 0x2)
	{																	// 0,2
		memory_bank_offset_addr = memory_bank_offset_addr + 0x00000000; // channel 0,1 , 4,5, .....
	}
	else
	{
		memory_bank_offset_addr = memory_bank_offset_addr + 0x10000000; // channel 2,3 , 6,7 .....
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + (((channel_no >> 2) & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	// printf("req_nof_32bit_words    = %d     \n", req_nof_32bit_words);

	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	// new
	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + (((channel_no >> 2) & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}

/************************************************************************************************************************************************/
/*  read_Channel_PeakSum_Neutron_HistogramBuffer requires the ADC-FPGA Firmware V_0201 (SIS3316-14bit-250MHz)                                   */
int sis3316_adc::read_Channel_Statistic_Buffer(unsigned int fpga_goup /* 0 to 3 */, unsigned int lenght, unsigned int* uint_buffer)
{
	int return_code;
	unsigned int data;
	unsigned int addr;
	unsigned int req_nof_32bit_words;
	unsigned int got_nof_32bit_words;
	unsigned int memory_bank_offset_addr;
	//	unsigned int max_poll_counter ;

	memory_bank_offset_addr = 0x30000000; //   Space Select

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + ((fpga_goup & 0x3) * 4);
	data = 0x80000000 + memory_bank_offset_addr;
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	// readout
	addr = SIS3316_FPGA_ADC1_MEM_BASE + ((fpga_goup & 0x3) * SIS3316_FPGA_ADC_MEM_OFFSET);
	req_nof_32bit_words = lenght;
	// printf("req_nof_32bit_words    = %d     \n", req_nof_32bit_words);

	return_code = i->vme_A32MBLT64FIFO_read(this->baseaddress + addr, uint_buffer, ((req_nof_32bit_words + 1) & 0xfffffE), &got_nof_32bit_words); // N * 8-byte length  !!!
	if (return_code != 0)
	{
		return return_code;
	}

	addr = SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG + ((fpga_goup & 0x3) * 4);
	data = 0x00000000; // Reset
	return_code = register_write(addr, data);
	if (return_code != 0)
	{
		return return_code;
	}

	return 0;
}
