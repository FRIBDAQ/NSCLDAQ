/**
 * @file DDASHitUnpacker.h
 * @brief Define an unpacker for DDAS data recorded by NSCLDAQ/FRIBDAQ.
 */

#ifndef DAQ_DDAS_DDASHITUNPACKER_H
#define DAQ_DDAS_DDASHITUNPACKER_H

#include "DDASHit.h"

#include <vector>
#include <cstdint>
#include <tuple>

/** @namespace DAQ */
namespace DAQ {
    /** @namespace DAQ::DDAS */
    namespace DDAS {
	/**
	 * @addtogroup format libddasformat.so
	 * @{
	 */
	
	/** 
	 * @class DDASHitUnpacker DDASHitUnpacker.h
	 *
	 * @brief Unpacker for DDAS data recorded by NSCLDAQ/FRIBDAQ.
	 *
	 * This class unpacks NSCLDAQ-formatted Pixie-16 data recorded by a
	 * DDAS readout program into DDASHits which encapsulate the information
	 * recorded by a single DDAS channel. This is a generic unpacker which 
	 * can accomodate information from all Pixie-16 digitizer types at 
	 * FRIB. In general, all of the digitizer outputs contain the same 
	 * information but importantly the meaning of the CFD data depends on 
	 * the digitizer type. The unpacker class abstracts this difference 
	 * away from the user. 
	 *
	 * This class does not provide any parsing capabilities likes its
	 * companion class ddasdumper. To fill this with data, you should use 
	 * the associated DDASHit class. Here is how you use it:
	 *
	 * \code
	 * DDASHit hit;
	 * DDASHitUnpacker unpacker;
	 * unpacker.unpack(pData, pData+sizeOfData, hit);
	 * \endcode
	 *
	 * where pData is a pointer to the first word of the event body.
	 */
	class DDASHitUnpacker {
	public:
	    /**
	     * @brief Unpack data into a DDASHit.
	     * @param beg      Pointer to the first word of the hit body.
	     * @param sentinel Pointer to the first word after the end of the 
	     *   body.
	     * @return Tuple of (DDASHit, nextWord).
	     */
	    std::tuple<DDASHit, const uint32_t*> unpack(
		const uint32_t* beg, const uint32_t* sentinel
		);
	    /**
	     * @brief Unpack data into a DDASHit.
	     * @param[in] beg  Pointer to the first 32-bit word of the hit 
	     *   body.
	     * @param[in] sentinel Pointer to the first word after the end of 
	     *   the body.
	     * @param[in,out] hit  Reference to the DDASHit object filled 
	     *   during unpacking.
	     * @throw std::runtime_error If the hit data buffer is empty.
	     * @throw std::runtime_error If the hit's length is not the value
	     *   specified in the header.
	     * @return Pointer to the next data word after the hit.
	     */
	    const uint32_t* unpack(
		const uint32_t* beg, const uint32_t* sentinel, DDASHit& hit
		); 

	protected:
	    /**
	     * @brief Ensure there is enough data to parse.
	     * @param data     Pointer to the hit body.
	     * @param sentinel Pointer to the first word after the body.
	     * @throws std::runtime_error If there are an incorrect number of 
	     *   16-bit data words in the event (words exceed sentinal boundry)
	     *   and the pointer to the last data word is not a nullptr.
	     * @return Pointer to the next data word. 
	     */
	    const uint32_t* parseBodySize(
		const uint32_t* beg, const uint32_t* sentinel
		);
	    /**
	     * @brief Parse the module identifying information encoded in the 
	     *   hit.
	     * @param hit  References the DDASHit we are unpacking.
	     * @param data The 32-bit data word containing the module 
	     *   identifying information.
	     * @return Pointer to the next data word.
	     */
	    const uint32_t* parseModuleInfo(DDASHit& hit, const uint32_t* beg);
	    /**
	     * @brief Parse the word 0 of the Pixie-16 data header.
	     * @param hit  References the DDASHit we are unpacking.
	     * @param data  The 32-bit data word to parse.
	     * @return Pointer to the next data word (word 1).

	     */
	    const uint32_t* parseHeaderWord0(DDASHit& hit, const uint32_t* beg);
	    /**
	     * @brief Parse words 1 and 2 of the Pixie-16 data header.
	     * @param hit  References the DDASHit we are unpacking.
	     * @param data Pointer to word 1.
	     * @return Pointer to word 3.
	     */
	    const uint32_t* parseHeaderWords1And2(
		DDASHit& hit, const uint32_t* beg
		);
	    /**
	     * @brief Parse word 3 of the Pixie-16 data header.
	     * @param hit  References the DDASHit we are unpacking.
	     * @param data Pointer to word 3.
	     * @return Pointer to the first word of the Pixie-16 data body.
	     */
	    const uint32_t* parseHeaderWord3(DDASHit& hit, const uint32_t* beg);
	    /**
	     * @brief Unpack the trace data.
	     * @param hit  References the hit we are unpacking.
	     * @param data Pointer to the 32-bit trace word to unpack. 
	     *   The 32-bit trace word contains two 16-bit trace ADC values.
	     * @return Pointer to the next 32-bit word.
	     */
	    const uint32_t* parseTraceData(DDASHit& hit, const uint32_t* beg);
	    /**
	     * @brief Determine the CFD correction to the leading-edge time in 
	     * nanoseconds from the CFD word.
	     * @param ModMSPS The module ADC frequency in MSPS.
	     * @param data    The 32-bit data word encoding the CFD information.
	     * @return (CFD correction in nanoseconds, value of the CFD encoded
	     *    in the data, CFD trigger source bit, CFD fail bit).
	     */
	    std::tuple<double, uint32_t, uint32_t, uint32_t>
	    parseAndComputeCFD(uint32_t ModMSPS, uint32_t data);
	    /**
	     * @brief Determine the CFD correction to the leading-edge time in 
	     *   nanoseconds from the CFD word.
	     * @param hit  References the DDASHit we are unpacking.
	     * @param data The 32-bit data word encoding the CFD information.
	     * @return double The CFD correction in nanoseconds.
	     */
	    double parseAndComputeCFD(DDASHit& hit, uint32_t data);
	    /**
	     * @brief Compute time in nanoseconds from raw data (no CFD 
	     *   correction).
	     * @param adcFrequency Module ADC frequency in MSPS.
	     * @param timelow      Data word containing the lower 32 bits of 
	     *   the 48-bit timestamp.
	     * @param timehigh     Data word containing the upper 16 bits of 
	     *   the 48-bit timestamp. 
	     * @return The 48-bit coarse timestamp in nanoseconds.
	     */
	    uint64_t computeCoarseTime(
		uint32_t adcFrequency, uint32_t timelow, uint32_t timehigh
		);
	    /**
	     * @brief Unpack energy sums.
	     * @param data Pointer to the first 32-bit word containing the 
	     *   energy sum data.
	     * @param hit  References the DDASHit we are unpacking.
	     * @return Pointer to the word after the energy sums.
	     */
	    const uint32_t* extractEnergySums(
		const uint32_t* data, DDASHit& hit
		);
	    /** 
	     * @brief Unpack QDC values.
	     * @param data Pointer to the first 32-bit word containing the 
	     *   QDC sum data.
	     * @param hit  References the DDASHit we are unpacking.
	     * @return Pointer to the word after the QDC sums.
	     */
	    const uint32_t* extractQDC(
		const uint32_t* data, DDASHit& hit
		);
	    /**
	     * @brief Unpack the external timestamp data.
	     * @param data Pointer to the 32-bit word containing the lower 
	     *   16 bits of the 48-bit external timestamp.
	     * @param hit  References the DDASHit we are unpacking.
	     * @return Pointer to the word after the external timestamp.
	     */
	    const uint32_t* extractExternalTimestamp(
		const uint32_t* data, DDASHit& hit
		);
	};

	/** @} */

    } // end DDAS namespace
} // end DAQ namespace

#endif

