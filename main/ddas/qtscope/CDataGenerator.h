/**
 * @file CDataGenerator.h
 * @brief Defines a class for generating offline data for testing/debugging 
 * and a ctypes interface for the class.
 */

#ifndef CDATAGENERATOR_H
#define CDATAGENERATOR_H

#include <random>

/**
 * @addtogroup utilities libPixieUtilities.so
 * @brief Pixie-16 utilities for QtScope.
 *
 * @details
 * This utility library is used by QtScope. It contains a number of classes
 * which call other parts of the DDAS code to boot and manage the modules. 
 * This library defines an API by which the pure-Python QtScope code can 
 * interact with the C/C++ FRIBDAQ and XIA API code needed to run a system of 
 * Pixie modules.
 * @{ 
 */

/**
 * @class CDataGenerator
 * @brief A class to generate test pulse, run, and baseline data for offline 
 * operation of QtScope.
 */

class CDataGenerator
{
private:
    // Previous 12.x versions had the engine as a non-static member.
    // I cannot quite figure out why but initialization of non-static
    // engine fails and the program hangs when generating random numbers.
    // Possibly related to threading??? Anyway, this works:
    static std::mt19937 m_engine; //!< Random number generator engine.
    static std::uniform_real_distribution<double> m_C; //!< Trace offset.
    static std::uniform_real_distribution<double> m_A; //!< Trace amplitude.
    static std::normal_distribution<double> m_rise; //!< Trace exp. rise.
    static std::normal_distribution<double> m_decay; //!< Trace exp. decay.
    static std::normal_distribution<double> m_noise; //!< Trace noise.
    static std::uniform_real_distribution<double> m_baseline; //!< Baseline run.
    
public:   
    /**
     * @brief Generate test trace data.
     * @param[in,out] data Pointer to the start of the trace data storage.
     * @param[in] dataSize How many data points to store.
     * @param[in] binWidth Histogram bin width in microseconds.
     * @return int  
     * @retval 0 Success.
     */
    int GetTraceData(unsigned short* data, int dataSize, double binWidth);
    /**
     * @brief Generate test Gaussian-distributed data.
     * @param[in,out] data Pointer to the start of the baseline data storage.
     * @param[in] dataSize How many data points to store.
     * @return 0 (always).
     */
    int GetHistogramData(unsigned int* data, int dataSize);
    /**
     * @brief Generate randomly distributed test baseline data.
     * @param[in,out] data Pointer to the start of the baseline data storarge.
     * @param[in] dataSize How many data points to store.
     * @return int  
     * @retval 0 Success.
     */
    int GetBaselineData(double* data, int dataSize);
  
private:
    
    /**
     * @brief Analytical function for a single pulse with exponential rise and 
     * decay constants.
     * @param C      Constant baseline.
     * @param A      Pulse amplitude.
     * @param t0     Start of the pulse.
     * @param rise   Pulse risetime in ns.
     * @param decay  Pulse exponential decay time in ns.
     * @param sample Sample number where we compute the pulse.
     * @return Pulse value at input sample number.
     */
    unsigned short SinglePulse(
	double C, double A, double t0, double rise, double decay,
	int sample, double binWidth
	);
};

/** @} */

#endif
