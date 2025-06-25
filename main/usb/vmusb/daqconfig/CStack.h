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
#ifndef __CSTACK_H
#define __CSTACK_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif


// forward definitions

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

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
*/
class CStack : public CReadoutHardware
{
private:
  // Data types:

  typedef std::list<CReadoutHardware*>  StackElements;
public:
  typedef enum _TriggerType {
    Nim1,
    Scaler,
    Interrupt
  } TriggerType;

private:
  CReadoutModule*    m_pConfiguration;
  static size_t      m_listOffset; // Offsets for list loading
  static bool        m_incrementalScaler;
public:
  // Canonicals:

  CStack();
  CStack(const CStack& rhs);
  virtual ~CStack();
  CStack& operator=(const CStack& rhs);

  // forbidden canonicals
private:
  int operator==(const CStack& rhs) const;
  int operator!=(const CStack& rhs) const;
public:


  // The CReadoutHardware Interface:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual void onEndRun(CVMUSB& controller);
  virtual CReadoutHardware* clone() const;

  // Public interfaces specific to CStack

  static void   resetStackOffset(size_t to= static_cast<size_t>(0));
  static size_t getOffset();

  void loadStack(CVMUSB& controller);
  void enableStack(CVMUSB& controller);
  TriggerType     getTriggerType();

  static bool scalerIsIncremental()  {return m_incrementalScaler; }

  // Utility member functions:

private:

  unsigned int    getIntegerParameter(std::string name);
  StackElements   getStackElements();
  uint8_t         getListNumber();

  // Custom validators

  static bool moduleChecker(std::string name, std::string proposedValue, void* arg);

};


#endif
