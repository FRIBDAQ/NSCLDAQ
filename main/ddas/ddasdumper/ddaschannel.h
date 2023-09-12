/**
 * @file ddaschannel.h
 * @brief Encapsulate the information in a generic DDAS event in a class 
 * written to accomodate I/O operations in ROOT.
 */

#ifndef DDASCHANNEL_H
#define DDASCHANNEL_H

#include <vector>
#include <stdint.h>
#include "TObject.h"

namespace DAQ {
    namespace DDAS {
	class DDASHit;
    }
}

/**
 * @addtogroup libddaschannel libddaschannel.so
 * @brief DDAS data format for ROOT I/O e.g. produced by the ddasdumper 
 * program.
 * @{
 */

/**
 * @class ddaschannel ddaschannel.h
 * @brief Encapsulation of a generic DDAS event.
 *
 * @details
 * The ddaschannel class is intended to encapsulate the information that is
 * emitted by the Pixie-16 dgitizer for a single event. It contains information
 * for a single channel only. It is generic because it can store data for
 * the 100 MSPS, 250 MSPS, and 500 MSPS Pixie-16 digitizers used at the lab. 
 * In general all of these contain the same set of information, however, the
 * meaning of the CFD data is different for each. The ddaschannel class 
 * abstracts these differences away from the user.
 *
 * This class provides a raw data parser (\see UnpackChannelData) that should 
 * be used to fill the item with data. It is encouraged to construct complete
 * events from raw data using it. For example,
 *
 * \code
 * ddaschannel channel;
 * channel.UnpackChannelData(pDataBuffer);
 * \endcode
 *
 * It should also be noted that the ddaschannel can be persistently stored in a 
 * ROOT file. It inherits from TObject and has an appropriate dictionary 
 * generated for it that will stream it to and from a file. In fact, the 
 * ddasdumper program creates a TTree filled with ddaschannel object.
 *
 * This is very close to a ROOT-ized DDASHit, and in fact ddaschannel objects 
 * can be (and are in the ddasdumper!) copy-constructed from DDASHits. The 
 * classes differ in some key ways:
 * 1. The member data in ddaschannel is public.
 * 2. Some data members in ddaschannel are different than those in DDASHit.
 * The copy constructor handles this.
 */
class ddaschannel : public TObject {
public:
    // Ordering is important with regards to access and file size. Should
    // always try to maintain order from largest to smallest data type
    // Double_t, Int_t, Short_t, Bool_t, pointers.

    /* Channel events always have the following info. */
    Double_t time;              ///< Assembled time including CFD.
    Double_t coarsetime;        ///< Assembled time without CFD.
    Double_t cfd;               ///< CFD time only \deprecated (ASC 7/12/23) Remove.

    UInt_t energy;              ///< Energy of event.
    UInt_t timehigh;            ///< Bits 32-47 of timestamp.
    UInt_t timelow;             ///< Bits 0-31 of timestamp.
    UInt_t timecfd;             ///< Raw CFD time.

    Int_t channelnum;           ///< \deprecated (ASC 7/12/23): Remove.
    Int_t finishcode;           ///< Indicates whether pile-up occurred.
    Int_t channellength;        ///< Number of 32-bit words of raw data.
    Int_t channelheaderlength;  ///< Length of header.
    Int_t overflowcode;         ///< ADC overflow code, 1 = overflow.
    Int_t chanid;               ///< Channel index.
    Int_t slotid;               ///< Slot index.
    Int_t crateid;              ///< Crate index.
    Int_t id;                   ///< \deprecated (ASC 7/12/23): Remove.

    Int_t cfdtrigsourcebit;     ///< Value of trigger source bit(s) for
                                ///< 250 MSPS and 500 MSPS.
    Int_t cfdfailbit;           ///< Indicates whether the CFD algo failed.

    Int_t tracelength;          ///< Length of stored trace.

    Int_t ModMSPS;              ///< Sampling rate of the module (MSPS).
    Int_t m_adcResolution;      ///< ADC resolution (i.e. bit depth).
    Int_t m_hdwrRevision;       ///< Hardware revision.
    Bool_t m_adcOverUnderflow;  ///< ADC over- and underflow flag.

    /* A channel may have extra information... */
    std::vector<UInt_t> energySums;  ///< Energy sum data.
    std::vector<UInt_t> qdcSums;     ///< QDC sum data.
  
    /* A waveform (trace) may be stored too. */
    std::vector<UShort_t> trace;     ///< Trace data.

    Double_t externalTimestamp;      ///< External clock.
    
    /////////////////////////////////////////////////////////////
    // Canonicals

    /** @brief Default constructor. */
    ddaschannel(); 
    /** @brief Default copy constructor. */
    ddaschannel(const ddaschannel& obj) = default;
    /** @brief Default assignment operator. */
    ddaschannel& operator=(const ddaschannel& obj) = default;
    /**
     * @brief Copy a DDASHit into a ddaschannel object.
     * @param hit References the hit to copy.
     * @return Reference to the object.
     */
    ddaschannel& operator=(DAQ::DDAS::DDASHit& hit);
    /** @brief Destructor. */
    ~ddaschannel() {};
    /**
     * @brief Parse event data from DDASReadout.
     * @param data Pointer to first 32-bit word of an event body.
     */
    void UnpackChannelData(const uint32_t *data);
    /** @brief Reset the member data */
    void Reset();
    
    /////////////////////////////////////////////////////////////  
    // Data accessors
  
    /** 
     * @brief Retrieve the energy.
     * @return The energy.  
     */
    UInt_t GetEnergy() const { return energy; }
    /** 
     * @brief Retrieve most significant 16-bits of raw timestamp.
     * @return The upper 16 bits of the 48-bit timestamp.
     */
    UInt_t GetTimeHigh() const { return timehigh; }
    /**
     * @brief Retrieve least significant 32-bit of raw timestamp.
     * @return The lower 32 bits of te 48-bit timestamp. 
     */
    UInt_t GetTimeLow() const { return timelow; }
    /** 
     * @brief Retrieve the raw CFD time.
     * @return The raw CFD time from the event data.
     */
    UInt_t GetCFDTime() const { return timecfd; }
    /** 
     * @brief Retrieve computed time 
     * @return The timestamp, with the CFD correction, in units of nanoseconds.
     *
     * @details
     * This method performs a computation that depends on the type of the 
     * digitizer that produced the data. In each case, the coarse timestamp 
     * is formed using the timelow and timehigh. This is coarse timestamp is 
     * then corrected using any CFD time that exists.
     *
     * The calculations for the various modules are as follows:
     *
     * For the 100 MSPS modules:
     *
     * \f[\text{time} = 10\times((\text{timehigh} << 32) + \text{timelow})\f]
     *  
     * For the 250 MSPS modules:
     *
     * \f[\text{time} = 8\times((\text{timehigh} << 32) 
     * + \text{timelow}) + 4\times(\text{timecfd}/2^{14}
     * - \text{cfdtrigsourcebit})\f]
     *
     * For the 500 MSPS modules:
     *
     * \f[\text{time} = 10\times((\text{timehigh} << 32) 
     * + \text{timelow}) + 2\times(\text{timecfd}/2^{13}
     * + \text{cfdtrigsourcebit} - 1)\f]
     */
    Double_t GetTime() const { return time; }
    /** 
     * @brief Retrieve the 48-bit timestamp in nanoseconds without any 
     * CFD correction
     * @return The 48-bit leading-edge timestamp without the CFD correction. 
     */
    Double_t GetCoarseTime() const { return coarsetime; }
    /** 
     * \deprecated (ASC 7/12/23) Remove.
     */
    Double_t GetCFD() const { return cfd; }
    /** 
     * @brief Retrieve a specific energy sum value.
     * @param idx Index of energy sum to access.
     * @return The energy sum specified by the input index.
     *     
     * @details
     * There is no bound checking here. The caller is responsible for 
     * ensuring that the data exists to be retrieved.
     */
    UInt_t GetEnergySums(Int_t idx) const { return energySums[idx]; }
    /** 
     * @brief Retrieve the channel number. 
     * @return The channel number.
     */
    Int_t GetChannelNum() const { return channelnum; }
    /** 
     * @brief Retrieve finish code.
     * @return The channel finish code.
     *
     * @details
     * The finish code will be set to 1 if pileup was detected.
     */
    Int_t GetFinishCode() const { return finishcode; }
    /** 
     * @brief Retrieve number of 32-bit words that were in the original 
     * data packet.
     * @return Int_t  The channel event length in 32-bit words.

     * @details
     * Note that this only really makes sense to be used if the object was 
     * filled with data using UnpackChannelData().
     */
    Int_t GetChannelLength() const { return channellength; }
    /** 
     * @brief Retrieve length of header in original data packet.
     * @return The channel event header length in 32-bit words. 
     */
    Int_t GetChannelLengthHeader() const { return channelheaderlength; }
    /** 
     * @brief Retrieve the overflow code.
     * @return The ADC channel overflow code.
     *
     * @details
     * The overflow code == 1 if the channel ADC overflowed for this hit.
     */
    Int_t GetOverflowCode() const { return overflowcode; }
    /** 
     * @brief Retrieve the slot that the module resided in.
     * @return The module slot number. 
     */
    Int_t GetSlotID() const { return slotid; }
    /** 
     * @brief Retrieve the index of the crate the module resided in. 
     * @return The crate ID value.
     */
    Int_t GetCrateID() const { return crateid; }
    /** 
     * @brief Retrieve the channel index. 
     * @return The channel ID value.
     */
    Int_t GetChannelID() const { return chanid; }
    /** 
     * \deprecated (ASC 7/12/23): Remove. 
     */
    Int_t GetID() const { return id; }
    /** 
     * @brief Retrieve the ADC frequency of the module. 
     * @return The ADC frequency of the module in MSPS.
     */
    Int_t GetModMSPS() const { return ModMSPS; }
    /** 
     * @brief Retrieve the ADC resolution.
     *@return Module ADC resolution (bit depth). 
     */
    Int_t GetADCResolution() const { return m_adcResolution; }
    /** 
     * @brief Retrieve the hardware revision. 
     * @return Module hardware revision number. 
     */
    Int_t GetHardwareRevision() const { return m_hdwrRevision; }
    /** 
     * @brief Retrieve the ADC overflow/underflow status
     * @return Bool_t
     * @retval true   If the ADC over- or underflows.
     * @retval false  Otherwise.
     *
     * @details
     * In the 12 and 14 bit modules, this is the value of bit 15 in 
     * the 4th header word. In the 16 bit modules, this is the value 
     * of bit 31 in the 4th header word.
     */
    Bool_t GetADCOverflowUnderflow() const { return m_adcOverUnderflow; }
    /** 
     * @brief Retrieve the trace data 
     * @return The ADC trace vector.
     */
    std::vector<UShort_t> GetTrace() const { return trace; }
    /** 
     * @brief Retrieve trigger source bit from CFD data. 
     * @return The CFD trigger source bit.
     */
    uint32_t GetCFDTrigSource() const { return cfdtrigsourcebit; };
  
    ClassDef(ddaschannel, 5)
};

/** @} */

#endif
