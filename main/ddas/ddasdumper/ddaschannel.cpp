/**
 * @file ddaschannel.cpp
 * @brief Encapsulate the information in a generic DDAS event in a class 
 * written to accomodate I/O operations in ROOT.
 */

#include "ddaschannel.h"

#include <iostream>
#include <limits>
#include <tuple>

#include <DDASHit.h>
#include <DDASHitUnpacker.h>
#include <DDASBitMasks.h>

using namespace std;

ClassImp(ddaschannel);

// Avoid the static initialization order fiasco with construct
// on first use idiom
// (see https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use)
DAQ::DDAS::DDASHit& getStaticHit() {
    static DAQ::DDAS::DDASHit* pHit = new DAQ::DDAS::DDASHit;
    return *pHit;
}

/** 
 * @details
 * All member data are zero initialized.
 */
ddaschannel::ddaschannel() : TObject() {
    time = 0;
    coarsetime = 0;
    cfd = 0;

    energy = 0;
    timehigh = 0;
    timelow = 0;
    timecfd = 0;

    channelnum = 0;
    finishcode = 0;
    channellength = 0;
    channelheaderlength = 0;
    overflowcode = 0;
    chanid = 0;
    slotid = 0;
    crateid = 0;
    id = 0;

    cfdtrigsourcebit = 0;
    cfdfailbit = 0;

    tracelength = 0;

    ModMSPS = 0;
    m_adcResolution = 0;
    m_hdwrRevision = 0;
    m_adcOverUnderflow = false;    
  
    energySums.reserve(4);
    qdcSums.reserve(8);
    trace.reserve(200);

    externalTimestamp = 0;
}

/**
 * @details
 * This copies the contents of an existing DDASHit into a ddaschannel object. 
 * For data members that are not identical between the DDASHit and ddaschannel
 * class, this does a best effort at handling them appropriately. These are 
 * those values and how they are handled:
 *
 * | ddaschannel data member |  handling               |
 * |-------------------------|-------------------------|
 * | cfd                     | set to zero             |
 * | channelnum              | DDASHit::GetChannelID() |
 * | id                      | set to zero             |
 */
ddaschannel& ddaschannel::operator=(DAQ::DDAS::DDASHit& hit)
{
    time                = hit.GetTime();
    coarsetime          = hit.GetCoarseTime();
    cfd                 = 0;
    energy              = hit.GetEnergy();
    timehigh            = hit.GetTimeHigh();
    timelow             = hit.GetTimeLow();
    timecfd             = hit.GetTimeCFD();
    channelnum          = hit.GetChannelID();
    finishcode          = hit.GetFinishCode();
    channellength       = hit.GetChannelLength();
    channelheaderlength = hit.GetChannelLengthHeader();
    overflowcode        = hit.GetOverflowCode();
    chanid              = hit.GetChannelID();
    slotid              = hit.GetSlotID();
    crateid             = hit.GetCrateID();
    id                  = 0;
    cfdtrigsourcebit    = hit.GetCFDTrigSource();
    cfdfailbit          = hit.GetCFDFailBit();
    tracelength         = hit.GetTraceLength();
    ModMSPS             = hit.GetModMSPS();
    energySums          = hit.GetEnergySums();
    qdcSums             = hit.GetQDCSums();
    trace               = hit.GetTrace();
    externalTimestamp   = hit.GetExternalTimestamp();
    m_adcResolution     = hit.GetADCResolution();
    m_hdwrRevision      = hit.GetHardwareRevision();
    m_adcOverUnderflow  = hit.GetADCOverflowUnderflow();

    return *this;
}

/** 
 * @details
 * This expects data from the DDASReadout program. It will parse the entire 
 * body of the event in a manner that is consistent with the data present. 
 * In other words, it uses the sizes of the event encoded in the data to 
 * determine when the parsing is complete. 
 *
 * While it parses, it stores the results into the data members of the object. 
 * Prior to parsing, all data members are reset to 0 using the Reset() method.
 */
void ddaschannel::UnpackChannelData(const uint32_t *data)
{
    using namespace DAQ::DDAS;
    DDASHitUnpacker unpacker;
    DDASHit& hit = getStaticHit();
    hit.Reset();
    size_t nShorts = *data;
    unpacker.unpack(
	data, data + nShorts*sizeof(uint32_t)/sizeof(uint16_t), hit
	); 
    // Copy the state
    *this = hit;
}


/**
 * @details
 * For primitive types, this sets the values to 0. For vector data 
 * (i.e. trace), the vector is cleared and resized to 0.
 */
void ddaschannel::Reset() {
    time = 0;
    coarsetime = 0;
    cfd = 0;

    energy = 0;
    timehigh = 0;
    timelow = 0;
    timecfd = 0;

    channelnum = 0;
    finishcode = 0;
    channellength = 0;
    channelheaderlength = 0;
    overflowcode = 0;
    chanid = 0;
    slotid = 0;
    crateid = 0;
    id = 0;

    cfdtrigsourcebit = 0;
    cfdfailbit = 0;

    tracelength = 0;

    ModMSPS = 0;    
    m_hdwrRevision = 0;
    m_adcResolution = 0;
    m_adcOverUnderflow = false;

    energySums.clear();
    qdcSums.clear();
    trace.clear();
  
    externalTimestamp = 0;
}
