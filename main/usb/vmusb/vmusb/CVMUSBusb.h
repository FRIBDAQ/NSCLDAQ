/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/




#ifndef CVMUSBUSB_H
#define CVMUSBUSB_H

#include "CVMUSB.h"

#include <vector>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <CMutex.h>

//  The structures below are defined in <usb.h> which is included
//  by the implementation and can be treated as opaque by any of our
//  clients (they are in fact opaque in usb.h if memory servers.

struct usb_device;
struct usb_dev_handle;


// Forward Class definitions:

class CVMUSBReadoutList;

/*!
   This class is part of the support package for the Wiener/JTEC VM-USB 
   USB to VME interface.  This class is intended to be used in conjunction
   with CVMUSBReadoutList.
   CVMUSB is used to directly manipulate the controller and to perform
   single shot VME operations.   CVMEReadoutList is intended to be used
   to compose lists of VME operations that can either be run immediately
   by calling executeList or downloaded for triggered operation when
   data taking is turned on via loadList.

   The class is instantiated on a usb_device.  The list of usb_devices
   that correspond to VM-USB's is gotten via a call to the static function
   CVMUSB::enumerate().

 

*/

class CVMUSBusb : public CVMUSB 
{

    // Class member data.
private:
    struct usb_dev_handle*  m_handle;	// Handle open on the device.
    struct usb_device*      m_device;   // Device we are open on.
    int                     m_timeout; // Timeout used when user doesn't give one.
    uint16_t                m_irqMask; // interrupt mask shadow register.
    std::string             m_serial;  // Attached serial number.
    CMutex*                 m_pMutex;  // Mutex for critical sections.	
	

    // Static functions.
public:
    static std::vector<struct usb_device*> enumerate();
    static std::string serialNo(struct usb_device* dev);

    // Constructors and other canonical functions.
    // Note that since destruction closes the handle and there's no
    // good way to share usb objects, copy construction and
    // assignment are forbidden.
    // Furthermore, since constructing implies a usb_claim_interface()
    // and destruction implies a usb_release_interface(),
    // equality comparison has no useful meaning either:

    CVMUSBusb(struct usb_device* vmUsbDevice);
    virtual ~CVMUSBusb();		// Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CVMUSBusb(const CVMUSBusb& rhs);
    CVMUSBusb& operator=(const CVMUSBusb& rhs);
    int operator==(const CVMUSBusb& rhs) const;
    int operator!=(const CVMUSBusb& rhs) const;
public:
    virtual bool reconnect();
    virtual std::string getSerialNumber() {
      return serialNo(m_device);
    }
    // List operations.

public:
    void writeActionRegister(uint16_t value);
    void  writeRegister(unsigned int address, uint32_t data);
    uint32_t readRegister(unsigned int address);

    int executeList(CVMUSBReadoutList& list,
		    void*               pReadBuffer,
		    size_t              readBufferSize,
		    size_t*             bytesRead);
    
    int loadList(uint8_t listNumber, CVMUSBReadoutList& list,
                 off_t listOffset = 0);
      

    // Once the interface is in DAQ auntonomous mode, the application
    // should call the following function to read acquired data.

    int usbRead(void* data, size_t bufferSize, size_t* transferCount,
		            int timeout = 2000);

    // Other administrative functions:

    void setDefaultTimeout(int ms); // Can alter internally used timeouts.
    int   getDefaultTimeout() const {return m_timeout;}
private:
    void openVMUsb();

    int transaction(void* writePacket, size_t writeSize,
		                void* readPacket,  size_t readSize);

    void initializeShadowRegisters();
    void resetVMUSB();
    void enumerateAndIdentify();
};

#endif
