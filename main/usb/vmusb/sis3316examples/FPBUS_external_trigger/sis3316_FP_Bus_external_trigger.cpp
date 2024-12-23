/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_FP_Bus_external_trigger.cpp                          */
/*                                                                         */
/*  Funktion:  trigger with pulse at the LEMO Input TI                     */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.06.2018                                       */
/*  last modification:    25.07.2024    (SIS3316-2 adaptation)             */
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
/*  http2://www.struck.de                                                  */
/*                                                                         */
/*  � 2022                                                                 */
/*                                                                         */
/***************************************************************************/


static int MAX_NOF_SIS3316_ADCS(3);   // We'll set it to the size of the found ones.
//#define MAX_NOF_SIS3316_ADCS			13
//#define MAX_NOF_SIS3316_ADCS			1

#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethnernet UDP)


#define CERN_ROOT_PLOT

#ifdef CERN_ROOT_PLOT
#include "rootIncludes.h"   // 
#include "sis3316_cern_root_class.h"  


#ifdef raus
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
#endif

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
		#include <winsock2.h>3
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

#ifdef VMUSB_INTERFACE
#include <CVMUSBFactory.h>
#include <CVMUSB.h>
#include <sis_vmusb_interface.h>
#include <stdint.h>
#include <string>
#include <vector>
namespace Globals {
        CVMUSB* pUSBController;
}
vme_interface_class* gl_vme_crate;
/**
 *  Connect to the fist VMUSB and
 *  set that as Globals::pUSBController and instantiate
 * a sis_vmusb_interface -> gl_vme_crate.
 * We open the crate though I think that might be done 
 * elsewhere it's harmless to do more than once (vmeopen).
 */
static int connectVME() {
        try {
                Globals::pUSBController =
                        CVMUSBFactory::createUSBController(
                          CVMUSBFactory::local, nullptr);
                gl_vme_crate = new sis_vmusb_interface;
                gl_vme_crate->vmeopen();
        }
        catch (std::string msg) {
                std::cerr << "Unable to connect to a VMUSB:  "
                          << msg << std::endl;
                return -1;             // Fail.
        }
        return 0;    // Success
}
/**
 @brief  Determine if a module is an SIS3316 module.
 @param crate  - interface class to the VME crate.
 @param base   - Module base address.
 @return bool - true if it's an SIS3316.
*/
static bool isSIS3316(vme_interface_class* crate, uint32_t base) {
        uint32_t value;
        int status = crate->vme_A32D32_read(base + SIS3316_MODID, &value);
        if (status ) return false;    // Read failed probably bus error.
        value = (value & 0xffff0000) >> 16;
        return value == 0x3316;          // Correct module id.
}

#endif



//vme_interface_class *intf;
//sis3316_adc *adc;

/*===========================================================================*/
/* Globals					  			     */
/*===========================================================================*/
#ifdef WINDOWS

BOOL gl_stopReq = FALSE;
#endif
/*===========================================================================*/
/* Prototypes			                               		  			     */
/*===========================================================================*/
void program_stop_and_wait(void);
#ifdef WINDOWS
BOOL CtrlHandler( DWORD ctrlType );
#endif
/*===========================================================================*/

int main(int argc, char *argv[])
{
	volatile int return_code ;
	int i_mod, i_ch, i_fpga;
	int rc;
	unsigned int serial_no;
	unsigned int data ;
	char char_messages[128] ;
	unsigned int nof_found_devices ;

	//char char_command[256];
	unsigned int dhcp_option ;
	unsigned int vme_base_address ;




int i_event;
	
unsigned int uint_fpag_config_reg_value[MAX_NOF_SIS3316_ADCS][4] ;
unsigned int header_length ;
unsigned int header_accu_6_values_offset ;
unsigned int header_accu_2_values_offset ;
unsigned int header_maw_3_values_offset ;
unsigned int uint_config_data_format ;
unsigned int event_length ;
unsigned int max_req_nof_32bit_words ;
unsigned int address_threshold ;

unsigned int bank1_armed_flag ;
unsigned int plot_counter ;
unsigned int ch_event_counter ;
unsigned int poll_counter ;
//unsigned int bank_buffer_counter ;
unsigned int loop_counter ;

unsigned int got_nof_32bit_words ;


/*********************************************************************************************************************/
/*  default values/parameters                                                                                        */


// global parameters for all channels 
unsigned int uint_software_key_trigger_flag ;
unsigned int stop_after_loop_counts ;

unsigned int nof_events_per_bank ;

unsigned int trigger_gate_window_length ;
unsigned int sample_length ;
unsigned int sample_start_index ;
unsigned int internal_fir_trigger_delay ;
unsigned int pre_trigger_delay ;

unsigned int header_accu_6_values_enable_flag ;
unsigned int header_accu_2_values_enable_flag ;
unsigned int header_maw_3_values_enable_flag ;

unsigned int maw_test_buffer_enable_flag ;
unsigned int maw_test_buffer_length ;
unsigned int maw_test_buffer_delay ;

unsigned int uint_pileup ;
unsigned int uint_re_pileup ;

unsigned int uint_channel_polarity_invert_flag[MAX_NOF_SIS3316_ADCS][16]  ;
unsigned int uint_channel_range0_flag[MAX_NOF_SIS3316_ADCS][16]  ;
unsigned int uint_channel_50ohm_termination_disable_flag[MAX_NOF_SIS3316_ADCS][16]  ;
unsigned int uint_channel_external_trigger_enable_flag[MAX_NOF_SIS3316_ADCS][16]  ;


printf("\n\n");
printf("sis3316_FP_BUS_external_trigger\n");
printf("\n\n");

	max_req_nof_32bit_words = SIS3316_ADC_MEMORY_BANK_32BIT_SIZE ; // max_request is limited by Memory Banks_Size
	uint_software_key_trigger_flag = 0 ;//

	stop_after_loop_counts      = 0;   	// 0: endless , could be changed by calling this program with "-N num" option
	nof_events_per_bank         = 1 ;	// events / Bank

	sample_length               = 500;	
	sample_start_index          = 0;	
	pre_trigger_delay           = 100 ;
	trigger_gate_window_length  = 500;	
	internal_fir_trigger_delay  =  0  ;  //  

	header_accu_6_values_enable_flag = 0 ;
	header_accu_2_values_enable_flag = 0 ;
	header_maw_3_values_enable_flag  = 0 ;

	maw_test_buffer_enable_flag = 0;
	maw_test_buffer_length      = 0;
	maw_test_buffer_delay       = 0;

	uint_pileup    = 4000;  //  
	uint_re_pileup = 4000;  //  

	pre_trigger_delay = pre_trigger_delay + (2 * internal_fir_trigger_delay) ;   


// individual parameters for each channels 


	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		// channel external trigger enable
		for(i_ch=0;i_ch<16;i_ch++) {
			uint_channel_external_trigger_enable_flag[i_mod][i_ch] = 1 ; // all enable
		}
		// channel polarity
		for(i_ch=0;i_ch<16;i_ch++) {
			uint_channel_polarity_invert_flag[i_mod][i_ch] = 0 ;
		}

		// Set Gain/Termination
		for(i_ch=0;i_ch<16;i_ch++) {
			uint_channel_range0_flag[i_mod][i_ch] = 1 ;
			uint_channel_50ohm_termination_disable_flag[i_mod][i_ch] = 0 ;
		}

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

	unsigned int *dma_read_buffer;
	dma_read_buffer = (unsigned int *)malloc(MAX_NUMBER_LWORDS_64MBYTE);
	if(dma_read_buffer == NULL){
		printf("Error allocating dma_read_buffer !\n");
		program_stop_and_wait();
	}


#ifdef ETHERNET_UDP_INTERFACE

	char  pc_ip_addr_string[32] ;
//	char  sis3316_ip_addr_string[32] ;

	//strcpy(sis3316_ip_addr_string, gl_sis3316_ip_addr_string) ; // SIS3316 IP address
	//strcpy(sis3316_ip_addr_string,"212.60.16.200") ; // SIS3316 IP address

	  if(MAX_NOF_SIS3316_ADCS >= 1) { 	strcpy(sis3316_ip_addr_string[0],"sis3316-1002") ; } // SIS3316-2 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 2) { 	strcpy(sis3316_ip_addr_string[1],"sis3316-1001") ; } // SIS3316-2 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 3) { 	strcpy(sis3316_ip_addr_string[2],"sis3316-0002") ; } // SIS3316 IP address

	  //if(MAX_NOF_SIS3316_ADCS >= 1) { 	strcpy(sis3316_ip_addr_string[0],"sis3316-1004") ; } // SIS3316 IP address
	  //if(MAX_NOF_SIS3316_ADCS >= 2) { 	strcpy(sis3316_ip_addr_string[1],"sis3316-1005") ; } // SIS3316 IP address
	  //if(MAX_NOF_SIS3316_ADCS >= 3) { 	strcpy(sis3316_ip_addr_string[2],"sis3316-0487") ; } // SIS3316 IP address
	  //if(MAX_NOF_SIS3316_ADCS >= 3) { 	strcpy(sis3316_ip_addr_string[3],"sis3316-1006") ; } // SIS3316 IP address


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

	  // default
	dhcp_option = 0 ;
	vme_base_address = 0x00000000 ;



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


 
	sis3316_adc  *sis3316_adc_array[MAX_NOF_SIS3316_ADCS] ;
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_adc_array[i_mod] = new sis3316_adc( sis3316_eth_device[i_mod], i_mod * 0x01000000); // base address (i_mod * 0x01000000) is used for header information !
	}

do {
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		return_code = sis3316_adc_array[i_mod]->register_read(0x4, &data);
		if ((return_code != 0) || ((data & 0xffff0000) != 0x33160000)) {
			printf("not valid SIS3316 vme base address\n");
			printf("return_code      = 0x%08X \n", return_code);
			printf("vme_base_address = 0x%08X \n", vme_base_address);
			printf("module ID        = 0x%08X \n", data);
			return -1;
		}
		else {
			return_code = sis3316_adc_array[i_mod]->register_read(SIS3316_SERIAL_NUMBER_REG, &serial_no);
			printf("Serial number   = %d  \t", serial_no&0xffff);
			printf("module ID/VME FPGA version = 0x%08X \t", data);
			rc = sis3316_adc_array[i_mod]->register_read(0x1100, &data);
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
#ifdef VMUSB_INTERFACE
	if(connectVME())  return -1;    /// Failed to open VME.

	// Look for SIS 3316 modules in all possible  base addresses:
	std::vector<sis3316_adc*> sis3316_adc_array;
	for (uint32_t i =0; i < 256; i++) {
		uint32_t base = i << 24;
		if (isSIS3316(gl_vme_crate, base)) {
			sis3316_adc_array.push_back(new sis3316_adc(gl_vme_crate, base));
			
			// Read and output the serial number and temp as SIS does for the
			// ethernet/udp interface:

			i_mod = i;    // to make a copy of their code work:
			return_code = sis3316_adc_array[i_mod]->register_read(SIS3316_SERIAL_NUMBER_REG, &serial_no);
			if (return_code) {
				std::cerr << "Failed to read serial number from module at "
					<< std::hex << base << std::dec << std::endl;
				return -1;
			}
			printf("Serial number   = %d  \t", serial_no&0xffff);
			printf("module ID/VME FPGA version = 0x%08X \t", data);
			rc = sis3316_adc_array[i_mod]->register_read(0x1100, &data);
			printf("ADC FPGA version = 0x%08X \n", data);

			return_code = sis3316_adc_array[i_mod]->register_read(SIS3316_INTERNAL_TEMPERATURE_REG, &data);
			if (return_code) {
				std::cerr << "Failed to read internal temp. from module at "
					<< std::hex << base << std::dec << std::endl;
				return -1;
			}
			signed short signed_short_temperature ;
			float float_temperature_c, float_temperature_f ;
			signed_short_temperature =  ((signed short) (data&0xffff) ) ;
			float_temperature_c =  (float) (signed_short_temperature) / 4.0 ;
			float_temperature_f =  32.0 + (float_temperature_c * 1.8) ;
			printf("Temperature     = %2.2f C    %3.2f F \n", float_temperature_c, float_temperature_f );
			printf("\n");
		} 
	}
	// The do this so we will too:
	MAX_NOF_SIS3316_ADCS = sis3316_adc_array.size();
	usleep(500000) ;
	printf("\n\n\n\n");
#endif


unsigned int clock_N1div, clock_HSdiv ;
unsigned int iob_delay_value ;
double double_clock_configure_fft_frequency;
unsigned int clock_freq_choice ;

unsigned int uint_lemo_out_CO_select ;
unsigned int uint_lemo_out_TO_select ;
unsigned int uint_lemo_out_UO_select ;
/******************************************************************************************/
/*                                                                                        */
/*    Parameter configuration                                                             */
/*                                                                                        */
/******************************************************************************************/



	if (pre_trigger_delay > 16378) {
		pre_trigger_delay  = 16378 ;
	}

	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		for(i_fpga=0;i_fpga<4;i_fpga++) {
			uint_fpag_config_reg_value[i_mod][i_fpga] =    ((uint_channel_external_trigger_enable_flag[i_mod][(i_fpga*4)+0] & 1) << 3) 
														 + ((uint_channel_external_trigger_enable_flag[i_mod][(i_fpga*4)+1] & 1) << 11)
														 + ((uint_channel_external_trigger_enable_flag[i_mod][(i_fpga*4)+2] & 1) << 19)
														 + ((uint_channel_external_trigger_enable_flag[i_mod][(i_fpga*4)+3] & 1) << 27)  
														 +  (uint_channel_polarity_invert_flag[i_mod][(i_fpga*4)+0] & 1) 
														 + ((uint_channel_polarity_invert_flag[i_mod][(i_fpga*4)+1] & 1) << 8)
														 + ((uint_channel_polarity_invert_flag[i_mod][(i_fpga*4)+2] & 1) << 16)
														 + ((uint_channel_polarity_invert_flag[i_mod][(i_fpga*4)+3] & 1) << 24) ;
		}
	}

// data format
	header_length = 3;	
	header_accu_6_values_offset = 2 ;
	header_accu_2_values_offset = 2 ;
	header_maw_3_values_offset  = 2 ;

	uint_config_data_format = 0 ;
	if (header_accu_6_values_enable_flag == 1) {
		header_length = header_length + 7 ;
		header_maw_3_values_offset  = header_maw_3_values_offset + 7 ;
		header_accu_2_values_offset  = header_accu_2_values_offset + 7 ;
		uint_config_data_format = uint_config_data_format + 0x1 ; // set bit 0
	}
	if (header_accu_2_values_enable_flag == 1) {
		header_length = header_length + 2 ;
		header_maw_3_values_offset  = header_maw_3_values_offset + 2 ;
		uint_config_data_format = uint_config_data_format + 0x2 ; // set bit 1
	}
	if (header_maw_3_values_enable_flag == 1) {
		header_length = header_length + 3 ;
		uint_config_data_format = uint_config_data_format + 0x4 ; // set bit 2
	}
	if (maw_test_buffer_enable_flag == 1) {
		uint_config_data_format = uint_config_data_format + 0x10 ; // set bit 4
		maw_test_buffer_length =  0 ;
	}

	event_length = (header_length + (sample_length / 2)  + maw_test_buffer_length);

	address_threshold = (nof_events_per_bank * event_length) - 1 ;  //  
	address_threshold = address_threshold + 0x80000000 ;  //  suppress saving following hits/events if addresshold flag is set  


	
	
	
	

	
	
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

		if (sis3316_adc_array[i_mod]->adc_125MHz_flag == 0) { // 250 MHz
			graph_raw[i_mod]->sis3316_set_14bit_Yaxis();
		}
		else {
			graph_raw[i_mod]->sis3316_set_16bit_Yaxis();
		}

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


	
do {
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
	//data = data + 0x0 ;   // Selects internal Clock oscillator
	//data = data + 0x20 ;   // Selects Lemo Clock In
	i_mod=0;	
	sis3316_adc_array[i_mod]->register_write(SIS3316_FP_LVDS_BUS_CONTROL, data) ;   

	sis3316_adc_array[i_mod]->register_read(SIS3316_FP_LVDS_BUS_CONTROL, &data);
		printf("SIS3316_FP_LVDS_BUS_CONTROL       = 0x%08X \n", data);


	if(MAX_NOF_SIS3316_ADCS > 1) {
		data =  0x2 ;   // Enables the Status lines to the FP-Bus, only
		for (i_mod=1; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			sis3316_adc_array[i_mod]->register_write(SIS3316_FP_LVDS_BUS_CONTROL, data) ;   
		}
	}



/************/
	// define the sample Clock on each module	
	data = 0 ; // Onboard Oscillator
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
		sis3316_adc_array[i_mod]->configure_adc_fpga_iob_delays(iob_delay_value) ;  // necessary after changing/seting clock
	}

	// Now are the clocks are configured onl modules


/******************************************************************************************/
/*  System External Trigger Configuration */

	// Enable LEMO Input "TI" as Trigger External Trigger on Master Module -> the Trigger will be routed to FP-Bus
	data = 0x10 ; // Enable Nim Input "TI"
	//data = data + 0x40 ; // Enable Nim Input "TI" level 
	i_mod=0;	
	sis3316_adc_array[i_mod]->register_write(SIS3316_NIM_INPUT_CONTROL_REG, data) ;   

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
	

/******************************************/

	uint_lemo_out_CO_select  = 0x1 ;        // select sample clock
	uint_lemo_out_TO_select  = 0x2000000 ;  // select external trigger
	uint_lemo_out_UO_select  = 0x80 ;       // external Timestamp_clr
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {

		// Select LEMO Output "Co"   
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_LEMO_OUT_CO_SELECT_REG, uint_lemo_out_CO_select ); //  

		// Select LEMO Output "To"   
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_LEMO_OUT_TO_SELECT_REG, uint_lemo_out_TO_select ); //  

		// Select LEMO Output "UO"
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_LEMO_OUT_UO_SELECT_REG, uint_lemo_out_UO_select ); //
	}

	
	
/******************************************/
// Led Application mode
	data = 0x70 ;
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_CONTROL_STATUS, data ); //
	}
	
/******************************************/


	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {

		//pileup ;  
		data = ((uint_re_pileup & 0xffff) << 16) + (uint_pileup & 0xffff) ;
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_PILEUP_CONFIG_REG, data ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_PILEUP_CONFIG_REG, data ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_PILEUP_CONFIG_REG, data ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_PILEUP_CONFIG_REG, data ); //  

		//pre_trigger_delay
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //  

		// trigger_gate_window_length
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG, ((trigger_gate_window_length -2) & 0xffff) ); // trigger_gate_window_length

		// Sample Length and Sample Start Index
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length

		//  Event Configuration
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_EVENT_CONFIG_REG,   uint_fpag_config_reg_value[i_mod][0] ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_EVENT_CONFIG_REG,   uint_fpag_config_reg_value[i_mod][1] ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_EVENT_CONFIG_REG,  uint_fpag_config_reg_value[i_mod][2] ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, uint_fpag_config_reg_value[i_mod][3] ); //  
			//printf("in Loop:   uint_fpag_config_reg_value[i_mod][0]  = 0x%08x     \n", uint_fpag_config_reg_value[0][0] );

		//  Data Format
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG,   uint_config_data_format ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_DATAFORMAT_CONFIG_REG,   uint_config_data_format ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_DATAFORMAT_CONFIG_REG,  uint_config_data_format); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_DATAFORMAT_CONFIG_REG, uint_config_data_format ); //  

		// MAW Test Buffer configuration
		data = maw_test_buffer_length + (maw_test_buffer_delay << 16) ;
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_MAW_TEST_BUFFER_CONFIG_REG, data ); 
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_MAW_TEST_BUFFER_CONFIG_REG, data ); 
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_MAW_TEST_BUFFER_CONFIG_REG, data ); 
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_MAW_TEST_BUFFER_CONFIG_REG, data ); 

		//address_threshold  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG, address_threshold ); //  
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH5_8_ADDRESS_THRESHOLD_REG, address_threshold); //   
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH9_12_ADDRESS_THRESHOLD_REG, address_threshold ); //     
		return_code = sis3316_adc_array[i_mod]->register_write(SIS3316_ADC_CH13_16_ADDRESS_THRESHOLD_REG, address_threshold ); //  

		//address_threshold  
		return_code = sis3316_adc_array[i_mod]->write_channel_header_ID( i_mod << 24 ); // bits 31 to 24 !!
		
	}



/******************************************/


	// Clear Timestamp on first module --> will clear via FB-Bus Timestamps on all modules */
	sis3316_adc_array[0]->register_write(SIS3316_KEY_TIMESTAMP_CLEAR, 0) ;   
	printf("SIS3316_KEY_TIMESTAMP_CLEAR \n");
	usleep(500000);

	// Start Readout Loop
	//Note: Start sampling on Bank on alternate Bank, check Bit 24 in the register "previous Bank sample address"
	sis3316_adc_array[0]->register_read( SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);
	if((data & 0x1000000) == 0x1000000 ) { 	// bank2 flag is set ?
		//printf("bank2 flag is set\n"); // start sampling an alternate bank
		bank1_armed_flag = 1 ;
		return_code = sis3316_adc_array[0]->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK1, 0 ); //    Arm
		printf("SIS3316_KEY_DISARM_AND_ARM_BANK1 \n");
	}
	else {
		//printf("bank2 flag is not set\n"); // start sampling an alternate bank
		bank1_armed_flag = 0 ;
		return_code = sis3316_adc_array[0]->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK2, 0 ); // //  Arm
		printf("SIS3316_KEY_DISARM_AND_ARM_BANK2 \n");
	}



	plot_counter           = 0;
	ch_event_counter       = 0;
	loop_counter           = 0;
	stop_after_loop_counts = 0;

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

	} while(0) ;
#endif

	do {

		poll_counter = 0 ;
		do {
			poll_counter++;
			if (poll_counter == 100) {
				if(uint_software_key_trigger_flag == 1) {
					return_code = sis3316_adc_array[0]->register_write(SIS3316_KEY_TRIGGER , 0);  //  Trigger
					usleep(1000); //   
				}
				#ifdef CERN_ROOT_PLOT
					gSystem->ProcessEvents();  // handle GUI events
				#else
					usleep(100); //   
				#endif
				poll_counter = 0 ;
    		}
#ifdef only_test
			for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
				sis3316_adc_array[i_mod]->register_read ( SIS3316_ACQUISITION_CONTROL_STATUS, &data);
				printf("i_mod = %d:   SIS3316_ACQUISITION_CONTROL_STATUS = 0x%08x     \n", i_mod, data);
			}
#endif
			sis3316_adc_array[0]->register_read ( SIS3316_ACQUISITION_CONTROL_STATUS, &data);  
			printf("waiting for external Trigger     \n");
			printf("in Loop   i_mod = 0:   SIS3316_ACQUISITION_CONTROL_STATUS = 0x%08x     \n", data);
			//if ((data & 0x200000) == 0x0) {
				usleep(1000000); //
			//}
		//} while (((data & 0x80000) == 0x0) && (gl_stopReq == FALSE))  ; // own  Address Threshold reached ?
		//gl_stopReq = TRUE ;
#ifdef WINDOWS
		} while (((data & 0x200000) == 0x0) && (gl_stopReq == FALSE))  ; // FP-Bus Address Threshold reached ?
		if (gl_stopReq == TRUE) { break ; }
#else
		} while (((data & 0x200000) == 0x0) )  ; // FP-Bus Address Threshold reached ?
#endif

		if (bank1_armed_flag == 1) {
			return_code = sis3316_adc_array[0]->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK2 , 0);  //  Arm Bank2
			bank1_armed_flag = 0; // bank 2 is armed
			printf("SIS3316_KEY_DISARM_AND_ARM_BANK2 \n");
		}
		else {
			return_code = sis3316_adc_array[0]->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK1 , 0);  //  Arm Bank1
			bank1_armed_flag = 1; // bank 1 is armed
			printf("SIS3316_KEY_DISARM_AND_ARM_BANK1 \n");
		}
#ifdef only_test
		
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			sis3316_adc_array[0]->register_read ( SIS3316_ACQUISITION_CONTROL_STATUS, &data);
			printf("after Disarm/Arm i_mod = %d:   SIS3316_ACQUISITION_CONTROL_STATUS = 0x%08x     \n", i_mod, data);
			sis3316_adc_array[i_mod]->register_read ( SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);  
			printf("after Disarm/Arm i_mod = %d:   SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG = 0x%08x     \n", i_mod, data);
		}
		printf("\n");
#endif
		#ifdef CERN_ROOT_PLOT
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
 			graph_raw[i_mod]->sis3316_draw_XYaxis (sample_length); // clear and draw X/Y		
			gSystem->ProcessEvents();  // handle GUI events
		}
		#endif

		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			for (i_ch=0; i_ch<16; i_ch++) {
				return_code = sis3316_adc_array[i_mod]->read_DMA_Channel_PreviousBankDataBuffer(bank1_armed_flag /*bank2_read_flag*/, i_ch /* 0 to 15 */, max_req_nof_32bit_words, &got_nof_32bit_words, dma_read_buffer ) ; // read maximun (all) events
				//printf("read_DMA_Channel_PreviousBankDataBuffer: i_ch %d  got_nof_32bit_words = 0x%08x  \n",i_ch,  got_nof_32bit_words);
				if (return_code != 0) {
					printf("read_DMA_Channel_PreviousBankDataBuffer: return_code = 0x%08x\n", return_code);
#ifdef WINDOWS
					gl_stopReq = TRUE;
#endif
				}
				ch_event_counter = (got_nof_32bit_words  / event_length) ;
				printf("read_DMA_Channel_PreviousBankDataBuffer: i_mod %d i_ch %d \t channel ID = 0x%03x  %d   \ttimestamp_upper = 0x%04x  timestamp_lower = 0x%08x  \n",i_mod, i_ch,  (dma_read_buffer[0] & 0xfff0) >> 4,   (dma_read_buffer[0] & 0xfff0) >> 4,  (dma_read_buffer[0] & 0xffff0000) >> 16, dma_read_buffer[1]);

				#ifdef CERN_ROOT_PLOT
				for (i_event=0; i_event<ch_event_counter; i_event++) {
					if (i_event==0) { // plot ony 1. event
						graph_raw[i_mod]->sis3316_draw_chN (sample_length, &dma_read_buffer[i_event*(event_length) + header_length], (i_mod*16) + i_ch); //  
						gSystem->ProcessEvents();  // handle GUI events
					}
				}
				#endif

			}
		}





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
		loop_counter++;
#ifdef WINDOWS
		} while(((loop_counter < stop_after_loop_counts) || (stop_after_loop_counts == 0)) && (gl_stopReq == FALSE) );
#else
		} while(((loop_counter < stop_after_loop_counts) || (stop_after_loop_counts == 0))  );
#endif



	//sis3316_adc_array[0]->register_write(SIS3316_KEY_DISARM, 0);
		for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
			sis3316_adc_array[i_mod]->register_write(SIS3316_KEY_DISARM, 0);
			//sis3316_adc_array[i_mod]->reset_adc_fpga_sample_clock_PLL() ;
		}
		printf("\n");
#ifdef WINDOWS
	} while (gl_stopReq == FALSE) ;
#else
	} while (1) ;
#endif

//program_stop_and_wait();
	return 0;
}



	
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
