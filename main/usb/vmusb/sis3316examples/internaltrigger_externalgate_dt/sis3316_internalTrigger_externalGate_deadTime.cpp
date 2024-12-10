/******************************************************************************/
/*                                                                            */
/*  Filename: sis3316_ch1_to_ch16_internal_trigger_externalGate_deadtime.cpp  */
/*                                                                            */
/*  Funktion: Internal Trigger with External Gate                             */
/*            All parameter are defined in source code like:                  */
/*            - strcpy(gl_cmd_ip_string, "192.168.1.100"); // SIS3316 IP addr */
/*            - stop_after_loop_counts       = 0;    	// 0: endless         */
/*            - nof_events                   = 1000;    // each Bank          */
/*            - uint_DataEvent_OpenFlag      =  0 ;  // not save to file      */
/*            - ......                                                        */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/*  Autor:                TH                                                  */
/*  date:                 17.09.2019                                          */
/*  last modification:    25.07.2024    (SIS3316-2 adaptation)                */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  SIS  Struck Innovative Systeme GmbH                                       */
/*                                                                            */
/*  Harksheider Str. 102A                                                     */
/*  22399 Hamburg                                                             */
/*                                                                            */
/*  Tel. +49 (0)40 60 87 305 0                                                */
/*                                                                            */
/*  https://www.struck.de                                                     */
/*                                                                            */
/*  © 2024                                                                    */
/*                                                                            */
/******************************************************************************/

#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethnernet UDP)


#define CERN_ROOT_PLOT

#ifdef CERN_ROOT_PLOT
   #include "TApplication.h"
   #include "TObject.h"
   #include "TH1.h"
   #include "TH1D.h"
   #include "TH1F.h"
   #include "TH2D.h"
   #include "TGraph.h"
   #include "TMultiGraph.h"
   #include "TMath.h"
   #include "TCanvas.h"
    //#include "TRandom.h"
   //#include "TThread.h"
   #include <TSystem.h>
   #include "TLatex.h"
   #include "TGNumberEntry.h"
   #include "TRootEmbeddedCanvas.h"

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





#include "sis3316_class.h"
//sis3316_adc *gl_sis3316_adc1 ;

#define MAX_NOF_SIS3316_ADCS			1
//#define BROADCAST_BASE_ADDR				0x40000000
#define FIRST_MODULE_BASE_ADDR			0x36000000
#define MODULE_BASE_OFFSET				0x01000000

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
//#define MAX_NUMBER_LWORDS_64MBYTE			0x1000000       /* 64MByte */

unsigned int gl_rblt_data[MAX_NUMBER_LWORDS_64MBYTE] ;

BOOL gl_stopReq = FALSE;
FILE *gl_FILE_DataEvenFilePointer           ;


char gl_cmd_ip_string[64];


unsigned int gl_ch1_gate_bit_buf[0x10000];
unsigned int gl_ch2_gate_bit_buf[0x10000];
unsigned int gl_ch3_gate_bit_buf[0x10000];



/*===========================================================================*/
/* Prototypes			                               		  			     */
/*===========================================================================*/

int WriteBufferHeaderCounterNofChannelToDataFile (unsigned int buffer_no,unsigned int nof_events, unsigned int event_length);
int WriteEventsToDataFile (unsigned int* memory_data_array, unsigned int nof_write_length_lwords);

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
//unsigned int first_mod_base, nof_modules   ;
unsigned int module_base_addr   ;
unsigned int data;
unsigned int i, module_index;
unsigned int loop_counter;
unsigned int bank_buffer_counter ;

unsigned int uint_channel_event_counter[16];



unsigned int got_nof_32bit_words;
unsigned int sample_length;
unsigned int sample_start_index;
unsigned int trigger_gate_window_length;
unsigned int address_threshold;


unsigned int external_veto_gate_delay;
unsigned int external_trigger_function_deadtime;

unsigned int nof_events;
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


unsigned int i_gate;
unsigned int gate_length[8];
unsigned int gate_start_index[8];

unsigned int uint_DataEvent_OpenFlag ;

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


unsigned int uint_channel_external_gate_enable_flag[16];
unsigned int uint_channel_external_trigger_enable_flag[16];
unsigned int uint_channel_internal_trigger_enable_flag[16];
unsigned int uint_fpag_config_reg_value[4];


/******************************************************************************************/
/*                                                                                        */
/*  SIS3316 program parameter                                                             */
/*                                                                                        */
/******************************************************************************************/
	// Ethernet UDP IP address
	strcpy(gl_cmd_ip_string, "212.60.16.200"); // SIS3316 IP address
	strcpy(gl_cmd_ip_string, "sis3316-0373"); // SIS3316 IP address
	strcpy(gl_cmd_ip_string, "192.168.1.100"); // SIS3316 IP address
	strcpy(gl_cmd_ip_string, "sis3316-1002"); // SIS3316 IP address

	// VME module base address
	module_base_addr = FIRST_MODULE_BASE_ADDR ;

	stop_after_loop_counts       = 10;    	// 0: endless
	//stop_after_loop_counts       = 0;    	// 0: endless
	//nof_events                   = 1000 ;    // each Bank
	nof_events                   = 1000;    // each Bank

	// file write
	uint_DataEvent_OpenFlag      =  0 ;  // not save to file
	//uint_DataEvent_OpenFlag      =  1 ;  // save to file


// default Configuration paramters
	// Set Gain/Termination
	for (i_ch = 0; i_ch < 16; i_ch++) {
		//uint_channel_range_2V_flag[i_ch] = 1;  // 2 V Range
		uint_channel_range_2V_flag[i_ch] = 0;  // 5 V Range
		//uint_channel_50ohm_termination_disable_flag[i_warningch] = 1; // disable 50 Ohm Termination
		uint_channel_50ohm_termination_disable_flag[i_ch] = 0; // enable 50 Ohm Termination
	}
	uint_channel_50ohm_termination_disable_flag[0] = 1; // ch1: disable 50 Ohm Termination -> is used as "Spy" for the External Trigger/Gate signal

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
		uint_channel_analog_offset_dac_val[i_ch] = 0x8000; // middle
	}
	uint_channel_analog_offset_dac_val[0] = 20000; //  0V at ~5300
	uint_channel_analog_offset_dac_val[1] = 11000; //  0V at ~3100
	uint_channel_analog_offset_dac_val[2] = 1000; //  0V at ~700

// channel polarity
	for (i_ch = 0; i_ch < 16; i_ch++) {
		uint_channel_polarity_invert_flag[i_ch] = 1; // all inverted
	}
	//uint_channel_polarity_invert_flag[1] = 0; // ch2 not inverted


// Event Configuration Register parameters

	// channel external gate enable(1)/disable(0)
	for (i_ch = 0; i_ch < 16; i_ch++) {
		//uint_channel_external_gate_enable_flag[i_ch] = 1; // all enable
		uint_channel_external_gate_enable_flag[i_ch] = 0; //gate disable
	}

	// channel external trigger enable(1)/disable(0)
	for (i_ch = 0; i_ch < 16; i_ch++) {
		uint_channel_external_trigger_enable_flag[i_ch] = 0; // all disable
	}
	//uint_channel_external_trigger_enable_flag[1] = 1; // enable ch2

	// channel internal trigger enable(1)/disable(0)
	for (i_ch = 0; i_ch < 16; i_ch++) {
		uint_channel_internal_trigger_enable_flag[i_ch] = 1; // all enable
	}
	//uint_channel_internal_trigger_enable_flag[1] = 0; // disable ch2



// internal Trigger generation
	uint_trigger_pulse_length = 0x10;
	uint_trigger_gap          = 8;
	uint_trigger_peaking      = 10;
	uint_trigger_threshold_value = 100 * uint_trigger_peaking; // 2V Range -> 1 bin = 122 uV
															   // 100 --> 2V Range --> 12.2 mV
	//uint_trigger_threshold_value = 1000 * uint_trigger_peaking; // 2V Range -> 1 bin = 122 uV
															   // 1000 --> 2V Range --> 122 mV

	for (i_ch = 0; i_ch < 16; i_ch++) {
		uint_channel_trigger_generation_setup_reg[i_ch] = ((uint_trigger_pulse_length & 0xff) << 24)
														+ ((uint_trigger_gap & 0xfff) << 12)
														+ ((uint_trigger_peaking & 0xfff));
		uint_channel_trigger_generation_threshold_reg[i_ch] = 0x80000000 + 0x30000000  //
															+ 0x8000000  //  MAW offset
															+ (uint_trigger_threshold_value & 0x7FFFFFF) ; //  Threshold
		uint_channel_he_trigger_generation_threshold_reg[i_ch] = 0x0; //
	}



//

	gate_start_index[0] = 100;
	gate_length[0] = 100-1;

	gate_start_index[1] = 228;
	gate_length[1] = 1-1;

	gate_start_index[2] = 229;
	gate_length[2] = 1-1;

	gate_start_index[3] = 230;
	gate_length[3] = 1-1;

	gate_start_index[4] = 416;
	gate_length[4] = 1-1;

	gate_start_index[5] = 417;
	gate_length[5] = 1-1;


// data format
	header_accu_6_values_enable_flag = 1 ;
	header_accu_2_values_enable_flag = 0 ;
	header_maw_3_values_enable_flag  = 0 ;

	maw_test_buffer_enable_flag = 0 ;
	maw_test_buffer_length = 0;
	maw_test_buffer_delay = 0;    // max 1022

	if (maw_test_buffer_delay > 1022) {
		maw_test_buffer_delay = 1022;
	}




	// trigger_gate_window_length and sample length
	trigger_gate_window_length = 650;
	sample_length = 624;
	sample_start_index = 0;
	pre_trigger_delay = 100;

	// define TI Deadtime
	external_veto_gate_delay = 0;



	external_trigger_function_deadtime = ((trigger_gate_window_length + 250 ) / 16); // + 1us
	if (external_trigger_function_deadtime > 0x3fff) { external_trigger_function_deadtime = 0x3fff; }
	external_trigger_function_deadtime = external_trigger_function_deadtime + 0x4000; // Enable Deadtime with internal Address Threshold Flag
	//external_trigger_function_deadtime = external_trigger_function_deadtime + 0x8000; // Enable Deadtime with FP-Bus Address Threshold Flag


	// Note: internal_fir_trigger_delay value is two times of clock
	//internal_fir_trigger_delay = 72;  //  72 * 2 * 4ns = 576ns for ~400ns TO-Output to Gate-IN delay (Note 1: max 255; Note 2: value has to muiltiply by 2 clocks)
	internal_fir_trigger_delay = 0;  //  72 * 2 * 4ns = 576ns for ~400ns TO-Output to Gate-IN delay (Note 1: max 255; Note 2: value has to muiltiply by 2 clocks)

	pre_trigger_delay = pre_trigger_delay + uint_trigger_peaking + (uint_trigger_peaking >> 1) + uint_trigger_gap + 24;  // 16 is additional delay for 50% CFD trigger --> see edge at index 10
	pre_trigger_delay = pre_trigger_delay + (2 * internal_fir_trigger_delay);


// pileups
	uint_pileup    = trigger_gate_window_length;  //
	uint_re_pileup = trigger_gate_window_length;  //



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

	gl_vme_crate->udp_reset_cmd();
	gl_vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000); // kill request and grant from other vme interface
	gl_vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x1); // request access to SIS3316 from UDP interface

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

	return_code = gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_MODID, &data);
	printf("vme_A32D32_read: module_base_addr = 0x%08x      data = 0x%08x     return_code = 0x%08x\n", module_base_addr, data, return_code);


	// kill request and grant from vme interface (in case of use using ethernet interface)
	gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000);
	// arbitrate
	gl_vme_crate->vme_A32D32_write(module_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 1);



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

	//if (sis3316_not_OK != 0) {
	//	printf("sis3316_not_OK                 \n");
	//	program_stop_and_wait() ;
	//}


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
	sis3316_adc1->configure_adc_fpga_iob_delays(iob_delay_value);  // necessary after changing/seting clock




/******************************************/

// Gain/Termination
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[0]); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[1]); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[2]); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_ANALOG_CTRL_REG, uint_fpga_analog_ctrl_val[3]); //

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
	return_code = sis3316_adc1->register_write(SIS3316_LEMO_OUT_CO_SELECT_REG, uint_lemo_out_CO_select); //

	// Select LEMO Output "To"
	return_code = sis3316_adc1->register_write(SIS3316_LEMO_OUT_TO_SELECT_REG, uint_lemo_out_TO_select); //

	// Select LEMO Output "UO"
	return_code = sis3316_adc1->register_write(SIS3316_LEMO_OUT_UO_SELECT_REG, uint_lemo_out_UO_select); //


/******************************************/

	// channel Header
	data = module_base_addr + 0x0;
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG, data); //
	data = module_base_addr + 0x00400000;
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG, data); //
	data = module_base_addr + 0x00800000;
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG, data); //
	data = module_base_addr + 0x00C00000;
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG, data); //





/******************************************/


	// internal Trigger generation

	for (i_ch = 0; i_ch < 16; i_ch++) {
		sis3316_adc1->internal_trigger_generation_setup(uint_channel_trigger_generation_threshold_reg[i_ch], uint_channel_he_trigger_generation_threshold_reg[i_ch], uint_channel_trigger_generation_setup_reg[i_ch], i_ch);  //
	}




/******************************************/



	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG, (trigger_gate_window_length -2 & 0xffff) ); // trigger_gate_window_length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG, (trigger_gate_window_length -2 & 0xffff) ); // trigger_gate_window_length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG, (trigger_gate_window_length -2 & 0xffff) ); // trigger_gate_window_length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG, (trigger_gate_window_length -2 & 0xffff) ); // trigger_gate_window_length

/******************************************/

	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length



/******************************************/


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
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //

/******************************************/

	//pileup ;
	data = ((uint_re_pileup & 0xffff) << 16) + (uint_pileup & 0xffff) ;

	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_PILEUP_CONFIG_REG, data ); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_PILEUP_CONFIG_REG, data ); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_PILEUP_CONFIG_REG, data ); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_PILEUP_CONFIG_REG, data ); //



/******************************************/


/******************************************/

#ifdef NOT_USED
#define SETUP_NO_VETO
//#define SETUP_UI_AS_VETO
//#define SETUP_UI_AS_GATE
//#define SETUP_TI_AS_VETO
//#define SETUP_TI_AS_Gate


#ifdef SETUP_NO_VETO
	//  Event Configuration
	data = 0x04040404; // internal trigger ch4 , ch3, ch2 ch1
	//data = 0x02020202; // internal sum trigger ch4 , ch3, ch2 ch1
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, data ); //
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, data ); //

	data = 0x0 ; //
	data = data + 0x400; // enable "external Timestamp clear function" (NIM In, if enabled and VME key write)
	return_code = sis3316_adc1->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, data );
#endif


#ifdef SETUP_UI_AS_VETO
// Enable LEMO Input "UI" as Local Veto function
	data = 0x1000 ; // Enable Nim Input "UI" as Local Veto function
	return_code = sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data ); //

	//  Event Configuration
	data = 0x04040404 ; // internal trigger ch4 , ch3, ch2 ch1
	data = data + 0x80808080 ; // external Veto ch4 , ch3, ch2 ch1
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, data ); //

	// enbale external (global) functions
	data = 0x800 ; // enable "Local Veto function" as Veto
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, data );
#endif

#ifdef SETUP_UI_AS_GATE
// Enable LEMO Input "UI" as Local Veto function
	data = 0x1000 ; // Enable Nim Input "UI" as Local Veto function
	return_code = sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data ); //

	//  Event Configuration
	data = 0x04040404 ; // internal trigger ch4 , ch3, ch2 ch1
	data = data + 0x40404040 ; // external Gate ch4 , ch3, ch2 ch1
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, data ); //

	// enbale external (global) functions
	data = 0x800 ; // enable "Local Veto function" as Veto
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, data );
#endif

#ifdef SETUP_TI_AS_VETO
// Enable LEMO Input "TI" as Local Veto function
	data = 0x10 ; // Enable Nim Input "TI" as Trigger function
	data = data + 0x40 ; // set Level sensitiv
	return_code = sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data ); //

	//  Event Configuration
	data = 0x04040404 ; // internal trigger ch4 , ch3, ch2 ch1
	data = data + 0x80808080 ; // external Veto ch4 , ch3, ch2 ch1
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, data ); //

	// enbale external (global) functions
	data = 0x200 ; // enable "External Trigger function" as Veto
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, data );
#endif


#ifdef SETUP_TI_AS_Gate
// Enable LEMO Input "TI" as Local Veto function
	data = 0x10 ; // Enable Nim Input "TI" as Trigger function
	data = data + 0x40 ; // set Level sensitiv
	return_code = sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data ); //

	//  Event Configuration
	data = 0x04040404 ; // internal trigger ch4 , ch3, ch2 ch1
	data = data + 0x40404040 ; // external Gate ch4 , ch3, ch2 ch1
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, data ); //
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, data ); //

	// enbale external (global) functions
	data = 0x200 ; // enable "External Trigger function" as Veto
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, data );
#endif

#endif



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




	//  Test Mode: see internal Gate as Bit0 of ADC Data
	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_GATE_TEST_MODE_REG, 1); //
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_GATE_TEST_MODE_REG, 1); //
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_GATE_TEST_MODE_REG, 1); //
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_GATE_TEST_MODE_REG, 1); //

	//  Event Configuration
	sis3316_adc1->register_write(SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, uint_fpag_config_reg_value[0]); //
	sis3316_adc1->register_write(SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, uint_fpag_config_reg_value[1]); //
	sis3316_adc1->register_write(SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, uint_fpag_config_reg_value[2]); //
	sis3316_adc1->register_write(SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, uint_fpag_config_reg_value[3]); //

	// define TI Deadtime

	if (external_veto_gate_delay > 2044) { external_veto_gate_delay = 2044; }

	data = ((external_trigger_function_deadtime & 0xffff) << 16) + (external_veto_gate_delay & 0xffff); //
	return_code = sis3316_adc1->register_write(SIS3316_EXTERNAL_VETO_GATE_DELAY_REG, data);

//#ifdef SETUP_TI_AS_Gate
	// Enable LEMO Input "TI" as Local Veto function
	data = 0x10; // Enable Nim Input "TI" as Trigger function
	data = data + 0x40; // set Level sensitiv
 	data = data + 0x4000; // enable TI Deadtime logic
	return_code = sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data); //


	// enable external (global) functions
	data = 0x200; // enable "External Trigger function" as Veto
	data = data + 0x400; // enable "External Timestamp-Clear function" as Timestamp-Clear
	return_code = sis3316_adc1->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, data);



#ifdef raus
			// SETUP_UI_AS_GATE
// Enable LEMO Input "UI" as Local Veto function
	data = 0x1000; // Enable Nim Input "UI" as Local Veto function
	return_code = sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data); //


	// enbale external (global) functions
	data = 0x800; // enable "Local Veto function" as Veto
	data = data + 0x400; // enable "External Timestamp-Clear function" as Timestamp-Clear
	return_code = sis3316_adc1->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, data);
#endif


	if (maw_test_buffer_enable_flag == 0) {
		maw_test_buffer_length =  0 ;
	}



/******************************************/

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

	event_length = (header_length + (sample_length / 2) + maw_test_buffer_length);



// MAW Test Buffer configuration
	data = maw_test_buffer_length + (maw_test_buffer_delay << 16);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH1_4_MAW_TEST_BUFFER_CONFIG_REG, data);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH5_8_MAW_TEST_BUFFER_CONFIG_REG, data);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH9_12_MAW_TEST_BUFFER_CONFIG_REG, data);
	return_code = sis3316_adc1->register_write(SIS3316_ADC_CH13_16_MAW_TEST_BUFFER_CONFIG_REG, data);


	//address_threshold = 200;
	address_threshold = (nof_events * event_length) - 1 ;  //
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

	if (sample_length != 0) {
		gl_graph_raw->sis3316_draw_XYaxis (sample_length); // clear and draw X/Y
	}


/******************************************************************************************/

	for (i_ch=0; i_ch<16; i_ch++) {
		uint_channel_event_counter[i_ch]  = 0;
	}
	loop_counter        = 0;
	bank_buffer_counter = 0 ;



/******************************************************************************************/


	// file write
	if (uint_DataEvent_OpenFlag == 1) {   ; // Open
		gl_FILE_DataEvenFilePointer = fopen("sis3316_test_data.dat","wb") ;
	}

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



	do {

		poll_counter = 0 ;
		do {
			poll_counter++;
			if (poll_counter == 100) {
				gSystem->ProcessEvents();  // handle GUI events
				poll_counter = 0 ;
    		}
			return_code = gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, &data);
			//usleep(500000); //500ms
			//printf("in Loop:  return_code = 0x%08x    addr = 0x%08x   SIS3316_ACQUISITION_CONTROL_STATUS = 0x%08x     \n", return_code , module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, data);
		} while (((data & 0x80000) == 0x0) && (gl_stopReq == FALSE)); // Address Threshold reached ?

		//gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, &data);
		//printf("SIS3316_ACQUISITION_CONTROL_STATUS before bank switch = 0x%08x\n", data);

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

		gl_vme_crate->vme_A32D32_read ( module_base_addr + SIS3316_ACQUISITION_CONTROL_STATUS, &data);
		//printf("SIS3316_ACQUISITION_CONTROL_STATUS after bank switch = 0x%08x\n", data);

		unsigned int max_read_nof_words;
		max_read_nof_words = event_length;

//#define CHECK_GATE1_RAW_DATA
#ifdef CHECK_GATE1_RAW_DATA
		for (i_ch = 0; i_ch < 4; i_ch++) {
			// read channel events
			//return_code = sis3316_adc1->read_MBLT64_Channel_PreviousBankDataBuffer(bank1_armed_flag /*bank2_read_flag*/, i_ch /* 0 to 15 */, &got_nof_32bit_words, gl_rblt_data);
			//printf("read_MBLT64_Channel_PreviousBankDataBuffer: i_ch %d  got_nof_32bit_words = 0x%08x  return_code = 0x%08x\n",i_ch,  got_nof_32bit_words, return_code);

			return_code = sis3316_adc1->read_DMA_Channel_PreviousBankDataBuffer(bank1_armed_flag, i_ch, max_read_nof_words, &got_nof_32bit_words, gl_rblt_data);


			if (return_code != 0) {
				printf("read_MBLT64_Channel_PreviousBankDataBuffer: return_code = 0x%08x\n", return_code);
				gl_stopReq = TRUE;
			}

			unsigned int uint_raw_data_val;
			unsigned int uint_raw_data_g1_val;
			unsigned int uint_raw_data_g2_val;
			unsigned int uint_gate1_val;
			unsigned int uint_gate2_val;


			ch_event_counter = (got_nof_32bit_words / event_length);
			uint_channel_event_counter[i_ch] = uint_channel_event_counter[i_ch] + ch_event_counter;
			if (ch_event_counter > 0) {
// check 1. event

//				sample_start_index = 210;
//				gate_start_index[1] = 228;
//				gate_length[1] = 0;
//				gate_start_index[2] = 229;
//				gate_length[2] = 0;

				i = 0;
				uint_gate1_val    = (gl_rblt_data[i*(event_length)+4]) & 0xFFFFFF;
				uint_gate2_val    = (gl_rblt_data[i*(event_length)+5]) & 0xFFFFFFF;
				uint_raw_data_val = gl_rblt_data[i*(event_length)+header_length + 9];
				uint_raw_data_g2_val = (uint_raw_data_val >> 16) & 0xffff;
				uint_raw_data_g1_val = (uint_raw_data_val ) & 0xffff;
				if ((uint_gate1_val != uint_raw_data_g1_val) || (uint_gate2_val != uint_raw_data_g2_val)) {
					printf("Error: GATE1 = 0x%08x  raw_data GATE1 = 0x%08x \n", uint_gate1_val, uint_raw_data_g1_val);
					printf("Error: GATE2 = 0x%08x  raw_data GATE2 = 0x%08x \n", uint_gate2_val, uint_raw_data_g2_val);

				}

				//gl_graph_raw->sis3316_draw_chN(sample_length, &gl_rblt_data[i*(event_length)+header_length], i_ch); //
			}
		}

#endif



#define READ_OUT
#ifdef READ_OUT

		for (i_ch = 0; i_ch < 16; i_ch++) {
			// read channel events
			return_code = sis3316_adc1->read_MBLT64_Channel_PreviousBankDataBuffer(bank1_armed_flag /*bank2_read_flag*/, i_ch /* 0 to 15 */,  &got_nof_32bit_words, gl_rblt_data ) ;
			printf("read_MBLT64_Channel_PreviousBankDataBuffer: i_ch %d  got_nof_32bit_words = 0x%08x  return_code = 0x%08x\n",i_ch,  got_nof_32bit_words, return_code);
			if (return_code != 0) {
				printf("read_MBLT64_Channel_PreviousBankDataBuffer: return_code = 0x%08x\n", return_code);
				gl_stopReq = TRUE;
			}

			ch_event_counter = (got_nof_32bit_words  / event_length) ;
			uint_channel_event_counter[i_ch]  = uint_channel_event_counter[i_ch] + ch_event_counter;
			if (ch_event_counter > 0) {

				// plot events
				for (i=0; i<ch_event_counter; i++) {
					if (i<5) { // plot ony 5 events
						gl_graph_raw->sis3316_draw_chN (sample_length, &gl_rblt_data[i*(event_length) + header_length], i_ch); //
					}
				}
				if (uint_DataEvent_OpenFlag == 1) {     // Open
					WriteBufferHeaderCounterNofChannelToDataFile (bank_buffer_counter, ch_event_counter , got_nof_32bit_words) ;
					WriteEventsToDataFile (gl_rblt_data, got_nof_32bit_words)  ;
    			}

			}
		}


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

		loop_counter++;
		bank_buffer_counter++;
		printf("\n");

		printf("bank_buffer_counter = %d     \n",bank_buffer_counter);
		for (i_ch=0; i_ch<16; i_ch++) {
			if (uint_channel_event_counter[i_ch] != 0) {
				printf("ch %d:    event counter  = %d  (0x%08x)      \n", i_ch+1, uint_channel_event_counter[i_ch], uint_channel_event_counter[i_ch]);
			}
		}
		printf("\n");
		printf("\n");
		//printf("\n");
		gSystem->ProcessEvents();  // handle GUI events
#endif
	} while((gl_stopReq == FALSE) && ((loop_counter < stop_after_loop_counts) || (stop_after_loop_counts == 0) ));

	if (uint_DataEvent_OpenFlag == 1) {   ; // Open
		fclose(gl_FILE_DataEvenFilePointer);
	}
	return_code = gl_vme_crate->vme_A32D32_write ( module_base_addr + SIS3316_KEY_DISARM , 0);  //


	printf("sampling finished   \n");

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

int WriteBufferHeaderCounterNofChannelToDataFile (unsigned int buffer_no,unsigned int nof_events, unsigned int event_length)
{
int written ;
int data ;
  //header
	data = FILE_FORMAT_EVENT_HEADER ;
    written=fwrite(&data,0x4,0x1,gl_FILE_DataEvenFilePointer); // write one  uint word
	//gl_uint_DataEvent_LWordCounter = gl_uint_DataEvent_LWordCounter + written ;
  //Buffer No
    written=fwrite(&buffer_no,0x4,0x1,gl_FILE_DataEvenFilePointer); // write one  uint word
	//gl_uint_DataEvent_LWordCounter = gl_uint_DataEvent_LWordCounter + written ;
  //nof events
    written=fwrite(&nof_events,0x4,0x1,gl_FILE_DataEvenFilePointer); // write one  uint word
	//gl_uint_DataEvent_LWordCounter = gl_uint_DataEvent_LWordCounter + written ;
  //event length
    written=fwrite(&event_length,0x4,0x1,gl_FILE_DataEvenFilePointer); // write one  uint word
	//gl_uint_DataEvent_LWordCounter = gl_uint_DataEvent_LWordCounter + written ;

	//gl_uint_DataEvent_RunFile_EventChannelSize =  event_length;
	//gl_uint_DataEvent_RunFile_EventSize = nof_channels * gl_uint_DataEvent_RunFile_EventChannelSize ;

 	return 0;

}


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

