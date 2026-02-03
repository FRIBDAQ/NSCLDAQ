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

/**
 * @file CS800Triggernew.h
 * @brief Header for the new VME S800 trigger module 
 * @note When compiled for the MVLCGenerator the preprocessor symbol MVLC_GENERATOR is defined.
 */


#ifndef CS800TRIGGERNEW_H
#define CS800TRIGGERNEW_H

#include <CReadoutHardware.h>
#include <stdint.h>
#include <string>
#include <vector>
#ifdef MVLC_GENERATOR
#include "DeviceCommand.h"
#endif



class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;
class S800TriggerRegisters;

#ifdef MVLC_GENERATOR
namespace XXUSB {
    class CConfigurableObject;
}
#endif

/**
 * This class provides support to read the new s800 FPGA based trigger module with firmware
 * written by Nuclear Instruments (Andreas Abba), not to be confused with National Instruments
 * when you see the initials NI.
 * 
 * The following configuration parameters are supported:
 * \verbatim
 * Parameter         default   Meaning
 * -register-file    $DAQSHARE/s800trigger/RegisterFile.json 
 *                             Location of the module registser file definition.
 * -base             0         Module base address
 * -enable-extclear  true      Enable or diable the module's external clear.
 * \endverbatim
 */
class CS800TriggerNew : public CReadoutHardware {
private:
    S800TriggerRegisters* m_pAPI;
#ifdef MVLC_GENERATOR
  XXUSB::CConfigurableObject* m_pConfiguration;
#else
  CReadoutModule*    m_pConfiguration;
#endif

    // Canonical methods:

public:
    CS800TriggerNew();
    virtual ~CS800TriggerNew();

    // Canonicals not allowed in MVLC:

#ifdef MVLC_GENERATOR
private:
#endif
    CS800TriggerNew(const CS800TriggerNew&);  // copy construction.
    CS800TriggerNew& operator=(const CS800TriggerNew&);  // Assignment.

    // Canonical not allowed in either framework:
private:
    int operator==(const CS800TriggerNew&);
    int operator!=(const CS800TriggerNew&);

    // Override the CReadoutHardware API:
public:
#ifdef MVLC_GENERATOR
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
#else
  virtual void onAttach(CReadoutModule& configuration);
#endif
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual void onEndRun(CVMUSB& controller);

#ifndef MVLC_GENERATOR
  virtual CReadoutHardware* clone() const;
#endif
    // Utilities:

};

#ifdef MVLC_GENERATOR
// The class to provide the trigger command
// We'll register it as "s800trigger".
class CS800TriggerNewCommand : public DeviceCommand {
public:
    CS800TriggerNewCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
    virtual ~CS800TriggerNewCommand();
protected:
     virtual CReadoutModule* createDevice(std::string name);
};
#endif
#endif