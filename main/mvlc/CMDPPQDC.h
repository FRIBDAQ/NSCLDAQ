/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2024.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Genie Jhang
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef MVLC_CMDPP_H
#define MVLC_CMDPP_H
#include "CMDPP.h"
#endif

#ifndef MVLC_CMDPPQDC_H
#define MVLC_CMDPPQDC_H

#ifndef Const
#define Const(name) static const int name =
#endif

Const(SignalWidth)          0x6110;
Const(InputAmplitude)       0x6112;
Const(JumperRange)          0x6114;
Const(QDCJumper)            0x6116;
Const(IntegrationLong)      0x6118;
Const(IntegrationShort)     0x611a;
Const(LongGainCorrection)   0x612a;
Const(ShortGainCorrection)  0x612e;

#endif
