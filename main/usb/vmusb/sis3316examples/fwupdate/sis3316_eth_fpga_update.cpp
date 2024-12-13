
/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_eth_fpga_update.cpp                                  */
/*                                                                         */
/*  Function:                                                              */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 18.02.2015                                       */
/*  last modification:    24.07.2024    (SIS3316-2 adaptation)             */
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
vme_interface_class* vme_crate;
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
                vme_crate = new sis_vmusb_interface;
                vme_crate->vmeopen();
        }
        catch (std::string msg) {
                std::cerr << "Unable to connect to a VMUSB:  "
                          << msg << std::endl;
                return -1;             // Fail.
        }
        return 0;    // Success
}

#endif

#include "sis3316_class.h"


vme_interface_class *intf;
sis3316_adc *adc;

void progressCallback(int percent);

int main(int argc, char *argv[])
{
//	int i ;
	int rc;
//	unsigned int addr;
	unsigned int data ;
	int return_code ;
	//out << "sis3316_fpga_update_udp" << endl; // prints sis3316_fpga_update_udp


	   //char char_command[256];
	   char fpga_bin_file[64];
	   unsigned int fpga_spi_address_offset ;
	   unsigned int vme_base_address ;
	   int int_ch ;
	   int ch ;
		char ch_string[64] ;
	char  sis3316_ip_addr_string[32] ;
#ifdef ETHERNET_UDP_INTERFACE

	char  pc_ip_addr_string[32] ;
	
	

	//strcpy(sis3316_ip_addr_string, gl_sis3316_ip_addr_string) ; // SIS3316 IP address
	//strcpy(sis3316_ip_addr_string,"212.60.16.200") ; // SIS3316 IP address
	strcpy(sis3316_ip_addr_string,"192.168.1.100") ; // SIS3316 IP address
#endif


	  // default
	   sprintf(fpga_bin_file, "sis3316vme_top.bin");
	   fpga_spi_address_offset = 0x0 ;
	   vme_base_address = 0x00000000 ;

	   if (argc > 1) {
	#ifdef test_print
		   /* Save command line into string "command" */
		   memset(char_command,0,sizeof(char_command));
		   for (i=1;i<argc;i++) {
				strcat(char_command,argv[i]);
				strcat(char_command," ");
			}
			printf("gl_command %s    \n", char_command);
	#endif

			while ((int_ch = getopt(argc, argv, "?hVAF:I:")) != -1)
				switch (int_ch) {
				  case 'I':
						sscanf(optarg,"%s", ch_string) ;
						printf("-I %s    \n", ch_string );
						strcpy(sis3316_ip_addr_string,ch_string) ;
						break;
				  case 'V':
						fpga_spi_address_offset = 0x0 ;
					break;
				  case 'A':
						fpga_spi_address_offset = 0x400000 ;
					break;
				  case 'F':
					sscanf(optarg,"%s", fpga_bin_file) ;
					break;
				  case 'B':
					sscanf(optarg,"%X", &vme_base_address) ;
					break;
				  case '?':
				  case 'h':
				  default:
					printf("Usage: %s  [-?h]  [-I ip] [-V or -A] [-F filename] ", argv[0]);
					printf("   \n");
				    printf("   -I string     SIS3316 IP Address       Default = %s\n", sis3316_ip_addr_string);
					printf("   -V or -A      -V->VME FPGA or -A->ADC FPGA \n");
					printf("   -F filename   for example sis3316vme_top.bin or sis3316_adc_top.bin \n");
					printf("   -B base       base address of the VME for crate based interfaces e.g. VMUSB\n");
					printf("   \n");
					printf("   -h            Print this message\n");
					printf("   \n");
					printf("   \n");
					printf("   date: 24.07.2024 \n");
					printf("   \n");
					printf("   \n");
					exit(1);
				  }

		 } // if (argc > 1)
		printf("\n");




 


#ifdef ETHERNET_UDP_INTERFACE
	#ifdef WINDOWS
    //return_code = WSAStartup();
    return_code = WinsockStartup();
	#endif
	sis3316_eth *vme_crate = new sis3316_eth;
	// increase read_buffer size
	// SUSE needs following command as su: >sysctl -w net.core.rmem_max=33554432
	int	sockbufsize = 335544432 ; // 0x2000000
	return_code = vme_crate->set_UdpSocketOptionBufSize(sockbufsize) ;

	//strcpy(pc_ip_addr_string,"212.60.16.49") ; // Window example: secocnd Lan interface IP address is 212.60.16.49
	strcpy(pc_ip_addr_string,"") ; // empty if default Lan interface (Window: use IP address to bind in case of 2. 3. 4. .. LAN Interface)
	return_code = vme_crate->set_UdpSocketBindMyOwnPort( pc_ip_addr_string);

	vme_crate->set_UdpSocketSIS3316_IpAddress( sis3316_ip_addr_string);

	vme_crate->udp_reset_cmd();
	vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000); // kill request and grant from other vme interface
	vme_crate->vme_A32D32_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x1); // request access to SIS3316 from UDP interface

    return_code = vme_crate->vme_A32D32_read(SIS3316_MODID,&data);
	printf("return_code = 0X%08x   Module ID = 0X%08x \n",return_code, data);
    return_code = vme_crate->vme_A32D32_read(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL,&data);
	printf("return_code = 0X%08x   ACCESS    = 0X%08x \n",return_code, data);
	printf("\n");

#endif
#ifdef VMUSB_INTERFACE
	if (connectVME()) {
		return -1;
	}
#endif
	char char_messages[128] ;
	unsigned int nof_found_devices ;

	// open Vme Interface device
	return_code = vme_crate->vmeopen ();  // open Vme interface
	vme_crate->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface
   // printf("get_vmeopen_messages = %s , nof_found_devices %d \n",char_messages, nof_found_devices);


		adc = new sis3316_adc(vme_crate, vme_base_address);
		rc = adc->register_read(0x4, &data);
		if ((rc != 0) || ((data & 0xffff0000) != 0x33160000)) {
		    printf("not valid SIS3316 vme base address\n");
			printf("return_code      = 0x%08X \n", rc);
			printf("vme_base_address = 0x%08X \n", vme_base_address);
			printf("module ID        = 0x%08X \n", data);
		    return -1;
	 	}
		else {
			printf("module ID / VME FPGA version       = 0x%08X \n", data);
			rc = adc->register_read(0x1100, &data);
			rc = adc->register_read(0x1100, &data);
			printf("module ID / ADC FPGA version       = 0x%08X \n", data);

		}
		printf("\n");





		do{
			printf("\ndo you want to program bin file -%s-  into ", fpga_bin_file);
			if (fpga_spi_address_offset == 0) {
				 printf("VME FPGA Flash space ? [y/n]: ");
			}
			else {
				if (adc->device_variant == SIS::ADC::SIS3316::TYPE_SIS3316) { // SIS3316
					fpga_spi_address_offset = 0x400000; // SIS3316
				}
				else {
					fpga_spi_address_offset = 0x1000000; // SIS3316-2
				}

				printf("ADC FPGA Flash space ? [y/n]: ");

			}
	        fflush(stdout);
	        fflush(stdin);
	        ch = getchar();
	        fflush(stdin);
	    }while(ch != 'y' && ch != 'n');
	    if(ch == 'n'){
	        return -1;
	    }

	    printf("attempting flash update, standby.....\n");
	    rc = adc->update_firmware(fpga_bin_file, fpga_spi_address_offset, progressCallback);
		if (rc == 0) {
			printf("flashing done\n");
		}
		else {
			printf("flashing NOT done  , rc = %d (0x%08X) \n", rc, rc);
	        return -1;
		}


		// read back and verifier
	    printf("verify_firmware.....\n");
	    rc = adc->verify_firmware(fpga_bin_file, fpga_spi_address_offset, progressCallback);
		if (rc == 0) {
			printf("verify_firmware done\n");
		}
		else {
			printf("verify_firmware NOT done\n");
	        return -1;
		}




 		do{
			printf("\ndo you want to reboot the FPGAs ? [y/n]:");
	        fflush(stdout);
	        fflush(stdin);
	        do {
	        	ch = getchar();
		    }while(ch == 0xa);
			//printf("\n ch %d",ch);
	        fflush(stdout);
	        fflush(stdin);
	    }while(ch != 'y' && ch != 'n');
	    if(ch == 'n'){
	        return -1;
	    }

		rc = adc->register_write(0x0, 0x8000); // reboot FPGAs ; switch SW80-7 has to be on (Watchdog)

		return 0;
	}



	void progressCallback(int percent)
	{
		//printf("percent %d \n", percent);
		printf("\rprogress: %d%%", percent);
		fflush(stdout);

	}
