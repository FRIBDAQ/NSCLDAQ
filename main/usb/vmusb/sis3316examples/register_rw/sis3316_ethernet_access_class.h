/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_ethernet_access_class.h                              */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 26.07.2012                                       */
/*  last modification:    05.01.2016                                       */
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
/*  � 2016                                                                 */
/*                                                                         */
/***************************************************************************/


#ifndef _SIS3316_ETHERNET_ACCESS_CLASS_
#define _SIS3316_ETHERNET_ACCESS_CLASS_

#define VME_FPGA_VERSION_IS_0008_OR_HIGHER


#include "project_system_define.h"		//define LINUX or WIN


#define UDP_MAX_PACKETS_PER_REQUEST		    32            //  max. tramnsmit packets per Read Request to the PC
#define UDP_NORMAL_READ_PACKET_32bitSIZE    360           //  packet size = 1440 Bytes + 44/45 Bytes = 1484/1485 Bytes
#define UDP_JUMBO_READ_PACKET_32bitSIZE     2048          //  packet size = 8192 Bytes + 44/45 Bytes = 8236/8237 Bytes


/* nof_write_words: max. 0x100 (256 32-bit words = 1KBytes )  */
#define UDP_WRITE_PACKET_32bitSIZE		0x100


// error codes
#define PROTOCOL_ERROR_CODE_WRONG_ACK						0x120
#define PROTOCOL_ERROR_CODE_WRONG_NOF_RECEVEID_BYTES		0x121
#define PROTOCOL_ERROR_CODE_WRONG_PACKET_IDENTIFIER			0x122
#define PROTOCOL_ERROR_CODE_WRONG_RECEIVED_PACKET_ORDER		0x123
#define PROTOCOL_ERROR_CODE_TIMEOUT							0x211


#ifdef LINUX
	typedef char CHAR;
	typedef unsigned int UINT;
	typedef unsigned int* PUINT;

	#include <sys/types.h>
	#include <sys/socket.h>

	#include <sys/uio.h>

	#include <sys/time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <errno.h>
	#include <string.h>
	#include <stdlib.h>
#endif

#ifdef WIN
	#include <Windows.h>
#endif

#include "vme_interface_class.h"
#include "sis3316_class.h"

#define DHCP_DEVICE_NAME_LARGE_CASE "SIS3316-"
#define DHCP_DEVICE_NAME_LOWER_CASE "sis3316-"

// UDP commands
#define UDP_LINK_REG_READ	0x10
#define UDP_LINK_REG_WRITE	0x11
#define UDP_REG_READ		0x20
#define UDP_REG_WRITE		0x21
#define UDP_FIFO_READ		0x30
#define UDP_FIFO_WRITE		0x31
#define UDP_RESTRANSMIT		0xEE
#define UDP_RESET			0xFF

// UDP read socket retries
#define UDP_READ_RETRY			5
#define UDP_REQUEST_RETRY		4
#define UDP_RETRANSMIT_RETRY	3

// UDP reordering LUT
typedef struct {
	bool outstanding;
	size_t offs;
} reorder_lut_t;

class sis3316_eth : public  vme_interface_class
{
private:
	CHAR char_messages[128] ;
	int udp_socket_status;
	int udp_socket;
	unsigned int udp_port ;
	struct sockaddr_in SIS3316_sock_addr_in   ;
	struct sockaddr_in myPC_sock_addr   ;
	unsigned int recv_timeout_sec  ;
	unsigned int recv_timeout_usec  ;
    char udp_send_data[2048];
    char udp_recv_data[16384];

	unsigned int  jumbo_frame_enable;
	unsigned int  max_nofPacketsPerRequest;
	unsigned int  max_nof_read_lwords;
	unsigned int  max_nof_write_lwords;

	char  packet_identifier;

	void build_header(unsigned char *buf, unsigned char cmd, unsigned char id, unsigned short len);
	unsigned char packetNumberOffset;
	void reorderLutInit(reorder_lut_t *lut, size_t len, size_t packetLen);
	unsigned char reorderGetIdx(reorder_lut_t *lut, unsigned char currNum);

public: // info counter


	unsigned int   info_udp_receive_timeout_counter;
	unsigned int   info_wrong_cmd_ack_counter;
	unsigned int   info_wrong_received_nof_bytes_counter;
	unsigned int   info_wrong_received_packet_id_counter;


	unsigned int   info_clear_UdpReceiveBuffer_counter;
	unsigned int   info_read_sis3316_fifo_packet_reorder_counter;

	unsigned char  read_udp_register_receive_ack_retry_counter;
	unsigned char  read_udp_register_req_retry_counter;

	unsigned char  read_sis3316_register_receive_ack_retry_counter;
	unsigned char  read_sis3316_register_req_retry_counter;

	unsigned char  write_sis3316_register_receive_ack_retry_counter;
	unsigned char  write_sis3316_register_req_retry_counter;

	unsigned char  read_sis3316_fifo_receive_ack_retry_counter;
	unsigned char  read_sis3316_fifo_req_retry_counter;

	unsigned char  write_sis3316_fifo_receive_ack_retry_counter;

	unsigned char  write_sis3316_fifo_req_retry_counter;



public:
	sis3316_eth (void);


	//int get_device_informations( USHORT* idVendor, USHORT* idProduct, USHORT* idSerNo, USHORT* idFirmwareVersion,  USHORT* idDriverVersion );
	int get_UdpSocketStatus( void );
	int get_UdpSocketPort(void );
	int set_UdpSocketOptionTimeout( void );
	int set_UdpSocketOptionBufSize( int sockbufsize );
	int set_UdpSocketBindToDevice( char* eth_device);
	int set_UdpSocketBindMyOwnPort( char* pc_ip_addr_string);
	int set_UdpSocketSIS3316_IpAddress( char* sis3316_ip_addr_string);

	unsigned int get_UdpSocketNofReadMaxLWordsPerRequest(void);
	unsigned int get_UdpSocketNofWriteMaxLWordsPerRequest(void);

	int set_UdpSocketReceiveNofPackagesPerRequest(unsigned int nofPacketsPerRequest);

	int get_UdpSocketJumboFrameStatus(void);
	int set_UdpSocketEnableJumboFrame(void);
	int set_UdpSocketDisableJumboFrame(void);

	int udp_reset_cmd( void);
	int udp_register_read (UINT addr, UINT* data);
	int udp_sis3316_register_read ( unsigned int nof_read_registers, UINT* addr_ptr, UINT* data_ptr);
	int udp_register_write (UINT addr, UINT data);
	int udp_sis3316_register_write ( unsigned int nof_read_registers, UINT* addr_ptr, UINT* data_ptr);

	int udp_retransmit_cmd( int* receive_bytes, char* data_byte_ptr);
	int clear_UdpReceiveBuffer(void);


private:
	int udp_sub_sis3316_fifo_read ( unsigned int nof_read_words, UINT  addr, UINT* data_ptr);
	int udp_sub_sis3316_fifo_write ( unsigned int nof_write_words, UINT  addr, UINT* data_ptr);
public:
	int udp_sis3316_fifo_read ( unsigned int nof_read_words, UINT  addr, UINT* data_ptr, UINT* got_nof_words );
	int udp_sis3316_fifo_write ( unsigned int nof_write_words, UINT addr, UINT* data_ptr, UINT* written_nof_words );

	// just for test
	int udp_sis3316_block_of_packets_read ( unsigned int  nof_expected_read_words,  unsigned int* data_ptr);


public:
	int vmeopen ( void );
	int vmeclose( void );
	int get_vmeopen_messages( CHAR* messages, UINT* nof_found_devices );
	int vme_A32D32_read (UINT addr, UINT* data);
	int vme_A32DMA_D32_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32BLT32_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32MBLT64_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2EVME_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST160_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST267_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST320_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );

	int vme_A32DMA_D32FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32BLT32FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32MBLT64FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2EVMEFIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST160FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST267FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );
	int vme_A32_2ESST320FIFO_read (UINT addr, UINT* data, UINT request_nof_words, UINT* got_nof_words );

	int vme_A32D32_write (UINT addr, UINT data);
	int vme_A32DMA_D32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words );
	int vme_A32BLT32_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words );
	int vme_A32MBLT64_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words );
	int vme_A32DMA_D32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words );
	int vme_A32BLT32FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words );
	int vme_A32MBLT64FIFO_write (UINT addr, UINT* data, UINT request_nof_words, UINT* written_nof_words );

	int vme_IRQ_Status_read( UINT* data ) ;

};




// Return Codes
/**************************************************************************************/
/* udp_register_read, udp_sis3316_register_read and udp_sis3316_register_write        */
/* possible ReturnCodes:                                                              */     
/*       0     : OK                                                                   */
/*       -1    : Timeout after N Retransmit commands                                  */
/*       0x121 : wrong Packet Length after N Retransmit and M Request commands        */
/*       0x122 : wrong received Packet ID after N Retransmit and M Request commands   */ 
/**************************************************************************************/
/**************************************************************************************/
/* udp_sub_sis3316_fifo_read                                                          */ 
/* possible ReturnCodes:                                                              */  
/*       0     : OK                                                                   */
/*       -1    : Timeout after N Retransmit commands                                  */
/*       0x120 : wrong received Packet CMD after N Retransmit                         */
/*       0x122 : wrong received Packet ID after N Retransmit                          */
/**************************************************************************************/
/**************************************************************************************/
/* udp_sis3316_fifo_read                                                              */ 
/* possible ReturnCodes:                                                              */  
/*       0     : OK                                                                   */
/*       -1    : Timeout after N Retransmit commands                                  */
/*       0x120 : wrong received Packet CMD after N Retransmit                         */
/*       0x121 : wrong number of received Bytes                                       */
/*       0x122 : wrong received Packet ID after N Retransmit                          */
/**************************************************************************************/

/**************************************************************************************/
/* udp_sub_sis3316_fifo_write                                                         */ 
/* possible ReturnCodes:                                                              */  
/*       0     : OK                                                                   */
/*       -1    : Timeout after N Retransmit commands                                  */
/*       0x121 : wrong number of received Bytes                                       */
/*       0x122 : wrong received Packet ID after N Retransmit                          */
/**************************************************************************************/

/**************************************************************************************/
/* udp_sis3316_fifo_write                                                         */ 
/* possible ReturnCodes:                                                              */  
/*       0     : OK                                                                   */
/*       -1    : Timeout after N Retransmit commands                                  */
/*       0x121 : wrong number of received Bytes                                       */
/*       0x122 : wrong received Packet ID after N Retransmit                          */
/**************************************************************************************/


/**************************************************************************************/
/* vme_......                                                                         */ 
/* possible ReturnCodes:                                                              */  
/*       0     : OK                                                                   */
/*       0x211 : No Ackn. (Timeout/Buserror)		                                 */
/*       0x120 : wrong received Packet CMD after N Retransmit                         */
/*       0x121 : wrong number of received Bytes                                       */
/*       0x122 : wrong received Packet ID after N Retransmit                          */
/**************************************************************************************/


#endif
