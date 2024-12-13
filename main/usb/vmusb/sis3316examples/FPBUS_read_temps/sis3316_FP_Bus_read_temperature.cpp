/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_FP_Bus_read_temperature.cpp                          */
/*                                                                         */
/*  Function: read the temperature from several SIS3316                    */
/*            - see:  define MAX_NOF_SIS3316_ADCS			13             */
/*            - see:  strcpy(sis3316_ip_addr_string[0],"192.168.1.100") ;  */
/*                                                                         */
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
/*  https://www.struck.de                                                  */
/*                                                                         */
/*  � 2024                                                                 */
/*                                                                         */
/***************************************************************************/


//#define MAX_NOF_SIS3316_ADCS			1
static int MAX_NOF_SIS3316_ADCS(3);  // For VMUSB this will be # modules found.
//#define MAX_NOF_SIS3316_ADCS			13

#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethnernet UDP)


#ifdef LINUX

#include <iostream>
using namespace std;


#include <stdio.h>
#include <stdlib.h>
#endif

// #ifdef WINDOWS
// #include <iostream>
// #include <iomanip>
// using namespace std;
// #define WIN32_LEAN_AND_MEAN
// #include <windows.h>
// #include <winsock2.h>

// #include <stdlib.h>
// #include <string.h>
// #include <math.h>
// //	#include "wingetopt.h"
// #endif




#ifdef ETHERNET_UDP_INTERFACE
#include "sis3316_ethernet_access_class.h"
//sis3316_eth *gl_vme_crate ;

#ifdef LINUX
#include <sys/types.h>
#include <sys/socket.h>
#endif

// #ifdef WINDOWS
// #include <winsock2.h>
// #pragma comment(lib, "ws2_32.lib")
// //#pragma comment(lib, "wsock32.lib")
// typedef int socklen_t;
// char  gl_sis3316_ip_addr_string[32] ;

// long WinsockStartup()
// {
//   long rc;

//   WORD wVersionRequested;
//   WSADATA wsaData;
//   wVersionRequested = MAKEWORD(2, 1);

//   rc = WSAStartup( wVersionRequested, &wsaData );
//   return rc;
// }
// #endif

#endif


#include "sis3316_class.h"


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



int main(int argc, char *argv[])
{
  volatile int return_code ;
  int i_mod;
  int rc;
  unsigned int data ;
  unsigned int serial_no ;
  char char_messages[128] ;
  unsigned int nof_found_devices ;

  unsigned int vme_base_address ;


  printf("\n\n");
  printf("sis3316_FP_BUS_read_temperature\n");
  printf("\n\n");



	
  /*********************************************************************************************************************/

  /******************************************************************************************/
  // #ifdef WINDOWS
  // 	if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, TRUE ) ){
  // 	  printf( "Error setting Console-Ctrl Handler\n" );
  // 	  return -1;
  // 	}
  // #endif
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

  //if(MAX_NOF_SIS3316_ADCS >= 1) { 	strcpy(sis3316_ip_addr_string[0],"212.60.16.15") ; } // SIS3316 IP address
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




  // default
  vme_base_address = 0x00000000 ;

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
    return_code = sis3316_eth_device[i_mod]->set_UdpSocketBindMyOwnPort( pc_ip_addr_string); // Outputs the this->udp_port = 0x0000ex

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
#endif
#ifdef VMUSB_INTERFACE
  if(connectVME()) {
    return -1;                         // Failed to open the VME interface.
  }
  std::vector<sis3316_adc*> sis3316_adc_array;
  for (unsigned i = 0; i < 256; i++) {
    uint32_t base = i << 24;          // Potential base address:

    if (isSIS3316(gl_vme_crate, base)) {
      sis3316_adc_array.push_back(new sis3316_adc(gl_vme_crate, base));
    }
    MAX_NOF_SIS3316_ADCS = sis3316_adc_array.size();
  }
#endif
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
    }
    usleep(500000) ;
    printf("\n\n");
  } while (0) ;


  do {
    printf("\n");
    for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
      return_code = sis3316_adc_array[i_mod]->register_read(SIS3316_INTERNAL_TEMPERATURE_REG, &data);
      signed short signed_short_temperature ;
      float float_temperature_c, float_temperature_f ;
      signed_short_temperature =  ((signed short) (data&0xffff) ) ;
      float_temperature_c =  (float) (signed_short_temperature) / 4.0 ;
      float_temperature_f =  32.0 + (float_temperature_c * 1.8) ;
      printf("Temperature    i_mod = %d   \t->  %2.2f C    %3.2f F \n", i_mod, float_temperature_c, float_temperature_f );
      usleep(1000000);
    }
    printf("\n");

  } while(1) ;


  //program_stop_and_wait();
  return 0;
}




void program_stop_and_wait(void)
{
// #ifdef WINDOWS
// 	gl_stopReq = FALSE;
// #endif
	printf( "\n\nProgram stopped");
	printf( "\n\nEnter ctrl C");
	//SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, FALSE ) ;
	do {
			usleep(1000); //
#ifdef WINDOWS
	} while (gl_stopReq == FALSE) ;
#else
} while (1) ;
#endif
	//		result = scanf( "%s", line_in );
}

/***************************************************/
// #ifdef WINDOWS


// BOOL CtrlHandler( DWORD ctrlType ){
// 		printf( "\n\nCTRL-C pressed. finishing current task \n\n");
// 	switch( ctrlType ){
// 	case CTRL_C_EVENT:
// 		printf( "\n\nCTRL-C pressed. finishing current task \n\n");
// 		gl_stopReq = TRUE;
// 		//printf("CtrlHandler : gl_stopReq    = %d     \n", gl_stopReq);
// 		return( TRUE );
// 		break;
// 	default:
// 		printf( "\n\ndefault pressed. \n\n");
// 		return( FALSE );
// 		break;
// 	}
// }
// #endif
