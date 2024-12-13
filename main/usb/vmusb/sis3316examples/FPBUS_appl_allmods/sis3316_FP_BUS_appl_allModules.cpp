/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_FP_BUS_appl_allModules.cpp                           */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Function: Acquisition with External Trigger  or                        */
/*            Internal Trigger with External Gate                          */
/*            with N SIS3316s					                           */
/*            - see:  define MAX_NOF_SIS3316_ADCS			13             */
/*            - see:  strcpy(sis3316_ip_addr_string[0],"192.168.1.100") ;  */
/*                                                                         */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 18.09.2019                                       */
/*  last modification:    25.07.2024    (SIS3316-2 adaptation)             */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*                                                                         */
/*  https://www.struck.de                                                  */
/*                                                                         */
/*  © 2024                                                                 */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


#define MAX_NOF_SIS3316_ADCS			3
//#define MAX_NOF_SIS3316_ADCS			13
//#define MAX_NOF_SIS3316_ADCS			1

#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethnernet UDP)


#define CERN_ROOT_PLOT

#ifdef CERN_ROOT_PLOT
#include "rootIncludes.h"   // 
#include "sis3316_cern_root_class.h"  



#ifdef WINDOWS
   #pragma comment (lib, "libRio")
   #pragma comment (lib, "libcore")
   #pragma comment (lib, "libHist")
   #pragma comment (lib, "libTree")
   #pragma comment (lib, "libgpad")
   #pragma comment (lib, "libcint")
   #pragma comment (lib, "libGraf")
   #pragma comment (lib, "libGui")
#endif

#endif


#ifdef LINUX

#include <iostream>
using namespace std;

//#include <iostream>
//#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef int BOOL ;
#define TRUE  1
#define FALSE 0

#endif

#ifdef WINDOWS
	#include <iostream>
	#include <iomanip>
	using namespace std;
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winsock2.h>

	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
//	#include "wingetopt.h" 
#endif




#ifdef ETHERNET_UDP_INTERFACE
	#include "sis3316_ethernet_access_class.h"
	//sis3316_eth *gl_vme_crate ;

	#ifdef LINUX
		#include <sys/types.h>
		#include <sys/socket.h>
	#endif

	#ifdef WINDOWS
		#include <winsock2.h>
		#pragma comment(lib, "ws2_32.lib")
		//#pragma comment(lib, "wsock32.lib")
		typedef int socklen_t;
		char  gl_sis3316_ip_addr_string[32] ;

		long WinsockStartup()
		{
		  long rc;

		  WORD wVersionRequested;
		  WSADATA wsaData;
		  wVersionRequested = MAKEWORD(2, 1);

		  rc = WSAStartup( wVersionRequested, &wsaData );
		  return rc;
		}
	#endif

#endif


#include "sis3316_class.h"
#include "sis3316.h"
#include "get_configuration_parameter_appl.h"

/*===========================================================================*/
/* Globals					  			     */
/*===========================================================================*/


BOOL gl_stopReq = FALSE;


sis3316_get_configuration_parameters *gl_sis3316_get_configuration_parameters ;

/*===========================================================================*/
/* Prototypes			                               		  			     */
/*===========================================================================*/
int SIS3316_WriteBankChannelHeaderToDataFile (FILE *file_data_ptr, unsigned int indentifier, unsigned int bank_loop_no, unsigned int channel_no, unsigned int nof_events, unsigned int event_length, unsigned int maw_length, unsigned int reserved);
int SIS3316_WriteBankChannelEventBufferToDataFile (FILE *file_data_ptr, unsigned int* memory_data_array, unsigned int nof_write_length_lwords);

int SIS3316_WriteStatisticCounterHeaderToDataFile (FILE *file_data_ptr, unsigned int nof_modules, unsigned int bank_loop_no, unsigned int reserved);
int SIS3316_WriteStatisticCounterToDataFile (FILE *file_data_ptr, unsigned int* memory_data_array, unsigned int nof_write_length_lwords);

void program_stop_and_wait(void);

#ifdef WINDOWS
BOOL CtrlHandler( DWORD ctrlType );
#endif

/*===========================================================================*/

int main(int argc, char *argv[])
{
volatile int return_code ;
unsigned int i ;
unsigned int i_mod ;
unsigned int i_ch ;
unsigned int i_fpga;

unsigned int serial_no;
unsigned int data ;
char char_messages[128] ;
unsigned int nof_found_devices ;

unsigned int clock_N1div, clock_HSdiv ;
unsigned int iob_delay_value ;
double double_clock_configure_fft_frequency;
unsigned int clock_freq_choice ;

int i_event;
	
unsigned int header_length ;
//unsigned int header_accu_6_values_offset ;
unsigned int header_accu_2_values_offset ;
unsigned int header_maw_3_values_offset ;
//unsigned int uint_config_data_format ;
unsigned int event_length ;
unsigned int max_req_nof_32bit_words ;
unsigned int address_threshold ;

unsigned int bank1_armed_flag ;
//unsigned int plot_counter ;
unsigned int ch_event_counter ;
unsigned int poll_counter ;
//unsigned int bank_buffer_counter ;

unsigned int got_nof_32bit_words ;

unsigned int loop_counter ;
unsigned int bank_buffer_counter ;
unsigned int uint_channel_event_counter[MAX_NOF_SIS3316_ADCS][16];


/*********************************************************************************************************************/
/*  default values/parameters                                                                                        */


// global parameters for all channels 
unsigned int stop_after_loop_counts ;
unsigned int nof_events_per_bank ;

unsigned int uint_channel_polarity_invert_flag[MAX_NOF_SIS3316_ADCS][16]  ;
unsigned int uint_channel_range_2V_flag[MAX_NOF_SIS3316_ADCS][16]  ;
unsigned int uint_channel_50ohm_termination_disable_flag[MAX_NOF_SIS3316_ADCS][16]  ;
unsigned int uint_fpga_analog_ctrl_val[MAX_NOF_SIS3316_ADCS][4];
unsigned int uint_channel_analog_offset_dac_val[MAX_NOF_SIS3316_ADCS][16];

unsigned int uint_acquisition_trigger_mode ;  // 0: TI External Trigger   	: TI External Gate/Internal Trigger
unsigned int uint_channel_external_gate_enable_flag[MAX_NOF_SIS3316_ADCS][16]  ;
unsigned int uint_channel_external_trigger_enable_flag[MAX_NOF_SIS3316_ADCS][16]  ;
unsigned int uint_channel_internal_trigger_enable_flag[MAX_NOF_SIS3316_ADCS][16]  ;
unsigned int uint_fpag_config_reg_value[MAX_NOF_SIS3316_ADCS][4];

unsigned int uint_channel_trigger_generation_setup_reg[MAX_NOF_SIS3316_ADCS][16];
unsigned int uint_channel_trigger_generation_threshold_reg[MAX_NOF_SIS3316_ADCS][16];
unsigned int uint_channel_he_trigger_generation_threshold_reg[MAX_NOF_SIS3316_ADCS][16];

unsigned int uint_trigger_pulse_length;
unsigned int uint_trigger_gap;
unsigned int uint_trigger_peaking;
unsigned int uint_trigger_threshold_value;

unsigned int external_veto_gate_delay;

unsigned int uint_trigger_function_deadtime_block_with_addrThres_enable;
unsigned int uint_trigger_function_deadtime_logic_enable;
unsigned int uint_trigger_function_deadtime_length;


unsigned int trigger_gate_window_length ;
unsigned int uint_save_raw_sample_length ;
unsigned int uint_save_raw_sample_start_index ;
unsigned int uint_save_raw_sample_first_event_only_mode;
unsigned int internal_fir_trigger_delay ;
unsigned int pre_trigger_delay ;

unsigned int header_accu_6_values_enable_flag ;
unsigned int header_accu_2_values_enable_flag ;
unsigned int header_maw_3_values_enable_flag ;

unsigned int gate_length[8];
unsigned int gate_start_index[8];

unsigned int maw_test_buffer_enable_flag ;
unsigned int maw_test_buffer_length ;
unsigned int maw_test_buffer_delay ;

unsigned int uint_pileup ;
unsigned int uint_re_pileup ;

unsigned int uint_lemo_out_CO_select;
unsigned int uint_lemo_out_TO_select;
unsigned int uint_lemo_out_UO_select;


unsigned int uint_udp_jumbo_mode;
unsigned int uint_udp_nofPacketsPerRequest;

// command line interpreter
	int int_ch ;
	char char_config_file[512];
	int configurationFile_rc ;

	unsigned int uint_configurationFile_valid_flag = 0 ;
	unsigned int uint_print_configurationParameterOnly_flag = 0 ;
	// file write
	#define WRITE_DTATA_TO_FILE_MAX_32BIT_WORDS	 0x10000000   // 256M 32-bit words -> 1GByte
	unsigned int uint_WritenData_to_File_32bit_words  = 0 ;
	unsigned int uint_WriteData_to_File_EnableFlag    = 0 ;
	unsigned int uint_WriteData_to_MultipleFiles_Flag = 0 ;
	unsigned int uint_WriteData_to_File_OpenFlag      = 0 ;
	unsigned int uint_WriteData_to_File_counter       = 0 ;
	char char_WriteData_to_File_initialize_filename[512]  ;
	char char_WriteData_to_File_filename[512]  ;

	FILE *file_WriteData_to_File_Pointer           ;
	unsigned file_header_indentifier;
	unsigned file_header_reserved;
	unsigned event_length_for_file_write_header;

	uint_WriteData_to_File_OpenFlag      = 0 ;
	uint_WriteData_to_File_counter       = 0 ;
	uint_WriteData_to_MultipleFiles_Flag = 0 ;
	sprintf(char_WriteData_to_File_initialize_filename,"sis3316_test_data") ;


	/******************************************************************************************/
	/*                                                                                        */
	/*  SIS3316 default program parameter                                                     */
	/*                                                                                        */
	/******************************************************************************************/

	// Ethernet UDP transfer options/mode
	//uint_udp_jumbo_mode = 0; // disable Jumbo mode -> max. transfer block size 1440 bytes
	uint_udp_jumbo_mode = 1; // enable Jumbo mode  -> max. transfer block size 8000 bytes -> set MTU to 9000 !

	//uint_udp_nofPacketsPerRequest = 1 ;
	//uint_udp_nofPacketsPerRequest = 5 ;
	//uint_udp_nofPacketsPerRequest = 10 ;
	uint_udp_nofPacketsPerRequest = 20 ;
	//uint_udp_nofPacketsPerRequest = UDP_MAX_PACKETS_PER_REQUEST ;



	max_req_nof_32bit_words = SIS3316_ADC_MEMORY_BANK_32BIT_SIZE ; // max_request is limited by Memory Banks_Size
	//uint_software_key_trigger_flag = 0 ;

	stop_after_loop_counts      = 1;   	// 0: endless , could be changed by calling this program with "-N num" option
	nof_events_per_bank         = 500 ;	// events / Bank



	// default Configuration parameters
		// Set Gain/Termination
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_range_2V_flag[i_mod][i_ch] = 1;  // 2 V Range
			//uint_channel_range_2V_flag[i_ch] = 0;  // 5 V Range
			//uint_channel_50ohm_termination_disable_flag[i_ch] = 1; // disable 50 Ohm Termination
			uint_channel_50ohm_termination_disable_flag[i_mod][i_ch] = 0; // enable 50 Ohm Termination
		}
	}

	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		for (i_fpga = 0; i_fpga < 4; i_fpga++) {
			uint_fpga_analog_ctrl_val[i_mod][i_fpga] = (uint_channel_range_2V_flag[i_mod][(i_fpga * 4) + 0] & 1)
				+ ((uint_channel_range_2V_flag[i_mod][(i_fpga * 4) + 1] & 1) << 8)
				+ ((uint_channel_range_2V_flag[i_mod][(i_fpga * 4) + 2] & 1) << 16)
				+ ((uint_channel_range_2V_flag[i_mod][(i_fpga * 4) + 3] & 1) << 24)
				+ ((uint_channel_50ohm_termination_disable_flag[i_mod][(i_fpga * 4) + 0] & 1) << 2)
				+ ((uint_channel_50ohm_termination_disable_flag[i_mod][(i_fpga * 4) + 1] & 1) << 10)
				+ ((uint_channel_50ohm_termination_disable_flag[i_mod][(i_fpga * 4) + 2] & 1) << 18)
				+ ((uint_channel_50ohm_termination_disable_flag[i_mod][(i_fpga * 4) + 3] & 1) << 26);
		}
	}


	//set ADC offsets (DAC) :
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_analog_offset_dac_val[i_mod][i_ch] = 0x8000; // middle
			//uint_channel_analog_offset_dac_val[i_mod][i_ch] = 0x3000; //  0V at 700 counts
		}
	}

	// channel polarity
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_polarity_invert_flag[i_mod][i_ch] = 1; // all inverted
		}
		//uint_channel_polarity_invert_flag[1] = 0; // ch2 not inverted
	}



	// Event Configuration Register parameters

	uint_acquisition_trigger_mode = 0 ;  // 0: TI External Trigger   	1: TI External Gate/Internal Trigger

	if(uint_acquisition_trigger_mode == 0) {// 0: TI External Trigger
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		// channel external gate enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_external_gate_enable_flag[i_mod][i_ch] = 0; // all disable
			}

			// channel external trigger enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_external_trigger_enable_flag[i_mod][i_ch] = 1; // all enable
			}

			// channel internal trigger enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_internal_trigger_enable_flag[i_mod][i_ch] = 0; // all disable
			}
		}
	}
	else { // 1: TI External Gate/Internal Trigger
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			// channel external gate enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_external_gate_enable_flag[i_mod][i_ch] = 1; // all enable
			}

			// channel external trigger enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_external_trigger_enable_flag[i_mod][i_ch] = 0; // all disable
			}

			// channel internal trigger enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_internal_trigger_enable_flag[i_mod][i_ch] = 1; // all enable
			}
		}
	}


	// internal Trigger generation
	uint_trigger_pulse_length    = 0x10;
	uint_trigger_gap             = 8;
	uint_trigger_peaking         = 10;
	uint_trigger_threshold_value = 100 * uint_trigger_peaking; // 2V Range -> 1 bin = 122 uV
																   // 100 --> 2V Range --> 12.2 mV

	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_trigger_generation_setup_reg[i_mod][i_ch] = ((uint_trigger_pulse_length & 0xff) << 24)
															+ ((uint_trigger_gap & 0xfff) << 12)
															+ ((uint_trigger_peaking & 0xfff));
			uint_channel_trigger_generation_threshold_reg[i_mod][i_ch] = 0x80000000 + 0x30000000  //
																+ 0x8000000  //  MAW offset
																+ (uint_trigger_threshold_value & 0x7FFFFFF) ; //  Threshold
			uint_channel_he_trigger_generation_threshold_reg[i_mod][i_ch] = 0x0; //
		}
	}


	// Accumulator 1
	gate_start_index[0] = 0;
	gate_length[0] = 90-1;

	// Accumulator 2
	gate_start_index[1] = 95;
	gate_length[1] = 55-1;

	gate_start_index[2] = 150;
	gate_length[2] = 50-1;

	gate_start_index[3] = 200;
	gate_length[3] = 100-1;

	gate_start_index[4] = 300;
	gate_length[4] = 100-1;

	gate_start_index[5] = 400;
	gate_length[5] = 100-1;

	// data format
	header_accu_6_values_enable_flag = 1 ;  // not in configuration file -> always 1
	header_accu_2_values_enable_flag = 0 ;  // not in configuration file -> always 0
	header_maw_3_values_enable_flag  = 0 ;  // not in configuration file -> always 0

	maw_test_buffer_enable_flag = 0 ; // not in configuration file -> always 0
	maw_test_buffer_length      = 0;  // not in configuration file -> always 0
	maw_test_buffer_delay       = 0;    // max 1022
	if (maw_test_buffer_delay > 1022) {
		maw_test_buffer_delay = 1022;
	}



	// trigger_gate_window_length and sample length
	trigger_gate_window_length       = 650;
	uint_save_raw_sample_length      = 624;
	uint_save_raw_sample_start_index = 0;
	uint_save_raw_sample_first_event_only_mode = 0;
 	pre_trigger_delay = 100;


	// Note: internal_fir_trigger_delay value is two times of clock
	//internal_fir_trigger_delay = 72;  //  72 * 2 * 4ns = 576ns for ~400ns TO-Output to Gate-IN delay (Note 1: max 255; Note 2: value has to muiltiply by 2 clocks)
	internal_fir_trigger_delay = 0;  //  72 * 2 * 4ns = 576ns for ~400ns TO-Output to Gate-IN delay (Note 1: max 255; Note 2: value has to muiltiply by 2 clocks)

	pre_trigger_delay = pre_trigger_delay + uint_trigger_peaking + (uint_trigger_peaking >> 1) + uint_trigger_gap + 24;  // 16 is additional delay for 50% CFD trigger --> see edge at index 10
	pre_trigger_delay = pre_trigger_delay + (2 * internal_fir_trigger_delay);

	// pileups
	uint_pileup    = trigger_gate_window_length;  //
	uint_re_pileup = trigger_gate_window_length;  //


	// define TI Deadtime
	external_veto_gate_delay = 0;

	uint_trigger_function_deadtime_logic_enable                = 1 ;
	uint_trigger_function_deadtime_block_with_addrThres_enable = 1 ;

	uint_trigger_function_deadtime_length = ((trigger_gate_window_length + 250 ) / 16); // + 1us

	if (uint_trigger_function_deadtime_length > 0x3fff) { uint_trigger_function_deadtime_length = 0x3fff; }
	if(uint_trigger_function_deadtime_block_with_addrThres_enable == 1) {
		//uint_trigger_function_deadtime_length = uint_trigger_function_deadtime_length + 0x4000; // Enable Deadtime with internal Address Threshold Flag
		uint_trigger_function_deadtime_length = uint_trigger_function_deadtime_length + 0x8000; // Enable Deadtime with FP-Bus Address Threshold Flag
	}

	// Lemo Out definition
	//uint_lemo_out_CO_select = 0x0;
	uint_lemo_out_CO_select = 0x1; // Select Sample Clock

	//uint_lemo_out_TO_select = 0xFFFF; // Select all internal triggers
	//uint_lemo_out_TO_select = 0x1; // Select ch1 trigger
	//uint_lemo_out_TO_select = 0x4; // Select ch3 trigger
	//uint_lemo_out_TO_select = 0xF; // Select ch1-4 trigger
	uint_lemo_out_TO_select = 0xFFFF; // Select ch1-4 trigger

	//uint_lemo_out_UO_select = 0x4;  // Select LogicBusy
	//uint_lemo_out_UO_select = 0x2;  //  BankxArmed
	uint_lemo_out_UO_select = 0x08000000;  // Select Select External Veto/Gate to ADC FPGA

	/******************************************************************************************/
	/*                                                                                        */
	/*  command line interpreter                                                               */
	/*                                                                                        */
	/******************************************************************************************/

	if (argc > 1) {

		while ((int_ch = getopt(argc, argv, "?lhpN:C:F:f:")) != -1) {

			switch (int_ch) {
				//printf("ch %c    \n", int_ch );


				case 'N':
		    		sscanf(optarg,"%d",&data) ;
					stop_after_loop_counts                   = data ;    //
					break;


				case 'C':
					//printf("case 'F' \n" );
					sscanf(optarg,"%s", char_config_file) ;
					//printf("char_config_file %s    \n", char_config_file );
					gl_sis3316_get_configuration_parameters      = new sis3316_get_configuration_parameters() ;

					configurationFile_rc = gl_sis3316_get_configuration_parameters->read_parameter_file(char_config_file);
					if (configurationFile_rc != 0) {
						printf("\n" );
						printf("Error on opening configuration File   \n" );
						printf("program stopped !  \n" );
						printf("\n" );
						do {
							usleep(10000) ;
						} while(1) ;
					}
					uint_configurationFile_valid_flag = 1 ;

					break;


				case 'p':
					uint_print_configurationParameterOnly_flag = 1 ;
					break;

				case 'F':
					sscanf(optarg,"%s", char_WriteData_to_File_initialize_filename) ;
					printf("char_WriteData_to_File_initialize_filename %s    \n", char_WriteData_to_File_initialize_filename );
		    		uint_WriteData_to_File_EnableFlag      = 1  ;  // save to file
					uint_WriteData_to_MultipleFiles_Flag   = 0 ;
					break;
				case 'f':
					sscanf(optarg,"%s", char_WriteData_to_File_initialize_filename) ;
					//printf("char_WriteData_to_File_initialize_filename %s    \n", char_WriteData_to_File_initialize_filename );
		    		uint_WriteData_to_File_EnableFlag      = 1  ;  // save to file
					uint_WriteData_to_MultipleFiles_Flag   = 1 ;
					break;


				case '?':
				case 'h':
				default:
					printf("   \n");



					printf("Usage: %s  [-?h] [-p]  [-N number of loops] [-C config_filename.ini] [-F or -f data_filename] ", argv[0]);
					printf("   \n");
					printf("   \n");


					printf("   -N num      .....  number of loops (banks); default: 1   (-N 0: endless) \n");
					printf("   -C filename.ini .  configuration file name; for example sis3316_running_parameter.ini \n");
					printf("   \n");
					printf("   -F filename .....  write to one file (file name without .dat )\n");
					printf("   -f filename .....  write to N files  (max. file size is each 1 GByte)\n");
					printf("   \n");
					printf("   -p     ..........  print the parameter only\n");
					printf("   \n");
					printf("   -h     ..........  print this message only\n");
					printf("   \n");
					printf("   \n");
					printf("   date: 25.07.2024 \n");
					printf("   \n");
					printf("   \n");
					exit(1);
			}
		}
    } // if (argc > 1)

	printf("\n");

	if(uint_configurationFile_valid_flag == 1) { // take parameter from file

		uint_udp_jumbo_mode               = gl_sis3316_get_configuration_parameters->uint_udp_jumbo_mode  ;    //
		uint_udp_nofPacketsPerRequest     = gl_sis3316_get_configuration_parameters->uint_udp_nofPacketsPerRequest  ;

		nof_events_per_bank                        = gl_sis3316_get_configuration_parameters->uint_nof_events  ;    // each Bank

		// Set Gain/Termination
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_range_2V_flag[i_mod][i_ch]                  = gl_sis3316_get_configuration_parameters->uint_channel_range_2V;
				uint_channel_50ohm_termination_disable_flag[i_mod][i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_50ohm_termination_disable;
			}
		}


		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			for (i_fpga = 0; i_fpga < 4; i_fpga++) {
				uint_fpga_analog_ctrl_val[i_mod][i_fpga] = (uint_channel_range_2V_flag[i_mod][(i_fpga * 4) + 0] & 1)
					+ ((uint_channel_range_2V_flag[i_mod][(i_fpga * 4) + 1] & 1) << 8)
					+ ((uint_channel_range_2V_flag[i_mod][(i_fpga * 4) + 2] & 1) << 16)
					+ ((uint_channel_range_2V_flag[i_mod][(i_fpga * 4) + 3] & 1) << 24)
					+ ((uint_channel_50ohm_termination_disable_flag[i_mod][(i_fpga * 4) + 0] & 1) << 2)
					+ ((uint_channel_50ohm_termination_disable_flag[i_mod][(i_fpga * 4) + 1] & 1) << 10)
					+ ((uint_channel_50ohm_termination_disable_flag[i_mod][(i_fpga * 4) + 2] & 1) << 18)
					+ ((uint_channel_50ohm_termination_disable_flag[i_mod][(i_fpga * 4) + 3] & 1) << 26);
			}
		}

		//set ADC offsets (DAC) :
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_analog_offset_dac_val[i_mod][i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_adc_offset;
			}
		}

		// channel polarity
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_polarity_invert_flag[i_mod][i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_polarity_invert; //
			}
		}


		uint_acquisition_trigger_mode =  gl_sis3316_get_configuration_parameters->uint_acquisition_trigger_mode; //

		if(uint_acquisition_trigger_mode == 0) {// 0: TI External Trigger
			// channel external gate enable(1)/disable(0)
			for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
				for (i_ch = 0; i_ch < 16; i_ch++) {
					uint_channel_external_gate_enable_flag[i_mod][i_ch] = 0; // all disable
				}
			}

			// channel external trigger enable(1)/disable(0)
			for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
				for (i_ch = 0; i_ch < 16; i_ch++) {
					uint_channel_external_trigger_enable_flag[i_mod][i_ch] = 1; // all enable
				}
			}

			// channel internal trigger enable(1)/disable(0)
			for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
				for (i_ch = 0; i_ch < 16; i_ch++) {
					uint_channel_internal_trigger_enable_flag[i_mod][i_ch] = 0; // all disable
				}
			}

		}
		else { // 1: TI External Gate/Internal Trigger
			// channel external gate enable(1)/disable(0)
			for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
				for (i_ch = 0; i_ch < 16; i_ch++) {
					uint_channel_external_gate_enable_flag[i_mod][i_ch] = 1; // all enable
				}
			}

			// channel external trigger enable(1)/disable(0)
			for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
				for (i_ch = 0; i_ch < 16; i_ch++) {
					uint_channel_external_trigger_enable_flag[i_mod][i_ch] = 0; // all disable
				}
			}

			// channel internal trigger enable(1)/disable(0)
			for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
				for (i_ch = 0; i_ch < 16; i_ch++) {
					uint_channel_internal_trigger_enable_flag[i_mod][i_ch] = 1; // all enable
				}
			}
		}


		// internal Trigger generation
		uint_trigger_pulse_length    = gl_sis3316_get_configuration_parameters->uint_channel_trigger_pulse_length; //;
		uint_trigger_gap             = gl_sis3316_get_configuration_parameters->uint_channel_trigger_gap; //;
		uint_trigger_peaking         = gl_sis3316_get_configuration_parameters->uint_channel_trigger_peaking; //;
		uint_trigger_threshold_value = gl_sis3316_get_configuration_parameters->uint_channel_trigger_threshold * uint_trigger_peaking; //

		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_trigger_generation_setup_reg[i_mod][i_ch] = ((uint_trigger_pulse_length & 0xff) << 24)
																+ ((uint_trigger_gap & 0xfff) << 12)
																+ ((uint_trigger_peaking & 0xfff));
				uint_channel_trigger_generation_threshold_reg[i_mod][i_ch] = 0x80000000 + 0x30000000  // Enable and CFD = 3
																	+ 0x8000000  //  MAW offset
																	+ (uint_trigger_threshold_value & 0x7FFFFFF) ; //  Threshold
				uint_channel_he_trigger_generation_threshold_reg[i_mod][i_ch] = 0x0; //
			}
		}



		// trigger_gate_window_length and sample length
		trigger_gate_window_length                 = gl_sis3316_get_configuration_parameters->uint_trigger_gate_window_length; //
		uint_save_raw_sample_length                = gl_sis3316_get_configuration_parameters->uint_raw_sample_length; //
		uint_save_raw_sample_start_index           = gl_sis3316_get_configuration_parameters->uint_raw_sample_start_index; //
		uint_save_raw_sample_first_event_only_mode = gl_sis3316_get_configuration_parameters->uint_raw_sample_first_event_only_mode; //


		pre_trigger_delay                = gl_sis3316_get_configuration_parameters->uint_pre_trigger_delay; // ;

		internal_fir_trigger_delay = 0;  //  72 * 2 * 4ns = 576ns for ~400ns TO-Output to Gate-IN delay (Note 1: max 255; Note 2: value has to muiltiply by 2 clocks)

		pre_trigger_delay = pre_trigger_delay + uint_trigger_peaking + (uint_trigger_peaking >> 1) + uint_trigger_gap + 24;  // 16 is additional delay for 50% CFD trigger --> see edge at index 10
		pre_trigger_delay = pre_trigger_delay + (2 * internal_fir_trigger_delay);

		// define TI Deadtime
		external_veto_gate_delay = gl_sis3316_get_configuration_parameters->uint_veto_gate_delay; // ;



		// pileups
		uint_pileup    = gl_sis3316_get_configuration_parameters->uint_pileup_window_length; //
		uint_re_pileup = gl_sis3316_get_configuration_parameters->uint_re_pileup_window_length; //


		// Accumulator 1
		gate_start_index[0] =  gl_sis3316_get_configuration_parameters->uint_gate1_start_index; //
		gate_length[0]      =  gl_sis3316_get_configuration_parameters->uint_gate1_length -1  ;  // -1 !!

		// Accumulator 2
		gate_start_index[1] = gl_sis3316_get_configuration_parameters->uint_gate2_start_index; //
		gate_length[1]      = gl_sis3316_get_configuration_parameters->uint_gate2_length -1  ;  // -1 !!

		gate_start_index[2] = gl_sis3316_get_configuration_parameters->uint_gate3_start_index; // ;
		gate_length[2]      = gl_sis3316_get_configuration_parameters->uint_gate3_length -1  ;  // -1 !!


		gate_start_index[3] = gl_sis3316_get_configuration_parameters->uint_gate4_start_index; // ;
		gate_length[3]      = gl_sis3316_get_configuration_parameters->uint_gate4_length -1  ;  // -1 !!

		gate_start_index[4] = gl_sis3316_get_configuration_parameters->uint_gate5_start_index; // ;
		gate_length[4]      = gl_sis3316_get_configuration_parameters->uint_gate5_length -1  ;  // -1 !!

		gate_start_index[5] = gl_sis3316_get_configuration_parameters->uint_gate6_start_index; // ;
		gate_length[5]      = gl_sis3316_get_configuration_parameters->uint_gate6_length -1  ;  // -1 !!


		// Lemo Out definition
		uint_lemo_out_CO_select = gl_sis3316_get_configuration_parameters->uint_lemo_out_CO_select; //
		uint_lemo_out_TO_select = gl_sis3316_get_configuration_parameters->uint_lemo_out_TO_select; //
		uint_lemo_out_UO_select = gl_sis3316_get_configuration_parameters->uint_lemo_out_UO_select; //


		uint_trigger_function_deadtime_logic_enable                = gl_sis3316_get_configuration_parameters->uint_trigger_function_deadtime_logic_enable; //
		uint_trigger_function_deadtime_block_with_addrThres_enable = gl_sis3316_get_configuration_parameters->uint_trigger_function_deadtime_block_with_addrThres_enable; //
		uint_trigger_function_deadtime_length                      = gl_sis3316_get_configuration_parameters->uint_trigger_function_deadtime_length; //

		if (uint_trigger_function_deadtime_length > 0x3fff) { uint_trigger_function_deadtime_length = 0x3fff; }
		if(uint_trigger_function_deadtime_block_with_addrThres_enable == 1) {
			//uint_trigger_function_deadtime_length = uint_trigger_function_deadtime_length + 0x4000; // Enable Deadtime with internal Address Threshold Flag
			uint_trigger_function_deadtime_length = uint_trigger_function_deadtime_length + 0x8000; // Enable Deadtime with FP-Bus Address Threshold Flag
		}
	}

// print configuration
		if(uint_print_configurationParameterOnly_flag == 1) { ;

			printf("\n");
			printf("uint_udp_jumbo_mode bank                              = %d \n",uint_udp_jumbo_mode);
			printf("uint_udp_nofPacketsPerRequest                         = %d \n",uint_udp_nofPacketsPerRequest);
			printf("\n");
			printf("nof_events per bank                                   = %d \n",nof_events_per_bank);
  			printf("\n");
  			printf("\n");
	  		printf("channel polarity invert ch1 to ch16                   = ");
	  		printf(" %d",uint_channel_polarity_invert_flag[0][0]);
 			printf("\n");
 			printf("\n");
	  		printf("channel range_2V ch1 to ch16                          = ");
 			printf(" %d",uint_channel_range_2V_flag[0][0]);
  			printf("\n");
	  		printf("channel 50 Ohm termination disable ch1 to ch16        = ");
  			printf(" %d",uint_channel_50ohm_termination_disable_flag[0][0]);
  			printf("\n");
  			printf("\n");
	  		printf("channel ADC offset                                    = ");
  			printf(" %d",uint_channel_analog_offset_dac_val[0][0]);

			printf("\n");
 			printf("\n");
  			printf("lemo_out_CO_select                                    = 0x%08x \n", uint_lemo_out_CO_select);
  			printf("lemo_out_TO_select                                    = 0x%08x \n", uint_lemo_out_TO_select);
  			printf("lemo_out_UO_select                                    = 0x%08x \n", uint_lemo_out_UO_select);


			printf("\n");
	  		if(uint_acquisition_trigger_mode == 0) {
		  		printf("acquisition_trigger_mode                              = %d   ->  External Trigger (TI input) \n", uint_acquisition_trigger_mode);
	  		}
	  		else {
		  		printf("acquisition_trigger_mode                              = %d   ->  External Gate (TI input) / Internal Trigger\n", uint_acquisition_trigger_mode);
	  		}
	  		printf("\n");
	  		printf("pre_trigger_delay  (added 1.5*P+G)                    = %d \n", pre_trigger_delay);
	  		printf("raw_sample_start_index                                = %d \n", uint_save_raw_sample_start_index);
	  		printf("raw_sample_length                                     = %d \n", uint_save_raw_sample_length);
	  		printf("raw_sample_first_event_only_mode                      = %d \n", uint_save_raw_sample_first_event_only_mode);
  			printf("\n");
	  		printf("trigger_gate_window_length                            = %d \n", trigger_gate_window_length);
	  		printf("pileup_window_length                                  = %d \n", uint_pileup);
	  		printf("re_pileup_window_length                               = %d \n", uint_re_pileup);
  			printf("\n");

	  		printf("trigger_function_deadtime_logic_enable                = %d \n", uint_trigger_function_deadtime_logic_enable);
	  		printf("trigger_function_deadtime_block_with_addrThres_enable = %d \n", uint_trigger_function_deadtime_block_with_addrThres_enable);
	  		printf("trigger_function_deadtime_length                      = %d   -> write register value = %d\n", ((uint_trigger_function_deadtime_length & 0x3fff)*16), (uint_trigger_function_deadtime_length & 0x3fff));
  			printf("\n");

			printf("\n");
 			printf("Accumulator Gate 1 Start Index                        = %d \n", gate_start_index[0]);
  			printf("Accumulator Gate 1 Length                             = %d\n", gate_length[0] + 1);
			printf("\n");
 			printf("Accumulator Gate 2 Start Index                        = %d \n", gate_start_index[1]);
  			printf("Accumulator Gate 2 Length                             = %d\n", gate_length[1] + 1);
			printf("\n");
 			printf("Accumulator Gate 3 Start Index                        = %d \n", gate_start_index[2]);
  			printf("Accumulator Gate 3 Length                             = %d\n", gate_length[2] + 1);
			printf("\n");
 			printf("Accumulator Gate 4 Start Index                        = %d \n", gate_start_index[3]);
  			printf("Accumulator Gate 4 Length                             = %d\n", gate_length[3] + 1);
			printf("\n");
 			printf("Accumulator Gate 5 Start Index                        = %d \n", gate_start_index[4]);
  			printf("Accumulator Gate 5 Length                             = %d\n", gate_length[4] + 1);
			printf("\n");
 			printf("Accumulator Gate 6 Start Index                        = %d \n", gate_start_index[5]);
  			printf("Accumulator Gate 6 Length                             = %d\n", gate_length[5] + 1);

#ifdef TEST_PRINT
	  		printf("channel external trigger enable ch1 to ch16      = ");
			for(i_ch=0;i_ch<1;i_ch++) {
	  			printf(" %d",uint_channel_external_trigger_enable_flag[i_ch]);
			}

 			printf("\n");
	  		printf("channel trigger generation Setup_reg ch1 to ch16      = ");
			for(i_ch=0;i_ch<1;i_ch++) {
				if(i_ch == 8) {printf("\n                                                        ");}
				printf(" 0x%08x",uint_channel_trigger_generation_setup_reg[i_ch]);
			}

			printf("\n");
	  		printf("channel trigger generation Threshold reg ch1 to ch16  = ");
			for(i_ch=0;i_ch<1;i_ch++) {
				if(i_ch == 8) {printf("\n                                                        ");}
	  			printf(" 0x%08x",uint_channel_trigger_generation_threshold_reg[i_ch]);
			}
#endif
 			printf("\n");
 			printf("\n");


	  		printf("\n");
	 		printf("\n");
		    exit(1);
		}










/*********************************************************************************************************************/

/******************************************************************************************/
#ifdef WINDOWS
//#ifdef raus
	if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, TRUE ) ){
		printf( "Error setting Console-Ctrl Handler\n" );
		return -1;
	}
#endif
/******************************************************************************************/


	// allocating IP-address buffer
	char *sis3316_ip_addr_string[MAX_NOF_SIS3316_ADCS];
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_ip_addr_string[i_mod] = (char *)malloc(32);;
		if(sis3316_ip_addr_string[i_mod] == NULL){
			printf("Error allocating IP-Address buffers !\n");
			program_stop_and_wait();
		}
	}

	// allocating DMA Read Memory buffer
	unsigned int *dma_read_buffer;
	dma_read_buffer = (unsigned int *)malloc(MAX_NUMBER_LWORDS_64MBYTE * 4);
	if(dma_read_buffer == NULL){
		printf("Error allocating dma_read_buffer !\n");
		program_stop_and_wait();
	}


#ifdef ETHERNET_UDP_INTERFACE

	char  pc_ip_addr_string[32] ;

	  if(MAX_NOF_SIS3316_ADCS >= 1) { 	strcpy(sis3316_ip_addr_string[0],"sis3316-1002") ; } // SIS3316-2 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 2) { 	strcpy(sis3316_ip_addr_string[1],"sis3316-1001") ; } // SIS3316-2 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 3) { 	strcpy(sis3316_ip_addr_string[2],"sis3316-0002") ; } // SIS3316 IP address

	  //if(MAX_NOF_SIS3316_ADCS >= 1) { 	strcpy(sis3316_ip_addr_string[0],"192.168.1.100") ; } // SIS3316 IP address
	  //if(MAX_NOF_SIS3316_ADCS >= 2) { 	strcpy(sis3316_ip_addr_string[1],"192.168.1.101") ; } // SIS3316 IP address
	  //if(MAX_NOF_SIS3316_ADCS >= 3) { 	strcpy(sis3316_ip_addr_string[2],"192.168.1.102") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 4) { 	strcpy(sis3316_ip_addr_string[3],"192.168.1.103") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 5) { 	strcpy(sis3316_ip_addr_string[4],"192.168.1.104") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 6) { 	strcpy(sis3316_ip_addr_string[5],"192.168.1.105") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 7) { 	strcpy(sis3316_ip_addr_string[6],"192.168.1.106") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 8) { 	strcpy(sis3316_ip_addr_string[7],"192.168.1.107") ; } // SIS3316 IP address

	  if(MAX_NOF_SIS3316_ADCS >= 9) { 	strcpy(sis3316_ip_addr_string[8],"192.168.2.100") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 10) { 	strcpy(sis3316_ip_addr_string[9],"192.168.2.101") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 11) { 	strcpy(sis3316_ip_addr_string[10],"192.168.2.102") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 12) { 	strcpy(sis3316_ip_addr_string[11],"192.168.2.103") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 13) { 	strcpy(sis3316_ip_addr_string[12],"192.168.2.104") ; } // SIS3316 IP address

#endif


#ifdef ETHERNET_UDP_INTERFACE

	#ifdef WINDOWS
     return_code = WinsockStartup();
	#endif


	// virtual *vme_crates !!
	sis3316_eth  *sis3316_eth_device[MAX_NOF_SIS3316_ADCS] ; 
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_eth_device[i_mod] = new sis3316_eth ;

		// increase read_buffer size
		// SUSE needs following command as su: >sysctl -w net.core.rmem_max=33554432
		int	sockbufsize = 335544432 ; // 0x2000000
		return_code = sis3316_eth_device[i_mod]->set_UdpSocketOptionBufSize(sockbufsize) ;

		//strcpy(pc_ip_addr_string,"212.60.16.49") ; // Window example: secocnd Lan interface IP address is 212.60.16.49
		strcpy(pc_ip_addr_string,"") ; // empty if default Lan interface (Window: use IP address to bind in case of 2. 3. 4. .. LAN Interface)
		return_code = sis3316_eth_device[i_mod]->set_UdpSocketBindMyOwnPort( pc_ip_addr_string);

		return_code = sis3316_eth_device[i_mod]->set_UdpSocketSIS3316_IpAddress( sis3316_ip_addr_string[i_mod]);

	 	return_code = sis3316_eth_device[i_mod]->udp_reset_cmd();
		//return_code = vme_crate->udp_reset_cmd();
		return_code = sis3316_eth_device[i_mod]->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000); // kill request and grant from other vme interface
		if (return_code != 0) {
			printf("vme_A32D32_write return_code = 0X%08x   \n",return_code);
			return 0;
		}
		return_code = sis3316_eth_device[i_mod]->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x1); // request access to SIS3316 from UDP interface
		if (return_code != 0) {
			printf("vme_A32D32_write return_code = 0X%08x   \n",return_code);
			return 0;
		}
		sis3316_eth_device[i_mod]->vme_A32D32_write(SIS3316_KEY_ADC_FPGA_RESET, 0);

		return_code = sis3316_eth_device[i_mod]->vme_A32D32_read(SIS3316_MODID,&data);
		if (return_code != 0) {
			printf("vme_A32D32_read return_code = 0X%08x   \n",return_code);
			return 0;
		}
		//printf("return_code = 0X%08x   Module ID = 0X%08x \n",return_code, data);
 
		return_code = sis3316_eth_device[i_mod]->vme_A32D32_read(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL,&data);
		if (return_code != 0) {
			printf("i_mod = %d    vme_A32D32_read return_code = 0X%08x   \n",i_mod, return_code);
			return 0;
		}

		// open Vme Interface device
		return_code = sis3316_eth_device[i_mod]->vmeopen ();  // open Vme interface
		sis3316_eth_device[i_mod]->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface
	    //printf("get_vmeopen_messages = %s , nof_found_devices %d \n",char_messages, nof_found_devices);
	}

	// set UDP option/mode
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		if(uint_udp_jumbo_mode == 0) { sis3316_eth_device[i_mod]->set_UdpSocketDisableJumboFrame() ; }
		if(uint_udp_jumbo_mode == 1) { sis3316_eth_device[i_mod]->set_UdpSocketEnableJumboFrame() ; }
		sis3316_eth_device[i_mod]->set_UdpSocketReceiveNofPackagesPerRequest(uint_udp_nofPacketsPerRequest);
	}

 
	sis3316_adc  *sis3316_adc_array[MAX_NOF_SIS3316_ADCS] ;
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_adc_array[i_mod] = new sis3316_adc( sis3316_eth_device[i_mod], i_mod * 0x01000000); // base address (i_mod * 0x01000000) is used for header information !
	}

	do {
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			return_code = sis3316_adc_array[i_mod]->register_read(0x4, &data);
			if ((return_code != 0) || ((data & 0xffff0000) != 0x33160000)) {
				printf("not valid SIS3316 IP address\n");
				printf("return_code      = 0x%08X \n", return_code);
				printf("module ID index  = 0x%08X \n", i_mod);
				return -1;
			}
			else {
				return_code = sis3316_adc_array[i_mod]->register_read(SIS3316_SERIAL_NUMBER_REG, &serial_no);
				printf("Serial number   = %d  \t", serial_no&0xffff);
				printf("module ID/VME FPGA version = 0x%08X \t", data);
				sis3316_adc_array[i_mod]->register_read(0x1100, &data);
				printf("ADC FPGA version = 0x%08X \n", data);
			}
			//printf("\n");
			//printf("DHCP option     = %d\n", (data&0xff000000)>>24);
			return_code = sis3316_adc_array[i_mod]->register_read(SIS3316_INTERNAL_TEMPERATURE_REG, &data);
			signed short signed_short_temperature ;
			float float_temperature_c, float_temperature_f ;
			signed_short_temperature =  ((signed short) (data&0xffff) ) ;
			float_temperature_c =  (float) (signed_short_temperature) / 4.0 ;
			float_temperature_f =  32.0 + (float_temperature_c * 1.8) ;
			printf("Temperature     = %2.2f C    %3.2f F \n", float_temperature_c, float_temperature_f );
			printf("\n");
		}
		usleep(500000) ;
		printf("\n\n\n\n");
	} while (0) ;




#endif

 


	
	
/******************************************************************************************************************************/
/* CERN ROOT                                                                                                                  */
/******************************************************************************************************************************/

#ifdef CERN_ROOT_PLOT

	sis_root_graph *graph_raw[MAX_NOF_SIS3316_ADCS] ;

	int root_graph_x[MAX_NOF_SIS3316_ADCS] ;
	int root_graph_y[MAX_NOF_SIS3316_ADCS] ;
	int root_graph_x_size ;
	int root_graph_y_size ;
	char root_graph_text[80] ;

//	root_graph_x_size = 900 ;
//	root_graph_y_size = 500 ;
	root_graph_x_size = 350 ;
	root_graph_y_size = 300 ;

	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		if(i_mod <5) {
			root_graph_x[i_mod] = 10 + ((i_mod ) * (root_graph_x_size + 5));
			root_graph_y[i_mod] = 10 + (0 * (root_graph_y_size + 30));
		}
		else {
			if(i_mod <10) {
				root_graph_x[i_mod] = 10 + ((i_mod - 5) * (root_graph_x_size + 5));
				root_graph_y[i_mod] = 10 + (1 * (root_graph_y_size + 30));
			}
			else { //  11 to 13
				root_graph_x[i_mod] = 10 + ((i_mod - 10) * (root_graph_x_size + 5));
				root_graph_y[i_mod] = 10 + (2 * (root_graph_y_size + 30));
			}
		}
		//root_graph_x[i_mod] = 10 + ((i_mod & 3) * (root_graph_x_size + 5));
		//root_graph_y[i_mod] = 10 + (((i_mod & 0x1c) / 4) * (root_graph_y_size + 30));
	}
	 
	TApplication theApp("SIS3316 Application: Test", &argc, (char**)argv);

	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		//strcpy(root_graph_text,"SIS3316 Graph: Raw data") ;
		sprintf(root_graph_text,"SIS3316 #%d Graph: Raw data ", i_mod+1);
		graph_raw[i_mod]      = new sis_root_graph(root_graph_text, root_graph_x[i_mod], root_graph_y[i_mod], root_graph_x_size, root_graph_y_size) ;
		graph_raw[i_mod]->sis3316_set_14bit_Yaxis();
	}
#endif



	
/******************************************************************************************/
	
#ifdef not_used
	
	// reset ADC FPGA  
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_adc_array[i_mod]->reset_adc_fpga_and_DDR_memory() ;   
	}
	usleep(100000) ;
#endif


	

/******************************************************************************************/
/*                                                                                        */
/*     Hardware configuration                                                             */
/*                                                                                        */
/******************************************************************************************/

// disarm all modules (preventive in case of that at least one module is armed)
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_adc_array[i_mod]->register_write(SIS3316_KEY_DISARM, 0) ;  // disarm
	}

// reset all modules except the first module (FP-Master) (preventive in case of that all modules are using the FP-Bus for clock, Trigger, Sample Control)
	for (i_mod=1; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_adc_array[i_mod]->register_write(SIS3316_KEY_RESET, 0) ;  // reset
	}
// reset the first module (FP-Master) (preventive in case of that all modules are using the FP-Bus for clock, Trigger, Sample Control)
	i_mod=0 ;
	sis3316_adc_array[i_mod]->register_write(SIS3316_KEY_RESET, 0) ;  // reset


/*  System Sample Clock Configuration */
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_adc_array[i_mod]->register_write(SIS3316_KEY_DISARM, 0) ;  // disarm
		sis3316_adc_array[i_mod]->register_write(SIS3316_KEY_RESET, 0) ;  // reset
		sis3316_adc_array[i_mod]->adc_spi_reg_enable_adc_outputs() ;  // necessary after reset

		//setup clock
	#ifdef ONLY_SIS3316 // old
		if (sis3316_adc_array[i_mod]->adc_125MHz_flag == 0) { // 250 MHz
			// 250.000 MHz
			clock_N1div      =  4  ;
			clock_HSdiv      =  5  ;
			iob_delay_value  =  0x1002 ; // ADC FPGA version A_0250_0004 and higher
		}
		else {
			// 125.000 MHz
			clock_N1div      =  8  ;
			clock_HSdiv      =  5  ;
			iob_delay_value  =  0x1008 ; // 16 bit ADC FPGA version A_0125_0004 and higher
		}
		sis3316_adc_array[i_mod]->change_frequency_HSdiv_N1div(0, clock_HSdiv, clock_N1div) ;
	#endif


		// new with SIS3316 / SIS3316-2
		if (sis3316_adc_array[i_mod]->adc_125MHz_flag == 0) { // 250 MHz
			// 250.000 MHz
			clock_freq_choice = SIS::ADC::SIS3316::SAMPLERATE_250MSPS;
		}
		else {
			// 125.000 MHz
			clock_freq_choice = SIS::ADC::SIS3316::SAMPLERATE_125MSPS;
		}
		sis3316_adc_array[i_mod]->get_SI570_oscillator_hs_div_and_n1_div_values(clock_freq_choice, &clock_HSdiv, &clock_N1div, &double_clock_configure_fft_frequency);
		sis3316_adc_array[i_mod]->get_adc_fpga_iob_delay_value(clock_freq_choice, &iob_delay_value);

		sis3316_adc_array[i_mod]->change_frequency_HSdiv_N1div(0, clock_HSdiv, clock_N1div);
		sis3316_adc_array[i_mod]->configure_adc_fpga_iob_delays(iob_delay_value);  // necessary after changing/setting clock
	}

/************/

	// Setup of Sample Clock on Frontpanel LVDS Bus:
	// first SIS3316 drives the Clock and the external trigger
	data = 0 ;
	data = data + 0x1 ;   // Enables the Control lines to the FP-Bus
	data = data + 0x2 ;   // Enables the Status lines to the FP-Bus
	data = data + 0x10 ;  // Enables Sample Clock driver to the FP-Bus
	data = data + 0x0 ;   // Selects internal Clock oscillator
	//data = data + 0x20 ;   // Selects Lemo Clock In 
	i_mod=0;	
	sis3316_adc_array[i_mod]->register_write(SIS3316_FP_LVDS_BUS_CONTROL, data) ;   
	if(MAX_NOF_SIS3316_ADCS > 1) {
		data =  0x2 ;   // Enables the Status lines to the FP-Bus, only
		for (i_mod=1; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			sis3316_adc_array[i_mod]->register_write(SIS3316_FP_LVDS_BUS_CONTROL, data) ;   
		}
	}


/************/
	// define the sample Clock on each module	
	//data = 0 ; // Onboard Oscillator
	//data = 1 ; // VXS-Bus Clock
	data = 2 ; // FP-LVDS-Bus Clock
	//data = 3 ; // External NIM Clock
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_adc_array[i_mod]->register_write(SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL, data) ;   
	}
	// reset ADC FPGA sample clock 
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_adc_array[i_mod]->reset_adc_fpga_sample_clock_PLL() ;   
	}

	// set tap_delay
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_adc_array[i_mod]->get_adc_fpga_iob_delay_value(clock_freq_choice, &iob_delay_value);
		printf("i = %d    iob_delay_value return_code = 0x%08x \n", i_mod, iob_delay_value);
		sis3316_adc_array[i_mod]->configure_adc_fpga_iob_delays(iob_delay_value) ;  // necessary after changing/setting clock
	}

	// Now are the clocks are configured on all modules


/******************************************************************************************/

/******************************************/


	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		printf("i_mod = %d:   Configuration   \n", i_mod);

		/******************************************/

		// Gain/Termination
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[i_mod][0]); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[i_mod][1]); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[i_mod][2]); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[i_mod][3]); //

		/******************************************/

		//  set ADC offsets (DAC)
		for (i_ch = 0; i_ch<16; i_ch++) {
			sis3316_adc_array[i_mod]->adc_dac_offset_ch_array[i_ch] = uint_channel_analog_offset_dac_val[i_mod][i_ch]; //
		}
		return_code = sis3316_adc_array[i_mod]->write_all_adc_dac_offsets();
		if (return_code != 0) {
			printf("Error write_all_adc_dac_offsets: return_code = 0x%08x \n", return_code);
		}

		/******************************************/

		// Select LEMO Output "Co"
		sis3316_adc_array[i_mod]->register_write(SIS3316_LEMO_OUT_CO_SELECT_REG, uint_lemo_out_CO_select); //

		// Select LEMO Output "To"
		sis3316_adc_array[i_mod]->register_write(SIS3316_LEMO_OUT_TO_SELECT_REG, uint_lemo_out_TO_select); //

		// Select LEMO Output "UO"
		sis3316_adc_array[i_mod]->register_write(SIS3316_LEMO_OUT_UO_SELECT_REG, uint_lemo_out_UO_select); //


		/******************************************/

		// channel Header
		data = (i_mod << 24) + 0x0;
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG, data); //
		data = (i_mod << 24) + 0x00400000;
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG, data); //
		data = (i_mod << 24) + 0x00800000;
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG, data); //
		data = (i_mod << 24) + 0x00C00000;
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG, data); //

		/******************************************/
		// internal Trigger generation
		for (i_ch = 0; i_ch < 16; i_ch++) {
			sis3316_adc_array[i_mod]->internal_trigger_generation_setup(uint_channel_trigger_generation_threshold_reg[i_mod][i_ch], uint_channel_he_trigger_generation_threshold_reg[i_mod][i_ch], uint_channel_trigger_generation_setup_reg[i_mod][i_ch], i_ch);  //
		}

		/******************************************/

		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length

		/******************************************/

		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG, ((uint_save_raw_sample_length & 0xffff) << 16) + (uint_save_raw_sample_start_index & 0xffff) ); // Sample Length
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG, ((uint_save_raw_sample_length & 0xffff) << 16) + (uint_save_raw_sample_start_index & 0xffff) ); // Sample Length
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG, ((uint_save_raw_sample_length & 0xffff) << 16) + (uint_save_raw_sample_start_index & 0xffff) ); // Sample Length
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG, ((uint_save_raw_sample_length & 0xffff) << 16) + (uint_save_raw_sample_start_index & 0xffff) ); // Sample Length

		/******************************************/
		for (i = 0; i < 6; i++) {
			data = ((gate_length[i] & 0x1ff) << 16) + (gate_start_index[i] & 0xffff);
			sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_ACCUMULATOR_GATE1_CONFIG_REG   + (i * 0x4), data); //
			sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_ACCUMULATOR_GATE1_CONFIG_REG   + (i * 0x4), data); //
			sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_ACCUMULATOR_GATE1_CONFIG_REG  + (i * 0x4), data); //
			sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_ACCUMULATOR_GATE1_CONFIG_REG + (i * 0x4), data); //
		}

		/******************************************/
		internal_fir_trigger_delay = internal_fir_trigger_delay & 0xff ;
		data = (internal_fir_trigger_delay << 24) + (internal_fir_trigger_delay << 16) + (internal_fir_trigger_delay << 8) + (internal_fir_trigger_delay ) ;

		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data ); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data ); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data ); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data ); //

		/******************************************/

		//pre_trigger_delay =  0;
		if (pre_trigger_delay > 16378) {
			pre_trigger_delay  = 16378;
		}
		//pre_trigger_delay = pre_trigger_delay + 0x8000 ; // set "Additional Delay of Fir Trigger P+G" Bit
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //

		/******************************************/

		//pileup ;
		data = ((uint_re_pileup & 0xffff) << 16) + (uint_pileup & 0xffff) ;

		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_PILEUP_CONFIG_REG, data ); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_PILEUP_CONFIG_REG, data ); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_PILEUP_CONFIG_REG, data ); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_PILEUP_CONFIG_REG, data ); //

		/******************************************/
		// External Veto Delay and TI Trigger Function Deadtime Setup
		if (external_veto_gate_delay > 2044) {
			external_veto_gate_delay = 2044;
		}
		data = ((uint_trigger_function_deadtime_length & 0xffff) << 16) + (external_veto_gate_delay & 0xffff); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_EXTERNAL_VETO_GATE_DELAY_REG, data);

		/******************************************/


		//  Event Configuration
		for (i_fpga = 0; i_fpga < 4; i_fpga++) {
			uint_fpag_config_reg_value[i_mod][i_fpga] =
				((uint_channel_external_trigger_enable_flag[i_mod][(i_fpga * 4) + 0] & 1) << 3)
				+ ((uint_channel_external_trigger_enable_flag[i_mod][(i_fpga * 4) + 1] & 1) << 11)
				+ ((uint_channel_external_trigger_enable_flag[i_mod][(i_fpga * 4) + 2] & 1) << 19)
				+ ((uint_channel_external_trigger_enable_flag[i_mod][(i_fpga * 4) + 3] & 1) << 27)

				+ ((uint_channel_external_gate_enable_flag[i_mod][(i_fpga * 4) + 0] & 1) << 6)
				+ ((uint_channel_external_gate_enable_flag[i_mod][(i_fpga * 4) + 1] & 1) << 14)
				+ ((uint_channel_external_gate_enable_flag[i_mod][(i_fpga * 4) + 2] & 1) << 22)
				+ ((uint_channel_external_gate_enable_flag[i_mod][(i_fpga * 4) + 3] & 1) << 30)

				+ ((uint_channel_internal_trigger_enable_flag[i_mod][(i_fpga * 4) + 0] & 1) << 2)
				+ ((uint_channel_internal_trigger_enable_flag[i_mod][(i_fpga * 4) + 1] & 1) << 10)
				+ ((uint_channel_internal_trigger_enable_flag[i_mod][(i_fpga * 4) + 2] & 1) << 18)
				+ ((uint_channel_internal_trigger_enable_flag[i_mod][(i_fpga * 4) + 3] & 1) << 26)

				+ ((uint_channel_polarity_invert_flag[i_mod][(i_fpga * 4) + 0] & 1))
				+ ((uint_channel_polarity_invert_flag[i_mod][(i_fpga * 4) + 1] & 1) << 8)
				+ ((uint_channel_polarity_invert_flag[i_mod][(i_fpga * 4) + 2] & 1) << 16)
				+ ((uint_channel_polarity_invert_flag[i_mod][(i_fpga * 4) + 3] & 1) << 24);
		}
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, uint_fpag_config_reg_value[i_mod][0]); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, uint_fpag_config_reg_value[i_mod][1]); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, uint_fpag_config_reg_value[i_mod][2]); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, uint_fpag_config_reg_value[i_mod][3]); //
		//printf("i_mod = %d:   SIS3316_ADC_CH1_4_EVENT_CONFIG_REG                       = 0x%08x     \n", i_mod, uint_fpag_config_reg_value[i_mod][0]);
		//printf("i_mod = %d:   SIS3316_ADC_CH5_8_EVENT_CONFIG_REG                       = 0x%08x     \n", i_mod, uint_fpag_config_reg_value[i_mod][1]);
		//printf("i_mod = %d:   SIS3316_ADC_CH9_12_EVENT_CONFIG_REG                       = 0x%08x     \n", i_mod, uint_fpag_config_reg_value[i_mod][2]);
		//printf("i_mod = %d:   SIS3316_ADC_CH13_16_EVENT_CONFIG_REG                       = 0x%08x     \n", i_mod, uint_fpag_config_reg_value[i_mod][3]);

		//  Extended Event Configuration
		data = 0 ;
		if(uint_save_raw_sample_first_event_only_mode == 1) {
			data = 0x10101010 ;		// Enable save_raw_sample_first_event_only_mode
		}
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_EXTENDED_EVENT_CONFIG_REG, data); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_EXTENDED_EVENT_CONFIG_REG, data); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_EXTENDED_EVENT_CONFIG_REG, data); //
		sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_EXTENDED_EVENT_CONFIG_REG, data); //



		/******************************************/
		if (maw_test_buffer_enable_flag == 0) {
			maw_test_buffer_length =  0 ;
		}

		// data format
		header_length = 3;
		header_accu_2_values_offset = 2 ;
		header_maw_3_values_offset  = 2 ;

		data = 0 ;
		if (header_accu_6_values_enable_flag == 1) {
			header_length = header_length + 7 ;
			header_maw_3_values_offset  = header_maw_3_values_offset + 7 ;
			header_accu_2_values_offset  = header_accu_2_values_offset + 7 ;
			data = data + 0x1 ; // set bit 0
		}
		if (header_accu_2_values_enable_flag == 1) {
			header_length = header_length + 2 ;
			header_maw_3_values_offset  = header_maw_3_values_offset + 2 ;
			data = data + 0x2 ; // set bit 1
		}
		if (header_maw_3_values_enable_flag == 1) {
			header_length = header_length + 3 ;
			data = data + 0x4 ; // set bit 2
		}
		if (maw_test_buffer_enable_flag == 1) {
			data = data + 0x10 ; // set bit 4
		}

		data = data + (data << 8) + (data << 16) + (data << 24);
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG, data);
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_DATAFORMAT_CONFIG_REG, data);
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_DATAFORMAT_CONFIG_REG, data);
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_DATAFORMAT_CONFIG_REG, data);

		event_length = (header_length + (uint_save_raw_sample_length / 2) + maw_test_buffer_length);




		// MAW Test Buffer configuration
		data = maw_test_buffer_length + (maw_test_buffer_delay << 16);
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_MAW_TEST_BUFFER_CONFIG_REG, data);
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_MAW_TEST_BUFFER_CONFIG_REG, data);
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_MAW_TEST_BUFFER_CONFIG_REG, data);
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_MAW_TEST_BUFFER_CONFIG_REG, data);

		if(uint_save_raw_sample_first_event_only_mode == 0) {
			address_threshold = (nof_events_per_bank * event_length) - 1 ;  //
		}
		else {
			address_threshold = event_length + ((nof_events_per_bank - 1) * header_length) - 1 ;  //
		}

		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG, address_threshold); //
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_ADDRESS_THRESHOLD_REG, address_threshold); //
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_ADDRESS_THRESHOLD_REG, address_threshold); //
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_ADDRESS_THRESHOLD_REG, address_threshold); //

		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1 ); //  update and freeze with each bank switching
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1); //
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1 ); //
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1 ); //


		// Led Application mode
		data = 0x70 ;
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_CONTROL_STATUS, data ); //

	} // loop over modules
	/******************************************************************************************/


	/*  System TI (External Trigger / External Gate)  Configuration */

	if(uint_acquisition_trigger_mode == 0) {// 0: TI External Trigger
		// only on 1. module called FP_BUS Master
		// Enable LEMO Input "TI" as External Trigger function
		data = 0x10; // Enable Nim Input "TI" as Trigger function
		//data = data + 0x40; // set Level sensitiv
		if(uint_trigger_function_deadtime_logic_enable == 1) {
			data = data + 0x4000; // enable TI Deadtime logic
		}
		i_mod=0;
		sis3316_adc_array[i_mod]->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data); //


		// Enable external trigger, Timestamp clear and Sample Control from FP-Bus
		data = 0  ; //
		data = data + 0x10 ; // enable "FP-Bus-In Control 1 as Trigger"
		data = data + 0x40 ; // enable "FP-Bus-In Timestamp clear function"
		data = data + 0x80 ; // enable "FP-Bus-In Sample Control"
		//data = data + 0x100 ; // enable "external Trigger function" (NIM In, if enabled and VME key write)
		//data = data + 0x400 ; // enable "external Timestamp clear function" (NIM In, if enabled and VME key write)
		//data = data + 0x8000 ; // enable "external_trigger_function_disable_if_busy"
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			sis3316_adc_array[i_mod]->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, data) ;
		}

	}
	else { // 1: TI External Gate/Internal Trigger (SETUP_TI_AS_Gate)
		// only on 1. module called FP_BUS Master
		// Enable LEMO Input "TI" as Local Veto function
		data = 0x10; // Enable Nim Input "TI" as Trigger function
		data = data + 0x40; // set Level sensitiv
		if(uint_trigger_function_deadtime_logic_enable == 1) {
			data = data + 0x4000; // enable TI Deadtime logic
		}
		i_mod=0;
		sis3316_adc_array[i_mod]->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data); //

		// enable external (global) functions
		data = 0  ; //
		data = data + 0x20 ; // enable "FP-Bus-In Control 1 as Veto"
		data = data + 0x40 ; // enable "FP-Bus-In Timestamp clear function"
		data = data + 0x80 ; // enable "FP-Bus-In Sample Control"
		//data = 0x200; // enable "External Trigger function" as Veto
		//data = data + 0x400; // enable "External Timestamp-Clear function" as Timestamp-Clear
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			sis3316_adc_array[i_mod]->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, data);
		}
	}



	/******************************************************************************************/



/******************************************/
	if(uint_save_raw_sample_first_event_only_mode == 0) {
		event_length_for_file_write_header = event_length & 0xffffff;		//
	}
	else {
		event_length_for_file_write_header = ((header_length & 0xff) << 24) + (event_length & 0xffffff);		// Enable save_raw_sample_first_event_only_mode
	}

	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		for (i_ch=0; i_ch<16; i_ch++) {
			uint_channel_event_counter[i_mod] [i_ch]  = 0;
		}
	}

	loop_counter        = 0;
	bank_buffer_counter = 0 ;
	if (sis3316_adc_array[0]->adc_125MHz_flag == 0) {
		file_header_indentifier = 0 ; // 250MHz
	}
	else {
		file_header_indentifier = 1 ;
	}

	file_header_reserved = 0 ;
	file_header_reserved = file_header_reserved + (uint_acquisition_trigger_mode & 0xf) ;
	file_header_reserved = file_header_reserved + ((uint_save_raw_sample_first_event_only_mode & 0x1) < 4);

	ch_event_counter       = 0;

	// Clear Timestamp on first module --> will clear via FB-Bus Timestamps on all modules */
	sis3316_adc_array[0]->register_write(SIS3316_KEY_TIMESTAMP_CLEAR, 0) ;
	//printf("SIS3316_KEY_TIMESTAMP_CLEAR \n");
	//usleep(500000);

	// Start Readout Loop
	//Note: Start sampling on Bank on alternate Bank, check Bit 24 in the register "previous Bank sample address"
	sis3316_adc_array[0]->register_read( SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);
	if((data & 0x1000000) == 0x1000000 ) { 	// bank2 flag is set ?
		//printf("bank2 flag is set\n"); // start sampling an alternate bank
		bank1_armed_flag = 1 ;
		return_code = sis3316_adc_array[0]->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK1, 0 ); //    Arm
		printf("Start SIS3316_KEY_DISARM_AND_ARM_BANK1 \n");
	}
	else {
		//printf("bank2 flag is not set\n"); // start sampling an alternate bank
		bank1_armed_flag = 0 ;
		return_code = sis3316_adc_array[0]->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK2, 0 ); // //  Arm
		printf("Start SIS3316_KEY_DISARM_AND_ARM_BANK2 \n");
	}



#ifdef only_test
	do {
		printf("\n");


		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
				return_code = sis3316_adc_array[i_mod]->register_read(SIS3316_INTERNAL_TEMPERATURE_REG, &data);
			signed short signed_short_temperature ;
			float float_temperature_c, float_temperature_f ;
			signed_short_temperature =  ((signed short) (data&0xffff) ) ;
			float_temperature_c =  (float) (signed_short_temperature) / 4.0 ;
			float_temperature_f =  32.0 + (float_temperature_c * 1.8) ;
			printf("Temperature    i_mod = %d   = %2.2f C    %3.2f F \n", i_mod, float_temperature_c, float_temperature_f );
			usleep(50000);
		}
		printf("\n");

	} while(1) ;
#endif




	do {
		/*********************************************/
		// prepare file write
		if (uint_WriteData_to_File_EnableFlag == 1) {   ; //
			if (uint_WriteData_to_File_OpenFlag == 0) {   // first time
				if (uint_WriteData_to_MultipleFiles_Flag == 0) {   //
					sprintf(char_WriteData_to_File_filename,"%s.dat",char_WriteData_to_File_initialize_filename ) ;
				}
				else {
					sprintf(char_WriteData_to_File_filename,"%s_%d.dat",char_WriteData_to_File_initialize_filename ,  uint_WriteData_to_File_counter) ;
				}
				file_WriteData_to_File_Pointer = fopen(char_WriteData_to_File_filename,"wb") ;
				uint_WriteData_to_File_OpenFlag = 1 ;
			}
			else { // file is open
				if (uint_WriteData_to_MultipleFiles_Flag == 1) {   //
					if (uint_WritenData_to_File_32bit_words > WRITE_DTATA_TO_FILE_MAX_32BIT_WORDS) {
						uint_WritenData_to_File_32bit_words = 0 ;
						fclose(file_WriteData_to_File_Pointer);
						uint_WriteData_to_File_counter++;
						sprintf(char_WriteData_to_File_filename,"%s_%d.dat",char_WriteData_to_File_initialize_filename ,  uint_WriteData_to_File_counter) ;
						file_WriteData_to_File_Pointer = fopen(char_WriteData_to_File_filename,"wb") ;
					}
				}
			}
		}
		/*********************************************/

		poll_counter = 0 ;
		do {
			poll_counter++;
			if (poll_counter == 100) {
				usleep(100); //
				#ifdef CERN_ROOT_PLOT
					gSystem->ProcessEvents();  // handle GUI events
				#endif
				poll_counter = 0 ;
    		}
			sis3316_adc_array[0]->register_read ( SIS3316_ACQUISITION_CONTROL_STATUS, &data);  // poll on FP_BUS Master SIS3316
#ifdef WINDOWS
		} while (((data & 0x200000) == 0x0) && (gl_stopReq == FALSE))  ; // FP-Bus Address Threshold reached ?
		if (gl_stopReq == TRUE) { break ; }
#else
		//} while (((data & 0x200000) == 0x0) && (gl_stopReq == FALSE))  ; // FP-Bus Address Threshold reached ?
		//if (gl_stopReq == TRUE) { break ; }
		} while (((data & 0x200000) == 0x0) )  ; // FP-Bus Address Threshold reached ?
#endif

		if (bank1_armed_flag == 1) {
			return_code = sis3316_adc_array[0]->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK2 , 0);  //  Arm Bank2
			bank1_armed_flag = 0; // bank 2 is armed
			printf("SIS3316_KEY_DISARM_AND_ARM_BANK2 \n");
			//	usleep(1000000); //
		}
		else {
			return_code = sis3316_adc_array[0]->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK1 , 0);  //  Arm Bank1
			bank1_armed_flag = 1; // bank 1 is armed
			printf("SIS3316_KEY_DISARM_AND_ARM_BANK1 \n");
			//	usleep(1000000); //
		}
//#ifdef TEST_DEBUG_PRINTS
		
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			//printf("\n");
			//sis3316_adc_array[i_mod]->register_read ( SIS3316_FP_LVDS_BUS_CONTROL, &data);
			//printf("i_mod = %d:   SIS3316_FP_LVDS_BUS_CONTROL                     = 0x%08x     \n", i_mod, data);
			//sis3316_adc_array[0]->register_read ( SIS3316_ACQUISITION_CONTROL_STATUS, &data);
			//printf("i_mod = %d:   SIS3316_ACQUISITION_CONTROL_STATUS = 0x%08x     \n", i_mod, data);
			sis3316_adc_array[i_mod]->register_read ( SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);  
			printf("i_mod = %d:   SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG = 0x%08x     \n", i_mod, data);
			//sis3316_adc_array[i_mod]->register_read ( SIS3316_ADC_CH2_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);
			//printf("i_mod = %d:   SIS3316_ADC_CH2_PREVIOUS_BANK_SAMPLE_ADDRESS_REG = 0x%08x     \n", i_mod, data);
			//sis3316_adc_array[i_mod]->register_read ( SIS3316_ADC_CH3_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);
			//printf("i_mod = %d:   SIS3316_ADC_CH3_PREVIOUS_BANK_SAMPLE_ADDRESS_REG = 0x%08x     \n", i_mod, data);
			//sis3316_adc_array[i_mod]->register_read ( SIS3316_ADC_CH4_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);
			//printf("i_mod = %d:   SIS3316_ADC_CH4_PREVIOUS_BANK_SAMPLE_ADDRESS_REG = 0x%08x     \n", i_mod, data);

				//sis3316_adc_array[i_mod]->register_read ( SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, &data);
				//printf("i_mod = %d:   SIS3316_ADC_CH1_4_EVENT_CONFIG_REG                       = 0x%08x     \n", i_mod, data);
				//sis3316_adc_array[i_mod]->register_read ( SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, &data);
				//printf("i_mod = %d:   SIS3316_ADC_CH5_8_EVENT_CONFIG_REG                       = 0x%08x     \n", i_mod, data);
				//sis3316_adc_array[i_mod]->register_read ( SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, &data);
				//printf("i_mod = %d:   SIS3316_ADC_CH9_12_EVENT_CONFIG_REG                      = 0x%08x     \n", i_mod, data);
				//sis3316_adc_array[i_mod]->register_read ( SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, &data);
				//printf("i_mod = %d:   SIS3316_ADC_CH13_16_EVENT_CONFIG_REG                     = 0x%08x     \n", i_mod, data);


		}
		printf("\n");

//#endif

#define SEQUENTIAL_READOUT
#ifdef SEQUENTIAL_READOUT

		#ifdef CERN_ROOT_PLOT
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
 			graph_raw[i_mod]->sis3316_draw_XYaxis (uint_save_raw_sample_length); // clear and draw X/Y
			gSystem->ProcessEvents();  // handle GUI events
		}
		#endif

		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			for (i_ch=0; i_ch<16; i_ch++) {
				return_code = sis3316_adc_array[i_mod]->read_DMA_Channel_PreviousBankDataBuffer(bank1_armed_flag /*bank2_read_flag*/, i_ch /* 0 to 15 */, max_req_nof_32bit_words, &got_nof_32bit_words, dma_read_buffer ) ; // read maximun (all) events
				//printf("read_DMA_Channel_PreviousBankDataBuffer: i_ch %d  got_nof_32bit_words = 0x%08x  \n",i_ch,  got_nof_32bit_words);
				if (return_code != 0) {
					printf("read_DMA_Channel_PreviousBankDataBuffer: return_code = 0x%08x\n", return_code);
					gl_stopReq = TRUE;
				}
				if(uint_save_raw_sample_first_event_only_mode == 0) {
					ch_event_counter = (got_nof_32bit_words  / event_length) ;
				}
				else {
					if(got_nof_32bit_words < event_length) {
						ch_event_counter = 0 ;
					}
					else {
						ch_event_counter = 1 + ((got_nof_32bit_words - event_length)  / header_length) ;
					}
				}
				//printf("got_nof_32bit_words %d   event_length = %d   header_length = %d   ch_event_counter = %d \n", got_nof_32bit_words, event_length, header_length,ch_event_counter);
				uint_channel_event_counter[i_mod][i_ch]  = uint_channel_event_counter[i_mod][i_ch] + ch_event_counter;

				if (ch_event_counter > 0) {
					// file write
					if (uint_WriteData_to_File_OpenFlag == 1) {   ; //
						uint_WritenData_to_File_32bit_words += SIS3316_WriteBankChannelHeaderToDataFile (file_WriteData_to_File_Pointer, file_header_indentifier, bank_buffer_counter, i_ch, ch_event_counter , event_length_for_file_write_header, maw_test_buffer_length, file_header_reserved) ;
						uint_WritenData_to_File_32bit_words += SIS3316_WriteBankChannelEventBufferToDataFile (file_WriteData_to_File_Pointer, dma_read_buffer, got_nof_32bit_words)  ;
					}
					// draw graph
					#ifdef CERN_ROOT_PLOT
					//for (i_event=0; i_event<ch_event_counter; i_event++) {
					//if (i_event==0) { // plot ony 1. event
							i_event=0 ; // plot ony 1. event
							graph_raw[i_mod]->sis3316_draw_chN (uint_save_raw_sample_length, &dma_read_buffer[i_event*(event_length) + header_length], (i_mod*16) + i_ch); //
							gSystem->ProcessEvents();  // handle GUI events
						//}
					//}
				}
				#endif
			}
		}



#endif

#ifdef TEMPERATURE
		printf("test\n");
			usleep(1000000); //

		printf("\n");
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
 			return_code = sis3316_adc_array[i_mod]->register_read(SIS3316_INTERNAL_TEMPERATURE_REG, &data);
			signed short signed_short_temperature ;
			float float_temperature_c, float_temperature_f ;
			signed_short_temperature =  ((signed short) (data&0xffff) ) ;
			float_temperature_c =  (float) (signed_short_temperature) / 4.0 ;
			float_temperature_f =  32.0 + (float_temperature_c * 1.8) ;
			printf("Temperature    i_mod = %d   = %2.2f C    %3.2f F \n", i_mod, float_temperature_c, float_temperature_f );
		}
		printf("\n");
#endif
		loop_counter++;
		bank_buffer_counter++;

#ifdef WINDOWS
		} while(((loop_counter < stop_after_loop_counts) || (stop_after_loop_counts == 0)) && (gl_stopReq == FALSE) );
#else
		} while(((loop_counter < stop_after_loop_counts) || (stop_after_loop_counts == 0))  );
#endif



		printf("\n");


		// close file (if it was open)
	 	if (uint_WriteData_to_File_OpenFlag == 1) {   ; //
			fclose(file_WriteData_to_File_Pointer);
			uint_WriteData_to_File_OpenFlag = 0 ;
		}
	//---------------------------------------------------------------------------

		printf("bank_buffer_counter = %d     \n",bank_buffer_counter);
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			for (i_ch=0; i_ch<16; i_ch++) {
				if (uint_channel_event_counter[i_mod][i_ch] != 0) {
					printf("module %d    \tch %d:    \tevent counter  = %d  \t(0x%08x)      \n", i_mod, i_ch+1, uint_channel_event_counter[i_mod][i_ch], uint_channel_event_counter[i_mod][i_ch]);
				}
			}
		}


//***************************************************************************************************************
		// prepare statistic file write
		if (uint_WriteData_to_File_EnableFlag == 1) {   ; //
			sprintf(char_WriteData_to_File_filename,"%s_Statistic_Counter.dat",char_WriteData_to_File_initialize_filename ) ;
			file_WriteData_to_File_Pointer = fopen(char_WriteData_to_File_filename,"wb") ;
			uint_WritenData_to_File_32bit_words = 0 ;
			uint_WritenData_to_File_32bit_words += SIS3316_WriteStatisticCounterHeaderToDataFile (file_WriteData_to_File_Pointer, MAX_NOF_SIS3316_ADCS, bank_buffer_counter, 0) ;
		}


		printf("\n");
		// Statistic Counters
		unsigned int uint_statistic_buffer[24];
		printf("     \t All          Hits/Events  Deadtime     Pileup       Veto         High Energy supressed \n");
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			for (i_fpga = 0; i_fpga < 4; i_fpga++) {
				sis3316_adc_array[i_mod]->read_Channel_StatisticCounter(i_fpga /* 0 to 3 */, uint_statistic_buffer); // new 27.08.2019
				if (uint_WriteData_to_File_EnableFlag == 1) {   ; //
					uint_WritenData_to_File_32bit_words += SIS3316_WriteStatisticCounterToDataFile (file_WriteData_to_File_Pointer, uint_statistic_buffer, 24)  ;
				}

				for (i_ch = 0; i_ch < 4; i_ch++) {
					printf("ch%d \t", (i_fpga * 4) + i_ch + 1);
					for (i = 0; i < 6; i++) {
						printf(" 0x%08x  ", uint_statistic_buffer[(i_ch * 6) + i]);
					}
					printf("\n");
				}
			}
		}

	 	if (uint_WriteData_to_File_EnableFlag == 1) {   ; //
			fclose(file_WriteData_to_File_Pointer);
		}
	//***************************************************************************************************************

		//Disarm all modules
		//sis3316_adc_array[0]->register_write(SIS3316_KEY_DISARM, 0);
			for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
				sis3316_adc_array[i_mod]->register_write(SIS3316_KEY_DISARM, 0);
				//sis3316_adc_array[i_mod]->reset_adc_fpga_sample_clock_PLL() ;
			}


		printf("\n");
		printf("sampling finished   \n");
		printf("\n");

		gl_stopReq = FALSE;
		do {
			gSystem->ProcessEvents();  // handle GUI events
		} while((gl_stopReq == FALSE) );


//program_stop_and_wait();
	return 0;
}




void program_stop_and_wait(void)
{
	gl_stopReq = FALSE;
	printf( "\n\nProgram stopped");
	printf( "\n\nEnter ctrl C");
	do {
		usleep(1000);
	} while (gl_stopReq == FALSE) ;

	//		result = scanf( "%s", line_in );
}

#ifdef not_used
void program_stop_and_wait(void)
{
#ifdef WINDOWS
	gl_stopReq = FALSE;
#endif
	printf( "\n\nProgram stopped");
	printf( "\n\nEnter ctrl C");
	//SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, FALSE ) ;
	do {
		#ifdef CERN_ROOT_PLOT
			gSystem->ProcessEvents();  // handle GUI events
		#else
			usleep(1000); //   
		#endif
		//printf("program_stop_and_wait : gl_stopReq    = %d     \n", gl_stopReq);
#ifdef WINDOWS
	} while (gl_stopReq == FALSE) ;
#else
} while (1) ;
#endif
	//		result = scanf( "%s", line_in );
}

#endif



/***********************************************************************************************************************************************/

#define FILE_FORMAT_EVENT_HEADER        	0xDEADBEEF

int SIS3316_WriteBankChannelHeaderToDataFile (FILE *file_data_ptr, unsigned int indentifier, unsigned int bank_loop_no, unsigned int channel_no, unsigned int nof_events, unsigned int event_length, unsigned int maw_length, unsigned int reserved)
{
int written ;
int data ;
  //header
	data = FILE_FORMAT_EVENT_HEADER ;
    written=fwrite(&data,0x4,0x1, file_data_ptr); // write one  uint word
    written+=fwrite(&indentifier,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&bank_loop_no,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&channel_no,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&nof_events,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&event_length,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&maw_length,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&reserved,0x4,0x1,file_data_ptr); // write one  uint word
 	return written;

}

//---------------------------------------------------------------------------
int SIS3316_WriteBankChannelEventBufferToDataFile (FILE *file_data_ptr, unsigned int* memory_data_array, unsigned int nof_write_length_lwords)
{
int nof_write_elements ;
int written ;
		nof_write_elements = nof_write_length_lwords ;
		written=fwrite(memory_data_array,0x4, nof_write_elements, file_data_ptr); //
		//gl_uint_DataEvent_LWordCounter = gl_uint_DataEvent_LWordCounter + written  ;
		if(nof_write_elements != written) {
    		printf ("Data File Write Error in  WriteEventToDataFile()  \n");
 		 }

 	return written;

}

/***************************************************/
#define FILE_FORMAT_STATISTIC_HEADER        	0xDEADAFFE

int SIS3316_WriteStatisticCounterHeaderToDataFile (FILE *file_data_ptr, unsigned int nof_modules, unsigned int bank_loop_no, unsigned int reserved)
{
int written ;
int data ;
  //header
	data = FILE_FORMAT_STATISTIC_HEADER ;
    written=fwrite(&data,0x4,0x1, file_data_ptr); // write one  uint word
    written+=fwrite(&nof_modules,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&bank_loop_no,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&reserved,0x4,0x1,file_data_ptr); // write one  uint word
 	return written;

}

int SIS3316_WriteStatisticCounterToDataFile (FILE *file_data_ptr, unsigned int* memory_data_array, unsigned int nof_write_length_lwords)
{
int nof_write_elements ;
int written ;
		nof_write_elements = nof_write_length_lwords ;
		written=fwrite(memory_data_array,0x4, nof_write_elements, file_data_ptr); //
		//gl_uint_DataEvent_LWordCounter = gl_uint_DataEvent_LWordCounter + written  ;
		if(nof_write_elements != written) {
    		printf ("Data File Write Error in  WriteEventToDataFile()  \n");
 		 }

 	return written;

}


/***************************************************/
#ifdef WINDOWS


BOOL CtrlHandler( DWORD ctrlType ){
		printf( "\n\nCTRL-C pressed. finishing current task \n\n");
	switch( ctrlType ){
	case CTRL_C_EVENT:
		printf( "\n\nCTRL-C pressed. finishing current task \n\n");
		gl_stopReq = TRUE;
		//printf("CtrlHandler : gl_stopReq    = %d     \n", gl_stopReq);
		return( TRUE );
		break;
	default:
		printf( "\n\ndefault pressed. \n\n");
		return( FALSE );
		break;
	}
}
#endif
