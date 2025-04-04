/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for the packaging of device support and configuration.
*/
#ifndef MVLC_READOUTMODULE_H
#define MVLC_READOUTMODULE_H

namespace XXUSB {
    class CConfigurableObject;
}
class CReadoutHardware;

/**
 * @class CReadoutModule
 *    A readout module is a USB device support module instance (CReadoutHardware) coupled 
 * together with its configuration (XXUSB::ConfigurableObject).  They are normally created
 * by concrete instances of  a DeviceCommand through it's strategy/factory method createDevice.
 * We provide wrappers for the methods the CReadoutHardWare module exports so that this  is all in
 * one neat package.
 * 
 * TODO: Need to make CReadoutHardware - this is just what's needed to test DeviceCommand.
 */
class CReadoutModule {
private:
    XXUSB::CConfigurableObject* m_pConfiguration;
    CReadoutHardware*           m_pDeviceSupport;

    // Canonical methods:
public:
    
    CReadoutModule();
    virtual ~CReadoutModule();   // Not willing yet to declare this as final.

    // forbidden canonicals:
private:
    CReadoutModule(const CReadoutModule& rhs);
    CReadoutModule& operator=(const CReadoutModule& rhs);
    int operator==(CReadoutModule& rhs);
    int operator!=(CReadoutModule& rhs);

public:
    void Attach(XXUSB::CConfigurableObject* config);
    void SetDriver(CReadoutHardware* pDriver);
    XXUSB::CConfigurableObject* getConfiguration();

    // Todo - driver call jackets.
};

#endif

