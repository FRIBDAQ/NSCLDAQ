/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Aaron Chester
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file DDASDataSimulator.h
 * @brief Defines a class to simulate DDAS data recored by NSCLDAQ readout.
 */

#ifndef DDASDATASIMULATOR_H
#define DDASDATASIMULATOR_H

#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace ddasfmt {
	class DDASHit;
}
namespace ufmt {
    class RingItemFactoryBase;
}

namespace DAQ {
    namespace DDAS {
	
	/**
	 * @addtogroup libDDASDataSimulator libDDASDataSimulator.so
	 * @brief Data simulation library for DDAS.
	 * @details
	 * The DDAS data simulation library allows users to write NSCLDAQ event
	 * data with DDAS-style payloads to disk from any external program 
	 * e.g., Geant4.
	 * @{
	 */
	
	/**
	 * @class DAQ::DDAS::DDASDataSimulator
	 * @brief Simulates data produced by a DDAS readout program in user 
	 * code.
	 * @details
	 * This class provides an interface to simulate the data output by an 
	 * NSCLDAQ readout program running DDAS electronics. The 
	 * ddasfmt::DDASHit class is used to encapsulate the hit information.
	 * The `putHit()` method of this class fills an event buffer with the 
	 * Pixie data payload based on the contents of the passed 
	 * ddasfmt::DDASHit, including optional data like QDC sums or traces,
	 * wraps it in an NSCLDAQ header and writes it to a file data sink.
	 *
	 * The output NSCLDAQ data format is specified by the user when they 
	 * instantiate the class. For completeness, the `beginRun()` and 
	 * `endRun()` methods will write the expected data format and state 
	 * change items you would see when starting and stopping a run.
	 *
	 * In general the code uses the information contained within the 
	 * ddasfmt::DDASHit to figure out its size. The PHYSICS_ITEM's body 
	 * header timestamp is set using a calibrated timestamp value derived 
	 * from the full timestamp, which requires either the module MSPS OR an 
	 * external timestamp AND clock calibration to be specified. In the 
	 * case of the module timestamp, this value is given in nanoseconds, 
	 * whereas an external timestamp is given in clock ticks. Trace data 
	 * is _not checked_ for overflows or to ensure that the range of the 
	 * trace matches the bit depth of the module, that responsiblity is 
	 * on the user. It is  assumed that the CFD always succeeds, even if 
	 * the correction is 0.
	 *
	 * To use this class in your own code:
	 *   - Ensure `$DAQINC`, `$DAQROOT`/unifiedformat/include and 
	 *     `$DAQROOT`/ddasformat/include are in the compiler include files 
	 *     search path (add `-I<path>` in e.g., `CXXFLAGS`) and include
	 *     DDASDataSimulator.h and DDASHit.h.
	 *   - Link against the installed DDASDataSimulator.so and 
	 *     DDASFormat.so libraries in `$DAQLIB` and 
	 *     `$DAQROOT`/ddasformat/lib (add `-L<path> 
	 *     -Wl,-rpath=<path>` to `LDFLAGS` and `-l<libname>` to `LDLIBS`).
	 *   - Build your executable with the proper compiler and linker flags 
	 *     described above.
	 *
	 * Example code to create a complete run with a single event:
	 ```
	 #include <DDASDataSimulator.h>
	 #include <DDASHit.h>

	 using namespace DAQ::DDAS;
	 using namespace ddasfmt;

	 int main () {
	     DDASDataSimulator sim("data.evt", 12);
	     DDASHit hit;

	     sim.beginRun();

	     // Coarse TS and CFD derived from full time:

	     hit.setModMSPS(250);
	     hit.setCrateID(0);
	     hit.setSlotID(2);
	     hit.setChannelID(0);
	     hit.setEnergy(1000);
	     hit.setTime(1234.5678);
	     sim.putHit(hit);

	     sim.endRun();
 
	     return 0;
	 }
	 ```
	 * 
	 */

	class DDASDataSimulator
	{
	private:
	    std::string m_fname; //!< Output file name.
	    int m_fd; //!< File descriptor for output file.
	    ufmt::RingItemFactoryBase* m_pFactory; //!< Creates ring items.
	    std::vector<uint32_t> m_evtBuf; //!< Buffer for Pixie payload.
	    std::time_t m_start; //!< Start time of the "run."
	    std::time_t m_stop;  //!< Stop time of the "run."

	public:
	    /**
	     * @brief Constructor.
	     * @param fname Output file name. 
	     * @param version NSLCDAQ data format version. Mapped to ufmt 
	     *   enum value.
	     */
	    DDASDataSimulator(std::string fname, int version);

	    /** 
	     * @brief Begin a simulated run.
	     * @param sourceID The source ID value (optional, default=0).
	     * @throws CErrnoExecption If the output file isn't opened properly.
	     */
	    void beginRun(int sourceID=0) ;
	    /** 
	     * @brief End a simulated run.
	     * @param sourceID The source ID value (optional, default=0).
	     * @throws CErrnoExecption If the output file isn't closed properly.
	     */
	    void endRun(int sourceID=0);
	    /**
	     * @brief Write a hit to the output file. This is the normal way 
	     * users will add simulated data to the output.
	     * @param hit References the hit to write.
	     * @param sourceID The source ID value (optional, default=0).
	     * @param useExtTS Use an external timestamp (optional, 
	     *   default=false).
	     * @param cal Clock calibration in nanoseconds per clock cycle. 
	     *   Required if useExtTS is true (optional, default=0.0).
	     * @throw std::runtime_error If timestamp or external timestamp
	     *   are invalid (<= 0 or not provided when expected).
	     * @throw std::runtime_error If using an external timestamp with 
	     *   an invalid clock calibration (<= 0.0).
	     */
	    void putHit(
		const ddasfmt::DDASHit& hit, int sourceID=0,
		bool useExtTS=false, double cal=0.0
		);

	    /**
	     * @brief Set the data buffer from a DDASHit.
	     * @param hit References the hit to process.
	     */
	    void setBuffer(const ddasfmt::DDASHit& hit);
	    /**
	     * @brief Get the data buffer.
	     * @return The data buffer. May or may not be empty, depending on 
	     *   whether `setBuffer()` is called first.
	     */
	    std::vector<uint32_t> getBuffer() { return m_evtBuf; }
	    /** @brief Formatted dump of data buffer to stdout. */
	    void dumpBuffer();

	private:
	    /** 
	     * @brief Set word 0 of the fixed Pixie list-mode event header. 
	     * Contains identifying information for the hit and its size.
	     * @param hit References the hit to process.
	     */
	    void setWord0(const ddasfmt::DDASHit& hit);
	    /** 
	     * @brief Set words 1 and 2 of the fixed Pixie list-mode event 
	     * header. Word 1 Contains lower 32 bits of 48-bit timestamp, 
	     * word 2 contains upper 16 bits of 48-bit timestamp and CFD 
	     * correction.
	     * @param hit References the hit to process.
	     * @throws std::runtime_error If the hit timestamp is < 0.
	     */
	    void setWords1And2(const ddasfmt::DDASHit& hit);
	    /** 
	     * @brief Set word 3 of the fixed Pixie list-mode event header. 
	     * Contains energy and trace length, overflow.
	     * @param hit References the hit to process.
	     */
	    void setWord3(const ddasfmt::DDASHit& hit);

	    /**
	     * @brief Set the external timestamp from the hit.
	     * @param hit References the hit to process.
	     */
	    void setExternalTS(const ddasfmt::DDASHit& hit);
	    /**
	     * @brief Set the energy sums from the hit.
	     * @param hit References the hit to process.
	     */
	    void setEnergySums(const ddasfmt::DDASHit& hit);
	    /**
	     * @brief Set the QDC sums from the hit.
	     * @param hit References the hit to process.
	     */
	    void setQDCSums(const ddasfmt::DDASHit& hit);
	    /**
	     * @brief Set the ADC trace from the hit.
	     * @param hit References the hit to process.
	     */
	    void setTraceData(const ddasfmt::DDASHit& hit);
    
	    /**
	     * @brief Get the Pixie header length.
	     * @param hit References the hit to process.
	     * @return The Pixie header length in 32-bit words.
	     */
	    uint32_t getHeaderLength(const ddasfmt::DDASHit& hit);
	    /**
	     * @brief Get the module identification word.
	     * @param hit References the hit to process.
	     * @return The module ID as a 32-bit data word.
	     */
	    uint32_t getModInfoWord(const ddasfmt::DDASHit& hit);
	    /**
	     * @brief Get the coarse timestamp from the hit.
	     * @param hit References the hit to process.
	     * @return The coarse timestamp in clock ticks.
	     */
	    uint64_t getCoarseTimestamp(const ddasfmt::DDASHit& hit);
	    /**
	     * @brief Get the FPGA clock period in nanoseconds.
	     * @param hit References the hit to process.
	     * @throws std::runtime_error If the module MSPS is not known.
	     * @return The clock period in nanoseconds.
	     */
	    int getClockPeriod(const ddasfmt::DDASHit& hit);
	    /**
	     * @brief Get the ADC sampling period in nanoseconds.
	     * @param hit References the hit to process.
	     * @throws std::runtime_error If the module MSPS is not known.
	     * @return The sampling period in nanoseconds.
	     */
	    int getSamplePeriod(const ddasfmt::DDASHit& hit);
	    /**
	     * @brief Get the packed CFD result (CFD as data word).
	     * @param hit References the hit to process.
	     * @return The packed CFD word as a uint32_t.
	     */
	    uint32_t getPackedCFDResult(
		const ddasfmt::DDASHit& hit, double corr
		);    
	};

	/** @} */

    } // namespace DDAS.
} // namespace DAQ.

#endif
