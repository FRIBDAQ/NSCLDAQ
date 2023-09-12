/*
*-------------------------------------------------------------
 
 CAEN SpA 
 Via Vetraia, 11 - 55049 - Viareggio ITALY
 +390594388398 - www.caen.it

------------------------------------------------------------

**************************************************************************
* @note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
*
* @file     VX2750EventSegment.h
* @brief    Define a readout event segment for the VX2x50 modules.
* @author   Ron Fox
*
*/

#ifndef VX2750EVENTSEGMENT_H
#define VX2750EVENTSEGMENT_H
#include <CEventSegment.h>
#include <string>
#include "VX2750Pha.h"

class CExperiment;

namespace caen_nscldaq {
class VX2750TclConfig;                    // May become XML later....


/**
 *    VX2750EventSegment
 *       This is an SBSReadout event segment for CAEN VX2x50 modules.
 *       Each module is associated with a module name that's used to
 *       look it up in the configuration at initialization time.
 *       At that time, we initialize the module in accordance with its configuration
 *       and gather data from that configuration that determine how we'll read
 *       the module when the time comes.
 *       Note that in order to support 3'd party software configuring the module
 *       and writing our configuration between runs, at the end of a run
 *       (shutdown) we disconnect from the module and reconnect at each init.
 *     @note in practice VX2750MultiEventSegment will be used to read data
 *        from a system of several modules using a Vx2750MultiTrigger to direct the
 *        'traffic'.
 */
class VX2750EventSegment : public ::CEventSegment
{
private:
    CExperiment*     m_pExperiment;
    uint32_t         m_sourceId;
    VX2750Pha*       m_pModule;                  // Only non-null when run is active.
    VX2750TclConfig* m_pConfiguration;           // Global configuration.
    std::string      m_moduleName;               // Cofiguration lookup key.
    std::string      m_hostOrPid;                // module connection - host/PID
    bool             m_isUsb;                    // module connection usb connection flg.
    VX2750Pha::DecodedEvent m_Event;
    size_t           m_chans;                    // Module channels.
    size_t           *m_traceSizes;              // Sizes of traces from each channel.
public:
    VX2750EventSegment(
        CExperiment *pExperiment, uint32_t sourceId,
        const char* pModuleName, VX2750TclConfig* pConfig,
        const char* pHostOrPid, bool fIsUsb = false
    );
    virtual ~VX2750EventSegment();
    
    // Getters:
    
    VX2750Pha* getModule() {return m_pModule;}
    
    void hwInit();                            // Addition for faster init.
    // Preparing and dropping modules:
    
    virtual void initialize();                  // at begin run.
    virtual void disable();                     // at end run.
    virtual void onPause();                     //  Just will disable.
    virtual void onResume();                    // just will initilialize.
    
    // Getting data from the module in response to a trigger.
    
    virtual size_t read(void* pBuffer, size_t maxwords);  // At trigger.
    
};

}                               // CAEN Namespace.

#endif
