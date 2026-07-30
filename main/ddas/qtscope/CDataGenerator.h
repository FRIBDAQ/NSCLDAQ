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

class CDataGenerator {
private:
  std::mt19937 m_engine; //!< Random number generator engine.
  std::uniform_real_distribution<double> m_C; //!< Trace offset.
  std::uniform_real_distribution<double> m_A; //!< Trace amplitude.
  std::normal_distribution<double> m_rise;    //!< Trace exponential rise.
  std::normal_distribution<double> m_decay;   //!< Trace exponential decay.
  std::normal_distribution<double> m_noise;   //!< Trace random noise.
  std::uniform_real_distribution<double> m_baseline; //!< Baseline run data.

public:
  /** @brief Constructor. */
  CDataGenerator();

  /**
   * @brief Generate test trace data.
   * @param[in,out] data Pointer to the start of the trace data storage.
   * @param[in] dataSize How many data points to store.
   * @param[in] binWidth Histogram bin width in microseconds.
   * @return int
   * @retval 0 Success.
   */
  int GetTraceData(unsigned short *data, int dataSize, double binWidth);
  /**
   * @brief Generate test Gaussian-distributed data.
   * @param[in,out] data Pointer to the start of the baseline data storage.
   * @param[in] dataSize How many data points to store.
   * @return 0 (always).
   */
  int GetHistogramData(unsigned int *data, int dataSize);
  /**
   * @brief Generate randomly distributed test baseline data.
   * @param[in,out] data Pointer to the start of the baseline data storage.
   * @param[in] dataSize How many data points to store.
   * @return int
   * @retval 0 Success.
   */
  int GetBaselineData(double *data, int dataSize);

private:
  /**
   * @brief Analytical function for a single pulse with exponential rise and
   * decay constants.
   * @param C        Constant baseline.
   * @param A        Pulse amplitude.
   * @param t0       Start of the pulse, in samples.
   * @param rise     Pulse risetime in microseconds.
   * @param decay    Pulse exponential decay time in microseconds.
   * @param sample   Sample number where we compute the pulse.
   * @param binWidth Sample width in microseconds (the XDT bin width).
   * @return Pulse value at input sample number.
   */
  unsigned short SinglePulse(double C, double A, double t0, double rise,
                             double decay, int sample, double binWidth);
};

/** @} */

#endif
