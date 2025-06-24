/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CVMUSBCONTROL_H
#define __CVMUSBCONTROL_H


/**
 * @file CVMUSBControl.h
 * @brief Defines a VMUSB module for the readout software.
 * @author Ron Fox
 * @note  This module also allows VM-USB scalers to be read to either a scaler or data stream.
 */


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

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif


// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

/**
 * The VMUSBControl module allows you to 
 * - Control the setup of the LED Source selectors register.
 * - Control the setup of the User device source selector register.
 * - Control the VM_USB Gate and delay register setup.
 * - Read the VM-USB scalers out either incrementally (clearing after read)
 *   or nonincrementally (not clearing after read).
 *
 * @note Since the user device source selector register is completely
 *       under your control you may also set the scalers up as a 64 bit
 *       scaler by selecting the source for scaler B to 'carry'.
 *
 *
 * The configuration options are:
 *  * -nimo1  - Sets the meaning of the O1 output. This can be one of:
 *              busy, trigger, busrequest, eventdatatobuffer, dgga, endevent, usbtrigger
 *              see Section 3.4.4 of the VM-USB manual for more information about the
 *              meaning of these values. Defaults to busy.
 *  * -latcho1 - Boolean, true to latch the O1 value once set.
 *  * -inverto1 - Boolean, true to invert the O1 value once set.
 *  * -nimo2  - Sets the meaning of the O1 output.  This can be one  of:
 *              usbtrigger, vmecommand, vmeas, eventdatatobuffer, dgga, dggb, endevent.
 *              See section 3.4.4 of the VM-USB manual for more information about the meaning
 *              of these values.  Defaults to vemas
 *  * -latcho2 - Boolean that, when true latches O2.
 *  * -inverto2 - Boolean that, when true, inverts O2.
 *  * -topyellow - Determines the meaning of the top yellow LED.  This can be one of:
 *                 outfifonotempty, infifonotempty, scaler, infifofull, dtack berr, vmebr vmebg
 *  * -inverttopyellow - Boolean that inverts the sense of the top yellow led selector.
 *  * -latchtopyellow  - Boolean that latches the top yellow led when it should be on.
 *  * -red        - Specifies the meaning of the red LED.  This can be one of the following:
 *                  trigger, nimi1, imi2, busy, dtack, berr, vmebr, vmebg
 *  * -invertred  - Boolean true if the red led should have an inverted sense.
 *  * -latchred   - True if the red led should latch.
 *  * -green     - Specifies the meaning of the green LED.  This can be one of the following strings:
 *                 acquire, stacknotempty, eventready, trigger, dtack, berr, vmebr, vmebg
 *  * -invertgreen - True if the green LED should have an inverted state.
 *  * -latchgreen  - True if the green LED should latch.
 *  * -bottomyellow - Specifies the meaning of the bottom yellow LED>  This can be one of the following
 *                   strings: notslot1, usbtrigger, usbreset, berr, dtack, vmebr, vmebg
 *  * -latchbottomyellow - True if the bottom yellow LED shouild latch its state.
 *  * -invertbottomyellow - True if the bottom yellow LED shouild have an inverted state.
 *  * -readscalers  - True if the scalers should be read.  If they are read, scaler A is read,
 *                    then scaler B. Note that for the duration of the read, the scalers are held
 *                    disabled.
 *  * -incremental  - True if the scalers should be cleared after a read.
 *  * -scalera      - Determines what causes scaler A to count.  This can be one of:
 *                    dgga, nimi1, nimi2, event
 *  * -scalerb      - Determines what causes scaler B to count.  This can be one of:
 *                    carry, nimi1, nimi2, event
 *  * -dgga         - Determines what starts DDG_A or if DGG_A is a pulser. This can be one of:
 *                    off, nimi1, nimi2, trigger, endofevent,usbtrigger, pulser.
 *  * -dggb         - Determines what starts DGG_B or if DGG_B is a pulser.  This can be on of:
 *                    off, nimi1, nimi2, trigger, endofevent, usbtrigger, pulser
 *  * -widtha       - DGGA output width in 12.5ns units. (uint16_t).
 *  * -delaya       - DGGA start delay in 12.ns units (uint32_t).
 *  * -widthb       - DGGB output width in 12.5ns units (uint16_t)
 *  * -delayb       - DGGB start delay in 12.5ns units (uint32_t).
 *  * -mixedbuffers - Allows mixing of scaler data and event data in same buffer (dflt: true)
 *  * -spanbuffers  - Allows events to span buffer boundaries (dflt: true)
 *  * -forcescalerdump  - Scaler buffer output immediately following scaler stack execution (dflt: false)
 *  * -busreqlevel  - Bus req level for vme bus arbitration (dflt: 4) 
 *  * -optionalheader  - Sets whether or not to output the additional buffer header (dflt: false) 
 
 */

class CVMUSBControl : public CReadoutHardware
{
  // Private per module data.
private:
  CReadoutModule* m_pConfiguration;
  uint32_t        m_nDeviceSourceSelector;
  uint32_t        m_nLedSourceSelector;

  //Canonicals:

public:
  CVMUSBControl();
  virtual ~ CVMUSBControl();

  // forbiddeen canonicals:

private:
  CVMUSBControl(const CVMUSBControl&);
  CVMUSBControl& operator=(const CVMUSBControl&);
  int operator==(const CVMUSBControl&) const;
  int operator!=(const CVMUSBControl&) const;

  // Virtual method overrides:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual void onEndRun(CVMUSB& controller);
  virtual CReadoutHardware* clone() const;

private:
  void configureGlobalMode(CVMUSB& controller);
  void configureEventsPerBuffer(CVMUSB& controller);
};

#endif


