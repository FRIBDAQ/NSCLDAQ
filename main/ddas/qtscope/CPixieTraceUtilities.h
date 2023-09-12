/**
 * @file  CPixieTraceUtilities.h
 * @brief Defines a class for trace management and a ctypes interface for 
 * the class.
 */

#ifndef CPIXIETRACEUTILITIES_H
#define CPIXIETRACEUTILITIES_H

#include <vector>

class CDataGenerator;

/**
 * @addtogroup utilities libPixieUtilities.so
 * @{ 
 */

/**
 * @class CPixieTraceUtilities CPixieTraceUtilities.h
 * @brief A class to read and fetch trace data from Pixie-16 modules.
 *
 * @details
 * This class provides a ctypes-friendly interface to acquire "validated" 
 * (traces which are likely to contain a good signal pulse) and unvalidated 
 * traces. The class also provides methods to access the trace data.
 *
 * @todo Instead of validated traces can we process the trace using the fast 
 * filter parameters and wait for a real trigger?
 */

class CPixieTraceUtilities
{
public:
    /** @brief Constructor. */
    CPixieTraceUtilities();
    /** @brief Destructor. */
    ~CPixieTraceUtilities();
    
    /**
     * @brief Read a validated ADC trace from single channel.
     * @param module  Module number.
     * @param channel Channel number on module for trace read.
     * @return int
     * @retval  0 Success.
     * @retval -1 XIA API call fails.
     * @retval -2 Acquired trace is empty (median undefined).
     */
    int ReadTrace(int module, int channel);
    /**
     * @brief Read a validated ADC trace from single channel.
     * @param module  Module number.
     * @param channel Channel number on module for trace read.
     * @return int
     * @retval  0 Success.
     * @retval -1 XIA API call fails.
     */
    int ReadFastTrace(int module, int channel);
    /**
     * @brief Return the trace data.
     * @return unsigned short*  Pointer to the underlying trace storage.
     */
    unsigned short* GetTraceData() {return m_trace.data();}
    /**
     * @brief Set the flag for offline mode using the data generator.
     * @param mode The generator flag is set to this input value.
     */
    void SetUseGenerator(bool mode) {m_useGenerator = mode;}
  
private:
    CDataGenerator* m_pGenerator; //!< The offline data generator.
    bool m_useGenerator; //!< True if using generated data, else online data.
    std::vector<unsigned short> m_trace; //!< Single channel trace data.
    double m_validAmplitude; //!< Minimum amplitude for a validated trace.
    /**
     * @brief Call to Pixie-16 API to acquire an ADC trace from a single 
     *   channel.
     * @param module  Module number.
     * @param channel Channel number on module for trace read.
     * @throws std::runtime_error If ADC traces cannot be acquired (internal
     *   DSP memory fails to fill).
     * @throws std::runtime_error If trace read fails.
     */
    void AcquireADCTrace(int module, int channel);
    /**
     * @brief Calculate the median value from a trace.
     * @param v Input vector of type T.
     * @throws std::invalid_argument If trace is empty (median is undefined).
     * @return Median value of the trace.
     */
    template<typename T> double GetMedianValue(std::vector<T> v);
};

/** @} */

extern "C" {
    /** @brief Wrapper for the class constructor. */
    CPixieTraceUtilities* CPixieTraceUtilities_new()
    {
	return new CPixieTraceUtilities();
    }
    /** @brief Wrapper for reading a validated trace. */
    int CPixieTraceUtilities_ReadTrace(
	CPixieTraceUtilities* utils, int mod, int chan
	)
    {
	return utils->ReadTrace(mod, chan);
    }
    /** @brief Wrapper for reading an unvalidated trace. */
    int CPixieTraceUtilities_ReadFastTrace(
	CPixieTraceUtilities* utils, int mod, int chan
	)
    {
	return utils->ReadFastTrace(mod, chan);
    }
    /** @brief Wrapper to get trace data. */
    unsigned short* CPixieTraceUtilities_GetTraceData(
	CPixieTraceUtilities* utils
	)
    {
	return utils->GetTraceData();
    }
    /** @brief Wrapper to set generator use. */
    void CPixieTraceUtilities_SetUseGenerator(
	CPixieTraceUtilities* utils, bool mode
	)
    {
	return utils->SetUseGenerator(mode);
    }

    /** @brief Wrapper for the class destructor. */
    void CPixieTraceUtilities_delete(CPixieTraceUtilities* utils)
    {
	if(utils) {
	    delete utils;
	    utils = nullptr;
	}
    }
}

#endif
