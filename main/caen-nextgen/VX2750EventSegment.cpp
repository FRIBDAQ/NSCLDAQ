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
* @file     VX2750EventSegment.cpp
* @brief    Implement an event segment for a VX27x0 module.
* @author   Ron Fox
* @todo     Can we only initialize on the init command unless changes to the
* configuration have beenmade?  That would speed up begin run tremendously esp.
* for larger collections of digitizers.
*/

#include "VX2750EventSegment.h"
#include "VX2750TclConfig.h"
#include "VX2750PHAConfiguration.h"
#include "VX2750Pha.h"
#include <Exception.h>
#include <stdexcept>
#include <sstream>
#include <string.h>
#include <CExperiment.h>
#include <unistd.h>
#include <iostream>
#include <memory>

namespace caen_nscldaq {
/**
 * constructor
 *    Initialize the data encapsulated by this class. Note that the
 *    Module is na nullpointer because we only connect to it at initialization
 *    time (we also disconnect at disable time)..
 *
 *   @param pExperiment - Ponter to the experiment object.
 *   @param sourceId    -  Event builder data source id assigned the module.
 *   @param pModuleName - name of the module in the configuration.
 *   @param pConfig     - The configuration database which will have our module config.
 *                        Note that the caller retains ownership of the configuration.
 *   @param pHostOrPid  - Host or USB PID of the module we're talking to.
 *   @param isUsb       - if true, the module is USB connected else Ethernet.
 *                        This parameter is optional and defaults to fals (ethernet).
 */
VX2750EventSegment::VX2750EventSegment(
        CExperiment* pExperiment, uint32_t sourceId,        
        const char* pModuleName, VX2750TclConfig* pConfig,
        const char* pHostOrPid, bool fIsUsb
    
) :
    m_pExperiment(pExperiment), m_sourceId(sourceId),
    m_pModule(nullptr), m_pConfiguration(pConfig), m_moduleName(pModuleName),
    m_hostOrPid(pHostOrPid), m_isUsb(fIsUsb), m_traceSizes(nullptr)
{}

/**
 * destructor
 *     In theory the module has already been disconnected however, just in case
 *     we have missed a use case we'll delete it here as well.
 *     The configuration is not owned by us so we leave it alone.
 */
VX2750EventSegment::~VX2750EventSegment()
{
    delete m_pModule;                // no-op if it's a nullptr.
    delete m_traceSizes;
}
/**
 * hwInit
 *   Hardware initialize the module from the configuration.
 *   This is separated from the initialize method to support
 *   faster run starts once the module is actually initialized.
 */
void
VX2750EventSegment::hwInit()
{
    try {
        auto pConfig = m_pConfiguration->getModule(m_moduleName.c_str());
	if (m_pModule) {
	  delete m_pModule;
	  m_pModule = nullptr;   // in case new throws this time.
	}
        m_pModule = new VX2750Pha(m_hostOrPid.c_str(), m_isUsb);
        pConfig->configureModule(*m_pModule);
    }
    catch (std::exception& e) {
        throw e.what();
    }
    catch (CException& e) {
        throw e.ReasonText();
    }
}
/**
 * initialize
 *    - locate our configuration in the Tcl configuration.  If we can't find it
 *      throw an exception which will abort the begin.
 *    - connect to the module - if we can't the exception rolls through us
 *      and propagates upwards to fail the begin.
 *  @note - I'm not actually sure the that SBSReadout understand std::exception
 *          derived exceptions so those too get mapped into std::string exceptions
 *          which I'm sure it does understand.
 */
void
VX2750EventSegment::initialize()
{
    
    try {
      if (!m_pModule) {
	throw std::logic_error("The module object should have been created but was not yet");
      }
        auto pConfig = m_pConfiguration->getModule(m_moduleName.c_str());
               
        // We need to ask the configuration what to expect from the module:
        //    Trace lengths.
        
        m_chans = m_pModule->channelCount();
        m_traceSizes= new size_t[m_chans];      // To hold trace lengths from each:
        for (int i =0; i < m_chans; i++) {
            m_traceSizes[i] = m_pModule->getRecordSamples(i);
        }
        
        
        // The configuration takes care of initializing the module:
        
        
        m_pModule->initDecodedBuffer(m_Event);
        m_pModule->setupDecodedBuffer(m_Event);
        
        // Set up the endpoint for PHA data based on our configuration
        // the module configuration includes enables for the things we can get
        // from the module.
        
        
        m_pModule->selectEndpoint(VX2750Pha::PHA);
        m_pModule->initializeDPPPHAReadout();
        
        // prep the module for data taking and start it
        // After this chunk of code the module should be able to generate triggers.
        
        m_pModule->Clear();
        m_pModule->Arm();
        // If one of the start sources is SwCommand we need to do that:
        //
        auto startSources = pConfig->getList("startsource");
        for (auto s : startSources) {
            if (s == "SWcmd") {
                std::cout << "SWcmd is enabled as a start source for "
                    << m_moduleName << " so invoking start\n";
                m_pModule->Start();
                break;                     // in case there's duplication.
            }
        }
        
        
    }
    catch (std::exception& e) {
        throw e.what();
    }
    catch (CException& e) {
        throw e.ReasonText();
    }
}
/**
 * disbale
 *    Turn the module off, stop, disarm and delete (disconnect)
 *    
 */
void
VX2750EventSegment::disable()
{
    delete []m_traceSizes;                 // Trace lengths might change
    m_traceSizes = nullptr;                // if config changes.
    
    // Defensive programming this if an end of run in the paused state will
    // find us without a module.
    
    
    if (m_pModule) {
        m_pModule->freeDecodedBuffer(m_Event);
        m_pModule->Stop();
        m_pModule->Disarm();
        //delete m_pModule;
        //m_pModule = nullptr;          // This disconnects.
    }
}
/**
 * onPause
 *   THe run is pausing - disable:
 */
void VX2750EventSegment::onPause()
{
    disable();
}
/**
 * onResume
 *    This is just enable again:
 */
void VX2750EventSegment::onResume()
{
    // I don't know how this is possible but be defensive anyway:
    
    if (m_pModule) {
        disable();                   // Get rid of the existing module.
    }
    initialize();                        // reprocesses the configuration too.
}
 
 /**
  * read
  *    Read an event in decoded mode.  We'll pre-determine if this event will
  *    fit.. If not we'll throw a string suggesting the buffer be enlarged.
  *
  *    Note that some of the data in the event won't make sense unless
  *    readout of that data is enabled, but we'll put place holders there
  *    in order to keep the format of the data constant, and honestly so I can
  *    be a bit lazy about the read.  The things which may not
  *    make sense are (*)'d.
  *
  *Event format:
  *\verbatim
  *    +------------------------------------+
  *    | Module name (cz string)            |  padded to unt16_t
  *    +------------------------------------+
  *    | uin16_t channel number             |
  *    +------------------------------------+
  *    | uint64_t timestamp in ns           |
  *    +------------------------------------+
  *    | uint64_t rawTimestamp(*)           |
  *    +------------------------------------+
  *    | uint16_t fineTimestamp(*)          |
  *    +------------------------------------+
  *    |  uint16_t energy                   |
  *    +------------------------------------+
  *    |  uint16_t low priority flags(*)    |
  *    +------------------------------------+
  *    |  uint16_t high priority flags(*)   |
  *    +------------------------------------+
  *    | uint16_t t downsample selection(*) |
  *    +------------------------------------+
  *    | uint16_t fail  flag                 |
  *    +------------------------------------+
  *    | uint16_t analog probe type 1  (*)  |
  *    +------------------------------------+
  *    | uint32_t aprobe1 nSamples          |  - 0 if no data.
  *    +------------------------------------+
  *    | int32_t probe data (probe1)        | - # of words in nsamples
  *                 ...                          Could be no data.
  *    +------------------------------------+
  *    | uint16_t analog probe type 2  (*)  |
  *    +------------------------------------+
  *    | uint32_t aprobe1 nSamples          |  - 0 if no data.
  *    +------------------------------------+
  *    | int32_t probe data (probe 2)       | - # of words in nsamples
  *                 ...                          Could be no data.
  *    +------------------------------------+
  *    |  Digital probe type 1 (*)          | uint16_t
  *    +------------------------------------|
  *    | digital probe bytes                | 0 if no data (e.g. disabled) uint32_t
  *    +------------------------------------+
  *    |  Digital probe data(1)             | 1 byte per sample
  *             ...                           could be no data
  *    +------------------------------------+
  *    |  Digital probe type 2 (*)          |
  *    +------------------------------------|
  *    | digital probe bytes                | 0 if no data (e.g. disabled)
  *    +------------------------------------+
  *    |  Digital probe data(2)             | 1 byte per sample
  *             ...                           could be no data
  *    +------------------------------------+
  *    |  Digital probe type 3 (*)          |
  *    +------------------------------------|
  *    | digital probe bytes                | 0 if no data (e.g. disabled)
  *    +------------------------------------+
  *    |  Digital probe data(3)             | 1 byte per sample
  *             ...                           could be no data
  *    +------------------------------------+
  *    |  Digital probe type 4 (*)          |
  *    +------------------------------------|
  *    | digital probe bytes                | 0 if no data (e.g. disabled)
  *    +------------------------------------+
  *    |  Digital probe data(4)             | 1 byteper sample
  *             ...                           could be no data
  *    +------------------------------------+
  *    
  *\endverbatim    
  *
  *
  *  @param pBuffer - buffer into which we put the data.
  *  @param maxwords - maximum number of 16 bit words available in the buffer.
  *  @return size_t - Number of 16 bit words read.
  *  @note it is possible an extra  pad byte will be added to the buffer. THe
  *        value of this pad byte is not deterministic.
  */
 size_t
 VX2750EventSegment::read(void* pBuffer, size_t maxwords)
 {
    
    // First we need to read the event into the decoded buffer:
    // FIgure out the trace sizes:
    
    m_pModule->readDPPPHAEndpoint(m_Event);
    size_t traceLength = m_traceSizes[m_Event.s_channel];
    
    // figure out if this will fit.
    
    
    size_t bytesNeeded =
        m_moduleName.size() + 1 + 7*sizeof(uint16_t) +
        2*sizeof(uint64_t);                                      // Fixed junk:
    if (m_moduleName.size() % 2 == 0) bytesNeeded++;          // Round to next uint16_t
    bytesNeeded += 6*(sizeof(uint16_t) + sizeof(uint32_t));  // Always present probe stuff.
    
    // Fold in any present traces. Null pointers in the event indicate
    // the trace is not enabled:
    
    if (m_Event.s_pAnalogProbe1) {
        bytesNeeded += traceLength * sizeof(int32_t);
    }
    if (m_Event.s_pAnalogProbe2) {
        bytesNeeded += traceLength * sizeof(int32_t);
    }
    size_t digitalProbeLength = traceLength;   // Byte per sample...
    
    if (m_Event.s_pDigitalProbe1) {
        bytesNeeded += digitalProbeLength;
    }
    if (m_Event.s_pDigitalProbe2) {
        bytesNeeded += digitalProbeLength;
    }
    if (m_Event.s_pDigitalProbe3) {
        bytesNeeded += digitalProbeLength;
    }
    if (m_Event.s_pDigitalProbe4) {
        bytesNeeded += digitalProbeLength;
    }
    //Yeah could assume sizeof(int16_t) is 2 but...
    
    while (bytesNeeded % sizeof(uint16_t) > 0) bytesNeeded++;
    
    
    // Get upset if our event won't fit in the ring item buffer:
    
    
    if (bytesNeeded > (maxwords*sizeof(uint16_t))) {
        std::stringstream strMsg;
        strMsg << "Reading out module " << m_moduleName << " channel " << m_Event.s_channel
            << " requires " << bytesNeeded << " bytes but there's only "
            << maxwords*sizeof(uint16_t) << " bytes available\n";
        strMsg << " Either increase the event buffer length or decrease the requirements for that channel";
        std::string msg = strMsg.str();
        throw msg;
    }
    
    // There are archaic processors for which the following does not work
    // We think we're running intel so should be ok:
    
    m_pExperiment->setSourceId(m_sourceId);
    union {
        uint8_t*  p8;
        uint16_t* p16;
        uint32_t* p32;
        uint64_t* p64;
    } p;
    p.p16 = reinterpret_cast<uint16_t*>(pBuffer);
    
    // First the module name:
    
    strcpy(reinterpret_cast<char*>(p.p8), m_moduleName.c_str());
    p.p8 += m_moduleName.size() + 1;
    if (m_moduleName.size() % 2 == 0) *p.p8++ = 0;  // Pad to uint16_t
    
    // Now the fixed part... we need to do this field by field because
    // we adjust some sizes:
    
    *p.p16++ = m_Event.s_channel;
    *p.p64++ = m_Event.s_nsTimestamp;
    m_pExperiment->setTimestamp(m_Event.s_nsTimestamp);
    *p.p64++ = m_Event.s_rawTimestamp;
    *p.p16++ = m_Event.s_fineTimestamp;
    *p.p16++ = m_Event.s_energy;
    *p.p16++ = m_Event.s_lowPriorityFlags;
    *p.p16++ = m_Event.s_highPriorityFlags;
    *p.p16++ = m_Event.s_timeDownSampling;
    
    // Ok, now analog probe 1:
    
    *p.p16++ = m_Event.s_analogProbe1Type;
    if(m_Event.s_pAnalogProbe1) {
        *p.p32++ = traceLength;
        memcpy(p.p32, m_Event.s_pAnalogProbe1, traceLength*sizeof(uint32_t));
        p.p32 += traceLength;
    } else {
      *p.p32++ = 0;        // no data.
    }
    
    // Ok, now analog probe 2:
    
    *p.p16++ = m_Event.s_analogProbe2Type;
    if(m_Event.s_pAnalogProbe2) {
        *p.p32++ = traceLength;
        memcpy(p.p32, m_Event.s_pAnalogProbe2, traceLength*sizeof(uint32_t));
        p.p32 += traceLength;
    } else {
      *p.p32++ = 0;        // no data.
    }
    
    // Now the digital probes.  Note that digitalProbeLength has the # of bytes
    // per present probe.
    
    *p.p16++ = m_Event.s_digitalProbe1Type;
    if (m_Event.s_pDigitalProbe1) {
        *p.p32++ = digitalProbeLength;
        memcpy(p.p8, m_Event.s_pDigitalProbe1, digitalProbeLength);
        p.p8 += digitalProbeLength;
    } else {
        *p.p32++ = 0;
    }
    
    *p.p16++ = m_Event.s_digitalProbe2Type;
    if (m_Event.s_pDigitalProbe2) {
        *p.p32++ = digitalProbeLength;
        memcpy(p.p8, m_Event.s_pDigitalProbe2, digitalProbeLength);
        p.p8 += digitalProbeLength;
    } else {
        *p.p32++ = 0;
    }
    
    *p.p16++ = m_Event.s_digitalProbe2Type;
    if (m_Event.s_pDigitalProbe3) {
        *p.p32++ = digitalProbeLength;
        memcpy(p.p8, m_Event.s_pDigitalProbe3, digitalProbeLength);
        p.p8 += digitalProbeLength;
    } else {
        *p.p32++ = 0;
    }
    
    *p.p16++ = m_Event.s_digitalProbe4Type;
    if (m_Event.s_pDigitalProbe4) {
        *p.p32++ = digitalProbeLength;
        memcpy(p.p8, m_Event.s_pDigitalProbe4, digitalProbeLength);
        p.p8 += digitalProbeLength;
    } else {
        *p.p32++ = 0;
    }
    // Compute/return the number of uint16_t's in thye buffer:
    // THe + 1 below adds an extra pad byte if bytesNeeded is odd.
    // (which I think is impossible since there are an even number
    // of digital probes).
    return (bytesNeeded + 1) / sizeof(uint16_t);
 }
 
 
}                     // caen_nscldaq namespace. 
