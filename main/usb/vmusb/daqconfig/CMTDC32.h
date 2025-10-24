/**
#******************************************************************************
#
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#***************************************************************************//**
# 

##
# @file MTDC32.h
# @brief Support for the Mesytec MTDC32  (header) 
# @note Works for both VMUSB and MVCL given MVLC_GENERATOR is defined when compiling MVLC
# @note This file was contributed to the FRIB/NSCLDAQ source tree with permission from 
#        CAEN Technologies.
# @author Ron Fox (rfoxkendo@gmail.com)

*/
#ifndef __MTDC32_H
#define __MTDC32_H

#include "CMesytecBase.h"
#include <stdint.h>
#include <string>
#ifdef MVLC_GENERATOR
#include <DeviceCommand.h>
#endif

// Forward definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;
#ifdef MVLC_GENERATOR
namespace XXUSB {
    class CConfigurableObject;
}
#endif

/**
 *  The MTDC32 is a 32 channel time digitizer from Mesytec. The module
 *  is capable of operating either in a trigger match mode or in continuous
 *  storage...where times are simply recorded relative to some initial t0.
 *  This support is initially for CAEN's contract with Phil Kerr at LLNL and
 *  he only requires the continuous storage mode.  NSCL may later require a
 *  trigger match mode to be layered on top of this...for a later day.
 *
 *  Options are:
 *  \verbatim
 *    -base                      Set base address.
 *    -id                        (0-255) Set the virtual slot number of the module.
 *    -ipl                       0-7 the interrupt priority level (0 means
 *                               interrupts off).
 *    -vector                    0-255 interrupt id (vector).
 *    -datalen                   Defines the pad level (data padded to 8, 16, 32, or 64 bits)
 *    -multievent                off on, limited where limited only transfers the number of events in
 *    -maxevents                 Maximum number of events transferred per read.
 *    -marktype                  eventcount, timestamp, extended-timestamp.
 *    -resolution                {500ps, 250ps, 125ps, 62.5ps, 31.3ps, 15.6ps, 7,8ps, 3.9ps}
 *    -format                    {standard, fulltime} (standard times gate
 *                               relative while fulltime is continuous
 *                               storage mode)
 *    -edge                      {rising, falling} - the signal edge used. NOTE, these values
 *                               assume the input jumper is in differential mode.  If
 *                               unipolar the sense is reversed.
 *    -busy                      {bothbanks, cbusoutput, bufferfull,
 *                               abovethreshold} Define the condition that
 *                               sets the busy.
 *    -bank0threshold            0-255  These are only used with inputs
 *    -bank1threshold            0-255  that are not differential.
 *    -timingsource              {vme, external} source of timestamp timing.
 *    -tstampdivisor             0-65535 - divisor for clock (determines ts resolution).
 *    -tstamp                    bool true if we want to digitize trigger inputs.
 *                               (continuous storage only).
 *    -multlow                   lowest multiplicity accepted.
 *    -multhi                    highest multiplicity accepted.
 *   \endverbatim
 */

class CMTDC32 : public CMesytecBase    // Can participate in chained readouts.
{
private:
#ifdef MVLC_GENERATOR
    XXUSB::CConfigurableObject*    m_pConfiguration;
#else
    CReadoutModule*    m_pConfiguration;
#endif
public:
    CMTDC32();
    
    virtual ~CMTDC32();
    CMTDC32& operator=(const CMTDC32& rhs);
#ifdef MVLC_GENERATOR
private:
#endif
    CMTDC32(const CMTDC32& rhs);
private:
    int operator==(CMTDC32& rhs);
    int operator!=(CMTDC32& rhs);
    
    
    // Interface for CReadout hardware
    
public:
#ifdef MVLC_GENERATOR
    virtual void onAttach(XXUSB::CConfigurableObject& configuration);
#else
    virtual void onAttach(CReadoutModule& configuration);
#endif
    virtual void Initialize(CVMUSB& controller);
    virtual void addReadoutList(CVMUSBReadoutList& list);
#ifndef MVLC_GENERATOR    
    virtual CReadoutHardware* clone() const;
#endif

    // Interface demanded by CMesytecBase.
    
    virtual void setChainAddresses( CVMUSB& controller,
                                    CMesytecBase::ChainPosition position,
                                    uint32_t      cbltBase,
                                    uint32_t      mcastBase);

    void initCBLTReadout(CVMUSB& controller,
                       uint32_t cbltAddress,
                       int wordsPermodule);


  // Utilities:

  
private:
    void addWrite(CVMUSBReadoutList& list, uint32_t address, uint16_t value);
    uint16_t computeMultiEventRegister();
    uint16_t getTermination();

};

#ifdef MVLC_GENERATOR

/**
 *  @class MTDCCommand
 *     Derivation of DeviceCommand that creates CMTDC32 objects.
 */
class MTDCCommand : public DeviceCommand {
public:
    MTDCCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
    virtual ~MTDCCommand();
protected:
    CReadoutModule* createDevice(std::string name);
};

#endif


#endif
