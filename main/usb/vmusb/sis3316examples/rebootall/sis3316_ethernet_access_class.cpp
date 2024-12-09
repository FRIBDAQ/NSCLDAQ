/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_ethernet_access_class.cpp                            */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 26.07.2012                                       */
/*  last modification:    19.03.2015                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  � 2015                                                                 */
/*                                                                         */
/***************************************************************************/

/************************************************************************************/
/*  The Ethernet UDP protocol has changed with VME FPGA Firmware Version 2008       */
/*  - a packet identifier byte is added to the Request and Acknowlegde packet       */
/*  - a "retransmit last packet" command is added, too						        */
/*                                                                                  */
/************************************************************************************/




#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethnernet UDP)

#ifdef ETHERNET_UDP_INTERFACE

#include "sis3316_ethernet_access_class.h"


#ifdef LINUX
	#include <netdb.h>
#endif

#ifdef WIN
	#include <iostream>
	#include <cstdio>
	//	#include <string.h>
#endif

using namespace std;

sis3316_eth::sis3316_eth (void)
{
 	int status;
	strcpy (this->char_messages ,  "no valid UDP socket");

	this->udp_socket_status = -1 ;
	this->udp_port = 0xE000 ; // default

	this->myPC_sock_addr.sin_family = AF_INET;
	this->myPC_sock_addr.sin_port = htons(udp_port);
	this->myPC_sock_addr.sin_addr.s_addr = 0x0 ; //ADDR_ANY;
	memset(&(myPC_sock_addr.sin_zero),0,8);

	this->SIS3316_sock_addr_in.sin_family = AF_INET;
	this->SIS3316_sock_addr_in.sin_port = htons(udp_port);
	//this->SIS3316_sock_addr_in.sin_port = htons(0xe0aa);                     // test
	this->SIS3316_sock_addr_in.sin_addr.s_addr = 0x0 ; //ADDR_ANY;
	memset(&(SIS3316_sock_addr_in.sin_zero),0,8);
	// Create udp_socket
	this->udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->udp_socket == -1) {
		this->udp_socket_status = -1 ;
	}
	else {
		this->udp_socket_status = 0 ;
	}
 
	// Set  Receive Timeout
	this->recv_timeout_sec  = 0 ;
	//this->recv_timeout_usec = 100000 ; // default 100ms
	this->recv_timeout_usec = 50000 ; // default 50ms
	//printf("recv_timeout_sec = %d   recv_timeout_usec = %d\n",this->recv_timeout_sec, this->recv_timeout_usec);
	status = this->set_UdpSocketOptionTimeout() ;

	this->jumbo_frame_enable   = 0 ;
	this->max_nof_read_lwords  = UDP_NORMAL_READ_PACKET_32bitSIZE;
	this->max_nof_write_lwords = UDP_WRITE_PACKET_32bitSIZE;

	this->packet_identifier   = 0 ;
	// Info counter
	this->read_udp_register_receive_ack_retry_counter       = 0 ;
	this->read_sis3316_register_receive_ack_retry_counter   = 0 ;
	this->write_sis3316_register_receive_ack_retry_counter  = 0 ;
	this->read_sis3316_fifo_receive_ack_retry_counter       = 0 ;
	this->write_sis3316_fifo_receive_ack_retry_counter      = 0 ;

}



/*************************************************************************************/

int sis3316_eth::get_UdpSocketStatus(void ){
	return this->udp_socket_status;
}

int sis3316_eth::get_UdpSocketPort(void ){
	return this->udp_port;
}

	

/*************************************************************************************/

int sis3316_eth::set_UdpSocketOptionTimeout(void){
	int return_code ;
#ifdef LINUX
	struct timeval struct_time;
	struct_time.tv_sec  = this->recv_timeout_sec;
	struct_time.tv_usec = this->recv_timeout_usec; //  
	return_code = (setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&struct_time,sizeof(struct timeval)));
	//printf("struct_time.tv_sec = %d   struct_time.tv_usec = %d\n",struct_time.tv_sec, struct_time.tv_usec);
#endif

#ifdef WINDOWS
	int msec;
	msec = (1000 * this->recv_timeout_sec) + ((this->recv_timeout_usec+999) / 1000) ;
	return_code = (setsockopt(this->udp_socket, SOL_SOCKET, SO_RCVTIMEO,(char*)&msec,sizeof(msec)));
	//printf("return_code = %d    msec = %d\n",return_code, msec);
#endif
	return return_code ;
}


/*************************************************************************************/

	 
int sis3316_eth::set_UdpSocketOptionBufSize( int sockbufsize ){
	int return_code ;
	return_code =  (setsockopt(this->udp_socket, SOL_SOCKET,SO_RCVBUF, (char *) &sockbufsize, (int)sizeof(sockbufsize)));
	return return_code ;
}


/*************************************************************************************/

int sis3316_eth::set_UdpSocketBindToDevice( char* eth_device){
	int return_code=0;
#ifdef LINUX
#ifndef MAC_OSX
	return_code = setsockopt(this->udp_socket, SOL_SOCKET, SO_BINDTODEVICE, eth_device, sizeof(eth_device)) ;
#endif
#endif
	return return_code;
}

/*************************************************************************************/

int sis3316_eth::set_UdpSocketBindMyOwnPort( char* pc_ip_addr_string){
	int return_code;

	//this->udp_port = 0xE000 ; // default start UDP port
	this->myPC_sock_addr.sin_family = AF_INET;
	this->myPC_sock_addr.sin_port = htons(this->udp_port);
	this->myPC_sock_addr.sin_addr.s_addr = 0x0 ; //ADDR_ANY;
	memset(&(myPC_sock_addr.sin_zero),0,8);

	if(strlen(pc_ip_addr_string) != 0) {
		this->myPC_sock_addr.sin_addr.s_addr = inet_addr(pc_ip_addr_string);
	}

	do {
		return_code = bind(this->udp_socket,  (struct sockaddr *)&this->myPC_sock_addr, sizeof(this->myPC_sock_addr));
		if (return_code != 0) {
			this->udp_port++;
			this->myPC_sock_addr.sin_port = htons(this->udp_port);
		}
	} while ((return_code == -1) && (this->udp_port < 0xF000)) ;
	return return_code;
}


/*************************************************************************************/

int sis3316_eth::set_UdpSocketSIS3316_IpAddress( char* sis3316_ip_addr_string){
	int return_code=0;
	char temp_ip_addr_string[32] ;
	struct hostent *hp;     /* host information */

	this->SIS3316_sock_addr_in.sin_family = AF_INET;
	this->SIS3316_sock_addr_in.sin_port = htons(this->udp_port);
	//this->SIS3316_sock_addr_in.sin_port = htons(0xe0AA);                      // test
	this->SIS3316_sock_addr_in.sin_addr.s_addr = 0x0 ; //ADDR_ANY;
	memset(&(SIS3316_sock_addr_in.sin_zero),0,8);

	if(strlen(sis3316_ip_addr_string) != 0) {

/* sis3316_ip_addr_string beginn with "SIS3316" or "sis3316" and contains 12 characters: eg. "SIS3316-0123" [0123: Device-SN 123] */
		if(     (   (strncmp(sis3316_ip_addr_string, DHCP_DEVICE_NAME_LARGE_CASE, strlen(DHCP_DEVICE_NAME_LARGE_CASE)) == 0) 
			     || (strncmp(sis3316_ip_addr_string, DHCP_DEVICE_NAME_LOWER_CASE, strlen(DHCP_DEVICE_NAME_LOWER_CASE)) == 0))  
			 && (strlen(sis3316_ip_addr_string) == 12) ){

			hp = gethostbyname(sis3316_ip_addr_string);
			if(hp){
				memcpy((void *)&this->SIS3316_sock_addr_in.sin_addr, hp->h_addr_list[0], hp->h_length);
			}
			else{
				this->SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr(sis3316_ip_addr_string); // invalid IP string will set the IP to 255.255.255.255
				return_code=-2;
			}
		}
		else{
			this->SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr(sis3316_ip_addr_string); // invalid IP string will set the IP to 255.255.255.255
		}

	}
	else {
		strcpy(temp_ip_addr_string,"255.255.255.255") ; // 
		this->SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr(temp_ip_addr_string); // invalid IP string will set the IP to 255.255.255.255
		return_code = -1;	
	}
	if(this->SIS3316_sock_addr_in.sin_addr.s_addr == 0xffffffff ) { // broadcast address
		return_code = -1 ;
	}
	if((this->SIS3316_sock_addr_in.sin_addr.s_addr & 0xff) == 0x00 ) { // IP addresses 0.0.0.0 to 0.255.255.255 not allowed as Destination ! 
		strcpy(temp_ip_addr_string,"255.255.255.255") ; // 
		this->SIS3316_sock_addr_in.sin_addr.s_addr = inet_addr(temp_ip_addr_string); // invalid IP string will set the IP to 255.255.255.255
		return_code = -3;	
	}
	usleep(100) ;
	return return_code;
}



/*************************************************************************************/

unsigned int sis3316_eth::get_UdpSocketNofReadMaxLWordsPerRequest(void){

	return this->max_nof_read_lwords;
}

/*************************************************************************************/



unsigned int sis3316_eth::get_UdpSocketNofWriteMaxLWordsPerRequest(void){

	return this->max_nof_write_lwords;
}

/*************************************************************************************/



 

/*************************************************************************************/

int sis3316_eth::set_UdpSocketReceiveNofPackagesPerRequest(unsigned int nofPacketsPerRequest){
unsigned int nof_packets ;

	nof_packets = nofPacketsPerRequest ;
	if(nofPacketsPerRequest == 0){
		nof_packets = 1 ;
	}
	if(nofPacketsPerRequest > UDP_MAX_PACKETS_PER_REQUEST){
		nof_packets = UDP_MAX_PACKETS_PER_REQUEST ;
	}
	if (this->jumbo_frame_enable == 0) {
		this->max_nof_read_lwords = nof_packets * UDP_NORMAL_READ_PACKET_32bitSIZE;
	}
	else {
		this->max_nof_read_lwords = nof_packets * UDP_JUMBO_READ_PACKET_32bitSIZE;
	}
	return 0;
}


/*************************************************************************************/

int sis3316_eth::get_UdpSocketJumboFrameStatus(void){
	UINT data=0;
	this->udp_register_read(SIS3316_UDP_PROT_CONFIGURATION, &data);
	return (data >> 4) & 0x1;
}

/*************************************************************************************/

int sis3316_eth::set_UdpSocketEnableJumboFrame(void){
	UINT data=0;
	this->udp_register_read(SIS3316_UDP_PROT_CONFIGURATION, &data);
	data |= 0x10;
	this->udp_register_write(SIS3316_UDP_PROT_CONFIGURATION, data);
	this->jumbo_frame_enable   = 1 ;
	this->max_nof_read_lwords  = UDP_JUMBO_READ_PACKET_32bitSIZE;
	return 0;
}

/*************************************************************************************/

int sis3316_eth::set_UdpSocketDisableJumboFrame(void){
	UINT data=0;
	this->udp_register_read(SIS3316_UDP_PROT_CONFIGURATION, &data);
	data &= ~0x10;
	this->udp_register_write(SIS3316_UDP_PROT_CONFIGURATION, data);
	this->jumbo_frame_enable   = 0 ;
	this->max_nof_read_lwords  = UDP_NORMAL_READ_PACKET_32bitSIZE;
	return 0;
}

/*************************************************************************************/

int sis3316_eth::udp_reset_cmd( void){
	int return_code;
     // write Cmd
    this->udp_send_data[0] = 0xFF ; // reset
    return_code = sendto(this->udp_socket, udp_send_data, 1, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
	this->jumbo_frame_enable   = 0 ;
	this->max_nof_read_lwords  = UDP_NORMAL_READ_PACKET_32bitSIZE;
	usleep(1000) ;
	return return_code;
}




/*************************************************************************************/


int sis3316_eth::udp_retransmit_cmd( int* receive_bytes, char* data_byte_ptr){
	int return_code;

#ifdef LINUX
    socklen_t addr_len;
#endif
#ifdef WIN
	int addr_len;
 #endif
    addr_len = sizeof(struct sockaddr);
	// write Cmd
    this->udp_send_data[0] = 0xEE ; // transmit
    return_code = sendto(this->udp_socket, udp_send_data, 1, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
	*receive_bytes = recvfrom(this->udp_socket, this->udp_recv_data, 9000, 0,   (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len);
	if (*receive_bytes == -1) { // Timeout
		return -1 ;
	}

	memcpy( data_byte_ptr, &udp_recv_data[0], *receive_bytes) ;
	return 0;
}



/**************************************************************************************/


int sis3316_eth::udp_register_read (UINT addr, UINT* data)
{
	int return_code;
	int send_len ;
#ifdef LINUX
    socklen_t addr_len;
#endif
#ifdef WIN
	int addr_len;
 #endif
	unsigned int *data_ptr;
	addr_len = sizeof(struct sockaddr);

	// send Read Req Cmd
    this->udp_send_data[0] = 0x10 ; // register read CMD
#ifdef VME_FPGA_VERSION_IS_0008_OR_HIGHER
	send_len = 6 ;
	this->udp_send_data[1] = (this->packet_identifier) ;             // packet_identifier
	this->udp_send_data[2] = (unsigned char)  (addr & 0xff) ;        // address(7 dwonto 0)
    this->udp_send_data[3] = (unsigned char) ((addr >>  8) & 0xff) ; // address(15 dwonto 8)
    this->udp_send_data[4] = (unsigned char) ((addr >> 16) & 0xff) ; // address(23 dwonto 16)
    this->udp_send_data[5] = (unsigned char) ((addr >> 24) & 0xff) ; // address(31 dwonto 24)
	// send Request	
	return_code = sendto(this->udp_socket, udp_send_data, send_len, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));

	int retry_counter;
	retry_counter = 0 ;
	do {
		// read Ackn.
		return_code = recvfrom(udp_socket, udp_recv_data, 11, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );
		if (return_code == -1) { // timeout
			this->udp_send_data[0] = 0xEE ; // retransmit command, sis3316 send last packet again
			sendto(this->udp_socket, udp_send_data, 1, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
			retry_counter++ ;
			this->read_udp_register_receive_ack_retry_counter++ ;
		}
	} while ((return_code == -1) && (retry_counter < 4)) ; // retry up to 3 times

	if(return_code == -1) { // timeout
		return_code = -1;
	}
	else {
		if((return_code == 10) && (udp_recv_data[1] == this->packet_identifier)) {
			data_ptr = (unsigned int*)&udp_recv_data[6];
			*data = *data_ptr;
			return_code = 0;
		}
		else { 
			if (return_code != 10) {
				return_code = 0x121;
			}
			else {
				return_code = 0x122;
			}
		}	
	}
	this->packet_identifier++;

#else
	send_len = 5 ;
	this->udp_send_data[1] = (unsigned char)  (addr & 0xff) ;        // address(7 dwonto 0)
    this->udp_send_data[2] = (unsigned char) ((addr >>  8) & 0xff) ; // address(15 dwonto 8)
    this->udp_send_data[3] = (unsigned char) ((addr >> 16) & 0xff) ; // address(23 dwonto 16)
    this->udp_send_data[4] = (unsigned char) ((addr >> 24) & 0xff) ; // address(31 dwonto 24)
	// send Request	
	return_code = sendto(this->udp_socket, udp_send_data, send_len, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
	// read Ackn.
    return_code = recvfrom(udp_socket, udp_recv_data, 11, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );
	if(return_code == 9) {
		data_ptr = (unsigned int*)&udp_recv_data[5];
		*data = *data_ptr;
		return_code = 0;
    }
	else {
		return_code = -1;
	}

#endif

	return return_code;
}

 

/**************************************************************************************/

int sis3316_eth::udp_sis3316_register_read ( unsigned int nof_read_registers, UINT* addr_ptr, UINT* data_ptr)
{
    unsigned int i;
	int return_code;
#ifdef LINUX
    socklen_t addr_len;
#endif
#ifdef WIN
	int addr_len;
 #endif
    unsigned int nof_32bit_word;
    unsigned int reg_addr;
  
	//unsigned int *data_ptr;
    addr_len = sizeof(struct sockaddr);

	nof_32bit_word = nof_read_registers ;
	if (nof_read_registers == 0) {
		nof_32bit_word = 1 ;
	}
	if (nof_read_registers > 64) {
		nof_32bit_word = 64 ;
	}
	
	// send Read Req Cmd
	this->udp_send_data[0] = 0x20 ; // send SIS3316 Register Read Req Cmd
#ifdef VME_FPGA_VERSION_IS_0008_OR_HIGHER
	this->udp_send_data[1] = (unsigned char)  (this->packet_identifier) ;        // address(7 dwonto 0)
	this->udp_send_data[2] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    this->udp_send_data[3] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length 
	for (i=0;i<nof_32bit_word;i++) {
		reg_addr = addr_ptr[i] ;
		this->udp_send_data[(4*i)+4] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
		this->udp_send_data[(4*i)+5] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
		this->udp_send_data[(4*i)+6] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
		this->udp_send_data[(4*i)+7] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
	}
    return_code = sendto(this->udp_socket, udp_send_data, 4 + (4*nof_32bit_word), 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));

#ifdef old
	return_code = recvfrom(udp_socket, udp_recv_data, 512, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );
		//for (i=0;i<return_code;i++) {
		//   printf("0x%02X   ",(unsigned char)udp_recv_data[i]);
		//}
    if(return_code == (int)(3 + (4*nof_32bit_word))) {
		for (i=0;i<nof_32bit_word;i++) {
			data_ptr[i] = *((unsigned int *)&udp_recv_data[3+(4*i)]) ;
		}
		return_code = 0;
    }
	else {
		return_code = -1;
	}
#endif
 
	int retry_counter;
	retry_counter = 0 ;
	do {
		// read Ackn.
		return_code = recvfrom(udp_socket, udp_recv_data, 512, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );
		if (return_code == -1) { // timeout
			this->udp_send_data[0] = 0xEE ; // retransmit command, sis3316 send last packet again
			sendto(this->udp_socket, udp_send_data, 1, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
			retry_counter++ ;
			this->read_sis3316_register_receive_ack_retry_counter++ ;
		}
	} while ((return_code == -1) && (retry_counter < 4)) ; // retry up to 3 times

	if(return_code == -1) { // timeout
		return_code = -1;
	}
	else {
		if((return_code == (int)(3 + (4*nof_32bit_word))) && (udp_recv_data[1] == this->packet_identifier)) {
			memcpy((unsigned char*)data_ptr, &udp_recv_data[3], nof_32bit_word*4) ;
			//for (i=0;i<nof_32bit_word;i++) {
			//	data_ptr[i] = *((unsigned int *)&udp_recv_data[3+(4*i)]) ;
			//}
			return_code = 0;
		}
		else { 
			if (return_code != 10) {
				return_code = 0x121;
			}
			else {
				return_code = 0x122;
			}
		}	
	}
	this->packet_identifier++;



#else
	this->udp_send_data[1] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    this->udp_send_data[2] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length 
	for (i=0;i<nof_32bit_word;i++) {
		reg_addr = addr_ptr[i] ;
		this->udp_send_data[(4*i)+3] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
		this->udp_send_data[(4*i)+4] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
		this->udp_send_data[(4*i)+5] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
		this->udp_send_data[(4*i)+6] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
	}
    return_code = sendto(this->udp_socket, udp_send_data, 3 + (4*nof_32bit_word), 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
	return_code = recvfrom(udp_socket, udp_recv_data, 512, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );
    if(return_code == (int)(2 + (4*nof_32bit_word))) {

		memcpy((unsigned char*)data_ptr, &udp_recv_data[2], nof_32bit_word*4) ;

    	//for (i=0;i<nof_32bit_word;i++) {
		//	data_ptr[i] = *((unsigned int *)&udp_recv_data[2+(4*i)]) ;
		//}
		return_code = 0;
    }
	else {
		return_code = -1;
	}
#endif



	return return_code;
}

/**************************************************************************************/




int sis3316_eth::udp_register_write (UINT addr, UINT data)
{
	volatile int return_code;
#ifdef LINUX
    socklen_t addr_len;
#endif
#ifdef WIN
	int addr_len;
 #endif
    //unsigned int *data_ptr;
    addr_len = sizeof(struct sockaddr);
  // send Read Req Cmd
    this->udp_send_data[0] = 0x11 ; // register write CMD  
    this->udp_send_data[1] = (unsigned char)  (addr & 0xff) ;        // address(7 dwonto 0)
    this->udp_send_data[2] = (unsigned char) ((addr >>  8) & 0xff) ; // address(15 dwonto 8)
    this->udp_send_data[3] = (unsigned char) ((addr >> 16) & 0xff) ; // address(23 dwonto 16)
    this->udp_send_data[4] = (unsigned char) ((addr >> 24) & 0xff) ; // address(31 dwonto 24)
    this->udp_send_data[5] = (unsigned char)  (data & 0xff) ;        // data(7 dwonto 0)
    this->udp_send_data[6] = (unsigned char) ((data >>  8) & 0xff) ; // data(15 dwonto 8)
    this->udp_send_data[7] = (unsigned char) ((data >> 16) & 0xff) ; // data(23 dwonto 16)
    this->udp_send_data[8] = (unsigned char) ((data >> 24) & 0xff) ; // data(31 dwonto 24)
    return_code = sendto(this->udp_socket, udp_send_data, 9, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
	if (return_code == 9) {
		return_code = 0;
	}
	else {
		usleep(1); // this call is needed because the VC10 compiler optimizer somtimes removes this call (udp_register_write)
	}

		//printf("udp_register_write:return_code = %d\n",return_code);

	return return_code;
}


/**************************************************************************************/

int sis3316_eth::udp_sis3316_register_write ( unsigned int nof_write_registers, UINT* addr_ptr, UINT* data_ptr)
{
    unsigned int i;
	int return_code;
#ifdef LINUX
    socklen_t addr_len;
#endif
#ifdef WIN
	int addr_len;
 #endif
    unsigned int nof_32bit_word;
    unsigned int reg_addr, reg_data;

    addr_len = sizeof(struct sockaddr);

	nof_32bit_word = nof_write_registers ;
	if (nof_write_registers == 0) {
		nof_32bit_word = 1 ;
	}
	if (nof_write_registers > 64) {
		nof_32bit_word = 64 ;
	}




	// send Write Req Cmd
	this->udp_send_data[0] = 0x21 ; // send SIS3316 Register Write Req Cmd
#ifdef VME_FPGA_VERSION_IS_0008_OR_HIGHER

	this->udp_send_data[1] = (unsigned char)  (this->packet_identifier) ;        // packet_identifier
	this->udp_send_data[2] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    this->udp_send_data[3] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length 

	for (i=0;i<nof_32bit_word;i++) {
		reg_addr = addr_ptr[i] ;
		this->udp_send_data[(8*i)+4] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
		this->udp_send_data[(8*i)+5] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
		this->udp_send_data[(8*i)+6] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
		this->udp_send_data[(8*i)+7] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
		reg_data = data_ptr[i] ;
		this->udp_send_data[(8*i)+8]  = (unsigned char)  (reg_data & 0xff) ;        // reg_data(7 dwonto 0)
		this->udp_send_data[(8*i)+9]  = (unsigned char) ((reg_data >>  8) & 0xff) ; // reg_data(15 dwonto 8)
		this->udp_send_data[(8*i)+10]  = (unsigned char) ((reg_data >> 16) & 0xff) ; // reg_data(23 dwonto 16)
		this->udp_send_data[(8*i)+11] = (unsigned char) ((reg_data >> 24) & 0xff) ; // reg_data(31 dwonto 24)
	}
    return_code = sendto(this->udp_socket, udp_send_data, 4 + (8*nof_32bit_word), 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
#ifdef old
	//printf("udp_sis3316_register_write: recvfrom\n");
	return_code = recvfrom(udp_socket, udp_recv_data, 512, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );

    if(return_code == 3 ) {
		return_code = 0;
    }
	else {
	  //printf("\n");
	  //printf("udp_sis3316_register_write: RecTimeout    return_code = %d    nof_32bit_word = %d  reg_data = 0x%08X \n\n",return_code, nof_32bit_word, data_ptr[0]);
		return_code = -1;
	}
#endif
 
	int retry_counter;
	retry_counter = 0 ;
	do {
		// read Ackn.
		return_code = recvfrom(this->udp_socket, udp_recv_data, 512, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );
		if (return_code == -1) { // timeout
			this->udp_send_data[0] = 0xEE ; // retransmit command, sis3316 send last packet again
			sendto(this->udp_socket, udp_send_data, 1, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
			retry_counter++ ;
			this->write_sis3316_register_receive_ack_retry_counter++ ;
		}
	} while ((return_code == -1) && (retry_counter < 4)) ; // retry up to 3 times

	if(return_code == -1) { // timeout
		return_code = -1;
	}
	else {
		if((return_code == 3) && (udp_recv_data[1] == this->packet_identifier)) {
			return_code = 0;
		}
		else { 
			if (return_code != 10) {
				return_code = 0x121;
			}
			else {
				return_code = 0x122;
			}
		}	
	}
	this->packet_identifier++;



#else
	this->udp_send_data[1] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    this->udp_send_data[2] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length 

	for (i=0;i<nof_32bit_word;i++) {
		reg_addr = addr_ptr[i] ;
		this->udp_send_data[(8*i)+3] = (unsigned char)  (reg_addr & 0xff) ;        // address(7 dwonto 0)
		this->udp_send_data[(8*i)+4] = (unsigned char) ((reg_addr >>  8) & 0xff) ; // address(15 dwonto 8)
		this->udp_send_data[(8*i)+5] = (unsigned char) ((reg_addr >> 16) & 0xff) ; // address(23 dwonto 16)
		this->udp_send_data[(8*i)+6] = (unsigned char) ((reg_addr >> 24) & 0xff) ; // address(31 dwonto 24)
		reg_data = data_ptr[i] ;
		this->udp_send_data[(8*i)+7]  = (unsigned char)  (reg_data & 0xff) ;        // reg_data(7 dwonto 0)
		this->udp_send_data[(8*i)+8]  = (unsigned char) ((reg_data >>  8) & 0xff) ; // reg_data(15 dwonto 8)
		this->udp_send_data[(8*i)+9]  = (unsigned char) ((reg_data >> 16) & 0xff) ; // reg_data(23 dwonto 16)
		this->udp_send_data[(8*i)+10] = (unsigned char) ((reg_data >> 24) & 0xff) ; // reg_data(31 dwonto 24)
	}
    return_code = sendto(this->udp_socket, udp_send_data, 3 + (8*nof_32bit_word), 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));

	//printf("udp_sis3316_register_write: recvfrom\n");
	return_code = recvfrom(udp_socket, udp_recv_data, 512, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );

    if(return_code == 2 ) {
		return_code = 0;
    }
	else {
	  //printf("\n");
	  //printf("udp_sis3316_register_write: RecTimeout    return_code = %d    nof_32bit_word = %d  reg_data = 0x%08X \n\n",return_code, nof_32bit_word, data_ptr[0]);
		return_code = -1;
	}
#endif

	return return_code;
}

/**************************************************************************************/


/**************************************************************************************/



int sis3316_eth::udp_sub_sis3316_fifo_read ( unsigned int nof_read_words, UINT  addr, UINT* data_ptr)
{
    //int i;
	int return_code;
	int send_return_code;
#ifdef LINUX
    socklen_t addr_len;
#endif
#ifdef WIN
	int addr_len;
 #endif
    unsigned int nof_32bit_word;
  	int send_length;
 
	unsigned int udp_data_copy_to_buffer_index ;
	unsigned int nof_read_data_bytes ;
	int rest_length_byte ;
	//unsigned int soft_packet_number;
	int receive_bytes;

#ifdef VME_FPGA_VERSION_IS_0008_OR_HIGHER
	unsigned char packet_order_error_counter;
	unsigned int  expected_nof_packets;
	int retry_counter;
#endif
	unsigned char* uchar_ptr;
	//unsigned char uchar_packet_cmd;
	//unsigned char uchar_packet_received_id;
	unsigned char uchar_packet_status;
	unsigned char uchar_packet_number;
	unsigned char uchar_soft_packet_number;
	
	addr_len = sizeof(struct sockaddr);
	nof_32bit_word = nof_read_words ;
	if (nof_read_words == 0) {
		return 0 ;
	}

	// prepare Read Req Cmd
	this->udp_send_data[0] = 0x30 ; // send SIS3316 Fifo Read Req Cmd

#ifdef VME_FPGA_VERSION_IS_0008_OR_HIGHER
	this->udp_send_data[1] = (unsigned char)  (this->packet_identifier) ;        // packet_identifier
	this->udp_send_data[2] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    this->udp_send_data[3] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length 

	this->udp_send_data[4] = (unsigned char)  (addr & 0xff) ;        // address(7 dwonto 0)
	this->udp_send_data[5] = (unsigned char) ((addr >>  8) & 0xff) ; // address(15 dwonto 8)
	this->udp_send_data[6] = (unsigned char) ((addr >> 16) & 0xff) ; // address(23 dwonto 16)
	this->udp_send_data[7] = (unsigned char) ((addr >> 24) & 0xff) ; // address(31 dwonto 24)
	send_length = 8 ;
	// send Read Req Cmd
    send_return_code = sendto(this->udp_socket, udp_send_data, send_length, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
	if (send_return_code != send_length) {
			return -2 ;
	}

	uchar_ptr = (unsigned char* ) data_ptr ;
	rest_length_byte = 4 * nof_32bit_word ;
	udp_data_copy_to_buffer_index = 0 ;
	uchar_soft_packet_number = 0;


	//expected_nof_packets is used to force a retry cycle in case of timeout if expected_nof_packets == 1
	if (this->jumbo_frame_enable == 0) {
		expected_nof_packets = (nof_32bit_word + (UDP_NORMAL_READ_PACKET_32bitSIZE - 1)) / UDP_NORMAL_READ_PACKET_32bitSIZE ;
	}
	else {
		expected_nof_packets = (nof_32bit_word + (UDP_JUMBO_READ_PACKET_32bitSIZE - 1)) / UDP_JUMBO_READ_PACKET_32bitSIZE ;
	}


	packet_order_error_counter = 0 ;
	retry_counter = 0 ;
	do {
		// read Ackn.
		do {
			receive_bytes = recvfrom(this->udp_socket, this->udp_recv_data, 9000, 0,   (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len);
			if (receive_bytes == -1) { // timeout
				if (expected_nof_packets == 1) { // in this case try a retry !
					this->udp_send_data[0] = 0xEE ; // retransmit command, sis3316 send last packet again
					sendto(this->udp_socket, udp_send_data, 1, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
					retry_counter++ ;
					this->read_sis3316_fifo_receive_ack_retry_counter++ ;
				}
				else { // multiple packets
					return -1 ;
				}
			}
		//} while ((receive_bytes == -1) && (retry_counter < 4) && ((rest_length_byte - nof_read_data_bytes) == 0)) ; // retry up to 3 times
		} while ((receive_bytes == -1) && (retry_counter < 4) && (expected_nof_packets == 1)) ; // retry up to 3 times

		if(receive_bytes == -1) { // timeout
			return -1 ;
		}

		// check Packet cmd
		if(udp_recv_data[0] != 0x30) {
			return PROTOCOL_ERROR_CODE_WRONG_ACK ;
		}
		// check packet_identifier
		if(udp_recv_data[1] != this->packet_identifier) {
			return PROTOCOL_ERROR_CODE_WRONG_PACKET_IDENTIFIER ;
		}


		uchar_packet_status       =  (udp_recv_data[2] & 0x70) >> 4 ;
		uchar_packet_number = (unsigned int) udp_recv_data[2] & 0xf ;
		if (uchar_packet_status != 0x0) {
			//printf("Error udp_sub_sis3316_fifo_read: packet status error\n");
			// packet status error !
		}
		// check Packet number
		if (uchar_packet_number != uchar_soft_packet_number) {
			packet_order_error_counter++;
			//printf("Error udp_sub_sis3316_fifo_read: lost packet  error\n");
			// Observation: Windows in combination with a fire-wall (symantec) changes in order of the incoming packets (runs on multiple cores !) 
		}
		uchar_soft_packet_number = (uchar_soft_packet_number + 1) & 0xf;

		nof_read_data_bytes = receive_bytes-3  ;
		memcpy(&uchar_ptr[udp_data_copy_to_buffer_index], &udp_recv_data[3], nof_read_data_bytes) ;
		udp_data_copy_to_buffer_index = udp_data_copy_to_buffer_index +  nof_read_data_bytes;
		rest_length_byte = rest_length_byte - nof_read_data_bytes ;
	} while (rest_length_byte > 0) ;
	this->packet_identifier++;

	if (packet_order_error_counter == 0) {
		return_code = 0 ;
	}
	else {
		return_code = PROTOCOL_ERROR_CODE_WRONG_RECEIVED_PACKET_ORDER ;
	}

 
#ifdef old
	do {
		receive_bytes = recvfrom(this->udp_socket, this->udp_recv_data, 9000, 0,   (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len);
		if (receive_bytes == -1) { // Timeout
			//printf("Error udp_sub_sis3316_fifo_read: Timeout\n");
			//printf("Error: recvfrom receive_bytes =  %d   \n",receive_bytes);
			//printf("receive_bytes  %d   Ack = %2x  Status = %2x  data = %2x \n",receive_bytes, (unsigned char) udp_recv_data[0], (unsigned char) udp_recv_data[1], (unsigned char) udp_recv_data[2]);
			//printf("soft_packet_number  %d   \n",soft_packet_number);
			//printf("udp_data_copy_to_buffer_index  %d   \n",udp_data_copy_to_buffer_index);
			return -1 ;
			//break ;
		}
		
		uchar_packet_cmd          =  udp_recv_data[0]  ;
		uchar_packet_received_id  =  udp_recv_data[1]  ;
		uchar_packet_status       =  (udp_recv_data[2] & 0x70) >> 4 ;
		uchar_packet_number = (unsigned int) udp_recv_data[2] & 0xf ;
		// check Packet cmd
		if (uchar_packet_cmd != 0x30) {
			//printf("Error udp_sub_sis3316_fifo_read: packet cmd error\n");
			// packet cmd error !
		}
		if (uchar_packet_status != 0x0) {
			//printf("Error udp_sub_sis3316_fifo_read: packet status error\n");
			// packet status error !
		}
		// check Packet number
		if (uchar_packet_number != uchar_soft_packet_number) {
			//printf("Error udp_sub_sis3316_fifo_read: lost packet  error\n");
			// lost packet error !
		}
		uchar_soft_packet_number = (uchar_soft_packet_number + 1) & 0xf;

		nof_read_data_bytes = receive_bytes-3  ;
		memcpy(&uchar_ptr[udp_data_copy_to_buffer_index], &udp_recv_data[3], nof_read_data_bytes) ;
		udp_data_copy_to_buffer_index = udp_data_copy_to_buffer_index +  nof_read_data_bytes;
		rest_length_byte = rest_length_byte - nof_read_data_bytes ;
	} while (rest_length_byte > 0) ;
#endif

#else
	this->udp_send_data[1] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    this->udp_send_data[2] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length 

	this->udp_send_data[3] = (unsigned char)  (addr & 0xff) ;        // address(7 dwonto 0)
	this->udp_send_data[4] = (unsigned char) ((addr >>  8) & 0xff) ; // address(15 dwonto 8)
	this->udp_send_data[5] = (unsigned char) ((addr >> 16) & 0xff) ; // address(23 dwonto 16)
	this->udp_send_data[6] = (unsigned char) ((addr >> 24) & 0xff) ; // address(31 dwonto 24)

	send_length = 7 ;
	// send Read Req Cmd
    send_return_code = sendto(this->udp_socket, udp_send_data, send_length, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
	if (send_return_code != send_length) {
			return -2 ;
	}


	uchar_ptr = (unsigned char* ) data_ptr ;
	rest_length_byte = 4 * nof_32bit_word ;
	udp_data_copy_to_buffer_index = 0 ;
	uchar_soft_packet_number = 0;


	do {
		receive_bytes = recvfrom(this->udp_socket, this->udp_recv_data, 9000, 0,   (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len);
		if (receive_bytes == -1) { // Timeout
			//printf("Error udp_sub_sis3316_fifo_read: Timeout\n");
			//printf("Error: recvfrom receive_bytes =  %d   \n",receive_bytes);
			//printf("receive_bytes  %d   Ack = %2x  Status = %2x  data = %2x \n",receive_bytes, (unsigned char) udp_recv_data[0], (unsigned char) udp_recv_data[1], (unsigned char) udp_recv_data[2]);
			//printf("soft_packet_number  %d   \n",soft_packet_number);
			//printf("udp_data_copy_to_buffer_index  %d   \n",udp_data_copy_to_buffer_index);
			return -1 ;
			//break ;
		}

		// check Packet cmd
		if(udp_recv_data[0] != 0x30) {
			return 0x120 ;
		}

		uchar_packet_status       =  (udp_recv_data[1] & 0x70) >> 4 ;
		uchar_packet_number = (unsigned int) udp_recv_data[1] & 0xf ;
		// check Packet cmd
		if (uchar_packet_status != 0x0) {
			//printf("Error udp_sub_sis3316_fifo_read: packet status error\n");
			// packet status error !
		}
		// check Packet number
		if (uchar_packet_number != uchar_soft_packet_number) {
			//printf("Error udp_sub_sis3316_fifo_read: lost packet  error\n");
			// lost packet error !
		}
		uchar_soft_packet_number = (uchar_soft_packet_number + 1) & 0xf;

		nof_read_data_bytes = receive_bytes-2  ;
		memcpy(&uchar_ptr[udp_data_copy_to_buffer_index], &udp_recv_data[2], nof_read_data_bytes) ;
		udp_data_copy_to_buffer_index = udp_data_copy_to_buffer_index +  nof_read_data_bytes;
		rest_length_byte = rest_length_byte - nof_read_data_bytes ;
	} while (rest_length_byte > 0) ;
	return_code = 0 ;
#endif
    return return_code ;
    // -1: timeout, > 0; wrong number in receive_bytes 

}


int sis3316_eth::udp_sis3316_fifo_read ( unsigned int nof_read_words, UINT addr, UINT* data_ptr, UINT* got_nof_words )
{
 //   int error;
	int return_code;
    unsigned int rest_length_words;
    unsigned int req_nof_words;
    unsigned int data_buffer_index;

	*got_nof_words = 0x0 ;
	if (nof_read_words == 0) {
		return 0 ;
	}
	
//	error = 0 ;
	rest_length_words = nof_read_words ;
	data_buffer_index = 0 ;


	do {
		if (rest_length_words >= this->max_nof_read_lwords) {
			req_nof_words = this->max_nof_read_lwords ;
		}
		else {
			req_nof_words = rest_length_words ;
		}
		return_code = this->udp_sub_sis3316_fifo_read ( req_nof_words, addr, &data_ptr[data_buffer_index]) ;
		//printf("udp_sub_sis3316_fifo_read: req_nof_words    = %d     \n", req_nof_words);

		//if (return_code == -1) { // Timeout
		//	error = -1 ;
		//}

		if (return_code == 0) {  
			data_buffer_index = data_buffer_index + req_nof_words ;
			rest_length_words = rest_length_words - req_nof_words ; 
		}

	} while ((return_code == 0) && (rest_length_words>0)) ;

	*got_nof_words = data_buffer_index ;
	//return_code = error ;

    return return_code;

}







int sis3316_eth::udp_sub_sis3316_fifo_write ( unsigned int nof_write_words, UINT  addr, UINT* data_ptr)
{
    int i;
	int return_code;
#ifdef LINUX
    socklen_t addr_len;
#endif
#ifdef WIN
	int addr_len;
 #endif
    unsigned int nof_32bit_word;
    unsigned int send_data;
  	int send_length;
 
	//unsigned int udp_data_copy_to_buffer_index ;
	//int rest_length_byte ;
	//unsigned int soft_packet_number;
#ifdef VME_FPGA_VERSION_IS_0008_OR_HIGHER
#else
	unsigned char uchar_packet_cmd;
	unsigned char uchar_packet_status;

#endif
	//unsigned int packet_status;
	//unsigned int packet_cmd;
	//unsigned char uchar_packet_cmd;
	//unsigned char uchar_packet_received_id;
	//unsigned char uchar_packet_status;

	//unsigned int *data_ptr;
    addr_len = sizeof(struct sockaddr);

	nof_32bit_word = nof_write_words ;
	if (nof_write_words == 0) {
		return 0 ;
		//nof_32bit_word = 1 ;
	}

	if (nof_write_words > this->max_nof_write_lwords) {
		nof_32bit_word = this->max_nof_write_lwords ;
	}
	
	// send Read Req Cmd
	this->udp_send_data[0] = 0x31 ; // send SIS3316 Fifo Write Req Cmd
	
#ifdef VME_FPGA_VERSION_IS_0008_OR_HIGHER
	this->udp_send_data[1] = (unsigned char)  (this->packet_identifier) ;        // address(7 dwonto 0)
	
	this->udp_send_data[2] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    this->udp_send_data[3] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length 

	this->udp_send_data[4] = (unsigned char)  (addr & 0xff) ;        // address(7 dwonto 0)
	this->udp_send_data[5] = (unsigned char) ((addr >>  8) & 0xff) ; // address(15 dwonto 8)
	this->udp_send_data[6] = (unsigned char) ((addr >> 16) & 0xff) ; // address(23 dwonto 16)
	this->udp_send_data[7] = (unsigned char) ((addr >> 24) & 0xff) ; // address(31 dwonto 24)


	send_length = 8 + (4*nof_32bit_word);
	for (i=0;i<(int)nof_32bit_word;i++) {
		send_data = data_ptr[i] ;
		this->udp_send_data[8+(4*i)] = (unsigned char)  (send_data & 0xff) ;        // send_data(7 dwonto 0)
		this->udp_send_data[9+(4*i)] = (unsigned char) ((send_data >>  8) & 0xff) ; // send_data(15 dwonto 8)
		this->udp_send_data[10+(4*i)] = (unsigned char) ((send_data >> 16) & 0xff) ; // send_data(23 dwonto 16)
		this->udp_send_data[11+(4*i)] = (unsigned char) ((send_data >> 24) & 0xff) ; // send_data(31 dwonto 24)
	}

    return_code = sendto(this->udp_socket, udp_send_data, send_length, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
	if (return_code != send_length) {
		//printf("sendto: return_code = 0x%08x \n",return_code);
	}



	int retry_counter;
	retry_counter = 0 ;
	do {
		// read Ackn.
		return_code = recvfrom(this->udp_socket, udp_recv_data, 512, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );
		if (return_code == -1) { // timeout
			this->udp_send_data[0] = 0xEE ; // retransmit command, sis3316 send last packet again
			sendto(this->udp_socket, udp_send_data, 1, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
			retry_counter++ ;
			this->write_sis3316_fifo_receive_ack_retry_counter++ ;
		}
	} while ((return_code == -1) && (retry_counter < 4)) ; // retry up to 3 times

	if(return_code == -1) { // timeout
		return_code = -1;
	}
	else {
		if((return_code == 3) && (udp_recv_data[1] == this->packet_identifier)) {
			return_code = 0;
		}
		else { 
			if (return_code != 3) {
				return_code = 0x121;
			}
			else {
				return_code = 0x122;
			}
		}	
	}
	this->packet_identifier++;


// error codes

#else	
	
	this->udp_send_data[1] = (unsigned char)  ((nof_32bit_word-1) & 0xff);           //  lower length
    this->udp_send_data[2] = (unsigned char) (((nof_32bit_word-1) >>  8) & 0xff);    //  upper length 

	this->udp_send_data[3] = (unsigned char)  (addr & 0xff) ;        // address(7 dwonto 0)
	this->udp_send_data[4] = (unsigned char) ((addr >>  8) & 0xff) ; // address(15 dwonto 8)
	this->udp_send_data[5] = (unsigned char) ((addr >> 16) & 0xff) ; // address(23 dwonto 16)
	this->udp_send_data[6] = (unsigned char) ((addr >> 24) & 0xff) ; // address(31 dwonto 24)


	send_length = 7 + (4*nof_32bit_word);
	for (i=0;i<(int)nof_32bit_word;i++) {
		send_data = data_ptr[i] ;
		this->udp_send_data[7+(4*i)] = (unsigned char)  (send_data & 0xff) ;        // send_data(7 dwonto 0)
		this->udp_send_data[8+(4*i)] = (unsigned char) ((send_data >>  8) & 0xff) ; // send_data(15 dwonto 8)
		this->udp_send_data[9+(4*i)] = (unsigned char) ((send_data >> 16) & 0xff) ; // send_data(23 dwonto 16)
		this->udp_send_data[10+(4*i)] = (unsigned char) ((send_data >> 24) & 0xff) ; // send_data(31 dwonto 24)
	}

    return_code = sendto(this->udp_socket, udp_send_data, send_length, 0, (struct sockaddr *)&this->SIS3316_sock_addr_in, sizeof(struct sockaddr));
	if (return_code != send_length) {
		//printf("sendto: return_code = 0x%08x \n",return_code);
	}

	return_code = recvfrom(udp_socket, udp_recv_data, 512, 0,  (struct sockaddr *)&this->SIS3316_sock_addr_in, &addr_len );
    if(return_code != 2 ) {
		//printf("recvfrom: return_code = 0x%08x \n",return_code);
		return -1; 
    }

	uchar_packet_cmd         =  udp_recv_data[0]  ;
	uchar_packet_status      =  (udp_recv_data[1] & 0x70) >> 4 ;
	if (uchar_packet_cmd != 0x31) {
		// packet cmd error !
		return -2; 
	}
	if (uchar_packet_status != 0x0) {
		return -3; 
		// packet status error !
	}
#endif
    return 0; 

}


int sis3316_eth::udp_sis3316_fifo_write ( unsigned int nof_write_words, UINT addr, UINT* data_ptr, UINT* written_nof_words )
{
    int error;
	int return_code;
    unsigned int rest_length_words;
    unsigned int req_nof_words;
    unsigned int data_buffer_index;

	*written_nof_words = 0x0 ;
	if (nof_write_words == 0) {
		return 0 ;
	}
	
	error = 0 ;
	rest_length_words = nof_write_words ;
	data_buffer_index = 0 ;

	//printf("receive_bytes  %d   Ack = %2x  Status = %2x  data = %2x \n",receive_bytes, (unsigned char) udp_recv_data[0], (unsigned char) udp_recv_data[1], (unsigned char) udp_recv_data[2]);
	do {
		if (rest_length_words >= this->max_nof_write_lwords) {
			req_nof_words = this->max_nof_write_lwords ;

		}
		else {
			req_nof_words = rest_length_words ;
		}
		return_code = this->udp_sub_sis3316_fifo_write ( req_nof_words, addr, &data_ptr[data_buffer_index]) ;
		error = return_code ;
		data_buffer_index = data_buffer_index + req_nof_words ;
		rest_length_words = rest_length_words - req_nof_words ; 
	} while ((error == 0) && (rest_length_words>0)) ;

	*written_nof_words = data_buffer_index ;
	return_code = error ;

    return return_code;

}





/*************************************************************************************************************************************************/
/*************************************************************************************************************************************************/
/***                                                                                                                                           ***/
/***     "emulate" VME access routines. This make it possible to use the virtual class "vme_interface_class"                                   ***/
/***                                                                                                                                           ***/
/*************************************************************************************************************************************************/
/*************************************************************************************************************************************************/


// vmeopen is implemented to b 
int sis3316_eth::vmeopen ( void )
{
	strcpy (this->char_messages ,  "sis3316 UDP port is open");
	return 0;
}



int sis3316_eth::vmeclose( void ){
	//CloseHandle (this->sis3150_device);
	return 0;
}

int sis3316_eth::get_vmeopen_messages( CHAR* messages, UINT* nof_found_devices ){

	strcpy (messages,  this->char_messages);
	*nof_found_devices = 1 ;

	return 0;
}



int sis3316_eth::vme_A32D32_read (UINT addr, UINT* data)
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range

	if (udp_address < 0x20) {
		return_code = this->udp_register_read(udp_address, data)  ;
		if(return_code == 0) {
			return_code = 0;
		}
		else {
			return_code = 0x211;
		}
	}
	else {
		return_code = this->udp_sis3316_register_read(1, &udp_address, data)  ;
		if(return_code == 0) {
			return_code = 0;
		}
		else {
			return_code = 0x211;
		}
	}
	return return_code;
}

/**************************************************************************************/


/**************************************************************************************/

int sis3316_eth::vme_A32DMA_D32_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	return -1; // not implemented
}

/**************************************************************************************/

int sis3316_eth::vme_A32BLT32_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_read(request_nof_words, udp_address, data, got_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 
}

/**************************************************************************************/

int sis3316_eth::vme_A32MBLT64_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_read(request_nof_words, udp_address, data, got_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 
}

/**************************************************************************************/

int sis3316_eth::vme_A32_2EVME_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	return -1; // not implemented
}

/**************************************************************************************/

int sis3316_eth::vme_A32_2ESST160_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	return -1; // not implemented
}

/**************************************************************************************/

int sis3316_eth::vme_A32_2ESST267_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	return -1; // not implemented
}

/**************************************************************************************/

int sis3316_eth::vme_A32_2ESST320_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	return -1; // not implemented
}

/**************************************************************************************/


/**************************************************************************************/

int sis3316_eth::vme_A32DMA_D32FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	return -1; // not implemented
}

/**************************************************************************************/

int sis3316_eth::vme_A32BLT32FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_read(request_nof_words, udp_address, data, got_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 
}

/**************************************************************************************/

int sis3316_eth::vme_A32MBLT64FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_read(request_nof_words, udp_address, data, got_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 
}

/**************************************************************************************/

int sis3316_eth::vme_A32_2EVMEFIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_read(request_nof_words, udp_address, data, got_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 

}

/**************************************************************************************/

int sis3316_eth::vme_A32_2ESST160FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_read(request_nof_words, udp_address, data, got_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 

}

/**************************************************************************************/

int sis3316_eth::vme_A32_2ESST267FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_read(request_nof_words, udp_address, data, got_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 

}

/**************************************************************************************/

int sis3316_eth::vme_A32_2ESST320FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_read(request_nof_words, udp_address, data, got_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 

}

/**************************************************************************************/








/**************************************************************************************/

int sis3316_eth::vme_A32D32_write (UINT addr, UINT data)
{
	int return_code;
	unsigned int udp_address ;
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	if (udp_address < 0x20) {
		return_code = this->udp_register_write(udp_address, data)  ;
		if(return_code == 0) {
			return_code = 0;
		}
		else {
			return_code = 0x211;
		}
	}
	else {
		return_code = this->udp_sis3316_register_write(1, &udp_address, &data)  ;
		if(return_code == 0) {
			return_code = 0;
		}
		else {
			return_code = 0x211;
		}
	}
	return return_code;
}


/**************************************************************************************/

int sis3316_eth::vme_A32DMA_D32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )
{
	return -1; // not implemented
}
/**************************************************************************************/

int sis3316_eth::vme_A32BLT32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )
{
	int return_code;
	unsigned int udp_address ;

	//printf("vme_A32BLT32_write =  %d   \n",request_nof_words);

	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_write(request_nof_words, udp_address, data, written_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 
}
/**************************************************************************************/

int sis3316_eth::vme_A32MBLT64_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	//printf("vme_A32MBLT64_write =  %d   \n",request_nof_words);
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_write(request_nof_words, udp_address, data, written_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 
}

/**************************************************************************************/

int sis3316_eth::vme_A32DMA_D32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )
{
#define MAX_LIST_WRITE_LENGTH 64
	unsigned int i ;
	int return_code ;
	unsigned int udp_address ;
	unsigned int address_array[MAX_LIST_WRITE_LENGTH] ; // max

	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	//printf("vme_A32DMA_D32FIFO_write =  %d   \n",request_nof_words);

	return_code = 0 ;
	if (udp_address < 0x20) {
		for (i=0;i<request_nof_words;i++) {
			//printf("vme_A32DMA_D32FIFO_write: data =  0x%08X   \n",data[i]);
			return_code = this->vme_A32D32_write (addr, data[i])	;
			if (return_code != 0) {
				break ;
			}
		}
		*written_nof_words = request_nof_words ;
		if (return_code != 0) {
			*written_nof_words = request_nof_words - i ;
		}
	}
	else { // >= 0x20 --> internal register space

#ifdef NO_LIST_MODE
		for (i=0;i<request_nof_words;i++) {
			//printf("vme_A32DMA_D32FIFO_write: data =  0x%08X   \n",data[i]);
			//return_code = this->vme_A32D32_write (udp_address, data[i])	;
			//printf("vme_A32DMA_D32FIFO_write:   ");
			return_code = this->udp_sis3316_register_write (1, &udp_address, &data[i])	;
			if (return_code != 0) {
				break ;
			}
		}
		*written_nof_words = request_nof_words ;
		if (return_code != 0) {
			*written_nof_words = request_nof_words - i ;
		}
#endif

//#ifdef LIST_MODE
		unsigned int write_length ;
		unsigned int rest_write_length ;
		unsigned int index ;
		rest_write_length = request_nof_words ;
		index = 0 ;
		for (i=0;i<MAX_LIST_WRITE_LENGTH;i++) {
			address_array[i] = udp_address;
		}
		do {
			if (rest_write_length > MAX_LIST_WRITE_LENGTH) {
				write_length = MAX_LIST_WRITE_LENGTH ;
			}
			else {
				write_length = rest_write_length ;
			}
			//printf("vme_A32DMA_D32FIFO_write:   ");
			return_code = this->udp_sis3316_register_write(write_length, address_array, &data[index])  ;
			if(return_code != 0) {
				return_code = 0x211;
				//printf("break vme_A32DMA_D32FIFO_write:  write_length  =  %d    return_code  0x%08X\n", write_length,return_code);
				//unsigned int dummy ;
				//return_code = this->udp_register_read( 0xc, &dummy  );
			 	//printf("SIS3316_LAST_UDP_ACKN_STATUS     0x%08X\n", dummy);
				//Sleep(100000);
				break ;
			}
			rest_write_length -= write_length ;
			index += write_length  ;

		} while(rest_write_length != 0) ;
		*written_nof_words = request_nof_words - rest_write_length ;
	//	printf("vme_A32DMA_D32FIFO_write:  rest_write_length =  %d   index = %d\n",rest_write_length, index);
//#endif
	}
	return return_code; //
}

/**************************************************************************************/

int sis3316_eth::vme_A32BLT32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )
{
	int return_code;
	unsigned int udp_address ;
	//printf("vme_A32BLT32FIFO_write =  %d   \n",request_nof_words);
	udp_address = addr & 0x00ffffff ; // only sis3316 address range
	return_code = this->udp_sis3316_fifo_write(request_nof_words, udp_address, data, written_nof_words)  ;
	if(return_code == 0) {
		return_code = 0;
	}
	else {
		return_code = 0x211;
	}
	return return_code; // 
}

/**************************************************************************************/


int sis3316_eth::vme_A32MBLT64FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words )
{
	return -1; // not implemented
}


/**************************************************************************************/

int sis3316_eth::vme_IRQ_Status_read( UINT* data ) 
{
	return -1; // not implemented
}


	

/**************************************************************************************/



#endif
