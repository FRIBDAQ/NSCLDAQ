/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

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
 * @file CS800TriggerScalers.h
 * @brief Header for the new VME S800 trigger module scaler readout.
 * @note When compiled for the MVLCGenerator the preprocessor symbol MVLC_GENERATOR is defined.
 * @note The file depends on the $DAQBIN/s800scaler_gen file to prepare a list of what to read.
 */

 #ifndef CS800TRIGGERSCALERS_H
 #define CS800TRIGGERSCALERS_H
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


#ifdef MVLC_GENERATOR
namespace XXUSB {
    class CConfigurableObject;
}
#endif


/**
 * @class CS800TriggerScaler
 *    Provides a module that can readout the scalers from the new S800 FPGA based
 * trigger module.
 * 
 * Configuration parameters:
 * \verbatim
 *  -base - base of the module
 *  -file - Output file from $DAQBIN/s800scaler_gen describing what's to be read.
 *        This is a file containing:
 *          offset count
 *     Where count is the size of a block read that could conceivably be done starting at offset.
 *   -block-read-threshold - The minimum size for count that will generate a block readout.
 * \endverbatim
 */
class CS800TriggerScalers : public CReadoutHardware {
private:
#ifdef MVLC_GENERATOR
    XXUSB::CConfigurableObject* m_pConfiguration;
#else
    CReadoutModule* m_pConfiguration;
#endif

public:
    CS800TriggerScalers();
    virtual ~CS800TriggerScalers();

    // Canonicals unimplemented for mvlcgenerate:

#ifdef MVLC_GENERATOR
private:
#endif
    CS800TriggerScalers(const CS800TriggerScalers& rhs);
    CS800TriggerScalers& operator=(const CS800TriggerScalers& rhs);
    
    // Canonicals unimplemented in all environments:

private:
    int operator==(const CS800TriggerScalers& );
    int operator!=(const CS800TriggerScalers& );

    // API implementations:

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
    // Utilities:
private:
    void throwNoFile();

};

// MVLC obgject command:

#ifdef MVLC_GENERATOR

class CS800TriggerScalerCommand : public DeviceCommand {
public:
    CS800TriggerScalerCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
    virtual ~CS800TriggerScalerCommand();
protected:
    virtual CReadoutModule* createDevice(std::string name);
};

#endif

#endif