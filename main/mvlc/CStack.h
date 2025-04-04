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
@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for the MVLC stack module.
*/
#ifndef MVLC_CSTACK_H
#define MVLC_CSTACK_H

#include <stdint.h>
#include <string>
#include <list>

#include "CReadoutHardware.h"


// forward definitions

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;
class TCLConfigParser;
namespace XXUSB {
  class CConfigurableObject;
}

/*!
  The CStack class represents a VM-USB triggered readout list.   Currently the VM-USB supports
  8 stacks, each stack can (should) have a different trigger.  Two stacks have pre-defined
  triggers.  Stack 0 is triggered by the NIM 1 input.  Stack 1 by the scaler trigger.
  In our organization, all the other stacks will be triggered by interrupts.

  To run a stack, you must create it and configure it to define which trigger/stack it is,
  and stuff it with modules (current implementation, C785's, Scalers or CCAENChain modules).


  Stacks have the following configuration options:
\verbatim
Option           value type               Value meaning
-trigger         enumeration              Defines the trigger source:
                                          nim1   - This will be stack 0 triggered by nim1.
					  scaler - This will be stack 1 triggered by timer
					  interrupt - this is some stack 2-n (see -stack)
                                                      triggered by an interrupt.
-period           integer                 Number of seconds between scaler triggers.  This
                                          is ignored if the trigger type is not a scaler stack.
-stack            integer                 Stack number.  This is ignored unless the -trigger
                                          option is interrupt.  The stack number will determine
                                          which interrupt list register will be programmed
                                          to trigger this list.
-vector           integer                 VME Interrupt status/ID that will be used to trigger this list.
                                          This is ignored if the trigger is not interrupt.
-ipl              integer 1-7             Interrupt priority level of the interrupt that will trigger
                                          this stack.  This will be ignored if the trigger is not 
					  interrupt.
-delay            integer 0-255           For stack 0, Number of microseconds to delay between
                                          NIM1 and starting the readout stack.
-modules          stringlist              List of ADC, Scaler, Chain modules that will be read by
                                          this stack.
\endverbatim
\note  The assumption is that all the stacks are managed by this class. The m_listOffset
       static member is used to keep track of the offsets at which each stack is loaded.

@note that the only options that matter in the MVLC world are -triger, -modules and -delay, if the
trigger was nim1.  In this iteration the -period for scaler triggers is fixed at 2 seconds.
*/
class CStack : public CReadoutHardware
{
private:
  // Data types:

  typedef std::list<CReadoutModule*>  StackElements;
public:
  typedef enum _TriggerType {
    Nim1,
    Scaler,
    Interrupt
  } TriggerType;

private:
  XXUSB::CConfigurableObject*    m_pConfiguration;
  TCLConfigParser*               m_pParser;
public:
  // Canonicals:

  CStack(TCLConfigParser* pParser);
  
  virtual ~CStack();
  
  // forbidden canonicals
private:
  CStack(const CStack& rhs);
  CStack& operator=(const CStack& rhs);
  int operator==(const CStack& rhs) const;
  int operator!=(const CStack& rhs) const;
  virtual CReadoutHardware* clone() const;  // not used in mvlc.
public:


  // The CReadoutHardware Interface:

public:
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual void onEndRun(CVMUSB& controller);
  
  TriggerType     getTriggerType();

  static bool scalerIsIncremental()  {return false; }    // always non-incrementtal in mvlc.

  // Utility member functions:

private:

  unsigned int    getIntegerParameter(std::string name);
  StackElements   getStackElements();
  uint8_t         getListNumber();

  // Custom validators

  static bool moduleChecker(std::string name, std::string proposedValue, void* arg);

};


#endif
