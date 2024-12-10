/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_ch1_to_ch16_external_trigger_test.cpp                */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*    - sample logic will trigger with external Trigger (TI)	           */
/*    - connect output LEMO TO with Input LEMO TI		                   */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  used Root Version: 6.32/02                                            */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 18.12.2014                                       */
/*  last modification:    24.07.2024                                       */
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
/*  � 2024                                                                 */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethernet UDP)


#include "get_configuration_parameters.h"

 
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
   //#pragma comment (lib, "libCint") remove with root_v6.xx.xx
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
	#include "wingetopt.h"
	#include <stdint.h> // portable: uint64_t   MSVC: __int64 

#endif


/******************************************************************************************************/

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



/******************************************************************************************************/


#include "sis3316_class.h"
//sis3316_adc *gl_sis3316_adc1 ;

#define MAX_NOF_SIS3316_ADCS			1
//#define BROADCAST_BASE_ADDR				0x40000000      
//#define FIRST_MODULE_BASE_ADDR			0x31000000      
//#define MODULE_BASE_OFFSET				0x01000000      

#include "sis3316.h"

/******************************************************************************************************/

#ifdef CERN_ROOT_PLOT

#include "sis3316_cern_root_class.h"  
sis_root_graph *gl_graph_raw ;

sis_root_channel_energy_histogram *gl_channel_energy_histogram ;

sis_root_graph_maw *gl_graph_maw ;
sis_root_intensity_graph *gl_intensity_raw ;

unsigned int gl_graph_zoom_raw_draw_length ;
unsigned int gl_graph_zoom_raw_draw_bin_offset ;

#endif
/******************************************************************************************************/


/*===========================================================================*/
/* Globals					  			     */
/*===========================================================================*/
#define MAX_NUMBER_LWORDS_64MBYTE			0x1000000       /* 64MByte */

unsigned int gl_rblt_data[MAX_NUMBER_LWORDS_64MBYTE] ;

#ifdef WINDOWS
BOOL gl_stopReq = FALSE;
#endif


char gl_char_command[128] ;
FILE *gl_FILE_DataEvenFilePointer           ;
char gl_cmd_ip_string[64];

sis3316_get_configuration_parameters *gl_sis3316_get_configuration_parameters ;

/*===========================================================================*/
/* Prototypes			                               		  			     */
/*===========================================================================*/

int SIS3316_WriteBankChannelHeaderToDataFile (FILE *file_data_ptr, unsigned int indentifier, unsigned int bank_loop_no, unsigned int channel_no, unsigned int nof_events, unsigned int event_length, unsigned int maw_length, unsigned int reserved);
int SIS3316_WriteBankChannelEventBufferToDataFile (FILE *file_data_ptr, unsigned int* memory_data_array, unsigned int nof_write_length_lwords);


#ifdef WINDOWS
int gettimeofday(struct timeval* tp, struct timezone* tzp);
// MSVC defines this in winsock2.h!?
typedef struct timeval {
	long tv_sec;
	long tv_usec;
} timeval;

#endif

void program_stop_and_wait(void);
#ifdef WINDOWS
BOOL CtrlHandler( DWORD ctrlType );
void usleep(unsigned int uint_usec) ;
#endif


int main(int argc, char *argv[])
{

CHAR char_messages[128];
UINT nof_found_devices ;

int return_code ;
unsigned int module_base_addr   ;
unsigned int data;
unsigned int i;
unsigned int loop_counter;

UINT got_nof_32bit_words;
unsigned int sample_length;
unsigned int sample_start_index;
unsigned int trigger_gate_window_length;
unsigned int address_threshold;

unsigned int bank1_armed_flag ;
unsigned int poll_counter ;
unsigned int sis3316_not_OK;
unsigned int ch_event_counter;

unsigned int max_req_nof_32bit_words;
unsigned int event_length;
unsigned int header_length;

unsigned int iob_delay_value ;

unsigned int maw_test_buffer_length ;
unsigned int maw_test_buffer_delay ;

unsigned int header_accu_6_values_enable_flag ;
unsigned int header_accu_2_values_enable_flag ;
unsigned int header_maw_3_values_enable_flag ;
unsigned int maw_test_buffer_enable_flag ;

unsigned int header_maw_3_values_offset ;
unsigned int header_accu_2_values_offset ;

unsigned int uint_pileup ;
unsigned int uint_re_pileup ;

unsigned int uint_internal_trigger_delay;

/******************************************************************************************/
/*                                                                                        */
/*  SIS3316 program parameter                                                             */
/*                                                                                        */
/******************************************************************************************/
	module_base_addr = 0x30000000 ;
	// Ethernet UDP IP address
	strcpy(gl_cmd_ip_string,"192.168.1.2") ; // SIS3316 IP address

#ifdef ETHERNET_UDP_INTERFACE
	module_base_addr = 0x0 ;
	//printf( "\n\nEnter  sis3316 Ip address            : ");
	//scanf( "%s", &gl_cmd_ip_string );
#endif

unsigned int uint_fpag_config_reg_value[4]  ;

unsigned int i_ch;
unsigned int i_fpga;
unsigned int nof_events;

unsigned int uint_channel_external_trigger_enable_flag[16]  ;
unsigned int uint_channel_polarity_invert_flag[16]  ;
unsigned int uint_channel_range_2V_flag[16]  ;
unsigned int uint_channel_50ohm_termination_disable_flag[16]  ;

unsigned int uint_fpga_analog_ctrl_val[4] ;
unsigned int uint_channel_analog_offset_dac_val[16]  ;

unsigned int uint_lemo_out_CO_select ;
unsigned int uint_lemo_out_TO_select ;
unsigned int uint_lemo_out_UO_select ;

unsigned int uint_channel_trigger_generation_setup_reg[16] ;
unsigned int uint_channel_trigger_generation_threshold_reg[16] ;
unsigned int uint_channel_he_trigger_generation_threshold_reg[16] ;

unsigned int pre_trigger_delay;
unsigned int uint_trigger_pulse_length;
unsigned int uint_trigger_gap;
unsigned int uint_trigger_peaking;
 
	int int_ch ;
	char ch_string[1024] ;

	char char_config_file[512];
	int configurationFile_rc ;
	unsigned int uint_configurationFile_valid_flag = 0 ;
	unsigned int uint_print_configurationParameterOnly_flag = 0 ;
	unsigned int uint_software_key_trigger_flag = 0 ;
	unsigned int uint_select_internalTrigger_as_extertnalTrigger_flag = 0 ;
	unsigned int stop_after_loop_counts ;
	unsigned int stop_after_seconds ;

	unsigned int bank_buffer_counter ;
	unsigned int ch_event_counter_error = 0;

#define TIMESTAMP_TEST
#ifdef TIMESTAMP_TEST
	unsigned int uint_upper_timestamp;
	unsigned int uint_lower_timestamp;
	unsigned int double_ns_per_sample_clk = 4 ; // 1ns
	double double_time_stamp_ns;
#endif

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

	struct timeval time_start, time_actual;
	double double_run_time_sec ;
	unsigned int uint_run_time_sec ;

	uint_WriteData_to_File_OpenFlag      = 0 ;
	uint_WriteData_to_File_counter       = 0 ;
	uint_WriteData_to_MultipleFiles_Flag = 0 ;
	sprintf(char_WriteData_to_File_initialize_filename,"sis3316_test_data") ;

	stop_after_seconds                   = 0 ;		// 0: endless , could be changed by calling this program with "-t num" option
	stop_after_loop_counts               = 0;    	// 0: endless , could be changed by calling this program with "-N num" option
	nof_events = 1  ;

// channel external trigger enable
	for(i_ch=0;i_ch<16;i_ch++) {
		uint_channel_external_trigger_enable_flag[i_ch] = 1 ; // all enable
	}
// channel polarity
	for(i_ch=0;i_ch<16;i_ch++) {
		uint_channel_polarity_invert_flag[i_ch] = 1 ;
	}


// Set Gain/Termination
	for(i_ch=0;i_ch<16;i_ch++) {
		uint_channel_range_2V_flag[i_ch] = 0 ;
		uint_channel_50ohm_termination_disable_flag[i_ch] = 0 ;
	}
	for(i_fpga=0;i_fpga<4;i_fpga++) {
		uint_fpga_analog_ctrl_val[i_fpga] = 0x00000000 ; // 5 V Range , 50 Ohm Termination
		//uint_fpga_analog_ctrl_val[i_fpga] = 0x01010101 ; // 2 V Range , 50 Ohm Termination
		//uint_fpga_analog_ctrl_val[i_fpga] = 0x05050505 ; // 2 V Range , disable 50 Ohm Termination
	}


//set ADC offsets (DAC) :
	for(i_ch=0;i_ch<16;i_ch++) {
		uint_channel_analog_offset_dac_val[i_ch] = 0x8000 ; // middle
	}

// Lemo Out definition
	uint_lemo_out_CO_select  = 0x0;	
	uint_lemo_out_TO_select  = 0x1;	
	uint_lemo_out_UO_select  = 0x0;	

// 2.5us sample length -> 2500ns / 4ns = 625 samples
	trigger_gate_window_length  = 650;	
	sample_length               = 624;
	sample_start_index          = 0;	
	pre_trigger_delay           = 70 ; // 280 ns

// internal Trigger generation
	uint_trigger_pulse_length    = 0x10;
	uint_trigger_gap     = 8;
	uint_trigger_peaking = 10;


	for(i_ch=0;i_ch<16;i_ch++) {
		uint_channel_trigger_generation_setup_reg[i_ch]   =   ((uint_trigger_pulse_length & 0xff) << 24) 
			                                                + ((uint_trigger_gap & 0xfff) << 12)
			                                                + ((uint_trigger_peaking & 0xfff));
		uint_channel_trigger_generation_threshold_reg[i_ch] =    0x80000000 + 0x30000000  //   
			                                                   + 0x8000000  //  MAW offset
		                                                       + 1000; //  Threshold
		uint_channel_he_trigger_generation_threshold_reg[i_ch] = 0x0 ; //  
	}

	header_accu_6_values_enable_flag = 1 ;
	header_accu_2_values_enable_flag = 0 ;
	header_maw_3_values_enable_flag  = 0 ;

	maw_test_buffer_enable_flag = 0;
	maw_test_buffer_length      = 0;
	maw_test_buffer_delay       = 0;

	uint_pileup    = trigger_gate_window_length;  //  
	uint_re_pileup = trigger_gate_window_length;  //  

	uint_internal_trigger_delay = 0; // Note: max. 255 and multiply by 2 clocks -> 0 -> x 2 4ns = 0ns 

	pre_trigger_delay = pre_trigger_delay + (2 * uint_internal_trigger_delay) ;
/******************************************************************************************************************************/

	if (argc > 1) {

		while ((int_ch = getopt(argc, argv, "?lhpTSI:X:t:N:C:F:f:")) != -1) {

			switch (int_ch) {
				//printf("ch %c    \n", int_ch );

				case 'X':
		    		sscanf(optarg,"%X", &module_base_addr) ;
					break;

				case 'I':
					sscanf(optarg,"%s", ch_string) ;
					//printf("-I %s    \n", ch_string );
					strcpy(gl_cmd_ip_string,ch_string) ;
					break;

				case 't':
		    		sscanf(optarg,"%d",&data) ;
					stop_after_seconds                  = data ;    //
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

				case 'T':
					uint_software_key_trigger_flag = 1 ;
					break;

				case 'S':
					uint_select_internalTrigger_as_extertnalTrigger_flag = 1 ;
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

#ifndef ETHERNET_UDP_INTERFACE
				  printf("Usage: %s  [-?h] [-X Vme Base Address] [-L sample length] [-E number of events/loop] [-B number of loops] [-F save flag] ", argv[0]);
			      printf("   \n");
			      printf("   \n");
				  printf("   -X num     	SIS3316 VME Base Address       Default = 0x%08x\n", module_base_addr);
#endif

#ifdef ETHERNET_UDP_INTERFACE
					printf("Usage: %s  [-?h] [-p] [-I ip] [-t number of seconds] [-N number of loops] [-C config_filename.ini] [-F or -f data_filename] ", argv[0]);
					printf("   \n");
					printf("   \n");
					printf("   -I string  ......  SIS3316 IP Address  Default = %s\n", gl_cmd_ip_string);
#endif

					printf("   \n");
					printf("   -t num      .....  Run Time: number of seconds; default: 0 (defined by -N) \n");
					printf("   -N num      .....  Run Time: number of loops (banks); default: 0 (endless) \n");
					printf("   \n");
					printf("   -C filename.ini .  configuration file name; for example sis3316_running_parameter.ini \n");
					printf("   \n");
					printf("   -S     ..........  select internal trigger feedback as external trigger\n");
					printf("   -T     ..........  trigger by software\n");
					printf("   \n");
					printf("   -F filename .....  write to one file (file name without .dat )\n");
					printf("   -f filename .....  write to N files  (max. file size is each 1 GByte)\n");
					printf("   \n");
					printf("   -p     ..........  print the parameter only\n");
					printf("   \n");
					printf("   -h     ..........  print this message only\n");
					printf("   \n");
					printf("   \n");
					printf("   date: 24.07.2024 \n");
					printf("   \n");
					printf("   \n");
					exit(1);
			}
		}
    } // if (argc > 1)

	printf("\n");
 
	if(uint_configurationFile_valid_flag == 1) { // take parameter from file

		nof_events                   = gl_sis3316_get_configuration_parameters->uint_nof_events  ;    // each Bank

		// enable external trigger
		for(i_ch=0;i_ch<16;i_ch++) {
			uint_channel_external_trigger_enable_flag[i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_external_trigger_enable[i_ch]  ;
		}

		// channel polarity
		for(i_ch=0;i_ch<16;i_ch++) {
			uint_channel_polarity_invert_flag[i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_polarity_invert[i_ch]  ;
		}

		// Set Gain/Termination
		for(i_ch=0;i_ch<16;i_ch++) {
			uint_channel_range_2V_flag[i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_range_2V[i_ch]  ;
			uint_channel_50ohm_termination_disable_flag[i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_50ohm_termination_disable[i_ch]  ;
		}

		for(i_fpga=0;i_fpga<4;i_fpga++) {
			uint_fpga_analog_ctrl_val[i_fpga] =     (gl_sis3316_get_configuration_parameters->uint_channel_range_2V[(i_fpga*4)+0] & 1) 
												 + ((gl_sis3316_get_configuration_parameters->uint_channel_range_2V[(i_fpga*4)+1] & 1) << 8)
												 + ((gl_sis3316_get_configuration_parameters->uint_channel_range_2V[(i_fpga*4)+2] & 1) << 16)
												 + ((gl_sis3316_get_configuration_parameters->uint_channel_range_2V[(i_fpga*4)+3] & 1) << 24)
												 + ((gl_sis3316_get_configuration_parameters->uint_channel_50ohm_termination_disable[(i_fpga*4)+0] & 1) << 2) 
												 + ((gl_sis3316_get_configuration_parameters->uint_channel_50ohm_termination_disable[(i_fpga*4)+1] & 1) << 10)
												 + ((gl_sis3316_get_configuration_parameters->uint_channel_50ohm_termination_disable[(i_fpga*4)+2] & 1) << 18)
												 + ((gl_sis3316_get_configuration_parameters->uint_channel_50ohm_termination_disable[(i_fpga*4)+3] & 1) << 26) ;
		}
		// set ADC offsets (DAC) 
		for(i_ch=0;i_ch<16;i_ch++) {
			uint_channel_analog_offset_dac_val[i_ch] = gl_sis3316_get_configuration_parameters->uint_channel_adc_offset[i_ch] ;
		}

		// Lemo Out definition
		uint_lemo_out_CO_select  = gl_sis3316_get_configuration_parameters->uint_lemo_out_CO_select;	
		uint_lemo_out_TO_select  = gl_sis3316_get_configuration_parameters->uint_lemo_out_TO_select;	
		uint_lemo_out_UO_select  = gl_sis3316_get_configuration_parameters->uint_lemo_out_UO_select;	



		pre_trigger_delay            =  gl_sis3316_get_configuration_parameters->uint_pre_trigger_delay  ;  //
		sample_length                =  gl_sis3316_get_configuration_parameters->uint_raw_sample_length; //
		sample_start_index           =  0;

		trigger_gate_window_length   =  gl_sis3316_get_configuration_parameters->uint_trigger_gate_window_length ;
		uint_pileup                  =  gl_sis3316_get_configuration_parameters->uint_pileup_window_length;  //
		uint_re_pileup               =  gl_sis3316_get_configuration_parameters->uint_re_pileup_window_length ;  //


		// internal trigger
		for(i_ch=0;i_ch<16;i_ch++) {
			uint_channel_trigger_generation_setup_reg[i_ch] =     ((gl_sis3316_get_configuration_parameters->uint_channel_trigger_pulse_length[i_ch] & 0xff) << 24)
				                                               +  ((gl_sis3316_get_configuration_parameters->uint_channel_trigger_gap[i_ch] & 0xfff) << 12)
				                                               +  ((gl_sis3316_get_configuration_parameters->uint_channel_trigger_peaking[i_ch] & 0xfff) ) ;
	  			//printf(" i_ch %d  uint_channel_trigger_generation_setup_reg = 0x%08x \n",i_ch, uint_channel_trigger_generation_setup_reg[i_ch]);
		}
 				
		for(i_ch=0;i_ch<16;i_ch++) {

			uint_channel_trigger_generation_threshold_reg[i_ch] =     ((gl_sis3316_get_configuration_parameters->uint_channel_trigger_enable[i_ch] & 1) << 31)
				                                                   +  ((gl_sis3316_get_configuration_parameters->uint_channel_trigger_cfd_enable[i_ch] & 1) << 29)
				                                                   +  ((gl_sis3316_get_configuration_parameters->uint_channel_trigger_cfd_enable[i_ch] & 1) << 28)
														           +  0x08000000
				                                                   +  ((gl_sis3316_get_configuration_parameters->uint_channel_trigger_threshold[i_ch] & 0x7FFFFFF) ) ;
	  			//printf(" i_ch %d  uint_channel_trigger_generation_threshold_reg = 0x%08x \n",i_ch, uint_channel_trigger_generation_threshold_reg[i_ch]);

		}

		//energy_p = gl_sis3316_get_configuration_parameters->uint_energy_p_value  ;
		//energy_gap = gl_sis3316_get_configuration_parameters->uint_energy_gap_value  ;
		//energy_tau =  gl_sis3316_get_configuration_parameters->uint_energy_tau_value ; //
		//energy_average =  gl_sis3316_get_configuration_parameters->uint_energy_average_value ; //


		maw_test_buffer_length      = gl_sis3316_get_configuration_parameters->uint_maw_test_buffer_length  ;
		maw_test_buffer_delay       = gl_sis3316_get_configuration_parameters->uint_maw_test_buffer_delay  ;
		maw_test_buffer_enable_flag =  gl_sis3316_get_configuration_parameters->uint_maw_test_buffer_enable_flag ; //

		//maw_test_buffer_select_energy_flag =  gl_sis3316_get_configuration_parameters->uint_maw_test_buffer_select_energy_flag ; //

		//energy_histogram_enable_flag = gl_sis3316_get_configuration_parameters->uint_energy_histogram_enable_flag  ;
		//energy_histogram_only_flag = gl_sis3316_get_configuration_parameters->uint_energy_histogram_only_flag  ;
		//energy_histogram_divider_value =  gl_sis3316_get_configuration_parameters->uint_energy_histogram_divider_value ; //
		//energy_histogram_offset_value =  gl_sis3316_get_configuration_parameters->uint_energy_histogram_offset_value ; //

	}


		if(uint_print_configurationParameterOnly_flag == 1) { ;

  			printf("\n");
	  		printf("nof_events per bank                = %d \n",nof_events);
  			printf("\n");
	  		printf("channel external trigger enable ch1 to ch16      = ");
			for(i_ch=0;i_ch<16;i_ch++) {
	  			printf(" %d",uint_channel_external_trigger_enable_flag[i_ch]);
			}
  			printf("\n");
	  		printf("channel polarity invert ch1 to ch16              = ");
			for(i_ch=0;i_ch<16;i_ch++) {
	  			printf(" %d",uint_channel_polarity_invert_flag[i_ch]);
			}
 			printf("\n");
 			printf("\n");
	  		printf("channel range_2V ch1 to ch16                     = ");
			for(i_ch=0;i_ch<16;i_ch++) {
	  			printf(" %d",uint_channel_range_2V_flag[i_ch]);
			}
  			printf("\n");
	  		printf("channel 50 Ohm termination disable ch1 to ch16   = ");
			for(i_ch=0;i_ch<16;i_ch++) {
	  			printf(" %d",uint_channel_50ohm_termination_disable_flag[i_ch]);
			}
  			printf("\n");
  			printf("\n");
	  		printf("channel ADC offset   = ");
			for(i_ch=0;i_ch<16;i_ch++) {
	  			printf(" %05d",uint_channel_analog_offset_dac_val[i_ch]);
			}
  			printf("\n");
 			printf("\n");
  			printf("lemo_out_CO_select                 = 0x%08x \n", uint_lemo_out_CO_select);
  			printf("lemo_out_TO_select                 = 0x%08x \n", uint_lemo_out_TO_select);
  			printf("lemo_out_UO_select                 = 0x%08x \n", uint_lemo_out_UO_select);


			printf("\n");
	  		printf("pre_trigger_delay                  = %d \n", pre_trigger_delay);
	  		printf("raw_sample_length                  = %d \n", sample_length);
  			printf("\n");
	  		printf("trigger_gate_window_length         = %d \n", trigger_gate_window_length);
	  		printf("pileup_window_length               = %d \n", uint_pileup);
	  		printf("re_pileup_window_length            = %d \n", uint_re_pileup);
  			printf("\n");


 			printf("\n");
	  		printf("channel trigger generation Setup_reg ch1 to ch16      = ");
			for(i_ch=0;i_ch<16;i_ch++) {
				if(i_ch == 8) {printf("\n                                                        ");}
				printf(" 0x%08x",uint_channel_trigger_generation_setup_reg[i_ch]);
			}
			printf("\n");
	  		printf("channel trigger generation Threshold reg ch1 to ch16  = ");
			for(i_ch=0;i_ch<16;i_ch++) {
				if(i_ch == 8) {printf("\n                                                        ");}
	  			printf(" 0x%08x",uint_channel_trigger_generation_threshold_reg[i_ch]);
			}
 			printf("\n");
 			printf("\n");
			
		
			//printf("energy_p_value                     = %d \n",energy_p );
	  		//printf("energy_gap_value                   = %d \n",energy_gap );
	  		//printf("energy_tau_value                   = %d \n",energy_tau );
	  		//printf("energy_average_value               = %d \n",energy_average );
  			//printf("\n");

	  		//printf("maw_test_buffer_length_value       = %d \n",maw_test_buffer_length );
	  		//printf("maw_test_buffer_delay_value        = %d \n",maw_test_buffer_delay );
	  		//printf("maw_test_buffer_enable_flag        = %d \n",maw_test_buffer_enable_flag );
	  		//printf("maw_test_buffer_select_energy_flag = %d \n",maw_test_buffer_select_energy_flag );
  			//printf("\n");

	  		//printf("energy_histogram_enable_flag       = %d \n",energy_histogram_enable_flag );
	  		//printf("energy_histogram_only_flag         = %d \n",energy_histogram_only_flag );
	  		//printf("energy_histogram_divider_value     = %d \n",energy_histogram_divider_value );
	  		//printf("energy_histogram_offset_value      = %d \n",energy_histogram_offset_value );
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
	gl_vme_crate = new sis1100(0);
#endif

#ifdef USB_VME_INTERFACE
USHORT idVendor;
USHORT idProduct;
USHORT idSerNo;
USHORT idFirmwareVersion;
USHORT idDriverVersion;
	// create SIS3150USB vme interface device
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

	gl_vme_crate->udp_reset_cmd();
	gl_vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000); // kill request and grant from other vme interface
	gl_vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x1); // request access to SIS3316 from UDP interface

    return_code = gl_vme_crate->vme_A32D32_read(SIS3316_MODID,&data);
	if(return_code != 0x0) {
		printf("ERROR: gl_vme_crate->vme_A32D32_read: return_code = 0x%08x\n\n", return_code);
		program_stop_and_wait();
		return -1 ;
	}
	printf("return_code = 0X%08x   Module ID = 0X%08x \n",return_code, data);
    return_code = gl_vme_crate->vme_A32D32_read(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL,&data);
	printf("return_code = 0X%08x   ACCESS    = 0X%08x \n",return_code, data);

#endif

// open Vme Interface device
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

#ifdef USB3_VME_INTERFACE
	gl_vme_crate->get_device_informations (&idVendor, &idProduct, &idSerNo, &idDriverVersion, &idFxFirmwareVersion, &idFpgaFirmwareVersion);  //  

	printf("idVendor:               0x%04X\n",idVendor);
	printf("idProduct:              0x%04X\n",idProduct);
	printf("idSerNo:                %d\n",idSerNo);
	printf("idDriverVersion:        0x%04X\n",idDriverVersion);
	printf("idFxFirmwareVersion:    0x%04X\n",idFxFirmwareVersion);
	printf("idFpgaFirmwareVersion:  0x%04X\n",idFpgaFirmwareVersion);
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

	return_code = gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_MODID, &data);  
	//printf("vme_A32D32_read: data = 0x%08x     return_code = 0x%08x\n", data, return_code);


	// kill request and grant from vme interface (in case of use using etehrnet interface)
	gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000);
	// arbitrate
	gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 1);


	sis3316_not_OK = 0 ;
	if (return_code == 0) {
		printf("SIS3316_MODID                    = 0x%08x\n\n", data);

		gl_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
		gl_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
		gl_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
		gl_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits

		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH1_4_FIRMWARE_REG, &data);  
		printf("SIS3316_ADC_CH1_4_FIRMWARE_REG   = 0x%08x \n", data);
		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH5_8_FIRMWARE_REG, &data);  
		printf("SIS3316_ADC_CH5_8_FIRMWARE_REG   = 0x%08x \n", data);
		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH9_12_FIRMWARE_REG, &data);  
		printf("SIS3316_ADC_CH9_12_FIRMWARE_REG  = 0x%08x \n", data);
		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ADC_CH13_16_FIRMWARE_REG, &data);  
		printf("SIS3316_ADC_CH13_16_FIRMWARE_REG = 0x%08x \n\n", data);


#define TEST_DEBUG
#ifdef TEST_DEBUG
		gl_vme_crate->vme_A32D32_write( module_base_addr + SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS, 0xE0E0E0E0);  // clear error Latch bits 
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
#endif
	}
	else {
		printf("SIS3316_MODID                  = 0x%08x     return_code = 0x%08x\n", data, return_code);
		program_stop_and_wait() ;
		return -1 ;
	}


	sis3316_adc  *sis3316_adc1 ;
	sis3316_adc1 = new sis3316_adc( gl_vme_crate, module_base_addr);
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

	sis3316_adc1->register_write(SIS3316_KEY_DISARM, 0) ;  // disarm
	sis3316_adc1->register_write(SIS3316_KEY_RESET, 0) ;  // reset
	sis3316_adc1->adc_spi_reg_enable_adc_outputs() ;  // necessary after reset

	double double_clock_configure_fft_frequency;
	unsigned int clock_freq_choice ;
	unsigned int clock_N1div, clock_HSdiv ;

#if 0 // only SIS3316
	if (sis3316_adc1->adc_125MHz_flag == 0) { // 250 MHz
		// 250.000 MHz
		clock_N1div      =  4  ;
		clock_HSdiv      =  5  ;
		iob_delay_value  =  0x1002 ; //
	}
	else {
		// 125.000 MHz
		clock_N1div      =  8  ;
		clock_HSdiv      =  5  ;
		iob_delay_value  =  0x1008 ; //
	}
#endif

// new with SIS3316 / SIS3316-2
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

	//printf("iob_delay_value = 0x%08x \n", iob_delay_value);


	sis3316_adc1->change_frequency_HSdiv_N1div(0, clock_HSdiv, clock_N1div) ;
	sis3316_adc1->configure_adc_fpga_iob_delays(iob_delay_value) ;  // necessary after changing/seting clock



/******************************************/

// Gain/Termination  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[0]); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[1] ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[2] ); // 
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[3] ); //  


/******************************************/

//  set ADC offsets (DAC)  
	// adc DAC offset setup
	for (i_ch=0;i_ch<16; i_ch++) {
		sis3316_adc1->adc_dac_offset_ch_array[i_ch] = uint_channel_analog_offset_dac_val[i_ch] ; //  
	}
	return_code=sis3316_adc1->write_all_adc_dac_offsets() ;
	if (return_code != 0) {
		printf("Error write_all_adc_dac_offsets: return_code = 0x%08x \n", return_code);
	}


/******************************************/

	// Select LEMO Output "Co"   
	return_code = sis3316_adc1->register_write(SIS3316_LEMO_OUT_CO_SELECT_REG, uint_lemo_out_CO_select ); //  

	// Select LEMO Output "To"   
	return_code = sis3316_adc1->register_write(SIS3316_LEMO_OUT_TO_SELECT_REG, uint_lemo_out_TO_select ); //  

	// Select LEMO Output "UO"
	return_code = sis3316_adc1->register_write(SIS3316_LEMO_OUT_UO_SELECT_REG, uint_lemo_out_UO_select ); //

/******************************************/
	// channel Header
	data = module_base_addr + 0x0 ;
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG, data); //  
	data = module_base_addr + 0x00400000 ;
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG, data ); //  
	data = module_base_addr + 0x00800000 ;
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG, data ); // 
	data = module_base_addr + 0x00C00000 ;
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG, data ); //  

/******************************************/


	// internal Trigger generation
	
	for (i_ch=0;i_ch<16; i_ch++) {
		sis3316_adc1->internal_trigger_generation_setup(uint_channel_trigger_generation_threshold_reg[i_ch], uint_channel_he_trigger_generation_threshold_reg[i_ch], uint_channel_trigger_generation_setup_reg[i_ch], i_ch) ;  //  
	}

/******************************************/


	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length

	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length



	//pre_trigger_delay =  0;
	if (pre_trigger_delay > 16378) {
		pre_trigger_delay  = 16378 ;
	}
	//pre_trigger_delay = pre_trigger_delay + 0x8000 ; // set "Additional Delay of Fir Trigger P+G" Bit  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //  


	//pileup ;  
	data = ((uint_re_pileup & 0xffff) << 16) + (uint_pileup & 0xffff) ;

	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_PILEUP_CONFIG_REG, data ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_PILEUP_CONFIG_REG, data ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_PILEUP_CONFIG_REG, data ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_PILEUP_CONFIG_REG, data ); //  

// internal Trigger Delay
	data = ((uint_internal_trigger_delay & 0xff) << 24) + ((uint_internal_trigger_delay & 0xff) << 16) + ((uint_internal_trigger_delay & 0xff) << 8) + (uint_internal_trigger_delay & 0xff);

	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_INTERNAL_TRIGGER_DELAY_CONFIG_REG, data); //  


/******************************************/

	// Enable LEMO Input "TI" as Trigger External Trigger 
	data = 0x10 ; // Enable Nim Input "TI"
	//data = data + 0x40 ; // Enable Nim Input "TI" level
	return_code = sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data ); //  

	for(i_fpga=0;i_fpga<4;i_fpga++) {
		uint_fpag_config_reg_value[i_fpga] =       ((uint_channel_external_trigger_enable_flag[(i_fpga*4)+0] & 1) << 3) 
												 + ((uint_channel_external_trigger_enable_flag[(i_fpga*4)+1] & 1) << 11)
												 + ((uint_channel_external_trigger_enable_flag[(i_fpga*4)+2] & 1) << 19)
												 + ((uint_channel_external_trigger_enable_flag[(i_fpga*4)+3] & 1) << 27)  
			                                     +  (uint_channel_polarity_invert_flag[(i_fpga*4)+0] & 1) 
												 + ((uint_channel_polarity_invert_flag[(i_fpga*4)+1] & 1) << 8)
												 + ((uint_channel_polarity_invert_flag[(i_fpga*4)+2] & 1) << 16)
												 + ((uint_channel_polarity_invert_flag[(i_fpga*4)+3] & 1) << 24) ;
	}


//  Event Configuration
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, uint_fpag_config_reg_value[0] ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, uint_fpag_config_reg_value[1] ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, uint_fpag_config_reg_value[2] ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, uint_fpag_config_reg_value[3] ); //  

	if (maw_test_buffer_enable_flag == 0) {
		maw_test_buffer_length =  0 ;
	}


// data format
	header_length = 3;	
//	header_accu_6_values_offset = 2 ;
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

	event_length = (header_length + (sample_length / 2)  + maw_test_buffer_length);
	//max_req_nof_32bit_words = nof_events_pro_bank * event_length ; // max_request is limited by request nof_events
	max_req_nof_32bit_words = SIS3316_ADC_MEMORY_BANK_32BIT_SIZE ; // max_request is limited by Memory Banks_Size

	data = data + (data << 8) + (data << 16) + (data << 24);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG, data ); 
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_DATAFORMAT_CONFIG_REG, data ); 
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_DATAFORMAT_CONFIG_REG, data ); 
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_DATAFORMAT_CONFIG_REG, data ); 

// MAW Test Buffer configuration
	data = maw_test_buffer_length + (maw_test_buffer_delay << 16) ;
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_MAW_TEST_BUFFER_CONFIG_REG, data ); 
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_MAW_TEST_BUFFER_CONFIG_REG, data ); 
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_MAW_TEST_BUFFER_CONFIG_REG, data ); 
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_MAW_TEST_BUFFER_CONFIG_REG, data ); 


	//address_threshold = 200;	
	address_threshold = (nof_events * event_length) - 1 ;  //  
	address_threshold = address_threshold + 0x80000000 ;  //  suppress saving following hits/events if addresshold flag is set  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG, address_threshold ); //  
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_ADDRESS_THRESHOLD_REG, address_threshold); //   
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_ADDRESS_THRESHOLD_REG, address_threshold ); //     
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_ADDRESS_THRESHOLD_REG, address_threshold ); //  

	
	gl_graph_raw->sis3316_draw_XYaxis (sample_length); // clear and draw X/Y
	//gl_graph_raw->sis3316_draw_XYaxis (20); // clear and draw X/Y



	if (sis3316_adc1->adc_125MHz_flag == 0) {
		// 250  MHz
		file_header_indentifier      =  0  ;
	}
	else {
		// 125 MHz
		file_header_indentifier      =  1  ;
	}


	// select all internal triggers as source for "Internal Trigger Feedback as External Trigger"
	return_code = sis3316_adc1->register_write(SIS3316_INTERNAL_TRIGGER_FEEDBACK_SELECT_REG, 0xffff );

	// enable external (global) functions
	data = 0x100 ; // enable "external Trigger function" (NIM In, if enabled and VME key write)
	data = data + 0x400 ; // enable "external Timestamp clear function" (NIM In, if enabled and VME key write)
	data = data + 0x8000 ; // enable "external_trigger_function_disable_if_busy"  

	if(uint_select_internalTrigger_as_extertnalTrigger_flag == 1) {
		data = data + 0x4000 ; // Select Internal Trigger Feedback as External Trigger Enable
	}
	return_code = sis3316_adc1->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, data );  
	

	printf("Start Multievent \n");

	loop_counter=0;
	bank_buffer_counter = 0 ;
	ch_event_counter = 0;
	// get Start Time
	gettimeofday(&time_start, NULL);



	// Clear Timestamp  */
	return_code = sis3316_adc1->register_write(SIS3316_KEY_TIMESTAMP_CLEAR , 0);  //   
	
	// Start Readout Loop  */
	//Note: Start sampling on Bank on alternate Bank, check Bit 24 in the register "previous Bank sample address" 
	sis3316_adc1->register_read( SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);
	if((data & 0x1000000) == 0x1000000 ) { 	// bank2 flag is set ?
		//printf("bank2 flag is set\n"); // start sampling an alternate bank
		bank1_armed_flag = 1 ;
		return_code = sis3316_adc1->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK1, 0 ); //    Arm
	}	
	else {
		//printf("bank2 flag is not set\n"); // start sampling an alternate bank
		bank1_armed_flag = 0 ;
		return_code = sis3316_adc1->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK2, 0 ); // //  Arm
	}

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
				if(uint_software_key_trigger_flag == 1) {
					return_code = sis3316_adc1->register_write(SIS3316_KEY_TRIGGER , 0);  //  Trigger
					usleep(1000); //   
				}
				gSystem->ProcessEvents();  // handle GUI events
				poll_counter = 0 ;
    		}
			gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, &data);  
			usleep(10); //   
			//printf("in Loop:   SIS3316_ACQUISITION_CONTROL_STATUS = 0x%08x     \n", data);

#ifdef WINDOWS
		} while (((data & 0x80000) == 0x0) && (gl_stopReq == FALSE))  ; // Address Threshold reached ?
		if (gl_stopReq == TRUE) { break ; }
#else
		} while (((data & 0x80000) == 0x0) )  ; // Address Threshold reached ?
#endif

		if (bank1_armed_flag == 1) {		
			return_code = sis3316_adc1->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK2 , 0);  //  Arm Bank2
			bank1_armed_flag = 0; // bank 2 is armed
			printf("SIS3316_KEY_DISARM_AND_ARM_BANK2 \n");
		}
		else {
			return_code = sis3316_adc1->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK1 , 0);  //  Arm Bank1
			bank1_armed_flag = 1; // bank 1 is armed
			printf("SIS3316_KEY_DISARM_AND_ARM_BANK1 \n");
		}

		gl_graph_raw->sis3316_draw_XYaxis (sample_length); // clear and draw X/Y

		for (i_ch=0; i_ch<16; i_ch++) {
			// read channel events 
			return_code = sis3316_adc1->read_DMA_Channel_PreviousBankDataBuffer(bank1_armed_flag /*bank2_read_flag*/, i_ch /* 0 to 15 */, max_req_nof_32bit_words, &got_nof_32bit_words, gl_rblt_data ) ; // read maximun (all) events
			//printf("read_DMA_Channel_PreviousBankDataBuffer: i_ch %d  got_nof_32bit_words = 0x%08x  return_code = 0x%08x\n",i_ch,  got_nof_32bit_words, return_code);
			if (return_code != 0) {
				printf("read_DMA_Channel_PreviousBankDataBuffer: return_code = 0x%08x\n", return_code);
#ifdef WINDOWS
				gl_stopReq = TRUE;
#endif
			}
			ch_event_counter = (got_nof_32bit_words  / event_length) ;

#ifdef TIMESTAMP_TEST
			if (ch_event_counter > 0) {
				// print first Timestamp
				uint_upper_timestamp = (gl_rblt_data[0] >> 16) & 0xffff; //
				uint_lower_timestamp = gl_rblt_data[1]; //
				double_time_stamp_ns = ((4294967296.0 * (double)uint_upper_timestamp) + ((double)uint_lower_timestamp)) * double_ns_per_sample_clk;
				printf("Ch%2d Timestamp of first Event:  = 0x%04x   0x%08x      %.9f sec \n", i_ch, uint_upper_timestamp, uint_lower_timestamp, (double_time_stamp_ns / 1000000000.0));
			}
#endif

			if (ch_event_counter > 0) {
				// file write
				if (uint_WriteData_to_File_OpenFlag == 1) {   ; //  
					uint_WritenData_to_File_32bit_words += SIS3316_WriteBankChannelHeaderToDataFile (file_WriteData_to_File_Pointer, file_header_indentifier, bank_buffer_counter, i_ch, ch_event_counter , event_length, maw_test_buffer_length, 0 /* reserved */) ;
					uint_WritenData_to_File_32bit_words += SIS3316_WriteBankChannelEventBufferToDataFile (file_WriteData_to_File_Pointer, gl_rblt_data, got_nof_32bit_words)  ;
				}

				//printf("i_ch = %d   sample_length = %d  \n", i_ch + 1, sample_length);
				//printf("i_ch = %d   event_length  = %d  \n", i_ch + 1, event_length);
				//printf("i_ch = %d   header_length = %d  \n", i_ch + 1, header_length);

//#ifdef plot_graph
				// plot events
				for (i=0; i<ch_event_counter; i++) {
					if (i==0) { // plot ony 1. event
						gl_graph_raw->sis3316_draw_chN (sample_length, &gl_rblt_data[i*(event_length) + header_length], i_ch); //  
						//printf("i_ch = %d   ->  Internal Trigger Flag = %d  \n",i_ch+1, (gl_rblt_data[i*(event_length) + header_length - 1] >> 26) & 1 );				
						//printf("i_ch = %d   ->  Information           = 0x%02x  \n",i_ch+1, ((gl_rblt_data[i*(event_length) + 3] >> 24) ) );
					}
				}
//#endif
			}

		}

		loop_counter++;
		bank_buffer_counter++;
#ifdef test
		for (i_ch=1; i_ch<16; i_ch++) {
			if(ch_event_counter_array[0] != ch_event_counter_array[i_ch]) {
				ch_event_counter_error++;
				printf("%d     %d      %d\n",i_ch, ch_event_counter_array[0],ch_event_counter_array[i_ch]);
			}
		}
#endif		
		printf("bank_buffer_counter = %d     \n",bank_buffer_counter);
		printf("ch_event_counter    = %d     \n", ch_event_counter);
		printf("ch_event_counter_error    = %d     \n", ch_event_counter_error);
		printf("\n");
		gSystem->ProcessEvents();  // handle GUI events

		gettimeofday(&time_actual, NULL);
	    double_run_time_sec = (time_actual.tv_sec - time_start.tv_sec)  ;
	    uint_run_time_sec   = (unsigned int)double_run_time_sec;
	    printf("uint_run_time_sec = %d\n", uint_run_time_sec);




#ifdef WINDOWS
	} while(((loop_counter < stop_after_loop_counts) || (stop_after_loop_counts == 0)) && ((uint_run_time_sec < stop_after_seconds) || (stop_after_seconds == 0)) && (gl_stopReq == FALSE) );
#else
	} while(((loop_counter < stop_after_loop_counts) || (stop_after_loop_counts == 0)) && ((uint_run_time_sec < stop_after_seconds) || (stop_after_seconds == 0))  );
#endif

    printf("uint_run_time_sec = %d\n", uint_run_time_sec);

	// disarm logic
	return_code = sis3316_adc1->register_write(SIS3316_KEY_DISARM , 0);  //   

	// close file (if it was open)
 	if (uint_WriteData_to_File_OpenFlag == 1) {   ; //  
		fclose(file_WriteData_to_File_Pointer);
		uint_WriteData_to_File_OpenFlag = 0 ;
	}

	// wait until Ctrl C
	printf("press CTRL-C to exit\n");
#ifdef WINDOWS
	gl_stopReq = FALSE;
#endif
	do {
		gSystem->ProcessEvents();  // handle GUI events
		//printf("gl_stopReq    = %d     \n", gl_stopReq);
		usleep(100000);
#ifdef WINDOWS
	} while (gl_stopReq == FALSE) ;
#else
	} while (1) ;
#endif


	return 0;
}

/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/
/**********************************************************************************************************************************/




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



/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/
/***********************************************************************************************************************************************/


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




#ifdef WINDOWS
int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}
#endif
