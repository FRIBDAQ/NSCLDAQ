#ifndef MVLC_CCAENV1X90REGISTERS_H
#define MVLC_CCAENV1X90REGISTERS_H

/*!
  This file contains register definitions for the CAEN V1190, V1290 TDC
  modules.  We will assume that the module will be acessed via VmePtrs:
  - The module address space is divided into 4 segments:
    - The Output buffer.
    - The Control registers.
    - The Configuration ROM
    - The compensation SRAM.
  - We will define the offsets of each of these segments relative to the
    module base address as byte offsets.
  - We will Define offsets within each region as byte offsets from the segment
    base (e.g. the InterruptVector register offset will be 0xc not 0x100c.

*/

namespace CCAENV1x90Registers
{
  // Can't use typedefs for these since you can't mix typedef with 
  // static, const etc.

  #define Offset      static const int
  #define Size        static const unsigned int
  #define RegisterBit unsigned short
  #define LongValue   static unsigned int

  // Segment offsets from the module base address:

  Offset OutputBuffer    (0);	   //!< Output buffer [where data goes].
  Size   OutputBufferSize(0x1000); //!< Size of the output buffer space.
  Offset Registers       (0x1000); //!< All the control registers.
  Size   RegisterSize   (0x1300); //!< Upper limit on register size.
  Offset ConfigRom       (0x4000); //!< The configuration prom.
  Size   ConfigRomSize   (0x0200); //!< Configuration prom size.
  Offset CompensationSram(0x8000); //!< The compensation sram.
  Size   CompensationSramSize(0x0200); //!< Size of compensation sram.

  // Offsets within to register page.  These are prefixed
  // W for word sized registers and L for longword sized registers.

  Offset WControlRegister   (0x1000);
  Offset WStatusRegister    (0x1002);
  Offset WInterruptLevel    (0x100a);
  Offset WInterruptVector   (0x100c);
  Offset WVirtualSlot       (0x100e);
  Offset WMulticastBase     (0x1010);
  Offset WMulticastControl  (0x1012);
  Offset WReset             (0x1014);
  Offset WClear             (0x1016);
  Offset WEventReset        (0x1018);
  Offset WSWTrigger         (0x101a);
  Offset LEventCounter      (0x101c);
  Offset WEventStored       (0x1020);
  Offset WAlmostFullLevel   (0x1022);
  Offset WBLTEventNumber    (0x1024);
  Offset WFirmwareRevision  (0x1026);
  Offset LTestRegister      (0x1028);
  Offset WOutputControl     (0x102c);
  Offset WMicroData         (0x102e);
  Offset WMicroHandshake    (0x1030);
  Offset WSelectFlash       (0x1032);
  Offset WFlashMemory       (0x1034);
  Offset WSramPage          (0x1036);
  Offset LEventFIFO         (0x1038);
  Offset WEventFIFOStored   (0x103c);
  Offset WEventFIFOStatus   (0x103e);
  Offset LDummy32           (0x1200);
  Offset WDummy16           (0x1204);

  // Offsets within the configuration prom.  Note that while these locations
  // all accept D16 accesses, usually, only the bottom byte has useful
  // information.  The top byte will usually be indeterminate.

  Offset WChecksum          (0x00);
  Offset WChecksum_Length2  (0x04);
  Offset WChecksum_Length1  (0x08);
  Offset WChecksum_Length0  (0x0c);
  Offset WConstant2         (0x10);
  Offset WConstant1         (0x14);
  Offset WConstant0         (0x18);
  Offset WC_Code            (0x1c);
  Offset WR_Code            (0x20);
  Offset WOUI2              (0x24);
  Offset WOUI1              (0x28);
  Offset WOUI0              (0x2c);
  Offset WBoardVersion      (0x30); // e.g. a,b,n
  Offset WModelNumber2      (0x34);
  Offset WModelNumber1      (0x38);
  Offset WModelNumber0      (0x3c);
  Offset WRevision3         (0x40);
  Offset WRevision2         (0x44);
  Offset WRevision1         (0x48);
  Offset WRevision0         (0x4c);
  Offset WSerialNumber1     (0x80);
  Offset WSerialNumber0     (0x84);

  // Note that the data buffer bits are defined in CCAENV1x90Data.h
  // The remainder of this file defines bits for each register for which
  // it makes sense. The bits themselves are defined in namespaces
  // that are named after the registers without the size prefix.
  //  Each bit is named exactly as the bit is named in the hardware
  // manual.

  namespace ControlRegister 
  {	//!< Control register definitions: Masks.
    RegisterBit BERREN              (  0x1);	
    RegisterBit NBERREN             (0);
    RegisterBit TERM                (  0x2);
    RegisterBit NTERM               (1);
    RegisterBit TERM_SW             (  0x4);
    RegisterBit NTERM_SW            (2);
    RegisterBit EMPTY_EVENT         (  0x08);
    RegisterBit NEMPTY_EVENT        (3);
    RegisterBit ALIGN64             ( 0x010);
    RegisterBit NALIGN64            (4);
    RegisterBit COMPENSATION_ENABLE ( 0x20);
    RegisterBit NCOMPENSATION_ENABLE (5);
    RegisterBit TEST_FIFO_ENABLE    ( 0x40);
    RegisterBit NTEST_FIFO_ENABLE   (6);
    RegisterBit READ_SRAM_ENABLE    ( 0x80); 
    RegisterBit NREAD_SRAM_ENABLE   (7);
    RegisterBit EVENT_FIFO_ENABLE   (0x100);
    RegisterBit NEVENT_FIFO_ENABLE  (8);
    RegisterBit TRIGGER_TAG_ENABLE (0x200);  
    RegisterBit NTRIGGER_TAG_ENABLE (9);
  }

  // The N.... are the bit numbers for the corresponding
  //      .... bit mask.
  namespace StatusRegister 
  {	// This is a read only register.
    RegisterBit DATA_READY          (0x0001);
    RegisterBit NDATA_READY         (0);
    RegisterBit ALM_FULL            (0x0002);
    RegisterBit NALM_FULL           (1);
    RegisterBit FULL                (0x0004);
    RegisterBit NFULL               (2);
    RegisterBit TRG_MATCH           (0x0008);
    RegisterBit NTRG_MATCH          (3);
    RegisterBit HEADER_EN           (0x0010);
    RegisterBit NHEADER_EN          (4);
    RegisterBit TERM_ON             (0x0020);
    RegisterBit NTERM_ON            (5);
    RegisterBit CHIP0_ERROR         (0x0040);
    RegisterBit NCHIP0_ERROR        (6);
    RegisterBit CHIP1_ERROR         (0x0080);
    RegisterBit NCHIP1_ERROR        (7);
    RegisterBit CHIP2_ERROR         (0x0100);
    RegisterBit NCHIP2_ERROR        (8);
    RegisterBit CHIP3_ERROR         (0x0200);
    RegisterBit NCHIP3_ERROR        (9);
    RegisterBit BERR_FLAG           (0x0400);
    RegisterBit NBERR_FLAG          (10);
    RegisterBit PURGE               (0x0800);
    RegisterBit NPURGE              (11);
    RegisterBit RESOLUTIONMASK      (0x3000); //    \ The resolution mask can
    RegisterBit RES_800ps           (0x0000); //     \ be anded with the 
    RegisterBit RES_200ps           (0x1000); //     / status reg. &  compared
    RegisterBit RES_100ps           (0x2000); //    /  with any of the RES_
    RegisterBit RES_25ps            (0x3000); //   /
    RegisterBit PAIR                (0x4000); // Hit detection is pair mode
    RegisterBit NPAIR               (14);
    RegisterBit TRIGGERLOST         (0x8000); // Triggers were lost.    
    RegisterBit NTRIGGERLOST        (15);
  } 
  // Values in the multicast control register.

  namespace MulticastControl 
  {
    RegisterBit MASK                (0x0003); // Bits in the register of use.
    RegisterBit ACTIVE_FIRST        (0x0002); // Module is first in chain.
    RegisterBit ACTIVE_LAST         (0x0001); // Module is last in chain.
    RegisterBit ACTIVE_INTERMEDIATE (0x0003); // Module in the middle of chain.
  }
  
  // The output control register determines which signal is reflected on 
  // the OUT_PROG ECL output of the module.

  namespace OutputControl 
  {
    RegisterBit MASK              (0x0007); // mask of the bits.
    RegisterBit DATA_READY        (0x0000); //  OUT_PROG reflects data ready
    RegisterBit FULL              (0x0001); //  OUT_PROG reflects full cond.
    RegisterBit ALM_FULL          (0x0002); //  OUT_PROG reflects almost full.
    RegisterBit ERROR             (0x0003); //  OUT_PROG reflects TDC error.

  }
  // Micro handshake controls interactions with the micro sequencer.

  namespace MicroHandshake 
  {
    RegisterBit WRITE_OK         (0x000001); // 1 if ok to write sequencer.
    RegisterBit READ_OK          (0x000002); // Nonzero if ok to read.

  }
  // The event fifo contains the event count and word count for each
  // event.

  namespace EventFIFO 
  {
    LongValue    FIFOCOUNT_MASK  (0x7ff);      // Mask of bits in FIFO count reg. 
    LongValue    WORDCOUNT_MASK  (0x0000ffff); // Mask off word count.
    LongValue    WORDCOUNT_RSHIFT (0);         // right shift count.
    LongValue    EVENTCOUNT_MASK  (0xffff0000); // Event counter mask.
    LongValue    EVENTCOUNT_RSHIFT(16);         // Right shift positioning count.
  }

  // FIFO status gives the fullness of the FIFO:

  namespace FIFOStatus 
  {
    RegisterBit  EVFIFODATA_READY(1); // The fifo has data.
    RegisterBit  EVFIFO_FULL(2);      // FIFO is full.
  }
  // In the configuration rom, the version contains the sub type of the module:
  namespace ModuleVersion 
  {
    RegisterBit  A(0);		// 1x90A
    RegisterBit  B(1);		// 1190B
    RegisterBit  N(2);		// 1290N
  }

}


#endif
