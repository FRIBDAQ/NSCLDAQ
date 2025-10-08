/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright MADCDELAY5.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __MADC32REGISTERS_H
#define __MADC32REGISTERS_H

#ifndef Const
#define Const(name) static const int name =
#endif

#include <VMEAddressModifier.h>

namespace MQDC32 {

  Const(MQDCDELAY)  1;

  // The address modifiers that will be used to access the module:

  static const uint8_t initamod(VMEAMod::a32UserData);   //  setup using user data access.
  static const uint8_t readamod(VMEAMod::a32UserBlock);  //  Read in block mode.

  static const uint8_t cbltamod(VMEAMod::a32UserBlock);
  static const uint8_t mcstamod(VMEAMod::a32UserData);


  // Module address map; for the most part I'm only defining the registers
  // we'll actually use.

  namespace Reg {
    Const(eventBuffer)          0;

    Const(Thresholds)           0x4000;

    Const(AddressSource)        0x6000;
    Const(Address)              0x6002;
    Const(ModuleId)             0x6004;
    Const(Reset)                0x6008; // write anything here to reset the module.

    Const(Ipl)                  0x6010;
    Const(Vector)               0x6012;
    Const(IrqReset)             0x6016;
    Const(IrqThreshold)         0x6018;
    Const(MaxTransfer)          0x601a;
    Const(WithdrawIrqOnEmpty)   0x601c;

    Const(CbltMcstControl)      0x6020;
    Const(CbltAddress)          0x6022;
    Const(McstAddress)          0x6024;


    Const(LongCount)            0x6030;
    Const(DataFormat)           0x6032;
    Const(ReadoutReset)         0x6034;
    Const(MultiEvent)           0x6036;
    Const(MarkType)             0x6038;
    Const(StartAcq)             0x603A;
    Const(InitFifo)             0x603c;
    Const(DataReady)            0x603e;

    Const(BankOperation)        0x6040;
    Const(Resolution)           0x6042;
    Const(BankOffset0)          0x6044;
    Const(BankOffset1)          0x6046;
    Const(IgnoreThresholds)     0x604c;

    Const(GateLimit0)           0x6050;
    Const(GateLimit1)           0x6052;
    Const(ExpTrigDelay0)        0x6054;
    Const(ExpTrigDelay1)        0x6056;

    // Input/Output 
    Const(InputCoupling)        0x6060;
    Const(ECLTermination)       0x6062;
    Const(ECLGate1)             0x6064; // not sure what to call this.
    Const(ECLFC)                0x6066;
    Const(GateSelect)           0x6068;
    Const(NIMGate1)             0x606a;
    Const(NIMFC)                0x606c;
    Const(NIMBusy)              0x606e;

    Const(TestPulser)           0x6070; // In order to ensure it's off !
    Const(PulserAmp)            0x6072; 

    Const(EventCounterReset)    0x6090;
    Const(TimingSource)         0x6096;
    Const(TimingDivisor)        0x6098;
    Const(TimestampReset)       EventCounterReset; // As of firmware 5.

    Const(MultLimitHigh0)       0x60b0;
    Const(MultLimitLow0)        0x60b2;
    Const(MultLimitHigh1)       0x60b4;
    Const(MultLimitLow1)        0x60b6;

    // RC-bus registers
    Const(RCBusNo)              0x6080;
    Const(RCModNum)             0x6082;
    Const(RCOpCode)             0x6084;
    Const(RCAddr)               0x6086;
    Const(RCData)               0x6088;
    Const(RCStatus)             0x608a;
  }

  // RC-bus op code bits
  Const(RCOP_ON)              0x03;
  Const(RCOP_OFF)             0x04;
  Const(RCOP_READID)          0x06;
  Const(RCOP_WRITEDATA)       0x10;
  Const(RCOP_READDATA)        0x12;

  // RC-bus status bits
  Const(RCSTAT_MASK)          0x01;
  Const(RCSTAT_ACTIVE)        0x00;
  Const(RCSTAT_ADDRCOLLISION) 0x02;
  Const(RCSTAT_NORESPONSE)    0x04;

  // Mcast/CBLT control register bits:

  Const(MCSTENB)              0x80;
  Const(MCSTDIS)              0x40;
  Const(FIRSTENB)             0x20;
  Const(FIRSTDIS)             0x10;
  Const(LASTENB)              0x08;
  Const(LASTDIS)              0x04;
  Const(CBLTENB)              0x02;
  Const(CBLTDIS)              0x01;


  namespace Thresholds {
    Const(Max)  0x1fff;
  }

  namespace BankOffsets {
    Const(Max)  0xff;
  }

  namespace MarkerType {
    Const(EventCount)     0;
    Const(Timestamp30Bit) 1;
    Const(Timestamp46Bit) 3;
  }

  namespace Pulser {
    Const(Off)            0;
    Const(FixedAmplitude) 4;
    Const(UserAmplitude)  5;
  }

  namespace ECLGate1 {
    Const(Gate)         0;
    Const(Oscillator)   1;
  }

  namespace ECLFC {
    Const(FastClear)    0;
    Const(ResetTstamp)  1;
    Const(SyncGate)     2;
  }

  namespace NIMGate1 {
    Const(Gate)         0;
    Const(Oscillator)   1;
  }

  namespace NIMFC {
    Const(FastClear)    0;
    Const(ResetTstamp)  1;
    Const(SyncGate)     2;
  }

  namespace NIMBusy {
    Const(Busy)           0;
    Const(RCBus)          3;
    Const(Full)           4;
    Const(OverThreshold)  8;
  }

  namespace TransferMode {
    Const(Single)       0;
    Const(Unlimited)    1;
    Const(Limited)      3;

    Const(EmitEOB)      4;
    Const(CountEvents)  8;
  }

  namespace GateLimit {
    Const(Max) 0xff;
  }

  namespace ExpTrigDelay {
    Const(Max) 0x3fff;
  }

  namespace CounterReset {
    Const(Never)        0x0;
    Const(CTRA)         0x1;
    Const(CTRB)         0x2;
    Const(External)     0xc;
  }

} // end of namespace
#endif
