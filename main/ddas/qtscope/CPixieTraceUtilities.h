/**
 * @file  CPixieTraceUtilities.h
 * @brief Defines a class for trace management and a ctypes interface for
 * the class.
 */

#ifndef CPIXIETRACEUTILITIES_H
#define CPIXIETRACEUTILITIES_H

#include <memory>
#include <string>
#include <vector>

#include "CPixieShimGuard.h"

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

class CPixieTraceUtilities {
private:
  bool m_useGenerator;     //!< True if using generated data, else online data.
  double m_validAmplitude; //!< Minimum amplitude for a validated trace.
  std::unique_ptr<CDataGenerator> m_pGenerator; //!< The offline data generator.
  std::vector<unsigned short> m_trace;          //!< Single channel trace data.
  std::string m_lastErrorMessage; //!< Last error message from the system.

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
   * @retval 0 Success.
   * @retval < 0 XIA API error code.
   * @retval CPIXIEERROR_TRACE_ACQUIRE Other acquisition errors (trace empty,
   * offline generator failures).
   */
  int ReadTrace(int module, int channel);
  /**
   * @brief Read an unvalidated ADC trace from single channel.
   * @param module  Module number.
   * @param channel Channel number on module for trace read.
   * @return int
   * @retval 0 Success.
   * @retval < 0 XIA API error code.
   * @retval CPIXIEERROR_TRACE_ACQUIRE Other acquisition errors (trace empty,
   * offline generator failures).
   */
  int ReadFastTrace(int module, int channel);
  /**
   * @brief Get the trace length for a given module/channel.
   * @param module Module number.
   * @param channel Channel number.
   * @return Trace length in samples.
   * @retval < 0 XIA error code, could not read trace length.
   */
  int GetTraceLength(int module, int channel);
  /**
   * @brief Return the trace data.
   * @return Pointer to the underlying trace storage.
   */
  unsigned short *GetTraceData() { return m_trace.data(); }
  /**
   * @brief Set the flag for offline mode using the data generator.
   * @param mode The generator flag is set to this input value.
   */
  void SetUseGenerator(bool mode) { m_useGenerator = mode; }

  /**
   * @brief Set the last error message. Used by the extern "C" shims when
   * their catch-all fires.
   * @param msg Message to store.
   */
  void SetLastErrorMessage(const char *msg) { m_lastErrorMessage = msg; };
  /**
   * @brief Get the reason text from the most recent failed operation.
   * @return Pointer to the stored message; empty string if none.
   */
  const char *GetLastErrorMessage() { return m_lastErrorMessage.c_str(); };

private:
  /**
   * @brief Call to Pixie-16 API to acquire an ADC trace from a single
   * channel.
   * @param module  Module number.
   * @param channel Channel number on module for trace read.
   * @throw CXIAException If trace allocation or read fails.
   * @throw CXIAException Failed to read module XDT (offline generator mode).
   * @throw std::runtime_error Failed to get trace data (offline generator
   * mode).
   */
  void AcquireADCTrace(int module, int channel);
  /**
   * @brief Call Pixie-16 API to acquire the trace length from a module. It is
   * assumed all channels on a single module have the same maximum trace length.
   * @param module Module number.
   * @param channel Channel number.
   * @return Maximum trace length in samples.
   * @throw CXIAException If the trace length cannot be read. It is the
   * responsibility of the caller to handle this exception.
   */
  unsigned int AcquireTraceLength(int module, int channel);
  /**
   * @brief Calculate the median value from a trace.
   * @param v Input vector of type T.
   * @throw std::invalid_argument If trace is empty (median is undefined).
   * @return Median value of the trace.
   */
  template <typename T> double GetMedianValue(std::vector<T> v);
  /**
   * @brief Reset the trace storage vector.
   * @param len Trace length to read.
   */
  void ResetTrace(unsigned int len);
};

/** @} */

extern "C" {
/** @brief Wrapper for the class constructor. */
CPixieTraceUtilities *CPixieTraceUtilities_new() {
  return shimGuardNew("CPixieTraceUtilities_new",
                      []() { return new CPixieTraceUtilities(); });
}
/** @brief Wrapper for reading a validated trace. */
int CPixieTraceUtilities_ReadTrace(CPixieTraceUtilities *utils, int mod,
                                   int chan) {
  return shimGuard(utils, "CPixieTraceUtilities_ReadTrace",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->ReadTrace(mod, chan); });
}
/** @brief Wrapper for reading an unvalidated trace. */
int CPixieTraceUtilities_ReadFastTrace(CPixieTraceUtilities *utils, int mod,
                                       int chan) {
  return shimGuard(utils, "CPixieTraceUtilities_ReadFastTrace",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->ReadFastTrace(mod, chan); });
}
/** @brief Wrapper to get trace length. */
int CPixieTraceUtilities_GetTraceLength(CPixieTraceUtilities *utils, int mod,
                                        int chan) {
  return shimGuard(utils, "CPixieTraceUtilities_GetTraceLength",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->GetTraceLength(mod, chan); });
}
/** @brief Wrapper to get trace data; cannot throw, unguarded. */
unsigned short *CPixieTraceUtilities_GetTraceData(CPixieTraceUtilities *utils) {
  return utils->GetTraceData();
}
/** @brief Wrapper to set generator use. */
void CPixieTraceUtilities_SetUseGenerator(CPixieTraceUtilities *utils,
                                          bool mode) {
  return shimGuardVoid(utils, "CPixieTraceUtilities_SetUseGenerator",
                       [=]() { return utils->SetUseGenerator(mode); });
}
/** @brief Wrapper to get the last error message. Cannot throw, unguarded. */
const char *
CPixieTraceUtilities_GetLastErrorMessage(CPixieTraceUtilities *utils) {
  return utils->GetLastErrorMessage();
}

/** @brief Wrapper for the class destructor. */
void CPixieTraceUtilities_delete(CPixieTraceUtilities *utils) {
  try {
    delete utils;
  } catch (...) {
    std::cerr << "CPixieTraceUtilities_delete unknown exception" << std::endl;
  }
};
}

#endif
