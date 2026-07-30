/**
 * @file CPixieRunUtilities.h
 * @brief Defines a class for managing list-mode and baseline runs and a
 * ctypes interface for the class.
 */

#ifndef CPIXIERUNUTILITIES_H
#define CPIXIERUNUTILITIES_H

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
 * @class CPixieRunUtilities CPixieRunUtilities.h
 * @brief Manage list-mode histogram and baseline runs for a
 * Pixie-16 system.
 *
 * This class provides functionality to start and stop runs as well
 * as read data from the modules and return it to the caller.
 */

class CPixieRunUtilities {
private:
  unsigned int m_histogramLength; //!< Length of histogram for current module
  /** Max number of baselines which can be read out. */
  unsigned int m_maxBaselines;
  bool m_runActive;    //!< True when running.
  bool m_useGenerator; //!< True to use generator test data.
  std::unique_ptr<CDataGenerator> m_pGenerator; //!< The offline data generator.
  std::vector<unsigned int> m_histogram;        //!< Single channel histo.
  std::vector<unsigned int> m_baseline; //!< Single channel baseline histo.
  /** Baseline histograms for all channels. */
  std::vector<std::vector<unsigned int>> m_baselineHistograms;
  /** Generated run data histograms for all channels. */
  std::vector<std::vector<unsigned int>> m_genHistograms;
  std::string m_lastErrorMessage; //!< Last error message from the system.

public:
  /** @brief Constructor. */
  CPixieRunUtilities();
  /** @brief Destructor. */
  ~CPixieRunUtilities();

  /**
   * @brief Begin a histogram (MCA) run for a single module. Explicitly sets
   * module synchronization to OFF.
   * @param module Module number.
   * @param nChannels Channels per module.
   * @return int
   * @retval 0   Success.
   * @retval !=0 XIA API error code.
   */
  int BeginHistogramRun(int module, int nChannels);
  /**
   * @brief End a histogram (MCA) run for a single module. Assumes module
   * synchronization is OFF __but__ only stops a run in a single module.
   * @param module Module number.
   * @return int
   * @retval 0   Run end attempted; may return 0 even if the run did not stop
   * within the retry limit.
   * @retval !=0 XIA API error code on failure.
   */
  int EndHistogramRun(int module);
  /**
   * @brief Read energy histogram from single channel.
   * @param module  Module number.
   * @param channel Channel number on module to read histogram from.
   * @return int
   * @retval 0   Success.
   * @retval !=0 XIA API error code.
   */
  int ReadHistogram(int module, int channel);
  /**
   * @brief Begin a baseline run.
   * @param module Module number.
   * @param nChannels Channels per module.
   * @return int
   * @retval 0 Always.
   */
  int BeginBaselineRun(int module, int nChannels);
  /**
   * @brief "End" a baseline run.
   * @param module Module number.
   * @return int
   * @retval 0 Always.
   */
  int EndBaselineRun(int module);
  /**
   * @brief Acquire baselines and read baseline data from a single channel.
   * @param module  Module number.
   * @param channel Channel number on the module.
   * @return int
   * @retval 0   Success.
   * @retval !=0 XIA API error code on failure.
   */
  int ReadBaseline(int module, int channel);
  /**
   * @brief Read statistics for a single module after a run is ended.
   * @param module Module number.
   * @return int
   * @retval 0   Success.
   * @retval !=0 XIA API error code.
   */
  int ReadModuleStats(int module);

  /**
   * @brief Set the use of the generator for offline data.
   * @param mode Set the generator use flag to this value.
   */
  void SetUseGenerator(bool mode) { m_useGenerator = mode; };
  /**
   * @brief Set the last error message. Used by the extern "C" shims when
   * their catch-all fires.
   * @param msg Message to store.
   */
  void SetLastErrorMessage(const char *msg) { m_lastErrorMessage = msg; };

  /**
   * @brief Get the histogram data from a list-mode run.
   * @return Pointer to the underlying histogram storage.
   */
  unsigned int *GetHistogramData() { return m_histogram.data(); };
  /**
   * @brief Get the baseline run data.
   * @return Pointer to the underlying baseline storage.
   */
  unsigned int *GetBaselineData() { return m_baseline.data(); };
  /**
   * @brief Get the current run status.
   * @return bool True if a run is active, false otherwise.
   */
  bool GetRunActive() { return m_runActive; };
  /**
   * @brief Get the histogram length for a module.
   * @param module Module number (zero-indexed).
   * @returns The histogram length for the module or XIA error code if failed.
   */
  int GetHistogramLength(int module);
  /**
   * @brief Get the maximum number of baselines for a module.
   * @param module Module number (zero-indexed).
   * @returns The maximum number of baseline values which can be read out at
   * once for this module or XIA error code if failed.
   */
  int GetMaxBaselines(int module);
  /**
   * @brief Get the reason text from the most recent failed operation.
   * @return Pointer to the stored message; empty string if none.
   */
  const char *GetLastErrorMessage() { return m_lastErrorMessage.c_str(); };

private:
  /**
   * @brief Update baseline histograms for all channels on a single module.
   * @param module Module number.
   * @throw CXIAException If reading the module info or the baseline fails.
   */
  void UpdateBaselineHistograms(int module);
  /**
   * @brief Set the histogram length for a module. This is only used for
   * testing with the generator and should not be used in normal operation.
   * @param module Module number (zero-indexed).
   * @note Histogram length is assume to be the same for all channels on a
   * module. This is a private functon since the histogram length is determined
   * by the module and should not be set from the outside.
   */
  void SetHistogramLength(int module) {
    m_histogramLength = static_cast<unsigned int>(GetHistogramLength(module));
  };
  /**
   * @brief Set the maximum number of baselines for a module
   * @param module Module number (zero-indexed)
   * @note Max number of baselines is assumed to be the same for all channels on
   * a module. This is a private method since the max number of baselines is
   * determined by the module and should not be set from the outside.
   */
  void SetMaxBaselines(int module) {
    m_maxBaselines = static_cast<unsigned int>(GetMaxBaselines(module));
  };
};

/** @} */

extern "C" {
/** @brief Wrapper for the class constructor. */
CPixieRunUtilities *CPixieRunUtilities_new() {
  return shimGuardNew("CPixieRunUtilities_new",
                      []() { return new CPixieRunUtilities(); });
}

/** @brief Wrapper to begin a list-mode histogram data run. */
int CPixieRunUtilities_BeginHistogramRun(CPixieRunUtilities *utils, int mod,
                                         unsigned nchan) {
  return shimGuard(utils, "CPixieRunUtilities_BeginHistogramRun",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->BeginHistogramRun(mod, nchan); });
}
/** @brief Wrapper to end a list-mode histogram data run. */
int CPixieRunUtilities_EndHistogramRun(CPixieRunUtilities *utils, int mod) {
  return shimGuard(utils, "CPixieRunUtilities_EndHistogramRun",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->EndHistogramRun(mod); });
}
/** @brief Wrapper to read histogram data. */
int CPixieRunUtilities_ReadHistogram(CPixieRunUtilities *utils, int mod,
                                     int chan) {
  return shimGuard(utils, "CPixieRunUtilities_ReadHistogram",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->ReadHistogram(mod, chan); });
}

/** @brief Wrapper to begin a baseline data run. */
int CPixieRunUtilities_BeginBaselineRun(CPixieRunUtilities *utils, int mod,
                                        unsigned nchan) {
  return shimGuard(utils, "CPixieRunUtilities_BeginBaselineRun",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->BeginBaselineRun(mod, nchan); });
}
/** @brief Wrapper to end a baseline data run. */
int CPixieRunUtilities_EndBaselineRun(CPixieRunUtilities *utils, int mod) {
  return shimGuard(utils, "CPixieRunUtilities_EndBaselineRun",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->EndBaselineRun(mod); });
}
/** @brief Wrapper to read the baseline data. */
int CPixieRunUtilities_ReadBaseline(CPixieRunUtilities *utils, int mod,
                                    int chan) {
  return shimGuard(utils, "CPixieRunUtilities_ReadBaseline",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->ReadBaseline(mod, chan); });
}

/** @brief Wrapper to read run statistics from the module. */
int CPixieRunUtilities_ReadModuleStats(CPixieRunUtilities *utils, int mod) {
  return shimGuard(utils, "CPixieRunUtilities_ReadModuleStats",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->ReadModuleStats(mod); });
}
/** @brief Wrapper to marshall the histogram data; cannot throw, unguarded. */
unsigned int *CPixieRunUtilities_GetHistogramData(CPixieRunUtilities *utils) {
  return utils->GetHistogramData();
}
/** @brief Wrapper to marshall the baseline data; cannot throw, unguarded. */
unsigned int *CPixieRunUtilities_GetBaselineData(CPixieRunUtilities *utils) {
  return utils->GetBaselineData();
}
/** @brief Wrapper to get the run active status. */
bool CPixieRunUtilities_GetRunActive(CPixieRunUtilities *utils) {
  return shimGuard(utils, "CPixieRunUtilities_GetRunActive", false,
                   [=]() { return utils->GetRunActive(); });
}
/** @brief Wrapper to setup the offline data generator; no return value,
 * unguarded. */
void CPixieRunUtilities_SetUseGenerator(CPixieRunUtilities *utils, bool mode) {
  return shimGuardVoid(utils, "CPixieRunUtilities_SetUseGenerator",
                       [=]() { return utils->SetUseGenerator(mode); });
}
/** @brief Wrapper to get histogram length for a single module. */
int CPixieRunUtilities_GetHistogramLength(CPixieRunUtilities *utils, int mod) {
  return shimGuard(utils, "CPixieRunUtilities_GetHistogramLength",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->GetHistogramLength(mod); });
}
/** @brief Wrapper to get the maximum number of baselines for a single module.
 */
int CPixieRunUtilities_GetMaxBaselines(CPixieRunUtilities *utils, int mod) {
  return shimGuard(utils, "CPixieRunUtilities_GetMaxBaselines",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->GetMaxBaselines(mod); });
}
/** @brief Wrapper to get the last error message. Cannot throw; unguarded. */
const char *CPixieRunUtilities_GetLastErrorMessage(CPixieRunUtilities *utils) {
  return utils->GetLastErrorMessage();
}

/** @brief Wrapper for the class destructor. */
void CPixieRunUtilities_delete(CPixieRunUtilities *utils) {
  try {
    delete utils;
  } catch (...) {
    std::cerr << "CPixieRunUtilities_delete unknown exception" << std::endl;
  }
};
}

#endif
