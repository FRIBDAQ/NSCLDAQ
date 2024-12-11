/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_InternalClk_appl_oneModule.cpp                       */
/*                                                                         */
/*  Function: Acquisition with External Trigger  or                        */
/*            Internal Trigger with External Gate                          */
/*            - one SIS3316 with Internal Sample Clk                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 17.09.2019                                       */
/*  last modification:    25.07.2024    (SIS3316-2 adaptation)             */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*                                                                         */
/*  used Root Version: 6.32/02                                             */
/*                                                                         */
/*                                                                         */
/* ----------------------------------------------------------------------- */
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

#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethnernet UDP)



#define CERN_ROOT_PLOT

#ifdef CERN_ROOT_PLOT
#include "rootIncludes.h"   //

	#ifdef WINDOWS
		#pragma comment (lib, "libRio")
	   #pragma comment (lib, "libcore")
	   #pragma comment (lib, "libHist")
	   #pragma comment (lib, "libTree")
	   #pragma comment (lib, "libgpad")
	// #pragma comment (lib, "libcint")
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
#include <ctime>
#include <sys/time.h>

typedef int BOOL ;
#define TRUE  1
#define FALSE 0

#endif


#ifdef WINDOWS

	using namespace std;
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winsock2.h>

#include <tchar.h>



	#include <stdio.h>
#include <iostream>
	#include <iomanip>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
//#include <time.h>
#endif

/******************************************************************************************************/


#ifdef PCI_VME_INTERFACE
	#include "sis1100w_vme_class.h"
	sis1100 *gl_vme_crate ;
#endif

#ifdef USB_VME_INTERFACE
	#include "sis3150w_vme_class.h"
	sis3150 *gl_vme_crate ;
#endif




#ifdef USB3_VME_INTERFACE
	#include "sis3153w_vme_class.h"
	sis3153 *gl_vme_crate ;
#endif

#ifdef ETHERNET_UDP_INTERFACE
	#include "sis3316_ethernet_access_class.h"
	sis3316_eth *gl_vme_crate ;

	#ifdef LINUX
		#include <sys/types.h>
		#include <sys/socket.h>
	#endif

	#ifdef WINDOWS
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

/******************************************************************************************************/


#define MAX_NOF_SIS3316_ADCS			1
#define FIRST_MODULE_BASE_ADDR			0x00000000 // upper Channel ID
#define MODULE_BASE_OFFSET				0x01000000 // following upper Channel ID



#include "sis3316_class.h"


#include "sis3316.h"
#include "get_configuration_parameter_appl.h"

/******************************************************************************************************/

#ifdef CERN_ROOT_PLOT

#include "sis3316_cern_root_class.h"
sis_root_graph *gl_graph_raw ;


unsigned int gl_graph_zoom_raw_draw_length ;
unsigned int gl_graph_zoom_raw_draw_bin_offset ;

#endif
/******************************************************************************************************/


/*===========================================================================*/
/* Globals					  			     */
/*===========================================================================*/
#define MAX_NUMBER_LWORDS_64MBYTE			0x1000000       /* 64MByte */
//#define MAX_NUMBER_LWORDS_64MBYTE			0x1000000       /* 64MByte */

unsigned int gl_rblt_data[MAX_NUMBER_LWORDS_64MBYTE] ;

BOOL gl_stopReq = FALSE;
FILE *gl_FILE_DataEvenFilePointer           ;


char gl_cmd_ip_string[64];
sis3316_get_configuration_parameters *gl_sis3316_get_configuration_parameters ;

/*===========================================================================*/
/* Prototypes			                               		  			     */
/*===========================================================================*/
int SIS3316_WriteBankChannelHeaderToDataFile (FILE *file_data_ptr, unsigned int indentifier, unsigned int bank_loop_no, unsigned int channel_no, unsigned int nof_events, unsigned int event_length, unsigned int maw_length, unsigned int reserved);
int SIS3316_WriteBankChannelEventBufferToDataFile (FILE *file_data_ptr, unsigned int* memory_data_array, unsigned int nof_write_length_lwords);
//int WriteBufferHeaderCounterNofChannelToDataFile (unsigned int buffer_no,unsigned int nof_events, unsigned int event_length);
//int WriteEventsToDataFile (unsigned int* memory_data_array, unsigned int nof_write_length_lwords);


void program_stop_and_wait(void);

#ifdef WINDOWS
BOOL CtrlHandler( DWORD ctrlType );
void usleep(unsigned int uint_usec) ;
#endif



//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{

CHAR char_messages[128];
UINT nof_found_devices ;

int return_code ;
unsigned int first_mod_base, nof_modules   ;
unsigned int module_base_addr   ;
unsigned int data;
unsigned int i, module_index;
unsigned int loop_counter;
unsigned int bank_buffer_counter ;

unsigned int uint_channel_event_counter[16];



unsigned int got_nof_32bit_words;
unsigned int uint_save_raw_sample_length;
unsigned int uint_save_raw_sample_start_index;
unsigned int uint_save_raw_sample_first_event_only_mode;
unsigned int trigger_gate_window_length;
unsigned int address_threshold;

unsigned int external_veto_gate_delay;

unsigned int uint_trigger_function_deadtime_block_with_addrThres_enable;
unsigned int uint_trigger_function_deadtime_logic_enable;
unsigned int uint_trigger_function_deadtime_length;




unsigned int nof_events_per_bank;
unsigned int pre_trigger_delay ;
unsigned int bank1_armed_flag ;
unsigned int poll_counter ;
unsigned int i_adc;
unsigned int sis3316_not_OK;

unsigned int ch_event_counter;

unsigned int event_length;
unsigned int header_length;
//unsigned int maw_length;

unsigned int iob_delay_value ;
//unsigned int TO_pulse_out_length;
//unsigned int p_val;
//unsigned int g_val ;
//unsigned int trigger_threshold_value ;

unsigned int uint_pileup ;
unsigned int uint_re_pileup ;
unsigned int internal_fir_trigger_delay ;

unsigned int i_adc_fpga ;
unsigned int i_ch ;
unsigned int i_fpga;

unsigned int maw_test_buffer_length ;
unsigned int maw_test_buffer_delay ;

unsigned int header_accu_6_values_enable_flag ;
unsigned int header_accu_2_values_enable_flag ;
unsigned int header_maw_3_values_enable_flag ;
unsigned int maw_test_buffer_enable_flag ;

unsigned int header_maw_3_values_offset ;
unsigned int header_accu_2_values_offset ;


//unsigned int i_gate;
unsigned int gate_length[8];
unsigned int gate_start_index[8];

//unsigned int uint_DataEvent_OpenFlag ;

unsigned int stop_after_loop_counts ;


unsigned int uint_lemo_out_CO_select;
unsigned int uint_lemo_out_TO_select;
unsigned int uint_lemo_out_UO_select;

unsigned int uint_channel_trigger_generation_setup_reg[16];
unsigned int uint_channel_trigger_generation_threshold_reg[16];
unsigned int uint_channel_he_trigger_generation_threshold_reg[16];

unsigned int uint_trigger_pulse_length;
unsigned int uint_trigger_gap;
unsigned int uint_trigger_peaking;
unsigned int uint_trigger_threshold_value;

unsigned int uint_channel_polarity_invert_flag[16];
unsigned int uint_channel_range_2V_flag[16];
unsigned int uint_channel_50ohm_termination_disable_flag[16];
unsigned int uint_fpga_analog_ctrl_val[4];
unsigned int uint_channel_analog_offset_dac_val[16];



unsigned int uint_acquisition_trigger_mode ;  // 0: TI External Trigger   	: TI External Gate/Internal Trigger

unsigned int uint_channel_external_gate_enable_flag[16];
unsigned int uint_channel_external_trigger_enable_flag[16];
unsigned int uint_channel_internal_trigger_enable_flag[16];
unsigned int uint_fpag_config_reg_value[4];

unsigned int uint_udp_jumbo_mode;
unsigned int uint_udp_nofPacketsPerRequest;


// command line interpreter
	int int_ch ;
	char ch_string[1024] ;
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
	// Ethernet UDP IP address
	strcpy(gl_cmd_ip_string, "212.60.16.200"); // SIS3316 IP address
	strcpy(gl_cmd_ip_string, "sis3316-0373"); // SIS3316 IP address
	strcpy(gl_cmd_ip_string, "192.168.1.100"); // SIS3316 IP address

	// Ethernet UDP transfer options/mode
	//uint_udp_jumbo_mode = 0; // disable Jumbo mode -> max. transfer block size 1440 bytes
	uint_udp_jumbo_mode = 1; // enable Jumbo mode  -> max. transfer block size 8000 bytes -> set MTU to 9000 !

	//uint_udp_nofPacketsPerRequest = 1 ;
	//uint_udp_nofPacketsPerRequest = 5 ;
	//uint_udp_nofPacketsPerRequest = 10 ;
	uint_udp_nofPacketsPerRequest = 20 ;
	//uint_udp_nofPacketsPerRequest = UDP_MAX_PACKETS_PER_REQUEST ;

	// VME module base address
	module_base_addr = FIRST_MODULE_BASE_ADDR ;


	//stop_after_loop_counts       = 10;    	// 0: endless
	stop_after_loop_counts       = 4;    	// 0: endless
	//nof_events_per_bank                   = 1000 ;    // each Bank
	nof_events_per_bank                   = 10000;    // each Bank



// default Configuration parameters
	// Set Gain/Termination
	for (i_ch = 0; i_ch < 16; i_ch++) {
		uint_channel_range_2V_flag[i_ch] = 1;  // 2 V Range
		//uint_channel_range_2V_flag[i_ch] = 0;  // 5 V Range
		//uint_channel_50ohm_termination_disable_flag[i_ch] = 1; // disable 50 Ohm Termination
		uint_channel_50ohm_termination_disable_flag[i_ch] = 0; // enable 50 Ohm Termination
	}

	for (i_fpga = 0; i_fpga < 4; i_fpga++) {
		uint_fpga_analog_ctrl_val[i_fpga] = (uint_channel_range_2V_flag[(i_fpga * 4) + 0] & 1)
			+ ((uint_channel_range_2V_flag[(i_fpga * 4) + 1] & 1) << 8)
			+ ((uint_channel_range_2V_flag[(i_fpga * 4) + 2] & 1) << 16)
			+ ((uint_channel_range_2V_flag[(i_fpga * 4) + 3] & 1) << 24)
			+ ((uint_channel_50ohm_termination_disable_flag[(i_fpga * 4) + 0] & 1) << 2)
			+ ((uint_channel_50ohm_termination_disable_flag[(i_fpga * 4) + 1] & 1) << 10)
			+ ((uint_channel_50ohm_termination_disable_flag[(i_fpga * 4) + 2] & 1) << 18)
			+ ((uint_channel_50ohm_termination_disable_flag[(i_fpga * 4) + 3] & 1) << 26);
	}


//set ADC offsets (DAC) :
	for (i_ch = 0; i_ch < 16; i_ch++) {
		//uint_channel_analog_offset_dac_val[i_ch] = 0x8000; // middle
		uint_channel_analog_offset_dac_val[i_ch] = 0x3000; //  0V at 700 counts
	}
	//uint_channel_analog_offset_dac_val[0] = 20000; //  0V at ~5300
	//uint_channel_analog_offset_dac_val[1] = 11000; //  0V at ~3100
	//uint_channel_analog_offset_dac_val[2] = 1000; //  0V at ~700

// channel polarity
	for (i_ch = 0; i_ch < 16; i_ch++) {
		uint_channel_polarity_invert_flag[i_ch] = 1; // all inverted
	}
	//uint_channel_polarity_invert_flag[1] = 0; // ch2 not inverted


// Event Configuration Register parameters

	uint_acquisition_trigger_mode = 1 ;  // 0: TI External Trigger   	1: TI External Gate/Internal Trigger

	if(uint_acquisition_trigger_mode == 0) {// 0: TI External Trigger
		// channel external gate enable(1)/disable(0)
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_external_gate_enable_flag[i_ch] = 0; // all disable
		}

		// channel external trigger enable(1)/disable(0)
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_external_trigger_enable_flag[i_ch] = 1; // all enable
		}

		// channel internal trigger enable(1)/disable(0)
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_internal_trigger_enable_flag[i_ch] = 0; // all disable
		}

	}
	else { // 1: TI External Gate/Internal Trigger
		// channel external gate enable(1)/disable(0)
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_external_gate_enable_flag[i_ch] = 1; // all enable
		}

		// channel external trigger enable(1)/disable(0)
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_external_trigger_enable_flag[i_ch] = 0; // all disable
		}

		// channel internal trigger enable(1)/disable(0)
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_internal_trigger_enable_flag[i_ch] = 1; // all enable
		}
	}



// internal Trigger generation
	uint_trigger_pulse_length    = 0x10;
	uint_trigger_gap             = 8;
	uint_trigger_peaking         = 10;
	uint_trigger_threshold_value = 100 * uint_trigger_peaking; // 2V Range -> 1 bin = 122 uV
															   // 100 --> 2V Range --> 12.2 mV

	for (i_ch = 0; i_ch < 16; i_ch++) {
		uint_channel_trigger_generation_setup_reg[i_ch] = ((uint_trigger_pulse_length & 0xff) << 24)
														+ ((uint_trigger_gap & 0xfff) << 12)
														+ ((uint_trigger_peaking & 0xfff));
		uint_channel_trigger_generation_threshold_reg[i_ch] = 0x80000000 + 0x30000000  //
															+ 0x8000000  //  MAW offset
															+ (uint_trigger_threshold_value & 0x7FFFFFF) ; //  Threshold
		uint_channel_he_trigger_generation_threshold_reg[i_ch] = 0x0; //
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
	trigger_gate_window_length = 650;
	uint_save_raw_sample_length = 624;
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
		uint_trigger_function_deadtime_length = uint_trigger_function_deadtime_length + 0x4000; // Enable Deadtime with internal Address Threshold Flag
		//uint_trigger_function_deadtime_length = uint_trigger_function_deadtime_length + 0x8000; // Enable Deadtime with FP-Bus Address Threshold Flag
	}


// Lemo Out definition
	//uint_lemo_out_CO_select = 0x0;
	uint_lemo_out_CO_select = 0x1; // Select Sample Clock

	//uint_lemo_out_TO_select = 0xFFFF; // Select all internal triggers
	//uint_lemo_out_TO_select = 0x1; // Select ch1 trigger
	//uint_lemo_out_TO_select = 0x4; // Select ch3 trigger
	//uint_lemo_out_TO_select = 0xF; // Select ch1-4 trigger
	uint_lemo_out_TO_select = 0xFFFF; // Select ch1-16 trigger

	//uint_lemo_out_UO_select = 0x4;  // Select LogicBusy
	//uint_lemo_out_UO_select = 0x2;  //  BankxArmed
	uint_lemo_out_UO_select = 0x08000000;  // Select Select External Veto/Gate to ADC FPGA






	/******************************************************************************************/
	/*                                                                                        */
	/*  command line interpreter                                                               */
	/*                                                                                        */
	/******************************************************************************************/

	if (argc > 1) {

		while ((int_ch = getopt(argc, argv, "?lhpI:A:N:C:F:f:")) != -1) {

			switch (int_ch) {
				//printf("ch %c    \n", int_ch );

				case 'I':
					sscanf(optarg,"%s", ch_string) ;
					//printf("-I %s    \n", ch_string );
					strcpy(gl_cmd_ip_string,ch_string) ;
					break;

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


#ifdef ETHERNET_UDP_INTERFACE
					printf("Usage: %s  [-?h] [-p] [-I ip] [-N number of loops] [-C config_filename.ini] [-F or -f data_filename] ", argv[0]);
					printf("   \n");
					printf("   \n");
					printf("   -I string  ......  SIS3316 IP Address  Default = %s\n", gl_cmd_ip_string);
#endif

					printf("   -N num      .....  number of loops (banks); default: 0 (endless) \n");
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
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_range_2V_flag[i_ch]                  = gl_sis3316_get_configuration_parameters->uint_channel_range_2V;
			uint_channel_50ohm_termination_disable_flag[i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_50ohm_termination_disable;
		}

		for (i_fpga = 0; i_fpga < 4; i_fpga++) {
			uint_fpga_analog_ctrl_val[i_fpga] = (uint_channel_range_2V_flag[(i_fpga * 4) + 0] & 1)
				+ ((uint_channel_range_2V_flag[(i_fpga * 4) + 1] & 1) << 8)
				+ ((uint_channel_range_2V_flag[(i_fpga * 4) + 2] & 1) << 16)
				+ ((uint_channel_range_2V_flag[(i_fpga * 4) + 3] & 1) << 24)
				+ ((uint_channel_50ohm_termination_disable_flag[(i_fpga * 4) + 0] & 1) << 2)
				+ ((uint_channel_50ohm_termination_disable_flag[(i_fpga * 4) + 1] & 1) << 10)
				+ ((uint_channel_50ohm_termination_disable_flag[(i_fpga * 4) + 2] & 1) << 18)
				+ ((uint_channel_50ohm_termination_disable_flag[(i_fpga * 4) + 3] & 1) << 26);
		}

		//set ADC offsets (DAC) :
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_analog_offset_dac_val[i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_adc_offset;
		}

		// channel polarity
		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_polarity_invert_flag[i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_polarity_invert; //
		}


		uint_acquisition_trigger_mode =  gl_sis3316_get_configuration_parameters->uint_acquisition_trigger_mode; //

		if(uint_acquisition_trigger_mode == 0) {// 0: TI External Trigger
			// channel external gate enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_external_gate_enable_flag[i_ch] = 0; // all disable
			}

			// channel external trigger enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_external_trigger_enable_flag[i_ch] = 1; // all enable
			}

			// channel internal trigger enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_internal_trigger_enable_flag[i_ch] = 0; // all disable
			}

		}
		else { // 1: TI External Gate/Internal Trigger
			// channel external gate enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_external_gate_enable_flag[i_ch] = 1; // all enable
			}

			// channel external trigger enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_external_trigger_enable_flag[i_ch] = 0; // all disable
			}

			// channel internal trigger enable(1)/disable(0)
			for (i_ch = 0; i_ch < 16; i_ch++) {
				uint_channel_internal_trigger_enable_flag[i_ch] = 1; // all enable
			}
		}


		// internal Trigger generation
		uint_trigger_pulse_length    = gl_sis3316_get_configuration_parameters->uint_channel_trigger_pulse_length; //;
		uint_trigger_gap             = gl_sis3316_get_configuration_parameters->uint_channel_trigger_gap; //;
		uint_trigger_peaking         = gl_sis3316_get_configuration_parameters->uint_channel_trigger_peaking; //;
		uint_trigger_threshold_value = gl_sis3316_get_configuration_parameters->uint_channel_trigger_threshold * uint_trigger_peaking; //

		for (i_ch = 0; i_ch < 16; i_ch++) {
			uint_channel_trigger_generation_setup_reg[i_ch] = ((uint_trigger_pulse_length & 0xff) << 24)
															+ ((uint_trigger_gap & 0xfff) << 12)
															+ ((uint_trigger_peaking & 0xfff));
			uint_channel_trigger_generation_threshold_reg[i_ch] = 0x80000000 + 0x30000000  // Enable and CFD = 3
																+ 0x8000000  //  MAW offset
																+ (uint_trigger_threshold_value & 0x7FFFFFF) ; //  Threshold
			uint_channel_he_trigger_generation_threshold_reg[i_ch] = 0x0; //
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
			uint_trigger_function_deadtime_length = uint_trigger_function_deadtime_length + 0x4000; // Enable Deadtime with internal Address Threshold Flag
			//uint_trigger_function_deadtime_length = uint_trigger_function_deadtime_length + 0x8000; // Enable Deadtime with FP-Bus Address Threshold Flag
		}





	}


		if(uint_print_configurationParameterOnly_flag == 1) { ;


		printf("\n");
		printf("uint_udp_jumbo_mode bank                              = %d \n",uint_udp_jumbo_mode);
		printf("uint_udp_nofPacketsPerRequest                         = %d \n",uint_udp_nofPacketsPerRequest);
		printf("\n");
		printf("nof_events per bank                                   = %d \n",nof_events_per_bank);
			printf("\n");
			printf("\n");
  		printf("channel polarity invert ch1 to ch16                   = ");
  		printf(" %d",uint_channel_polarity_invert_flag[0]);
			printf("\n");
			printf("\n");
  		printf("channel range_2V ch1 to ch16                          = ");
			printf(" %d",uint_channel_range_2V_flag[0]);
			printf("\n");
  		printf("channel 50 Ohm termination disable ch1 to ch16        = ");
			printf(" %d",uint_channel_50ohm_termination_disable_flag[0]);
			printf("\n");
			printf("\n");
  		printf("channel ADC offset                                    = ");
			printf(" %d",uint_channel_analog_offset_dac_val[0]);

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












/******************************************************************************************************************************/
/* VME Master Create, Open and Setup                                                                                          */
/******************************************************************************************************************************/


#ifdef PCI_VME_INTERFACE
	// create SIS1100/SIS310x vme interface device
	//sis1100 *vme_crate = new sis1100(0);
	gl_vme_crate = new sis1100(0);

#endif

#ifdef USB_VME_INTERFACE
USHORT idVendor;
USHORT idProduct;
USHORT idSerNo;
USHORT idFirmwareVersion;
USHORT idDriverVersion;
	// create SIS3150USB vme interface device
	//sis3150 *vme_crate = new sis3150(0);
	gl_vme_crate = new sis3150(0);
#endif

#ifdef USB3_VME_INTERFACE
USHORT idVendor;
USHORT idProduct;
USHORT idSerNo;
USHORT idDriverVersion;
USHORT idFxFirmwareVersion;
USHORT idFpgaFirmwareVersion;
	// create SIS3153USB vme interface device
	//sis3153 *vme_crate = new sis3153(0);
	gl_vme_crate = new sis3153(0);
#endif


#ifdef ETHERNET_UDP_INTERFACE

	char  pc_ip_addr_string[32] ;
	char  sis3316_ip_addr_string[32] ;

	strcpy(sis3316_ip_addr_string, gl_cmd_ip_string) ; // SIS3316 IP address
	//strcpy(sis3316_ip_addr_string,"192.168.1.100") ; // SIS3316 IP address
	#ifdef WINDOWS
    //return_code = WSAStartup();
    return_code = WinsockStartup();
	#endif

	//sis3316_eth *vme_crate = new sis3316_eth;
	gl_vme_crate = new sis3316_eth;

	// increase read_buffer size
	// SUSE needs following command as su: >sysctl -w net.core.rmem_max=33554432
	int	sockbufsize = 335544432 ; // 0x2000000
	return_code = gl_vme_crate->set_UdpSocketOptionBufSize(sockbufsize) ;

	//strcpy(pc_ip_addr_string,"212.60.16.49") ; // Window example: secocnd Lan interface IP address is 212.60.16.49
	strcpy(pc_ip_addr_string,"") ; // empty if default Lan interface (Window: use IP address to bind in case of 2. 3. 4. .. LAN Interface)
	return_code = gl_vme_crate->set_UdpSocketBindMyOwnPort( pc_ip_addr_string);

	gl_vme_crate->set_UdpSocketSIS3316_IpAddress( sis3316_ip_addr_string);

    return_code = gl_vme_crate->vme_A32D32_read(SIS3316_MODID,&data);
	printf("return_code = 0X%08x   Module ID = 0X%08x \n",return_code, data);
    return_code = gl_vme_crate->vme_A32D32_read(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL,&data);
	printf("return_code = 0X%08x   ACCESS    = 0X%08x \n",return_code, data);

#endif


// open Vme Interface device
//	gl_vme_crate = vme_crate ;

	return_code = gl_vme_crate->vmeopen ();  // open Vme interface
	gl_vme_crate->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface
	printf("\n%s    (found %d vme interface device[s])\n\n",char_messages, nof_found_devices);

	if(return_code != 0x0) {
		//printf("ERROR: gl_vme_crate->vmeopen: return_code = 0x%08x\n\n", return_code);
		program_stop_and_wait();
		return -1 ;
	}


/******************************************************************************************/
// additional Vme interface device informations
#ifdef USB_VME_INTERFACE
	gl_vme_crate->get_device_informations (&idVendor, &idProduct, &idSerNo, &idFirmwareVersion, &idDriverVersion);  //
	printf("idVendor:           %04X\n",idVendor);
	printf("idProduct:          %04X\n",idProduct);
	printf("idSerNo:            %d\n",idSerNo);
	printf("idFirmwareVersion:  %04X\n",idFirmwareVersion);
	printf("idDriverVersion:    %04X\n",idDriverVersion);
	printf("\n\n");

#endif
/******************************************************************************************/

#ifdef WINDOWS

	if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, TRUE ) ){
		printf( "Error setting Console-Ctrl Handler\n" );
		return -1;
	}
#endif
/******************************************************************************************/



/******************************************************************************************/

	return_code = gl_vme_crate->udp_reset_cmd();


	return_code = gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_MODID, &data);
	printf("vme_A32D32_read: module_base_addr = 0x%08x      data = 0x%08x     return_code = 0x%08x\n", module_base_addr, data, return_code);


	// kill request and grant from vme interface (in case of use using ethernet interface)
	gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000);
	// arbitrate
	gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 1);
	gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_KEY_ADC_FPGA_RESET, 0);



	sis3316_not_OK = 0 ;
	if (return_code == 0) {
		printf("SIS3316_MODID                    = 0x%08x\n\n", data);

		gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, 0x400); // Clear Link Error Latch bits (test feature)
		gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, 0x400); // Clear Link Error Latch bits
		gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, 0x400); // Clear Link Error Latch bits
		gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0x400); // Clear Link Error Latch bits

		gl_vme_crate->vme_A32D32_read(module_base_addr + SIS3316_ADC_CH1_4_FIRMWARE_REG, &data);
		printf("SIS3316_ADC_CH1_4_FIRMWARE_REG   = 0x%08x \n", data);
		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH5_8_FIRMWARE_REG, &data);
		printf("SIS3316_ADC_CH5_8_FIRMWARE_REG   = 0x%08x \n", data);
		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH9_12_FIRMWARE_REG, &data);
		printf("SIS3316_ADC_CH9_12_FIRMWARE_REG  = 0x%08x \n", data);
		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH13_16_FIRMWARE_REG, &data);
		printf("SIS3316_ADC_CH13_16_FIRMWARE_REG = 0x%08x \n\n", data);

// Test feature (not nesessary)
		gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS, 0xE0E0E0E0);  // clear error Latch bits
		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS, &data);
		printf("SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS: data = 0x%08x     return_code = 0x%08x\n", data, return_code);
		if (data != 0x18181818) { sis3316_not_OK = 1 ; }

		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH1_4_STATUS_REG, &data);
		printf("SIS3316_ADC_CH1_4_STATUS_REG     = 0x%08x \n", data);
		if (data != 0x130018) { sis3316_not_OK = 1 ; }

		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH5_8_STATUS_REG, &data);
		printf("SIS3316_ADC_CH5_8_STATUS_REG     = 0x%08x \n", data);
		if (data != 0x130018) { sis3316_not_OK = 1 ; }

		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH9_12_STATUS_REG, &data);
		printf("SIS3316_ADC_CH9_12_STATUS_REG    = 0x%08x \n", data);
		if (data != 0x130018) { sis3316_not_OK = 1 ; }

		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH13_16_STATUS_REG, &data);
		printf("SIS3316_ADC_CH13_16_STATUS_REG   = 0x%08x \n\n", data);
		if (data != 0x130018) { sis3316_not_OK = 1 ; }

	}
	else {
		printf("SIS3316_MODID                  = 0x%08x     return_code = 0x%08x\n", data, return_code);
		program_stop_and_wait() ;
	}


	// set UDP option/mode
	if(uint_udp_jumbo_mode == 0) { gl_vme_crate->set_UdpSocketDisableJumboFrame() ; }
	if(uint_udp_jumbo_mode == 1) { gl_vme_crate->set_UdpSocketEnableJumboFrame() ; }
	gl_vme_crate->set_UdpSocketReceiveNofPackagesPerRequest(uint_udp_nofPacketsPerRequest);

	sis3316_adc  *sis3316_adc1 ;
	sis3316_adc1 = new sis3316_adc( gl_vme_crate, module_base_addr);

	// new with SIS3316 / SIS3316-2
	if (sis3316_adc1->device_variant == SIS::ADC::SIS3316::TYPE_SIS3316) { // SIS3316
		if (sis3316_adc1->adc_125MHz_flag == 1) {
			printf("SIS3316  125MHz-16bit\n");
		}
		else {
			printf("SIS3316  250MHz-14bit\n");
		}
	}
	else { // SIS3316-2
		if (sis3316_adc1->adc_125MHz_flag == 1) {
			printf("SIS3316-2  125MHz-16bit\n");
		}
		else {
			printf("SIS3316-2  250MHz-14bit\n");
		}
	}



/******************************************************************************************************************************/
/* CERN ROOT                                                                                                                  */
/******************************************************************************************************************************/

#ifdef CERN_ROOT_PLOT

	int root_graph_x ;
	int root_graph_y ;
	int root_graph_x_size ;
	int root_graph_y_size ;
	char root_graph_text[80] ;


	root_graph_x_size = 1000 ;
	root_graph_y_size = 500 ;

	root_graph_x = 10 ;
	root_graph_y = 620 ;



	TApplication theApp("SIS3316 Application: Test", &argc, (char**)argv);
	//sis_root_graph *graph_raw = new sis_root_graph(root_graph_text, root_graph_x, root_graph_y, root_graph_x_size, root_graph_y_size) ;
	strcpy(root_graph_text,"SIS3316 Graph: Raw data") ;
	gl_graph_raw      = new sis_root_graph(root_graph_text, root_graph_x, root_graph_y, root_graph_x_size, root_graph_y_size) ;

	if (sis3316_adc1->adc_125MHz_flag == 1) {
		gl_graph_raw->sis3316_set_16bit_Yaxis();
	}
	else {
		gl_graph_raw->sis3316_set_14bit_Yaxis();
	}

#endif




/******************************************************************************************/


/******************************************************************************************/
/*                                                                                        */
/*  SIS3316 Setup                                                                         */
/*                                                                                        */
/******************************************************************************************/

	sis3316_adc1->register_write(SIS3316_KEY_DISARM, 0);  // disarm
	sis3316_adc1->register_write(SIS3316_KEY_RESET, 0);  // reset
	sis3316_adc1->adc_spi_reg_enable_adc_outputs();  // necessary after reset

	unsigned int clock_N1div, clock_HSdiv ;
#ifdef ONLY_SIS3316 // old
	if (sis3316_adc1->adc_125MHz_flag == 0) { // 250 MHz
											  // 250.000 MHz
		clock_N1div = 4;
		clock_HSdiv = 5;
		iob_delay_value = 0x1002; // ADC FPGA version A_0250_0004 and higher
	}
	else {
		// 125.000 MHz
		clock_N1div = 8;
		clock_HSdiv = 5;
		iob_delay_value = 0x1020; // 16 bit ADC FPGA version A_0125_0004 and higher
	}
#endif

	// new with SIS3316 / SIS3316-2
	double double_clock_configure_fft_frequency;
	unsigned int clock_freq_choice ;
	if (sis3316_adc1->adc_125MHz_flag == 0) { // 250 MHz
		// 250.000 MHz
		clock_freq_choice = SIS::ADC::SIS3316::SAMPLERATE_250MSPS;
	}
	else {
		// 125.000 MHz
		clock_freq_choice = SIS::ADC::SIS3316::SAMPLERATE_125MSPS;
	}
	sis3316_adc1->get_SI570_oscillator_hs_div_and_n1_div_values(clock_freq_choice, &clock_HSdiv, &clock_N1div, &double_clock_configure_fft_frequency);
	sis3316_adc1->get_adc_fpga_iob_delay_value(clock_freq_choice, &iob_delay_value);


	sis3316_adc1->change_frequency_HSdiv_N1div(0, clock_HSdiv, clock_N1div);
	sis3316_adc1->configure_adc_fpga_iob_delays(iob_delay_value);  // necessary after changing/setting clock




/******************************************/

// Gain/Termination
	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[0]); //
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[1]); //
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[2]); //
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[3]); //

/******************************************/

//  set ADC offsets (DAC)
	for (i_ch = 0; i_ch<16; i_ch++) {
		sis3316_adc1->adc_dac_offset_ch_array[i_ch] = uint_channel_analog_offset_dac_val[i_ch]; //
	}
	return_code = sis3316_adc1->write_all_adc_dac_offsets();
	if (return_code != 0) {
		printf("Error write_all_adc_dac_offsets: return_code = 0x%08x \n", return_code);
	}


/******************************************/

	// Select LEMO Output "Co"
	sis3316_adc1->register_write(SIS3316_LEMO_OUT_CO_SELECT_REG, uint_lemo_out_CO_select); //

	// Select LEMO Output "To"
	sis3316_adc1->register_write(SIS3316_LEMO_OUT_TO_SELECT_REG, uint_lemo_out_TO_select); //

	// Select LEMO Output "UO"
	sis3316_adc1->register_write(SIS3316_LEMO_OUT_UO_SELECT_REG, uint_lemo_out_UO_select); //


/******************************************/

	// channel Header
	data = module_base_addr + 0x0;
	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG, data); //
	data = module_base_addr + 0x00400000;
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG, data); //
	data = module_base_addr + 0x00800000;
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG, data); //
	data = module_base_addr + 0x00C00000;
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG, data); //


/******************************************/
	// internal Trigger generation
	for (i_ch = 0; i_ch < 16; i_ch++) {
		sis3316_adc1->internal_trigger_generation_setup(uint_channel_trigger_generation_threshold_reg[i_ch], uint_channel_he_trigger_generation_threshold_reg[i_ch], uint_channel_trigger_generation_setup_reg[i_ch], i_ch);  //
	}

/******************************************/

	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length

/******************************************/

	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG, ((uint_save_raw_sample_length & 0xffff) << 16) + (uint_save_raw_sample_start_index & 0xffff) ); // Sample Length
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG, ((uint_save_raw_sample_length & 0xffff) << 16) + (uint_save_raw_sample_start_index & 0xffff) ); // Sample Length
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG, ((uint_save_raw_sample_length & 0xffff) << 16) + (uint_save_raw_sample_start_index & 0xffff) ); // Sample Length
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG, ((uint_save_raw_sample_length & 0xffff) << 16) + (uint_save_raw_sample_start_index & 0xffff) ); // Sample Length



/******************************************/
	for (i = 0; i < 6; i++) {
		data = ((gate_length[i] & 0x1ff) << 16) + (gate_start_index[i] & 0xffff);
		sis3316_adc1->register_write(SIS3316_ADC_CH1_4_ACCUMULATOR_GATE1_CONFIG_REG   + (i * 0x4), data); //
		sis3316_adc1->register_write(SIS3316_ADC_CH5_8_ACCUMULATOR_GATE1_CONFIG_REG   + (i * 0x4), data); //
		sis3316_adc1->register_write(SIS3316_ADC_CH9_12_ACCUMULATOR_GATE1_CONFIG_REG  + (i * 0x4), data); //
		sis3316_adc1->register_write(SIS3316_ADC_CH13_16_ACCUMULATOR_GATE1_CONFIG_REG + (i * 0x4), data); //
	}


/******************************************/
	internal_fir_trigger_delay = internal_fir_trigger_delay & 0xff ;
	data = (internal_fir_trigger_delay << 24) + (internal_fir_trigger_delay << 16) + (internal_fir_trigger_delay << 8) + (internal_fir_trigger_delay ) ;

	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data ); //
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data ); //
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data ); //
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data ); //



/******************************************/

	//pre_trigger_delay =  0;
	if (pre_trigger_delay > 16378) {
		pre_trigger_delay  = 16378;
	}
	//pre_trigger_delay = pre_trigger_delay + 0x8000 ; // set "Additional Delay of Fir Trigger P+G" Bit
	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //

/******************************************/

	//pileup ;
	data = ((uint_re_pileup & 0xffff) << 16) + (uint_pileup & 0xffff) ;

	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_PILEUP_CONFIG_REG, data ); //
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_PILEUP_CONFIG_REG, data ); //
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_PILEUP_CONFIG_REG, data ); //
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_PILEUP_CONFIG_REG, data ); //

/******************************************/
	// External Veto Delay and TI Trigger Function Deadtime Setup
	if (external_veto_gate_delay > 2044) {
		external_veto_gate_delay = 2044;
	}
	data = ((uint_trigger_function_deadtime_length & 0xffff) << 16) + (external_veto_gate_delay & 0xffff); //
	sis3316_adc1->register_write(SIS3316_EXTERNAL_VETO_GATE_DELAY_REG, data);

/******************************************/


	//  Event Configuration
	for (i_fpga = 0; i_fpga < 4; i_fpga++) {
		uint_fpag_config_reg_value[i_fpga] =
			((uint_channel_external_trigger_enable_flag[(i_fpga * 4) + 0] & 1) << 3)
			+ ((uint_channel_external_trigger_enable_flag[(i_fpga * 4) + 1] & 1) << 11)
			+ ((uint_channel_external_trigger_enable_flag[(i_fpga * 4) + 2] & 1) << 19)
			+ ((uint_channel_external_trigger_enable_flag[(i_fpga * 4) + 3] & 1) << 27)

			+ ((uint_channel_external_gate_enable_flag[(i_fpga * 4) + 0] & 1) << 6)
			+ ((uint_channel_external_gate_enable_flag[(i_fpga * 4) + 1] & 1) << 14)
			+ ((uint_channel_external_gate_enable_flag[(i_fpga * 4) + 2] & 1) << 22)
			+ ((uint_channel_external_gate_enable_flag[(i_fpga * 4) + 3] & 1) << 30)

			+ ((uint_channel_internal_trigger_enable_flag[(i_fpga * 4) + 0] & 1) << 2)
			+ ((uint_channel_internal_trigger_enable_flag[(i_fpga * 4) + 1] & 1) << 10)
			+ ((uint_channel_internal_trigger_enable_flag[(i_fpga * 4) + 2] & 1) << 18)
			+ ((uint_channel_internal_trigger_enable_flag[(i_fpga * 4) + 3] & 1) << 26)

			+ ((uint_channel_polarity_invert_flag[(i_fpga * 4) + 0] & 1))
			+ ((uint_channel_polarity_invert_flag[(i_fpga * 4) + 1] & 1) << 8)
			+ ((uint_channel_polarity_invert_flag[(i_fpga * 4) + 2] & 1) << 16)
			+ ((uint_channel_polarity_invert_flag[(i_fpga * 4) + 3] & 1) << 24);
	}
	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, uint_fpag_config_reg_value[0]); //
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, uint_fpag_config_reg_value[1]); //
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, uint_fpag_config_reg_value[2]); //
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, uint_fpag_config_reg_value[3]); //

	//  Extended Event Configuration
	data = 0 ;
	if(uint_save_raw_sample_first_event_only_mode == 1) {
		data = 0x10101010 ;		// Enable save_raw_sample_first_event_only_mode
	}
	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_EXTENDED_EVENT_CONFIG_REG, data); //
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_EXTENDED_EVENT_CONFIG_REG, data); //
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_EXTENDED_EVENT_CONFIG_REG, data); //
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_EXTENDED_EVENT_CONFIG_REG, data); //


	if(uint_acquisition_trigger_mode == 0) {// 0: TI External Trigger
		// Enable LEMO Input "TI" as External Trigger function
		data = 0x10; // Enable Nim Input "TI" as Trigger function
		//data = data + 0x40; // set Level sensitiv
		if(uint_trigger_function_deadtime_logic_enable == 1) {
			data = data + 0x4000; // enable TI Deadtime logic
		}
		sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data); //

		// enable external (global) functions
		data = 0x100; // enable "External Trigger function" as Trigger
		data = data + 0x400; // enable "External Timestamp-Clear function" as Timestamp-Clear
		data = data + 0x8000; // enable "External Trigger function_disable_if_Busy"
		sis3316_adc1->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, data);

	}
	else { // 1: TI External Gate/Internal Trigger (SETUP_TI_AS_Gate)
		// Enable LEMO Input "TI" as Local Veto function
		data = 0x10; // Enable Nim Input "TI" as Trigger function
		data = data + 0x40; // set Level sensitiv
		if(uint_trigger_function_deadtime_logic_enable == 1) {
			data = data + 0x4000; // enable TI Deadtime logic
		}
		sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data); //

		// enable external (global) functions
		data = 0x200; // enable "External Trigger function" as Veto
		data = data + 0x400; // enable "External Timestamp-Clear function" as Timestamp-Clear
		sis3316_adc1->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, data);
	}



	// Led Application mode
		data = 0x70 ;
		sis3316_adc1->register_write(SIS3316_CONTROL_STATUS, data ); //

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
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG, data);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_DATAFORMAT_CONFIG_REG, data);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_DATAFORMAT_CONFIG_REG, data);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_DATAFORMAT_CONFIG_REG, data);

	event_length = (header_length + (uint_save_raw_sample_length / 2) + maw_test_buffer_length);

	if(uint_save_raw_sample_first_event_only_mode == 0) {
		event_length_for_file_write_header = event_length & 0xffffff;		//
	}
	else {
		event_length_for_file_write_header = ((header_length & 0xff) << 24) + (event_length & 0xffffff);		// Enable save_raw_sample_first_event_only_mode
	}



// MAW Test Buffer configuration
	data = maw_test_buffer_length + (maw_test_buffer_delay << 16);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_MAW_TEST_BUFFER_CONFIG_REG, data);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_MAW_TEST_BUFFER_CONFIG_REG, data);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_MAW_TEST_BUFFER_CONFIG_REG, data);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_MAW_TEST_BUFFER_CONFIG_REG, data);

	if(uint_save_raw_sample_first_event_only_mode == 0) {
		address_threshold = (nof_events_per_bank * event_length) - 1 ;  //
	}
	else {
		address_threshold = event_length + ((nof_events_per_bank - 1) * header_length) - 1 ;  //
	}

	//address_threshold = 200;
	//address_threshold = address_threshold + 0x80000000;  //  suppress saving following hits/events if addresshold flag is set
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG, address_threshold); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_ADDRESS_THRESHOLD_REG, address_threshold); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_ADDRESS_THRESHOLD_REG, address_threshold); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_ADDRESS_THRESHOLD_REG, address_threshold); //


	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1 ); //  update and freeze with each bank switching
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1 ); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1 ); //


/******************************************************************************************/

	if (uint_save_raw_sample_length != 0) {
		gl_graph_raw->sis3316_draw_XYaxis (uint_save_raw_sample_length); // clear and draw X/Y
	}


/******************************************************************************************/

	for (i_ch=0; i_ch<16; i_ch++) {
		uint_channel_event_counter[i_ch]  = 0;
	}

	loop_counter        = 0;
	bank_buffer_counter = 0 ;
	if (sis3316_adc1->adc_125MHz_flag == 0) {
		file_header_indentifier = 0 ; // 250MHz
	}
	else {
		file_header_indentifier = 1 ;
	}
	file_header_reserved = 0 ;
	file_header_reserved = file_header_reserved + (uint_acquisition_trigger_mode & 0xf) ;
	file_header_reserved = file_header_reserved + ((uint_save_raw_sample_first_event_only_mode & 0x1) < 4);



/******************************************************************************************/


	printf("Start Multievent \n");
	// Clear Timestamp  */
	return_code = sis3316_adc1->register_write(SIS3316_KEY_TIMESTAMP_CLEAR , 0);  //

	// Start Readout Loop  */
	//Note: Start sampling on Bank on alternate Bank, check Bit 24 in the register "previous Bank sample address"
	sis3316_adc1->register_read( SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);
	if((data & 0x1000000) == 0x1000000 ) { 	// bank2 flag is set ?
		//printf("bank2 flag is set\n"); // start sampling an alternate bank
		bank1_armed_flag = 1 ;
		return_code = sis3316_adc1->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK1, 0 ); //    Arm
		printf("SIS3316_KEY_DISARM_AND_ARM_BANK1 \n");
	}
	else {
		//printf("bank2 flag is not set\n"); // start sampling an alternate bank
		bank1_armed_flag = 0 ;
		return_code = sis3316_adc1->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK2, 0 ); // //  Arm
		printf("SIS3316_KEY_DISARM_AND_ARM_BANK2 \n");
	}


// Start of Muitibank Acquisition
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

		// wait for Address Threshold Flag
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
			return_code = gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, &data);
			//usleep(500000); //500ms
			//printf("in Loop:  return_code = 0x%08x    addr = 0x%08x   SIS3316_ACQUISITION_CONTROL_STATUS = 0x%08x     \n", return_code , module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, data);
		} while (((data & 0x80000) == 0x0) && (gl_stopReq == FALSE)); // Address Threshold reached ?

		//gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, &data);
		//printf("SIS3316_ACQUISITION_CONTROL_STATUS before bank switch = 0x%08x\n", data);

		// disarm active bank and arm alternate bank
		if (bank1_armed_flag == 1) {
			return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_KEY_DISARM_AND_ARM_BANK2 , 0);  //  Arm Bank2
			bank1_armed_flag = 0; // bank 2 is armed
			printf("\nSIS3316_KEY_DISARM_AND_ARM_BANK2 \n\n");
		}
		else {
			return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_KEY_DISARM_AND_ARM_BANK1 , 0);  //  Arm Bank1
			bank1_armed_flag = 1; // bank 1 is armed
			printf("\nSIS3316_KEY_DISARM_AND_ARM_BANK1 \n\n");
		}

		gl_graph_raw->sis3316_draw_XYaxis (uint_save_raw_sample_length); // clear and draw X/Y

		// use sis3316_adc1->read_DMA_Channel_PreviousBankDataBuffer to read maximal thenumber of max_read_nof_words insteadt of all
		//unsigned int max_read_nof_words;
		//max_read_nof_words = event_length;
		//return_code = sis3316_adc1->read_DMA_Channel_PreviousBankDataBuffer(bank1_armed_flag, i_ch, max_read_nof_words, &got_nof_32bit_words, gl_rblt_data);


		for (i_ch = 0; i_ch < 16; i_ch++) {
			// read channel events
			return_code = sis3316_adc1->read_MBLT64_Channel_PreviousBankDataBuffer(bank1_armed_flag /*bank2_read_flag*/, i_ch /* 0 to 15 */,  &got_nof_32bit_words, gl_rblt_data ) ;
			//printf("read_MBLT64_Channel_PreviousBankDataBuffer: i_ch %d  got_nof_32bit_words = 0x%08x  return_code = 0x%08x\n",i_ch,  got_nof_32bit_words, return_code);
			if (return_code != 0) {
				printf("read_MBLT64_Channel_PreviousBankDataBuffer: return_code = 0x%08x\n", return_code);
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


			uint_channel_event_counter[i_ch]  = uint_channel_event_counter[i_ch] + ch_event_counter;
			if (ch_event_counter > 0) {
				// file write
				if (uint_WriteData_to_File_OpenFlag == 1) {   ; //
					uint_WritenData_to_File_32bit_words += SIS3316_WriteBankChannelHeaderToDataFile (file_WriteData_to_File_Pointer, file_header_indentifier, bank_buffer_counter, i_ch, ch_event_counter , event_length_for_file_write_header, maw_test_buffer_length, file_header_reserved) ;
					uint_WritenData_to_File_32bit_words += SIS3316_WriteBankChannelEventBufferToDataFile (file_WriteData_to_File_Pointer, gl_rblt_data, got_nof_32bit_words)  ;
				}

				// draw graph
				//for (i = 0; i < ch_event_counter; i++) {
				for (i = 0; i < 1; i++) {// plot ony 1. event
					if (i == 0) { // plot ony 1. event
						gl_graph_raw->sis3316_draw_chN(uint_save_raw_sample_length, &gl_rblt_data[i*(event_length)+header_length], i_ch); //
						//printf("i_ch = %d   ->  Internal Trigger Flag = %d  \n", i_ch + 1, (gl_rblt_data[i*(event_length)+header_length - 1] >> 26) & 1);
						//printf("i_ch = %d   ->  Information           = 0x%02x  \n", i_ch + 1, ((gl_rblt_data[i*(event_length)+3] >> 24)));
					}
				}
			}
		}

		loop_counter++;
		bank_buffer_counter++;

		if ((loop_counter & 0xf) == 0xf) {
			printf("\n");
			// Statistic Counters
			unsigned int uint_statistic_buffer[24];
			printf("     \t All          Hits/Events  Deadtime     Pileup       Veto         High Energy supressed \n");
			for (i_adc = 0; i_adc < 4; i_adc++) {
				sis3316_adc1->read_Channel_StatisticCounter(i_adc /* 0 to 3 */, uint_statistic_buffer); // new 27.08.2019
				for (i_ch = 0; i_ch < 4; i_ch++) {
					printf("ch%d \t", (i_adc * 4) + i_ch + 1);
					for (i = 0; i < 6; i++) {
						printf(" 0x%08x  ", uint_statistic_buffer[(i_ch * 6) + i]);
					}
					printf("\n");
				}
			}


			printf("bank_buffer_counter = %d     \n",bank_buffer_counter);
			for (i_ch=0; i_ch<16; i_ch++) {
				if (uint_channel_event_counter[i_ch] != 0) {
					printf("ch %d:    event counter  = %d  (0x%08x)      \n", i_ch+1, uint_channel_event_counter[i_ch], uint_channel_event_counter[i_ch]);
				}
			}
			printf("\n");
			printf("\n");
		}


		//printf("\n");
		gSystem->ProcessEvents();  // handle GUI events

	} while((gl_stopReq == FALSE) && ((loop_counter < stop_after_loop_counts) || (stop_after_loop_counts == 0) ));


	// close file (if it was open)
 	if (uint_WriteData_to_File_OpenFlag == 1) {   ; //
		fclose(file_WriteData_to_File_Pointer);
		uint_WriteData_to_File_OpenFlag = 0 ;
	}
//---------------------------------------------------------------------------

	printf("bank_buffer_counter = %d     \n",bank_buffer_counter);
	for (i_ch=0; i_ch<16; i_ch++) {
		if (uint_channel_event_counter[i_ch] != 0) {
			printf("ch %d:    event counter  = %d  (0x%08x)      \n", i_ch+1, uint_channel_event_counter[i_ch], uint_channel_event_counter[i_ch]);
		}
	}



	printf("sampling finished   \n");
	printf("\n");
	// Statistic Counters
	unsigned int uint_statistic_buffer[24];
	printf("     \t All          Hits/Events  Deadtime     Pileup       Veto         High Energy supressed \n");
	for (i_adc = 0; i_adc < 4; i_adc++) {
		sis3316_adc1->read_Channel_StatisticCounter(i_adc /* 0 to 3 */, uint_statistic_buffer); // new 27.08.2019
		for (i_ch = 0; i_ch < 4; i_ch++) {
			printf("ch%d \t", (i_adc * 4) + i_ch + 1);
			for (i = 0; i < 6; i++) {
				printf(" 0x%08x  ", uint_statistic_buffer[(i_ch * 6) + i]);
			}
			printf("\n");
		}
	}
	//Disarm all modules
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_KEY_DISARM , 0);  //
	printf("\n");
	printf("sampling finished   \n");
	printf("\n");

	gl_stopReq = FALSE;
	do {
		gSystem->ProcessEvents();  // handle GUI events
	} while((gl_stopReq == FALSE) );


	return 0;
}

/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/

#define FILE_FORMAT_EVENT_HEADER        	0xDEADBEEF
#define FILE_FORMAT_EOF_TRAILER        		0x0E0F0E0F


//---------------------------------------------------------------------------
int WriteEventsToDataFile (unsigned int* memory_data_array, unsigned int nof_write_length_lwords)
{
int nof_write_elements ;
int written ;
//int data ;
//char messages_buffer[256] ;

// gl_uint_DataEvent_RunFile_EvenSize : length

		nof_write_elements = nof_write_length_lwords ;
		written=fwrite(memory_data_array,0x4,nof_write_elements,gl_FILE_DataEvenFilePointer); // write 3 uint value
		//gl_uint_DataEvent_LWordCounter = gl_uint_DataEvent_LWordCounter + written  ;
		if(nof_write_elements != written) {
    		printf ("Data File Write Error in  WriteEventToDataFile()  \n");
 		 }

 	return 0;

}


/***********************************************************************************************************************************************/
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



/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/


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


/***************************************************/
#ifdef WINDOWS

BOOL CtrlHandler( DWORD ctrlType ){
	switch( ctrlType ){
	case CTRL_C_EVENT:
		printf( "\n\nCTRL-C pressed. finishing current task.\n\n");
		gl_stopReq = TRUE;
		return( TRUE );
		break;
	default:
		printf( "\n\ndefault pressed. \n\n");
		return( FALSE );
		break;
	}
}
#endif


#ifdef not_defined_here
void usleep(unsigned int uint_usec)
{
    unsigned int msec;
	if (uint_usec <= 1000) {
		msec = 1 ;
	}
	else {
		msec = (uint_usec+999) / 1000 ;
	}
	Sleep(msec);

}
#endif

