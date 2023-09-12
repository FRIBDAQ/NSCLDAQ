/**
 * @file DDASBitMasks.h
 * @brief Define bit masks to extract data from specific locations of Pixie-16 data.
 */

#ifndef DDASBITMASKS_H
#define DDASBITMASKS_H

static const uint32_t CHANNELIDMASK     =        0xF; //!< Bits 0-3 inclusive.
static const uint32_t SLOTIDMASK        =       0xF0; //!< Bits 4-7 inclusive.
static const uint32_t CRATEIDMASK       =      0xF00; //!< Bits 8-11 inclusive.
static const uint32_t HEADERLENGTHMASK  =    0x1F000; //!< Bits 12-16 inclusive.
static const uint32_t CHANNELLENGTHMASK = 0x3FFE0000; //!< Bits 17-29 inclusive.
static const uint32_t OVERFLOWMASK      = 0x40000000; //!< Bit 30 has overflow information (1 == overflow).
static const uint32_t FINISHCODEMASK    = 0x80000000; //!< Bit 31 has pileup information (1 == pileup).
static const uint32_t LOWER16BITMASK    =     0xFFFF; //!< Lower 16 bits.
static const uint32_t UPPER16BITMASK    = 0xFFFF0000; //!< Upper 16 bits.
static const uint32_t LOWER12BITMASK    =      0xFFF; //!< Lower 12 bits.
static const uint32_t BIT31MASK         = 0x80000000; //!< Bit 31.
static const uint32_t BIT30MASK         = 0x40000000; //!< Bit 30.
static const uint32_t BIT30to29MASK     = 0x60000000; //!< Bits 30 through 29.
static const uint32_t BIT31to29MASK     = 0xE0000000; //!< Bits 31 through 29.
static const uint32_t BIT30to16MASK     = 0x7FFF0000; //!< Bits 30 through 16.
static const uint32_t BIT29to16MASK     = 0x3FFF0000; //!< Bits 29 through 16.
static const uint32_t BIT28to16MASK     = 0x1FFF0000; //!< Bits 28 through 16.

/** 
 * Number of words added to pixie16 channel event header when energy sums 
 * and baselines are recorded. 
 */
static const uint32_t SIZEOFESUMS = 4;
/** 
 * Number of words added to pixie16 channel event header when QDC sums are 
 * recorded. 
 */
static const uint32_t SIZEOFQDCSUMS = 8;

#endif
