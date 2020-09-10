/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  USBDeviceInfo.h
 *  @brief: Provides a class to encapsulate a libusb_device 
 */

#ifndef USBDEVICEINFO_H
#define USBDEVICEINFO_H
#include <libusb.h>

#include <string>

class USBDevice;

/**
 * @class USBDeviceInfo
 *    This class is instantiated by USBDevice when
 *    asked to enumerate.  Each libusb_device is wrapped
 *    in a USBDeviceInfo object; which has a reference count
 *    of 1 at the time. Destruction, dereferences the device,
 *    Copy construction and assignment increment the reference.
 *
 *    The default constructor is provided but using any of its
 *    methods before it's assigned to will result in a segfault.
 */
class USBDeviceInfo
{
private:
    libusb_device*  m_pDevice;
public:
    USBDeviceInfo();
    USBDeviceInfo(libusb_device* pDevice);
    USBDeviceInfo(const USBDeviceInfo& rhs);
    virtual ~USBDeviceInfo();
    USBDeviceInfo& operator=(const USBDeviceInfo& rhs);
    
    uint8_t getBus();
    uint8_t getPort();
    int getConfiguration();
    void setConfiguration(int config);
    uint16_t getVendor();
    uint16_t getProduct();
    std::string getSerial();
    
    void clearHalt();
    void reset();
    
    USBDevice* open();
    
};

#endif