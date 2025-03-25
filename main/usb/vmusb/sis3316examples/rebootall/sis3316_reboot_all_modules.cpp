
/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_reboot_all_modules.cpp                               */
/*                                                                         */
/***************************************************************************/
/*                                                                         */
/*  Function: reboot several SIS3316                                       */
/*            - see:  define MAX_NOF_SIS3316_ADCS			13             */
/*            - see:  strcpy(sis3316_ip_addr_string[0],"192.168.1.100") ;  */
/*                                                                         */
/***************************************************************************/
/*                                                                         */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 18.09.2019                                       */
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
/*                                                                         */
/***************************************************************************/

#define MAX_NOF_SIS3316_ADCS			3
//#define MAX_NOF_SIS3316_ADCS			13



#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethnernet UDP)


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
	#include "wingetopt.h" 
#endif




#include "vme_interface_class.h"

#define ASSERT_REBOOT 0x8000
#define DEASSERT_REBOOT 0x80000000
#define DELAY_US 1000   // Ms to reload?

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
#ifdef VMUSB_VME_INTERFACE
#include <sis_vmusb_interface.h>
#include <CVMUSB.h>
#include <CVMUSBFactory.h>
#include <stdint.h>
#include<string>
namespace Globals {
	CVMUSB* pUSBController(0);
}

// True if the module at the base address has a readable
// module id and the id indicates this is a 3316:
static bool isSIS3316(vme_interface_class& vme, uint32_t base) {
	UINT idreg;
	int status = vme.vme_A32D32_read(base+0x04, &idreg);

	// If not status == 0 then it's not a module.

	if (status) return false;

	idreg = (idreg & 0xffff0000) >> 16;
	return idreg == 0x3316;
}
#endif


#include "sis3316_class.h"


vme_interface_class *intf;
sis3316_adc *adc;


int main(int argc, char *argv[])
{
	unsigned int data ;
	char char_messages[128] ;
	unsigned int nof_found_devices ;
	cout << "sis3316_reboot_all_modules" << endl; // prints sis3316_fpga_update_udp



#ifdef ETHERNET_UDP_INTERFACE

	char  pc_ip_addr_string[32] ;
//	char  sis3316_ip_addr_string[32] ;
	int return_code ;
unsigned int i_mod ;
	// allocating IP-address buffer
	char *sis3316_ip_addr_string[MAX_NOF_SIS3316_ADCS];
	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		sis3316_ip_addr_string[i_mod] = (char *)malloc(32);;
		if(sis3316_ip_addr_string[i_mod] == NULL){
			printf("Error allocating IP-Address buffers !\n");
			return 0;
		}
	}

	  if(MAX_NOF_SIS3316_ADCS >= 1) { 	strcpy(sis3316_ip_addr_string[0],"sis3316-0002") ; } // SIS3316 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 2) { 	strcpy(sis3316_ip_addr_string[1],"sis3316-1002") ; } // SIS3316-2 IP address
	  if(MAX_NOF_SIS3316_ADCS >= 3) { 	strcpy(sis3316_ip_addr_string[2],"sis3316-1004") ; } // SIS3316-2 IP address

//	  if(MAX_NOF_SIS3316_ADCS >= 1) { 	strcpy(sis3316_ip_addr_string[0],"192.168.1.100") ; } // SIS3316 IP address
//	  if(MAX_NOF_SIS3316_ADCS >= 2) { 	strcpy(sis3316_ip_addr_string[1],"192.168.1.101") ; } // SIS3316 IP address
//	  if(MAX_NOF_SIS3316_ADCS >= 3) { 	strcpy(sis3316_ip_addr_string[2],"192.168.1.102") ; } // SIS3316 IP address
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
    //return_code = WSAStartup();
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

	// RF Depends on Ethernet because of the 
	// base address of zero thing.

	for (i_mod=0; i_mod<MAX_NOF_SIS3316_ADCS; i_mod++) {
		usleep(100000);
		return_code = sis3316_eth_device[i_mod]->vme_A32D32_write(0x0, 0x8000); // reboot
	}

#endif
#ifdef VMUSB_VME_INTERFACE
#define MAX_INTERFACES 256   // based on address switches
	
	// Table of potential base addressses:

	uint32_t base_addresses[MAX_INTERFACES];
	for (uint32_t i =0; i < MAX_INTERFACES; i++) {
		base_addresses[i] = i << 24;
	}

	// Open the VMUSB interface:

	try {
		Globals::pUSBController = 
			CVMUSBFactory::createUSBController(
				CVMUSBFactory::local, nullptr
			);
	} catch(std::string msg) {
		std::cerr << "Unable to connect to a VMUSB: " << msg << std::endl;
		return -1;
	}
	// For each address if that's the base address of an sis3316
	// then assert the boot bit, wait a bit and deassert.
	//

	sis_vmusb_interface interface;
	interface.vmeopen();               // Must succeed.
	for (int i =0; i <MAX_INTERFACES; i++) {
		auto base = base_addresses[i];
		if (isSIS3316(interface, base)) {
			interface.vme_A32D32_write(base, ASSERT_REBOOT);
			usleep(DELAY_US);
			interface.vme_A32D32_write(base, DEASSERT_REBOOT);
		}
	}
#endif



		return 0;
}	
void *gpApplication(0);