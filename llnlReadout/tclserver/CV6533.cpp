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

#include <config.h>
#include "CV6533.h"

#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <stdint.h>
#include <stdio.h>
#include <tcl.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

// Register offset:'

#define Const(name) static const uint32_t name =

// Board parameters (common to all channels):

Const BoardVmax     0x50;
Const BoardImax     0x54;
Const BoardStatus   0x58;
Const Firmware      0x5c;

// Each Channels has a common format that is described later:

Const(Channels[6])  = {
  0x080, 0x100, 0x180, 0x200, 0x280, 0x300
};

// Board configuration parameters:

Const(ChannelCount)   0x8100;
Const(Description)    0x8102;
Const(Model)          0x8116;
Const(SerialNo)       0x811e;
Const(VmeFirmware)    0x8120;

//  Below are the register offsets within each of the Channels array elements:

Const(Vset)      0x00;
Const(Iset)      0x04;
Const(VMon)      0x08;
Const(Imon)      0x0c;
Const(PW)        0x10;
Const(ChStatus)  0x14;
Const(TripTime)  0x18;
Const(SVMax)     0x1c;
Const(RampDown)  0x20;
Const(RampUp)    0x24;
Const(PwDown)    0x28;
Const(Polarity)  0x2c;
Const(Temp)      0x30;

// Global status register bits:

Const(Chan0Alarm)    0x0001;
Const(Chan1Alarm)    0x0002;
Const(Chan2Alarm)    0x0004;
Const(Chan3Alarm)    0x0008;
Const(Chan4Alarm)    0x0010;
Const(PwrFail)       0x0080;
Const(OverPwr)       0x0100;
Const(MaxVUncal)     0x0200;
Const(MaxIUncal)     0x0400;

// Individual channel status register bits.

Const(On)           0x0001;
Const(RampUp)       0x0002;
Const(RampDown)     0x0004;
Const(OverCurrent)  0x0008;
Const(OverVoltage)  0x0010;
Const(UnderVoltage) 0x0020;
Const(MaxV)         0x0040;
Const(MaxI)         0x0080;
Const(Trip)         0x0100;
Const(OverPower)    0x0200;
Const(Disabled)     0x0400;
Const(Interlocked)  0x0800;
Const(Uncalibrated) 0x1000;
