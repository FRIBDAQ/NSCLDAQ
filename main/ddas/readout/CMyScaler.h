/** 
 * @file CMyScaler.h
 * @brief Define the DDAS scaler class.
 */

#ifndef MYSCALER_H
#define MYSCALER_H

#include <config.h>
#include <CScaler.h>
#include <vector>
#include <stddef.h>

/**
 * @class CMyScaler
 * @brief Generate scaler data from run statistics.
 * @details 
 * Generates scaler information from the run statistics read from the 
 * module(s). A DDAS module with N channels has a scalar bank of 2N + 1 values.
 * The first value in index zero (0) for that module is used to store the crate
 * ID, which is read from the cfgPixie16.txt file. The crate ID value is 
 * reported on stdout when the modules are booted e.g. when running a readout 
 * code: "Scalers know crate ID = <myID>". Following the ID are N pairs of 
 * channel scaler data corresponding to the number of observed (input) and 
 * accepted (output) fast triggers since the last scaler read.
 *
 * For example, a 16-channel module scalar bank has the format:
 *
 * @verbatim
 scaler[0]  = crateID
 scaler[1]  = input[0]
 scaler[2]  = output[0]
 scaler[3]  = input[1]
 scaler[4]  = output[1]
 ...
 scaler[31] = input[15]
 scaler[32] = output[15]
 @endverbatim
 *
 * where input[0] and output[0] refer to the observed and accepted triggers 
 * seen by channel 0 on the module.
 *
 * @note (ASC 9/4/24): Based on the DDAS scaler class originally written by 
 * H. Crawford.
 */

class CMyScaler : public CScaler
{
public:
    /** @brief Count raw and accepted triggers. */
    typedef struct _Counters {
        size_t s_nTriggers;         //!< Raw triggers.
        size_t s_nAcceptedTriggers; //!< Accepted triggers (i.e. by the FPGA).
    } Counters;
    /** @brief Statistics are counters for cumulative and per-run triggers. */
    typedef struct _Statistics {
        CMyScaler::Counters s_cumulative; //!< Cumulative. Not cleared on initialize.
        CMyScaler::Counters s_perRun;     //!< Per-run. Cleared on initialize.
    } Statistics;
    
private:
    unsigned short m_crate;  //!< Crate ID value.
    unsigned short m_module; //!< Module number.    
    unsigned short m_nChannels; //!< Channels for this scaler object
    std::vector<unsigned int> m_prevIC; //!< Previous raw trigger count
    std::vector<unsigned int> m_prevOC; //!< Previous accepted trigger count
    std::vector<uint32_t> m_scalers; //!< Vector of scaler data for the module.
    Statistics m_statistics;         //!< Storage for calculated scaler data.

public:
    /**
     * @brief Constructor.
     * @param crate The crate ID where the module resides.
     * @param mod The module number.
     * @param nchan Number of channels in the module.
     */
    CMyScaler(unsigned short crate, unsigned short mod, unsigned short nchan);
    /** @brief Destructor. */
    ~CMyScaler();

    /** @brief Zero the per-run statistics and counters. */
    virtual void initialize();
    /**
     * @brief Read scalar data from a module.
     * @return Vector of scalar data for a single module.
     */
    virtual std::vector<uint32_t> read();
    /** @brief Cannot clear with Pixies. Does nothing. */
    virtual void clear() {};
    /** 
     * @brief Disable. Scalars do not need to be disabled at the end of a run.
     */
    virtual void disable() {};
    /** 
     * @brief Return the size of the scaler data.
     * @return Always 32 (only for 16-channel cards!)
     */
    virtual unsigned int size() { return 32; };
    /** 
     * @brief Get the run statistics.
     * @return Reference to the statistics storage object.
     */
    const Statistics& getStatistics() const { return m_statistics; }
    
private:
  void clearCounters(Counters& c);
};

#endif
