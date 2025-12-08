/**
 * @file CMyEventSegment.h
 * @brief Define a DDAS event segment.
 */

#ifndef CMYEVENTSEGMENT_H
#define CMYEVENTSEGMENT_H

#include <CEventSegment.h>

#include <vector>

#include <Configuration.h>
#include <SystemBooter.h>

const int MAX_MODULES_PER_CRATE = 13; //!< A full crate is 13 modules.

class CMyTrigger;
class CExperiment;

/**
 * @class CMyEventSegment
 * @brief Derived class for DDAS event segments.
 * @details
 * The event segment reads out a logical chunk of an experiment. In the DDAS 
 * case, data from a single crate (single source ID). An experiment may 
 * consist of multiple crates arranged in a CCompoundEventSegment container.
 */

class CMyEventSegment : public CEventSegment
{
private:
#pragma pack(push, 1) // Do not pad.
    /**
     * @struct HitHeader
     * @brief Pixie-16 list-mode event header data and methods to extract 
     * identifying information from the first word.
     */
    struct HitHeader {
	uint32_t s_id;            //!< Pixie list-mode event header word 0.
	uint32_t s_tstampLow;     //!< Pixie list-mode event header word 1.
	uint32_t s_tstampHighCFD; //!< Pixie list-mode event header word 2.
	uint32_t s_traceInfo;     //!< Pixie list-mode event header word 3.
        
	// Selectors -- a bit too magic numbery but sufficient for what we
	// want to do in debugging. See the Pixie-16 manual Sec. 4.2.2 "List
	// Mode Data Structures" for more information.

	/** 
	 * @brief Get the channel ID from word 0.
	 * @return The channel ID.
	 */
	unsigned getChan() const {
	    return s_id & 0xf;
	}
	/** 
	 * @brief Get the slot ID from word 0.
	 * @return The slot ID.
	 */
	unsigned getSlot() const {
	    return (s_id & 0xf0) >> 4;
	}
	/** 
	 * @brief Get the crate ID from word 0.
	 * @return The crate ID.
	 */
	unsigned getCrate() const {
	    return (s_id & 0xf00) >> 8;
	}
	/** 
	 * @brief Get the header length from word 0.
	 * @return The header length (32-bit words, inclusive).
	 */
	unsigned headerLength() const {
	    return (s_id & 0x1f000) >> 12;
	}
	/** 
	 * @brief Get the event length from word 0.
	 * @return The event length (32-bit words, inclusive).
	 */
	unsigned eventLength() const {
	    return (s_id & 0x7ffe0000) >> 17;
	}
    };
#pragma pack(pop)
    
private:
    size_t m_nModules;             //!< Number of modules in the crate.
    std::vector<int> m_modEvtLens; //!< Expected event lengths (32-bit words).
    /** Word to store rev, bit depth, and MSPS of module for insertion into 
     * the data stream.*/
    unsigned int m_modRevBitMSPSWord[MAX_MODULES_PER_CRATE];
    /** Calibration constants for each module in the crate in nanoseconds per 
     * clock tick. */
    double m_modClockCal[MAX_MODULES_PER_CRATE];
    DAQ::DDAS::Configuration m_config; //!< Configuration data for the segment.
    bool m_systemInitialized;          //!< Are modules are booted and online?
    bool m_firmwareLoadedRecently;     //!< True when loading FW on full boot.
    CMyTrigger* m_pTrigger;            //!< Trigger definition.
    CExperiment* m_pExperiment;        //!< Experiment we're reading data from.
   
    // Statistics:
    
    size_t m_nCumulativeBytes; //!< Cumulative bytes of data read.
    size_t m_nBytesPerRun;     //!< Bytes of data read this run.
    
public:
    /**
     * @brief Construct from trigger object and experiment 
     * @param trig Pointer to the DDAS trigger.
     * @param exp Reference to the experiment the event segment comes from.
     */
    CMyEventSegment(CMyTrigger *trig, CExperiment& exp);
    /** 
     * @brief Default constructor.
     * @note For unit testing purposes only!
     */
    CMyEventSegment(); // For unit testing only!!
    /** @brief Destructor. */
    ~CMyEventSegment();

    /** @brief Initialize the modules recording data in this segment. */
    virtual void initialize();
    /** 
     * @brief Read data from the modules following a valid trigger. 
     * @param[in,out] rBuffer  Read data into this buffer.
     * @param[in]     maxBytes Max bytes of data we can stuff in the buffer.
     * @return Number of 16-bit words in the ring item body.
     */
    virtual size_t read(void* rBuffer, size_t maxwords);
    /** @brief Nothing to disable. */
    virtual void disable();
    /** @brief Nothing to clear. */
    virtual void clear();

    /** @brief Manage run start operation. */
    virtual void onBegin();
    /** @brief Manage run resume operation. */
    virtual void onResume();
    /** @brief Just return. Sorting is offloaded into its own process. */
    virtual void onEnd(CExperiment* pExperiment) { return; };

    /** 
     * @brief Get the number of modules in the crate.
     * @return Number of modules.
     */
    size_t GetNumberOfModules() { return m_nModules; }
    /**
     * @brief Get the channel count in a given module.
     * @param mod The module index.
     * @return The channel count of that module.
     */
    unsigned short GetChannelCount(unsigned int mod)
	{ return m_config.getChannelCount(mod); }
    /** 
     * @brief Get the crate ID value from the configuration.
     * @return The crate ID.
     */
    int GetCrateID() const { return m_config.getCrateId(); }

    /**
     * @brief Perform clock synchronization.
     * @throw CDDASException If we fail to talk properly to the module while 
     *   setting the clock synchronization parameters.
     */
    void synchronize();

    /**
     * @brief Load firmware and boot the modules.
     * @param type The boot type (boot mask) passed to the system booter 
     *   (default = SystemBooter::FullBoot).
     * @throw CDDASException If the system is initialized and fails to exit 
     *   before attempting to boot again.
     */
    void boot(
	DAQ::DDAS::SystemBooter::BootType = DAQ::DDAS::SystemBooter::FullBoot
	);

    /**
     * @brief Get the cumulative and current run statistics.
     * @return Cumulative and current run stats as a std::pair.
     */
    std::pair<size_t, size_t>getStatistics() {
	return std::pair<size_t, size_t>(m_nCumulativeBytes, m_nBytesPerRun);
    }    
};

#endif
