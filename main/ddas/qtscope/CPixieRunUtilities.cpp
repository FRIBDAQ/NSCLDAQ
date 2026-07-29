/**
 * @file CPixieRunUtilities.cpp
 * @brief Implementation of the run utilities class.
 */

#include "CPixieRunUtilities.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <thread>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

#include "CDataGenerator.h"

/**
 * @details
 * The CPixieRunUtilities class has ownership of a CDataGenerator object and is
 * responsible for managing it.
 */
CPixieRunUtilities::CPixieRunUtilities()
    : m_histogramLength(0), m_maxBaselines(0), m_runActive(false),
      m_useGenerator(false), m_pGenerator(std::make_unique<CDataGenerator>()) {}

CPixieRunUtilities::~CPixieRunUtilities() {}

/**
 * @todo Disable multiple modules from running in non-sync mode.
 */
int CPixieRunUtilities::BeginHistogramRun(int module, int nChannels) {
  SetHistogramLength(module);

  m_genHistograms.assign(nChannels,
                         std::vector<unsigned int>(m_histogramLength, 0));
  if (m_histogram.size() != m_histogramLength) {
    m_histogram.resize(m_histogramLength);
  }
  std::fill(m_histogram.begin(), m_histogram.end(), 0);

  ///
  // Begin the run:
  //

  try {
    // Set the "infinite" run time of 99999 seconds:
    std::string paramName = "HOST_RT_PRESET";
    int retval = Pixie16WriteSglModPar(paramName.c_str(),
                                       Decimal2IEEEFloating(99999), module);
    if (retval < 0) {
      std::stringstream msg;
      msg << "Run time not properly set."
          << " CPixieRunUtilities::BeginHistogramRun() failed to write "
          << paramName << " to module " << module;
      throw CXIAException(msg.str(), "Pixie16WriteSglModPar()", retval);
    }

    // If the run time is properly set, begin a histogram run for this
    // module turn off synchronization (0):
    paramName = "SYNCH_WAIT";
    retval = Pixie16WriteSglModPar(paramName.c_str(), 0, module);
    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieRunUtilities::BeginHistogramRun() failed to write "
          << paramName << " to module " << module;
      throw CXIAException(msg.str(), "Pixie16WriteSglModPar()", retval);
    }

    // Begin the run:
    retval = Pixie16StartHistogramRun(module, NEW_RUN);

    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieRunUtilities::BeginHistogramRun() failed to start "
          << "run in module " << module;
      throw CXIAException(msg.str(), "Pixie16StartHistogramRun()", retval);
    } else {
      std::cout << "Beginning histogram run in Mod. " << module << std::endl;
      m_runActive = true;
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * If the run cannot be ended on the first attempt, retry 10 times before
 * reporting that the run could not be ended properly. Generally speaking, this
 * is caused when one or more channels has a very high trigger rate.
 */
int CPixieRunUtilities::EndHistogramRun(int module) {
  try {
    int retval = Pixie16EndRun(module);
    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieRunUtilities::EndHistogramRun() failed to "
          << "communicate end run operation to module " << module;
      throw CXIAException(msg.str(), "Pixie16EndRun()", retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  bool runEnded = false;
  int nRetries = 0;
  const int maxRetries = 10;
  while ((runEnded == false) && (nRetries < maxRetries)) {
    int retval; // Run ended iff Pixie16CheckRunStatus() returns 0.
    try {
      retval = Pixie16CheckRunStatus(module);
      if (retval < 0) {
        std::stringstream msg;
        msg << "CPixieRunUtilities::EndHistogramRun() failed to get"
            << " current run status in module " << module;
        throw CXIAException(msg.str(), "Pixie16CheckRunStatus()", retval);
      }
    } catch (const CXIAException &e) {
      m_lastErrorMessage = e.ReasonText();
      std::cerr << m_lastErrorMessage << std::endl;
      return e.ReasonCode();
    }
    runEnded = (retval == 0); // True if run ended.
    nRetries++;
    // Wait before checking again:
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (nRetries >= maxRetries) {
    std::cout << "CPixieRunUtilities::EndHistogramRun() failed to"
              << " end run in module " << module << std::endl;
  } else if (runEnded) {
    std::cout << "Ended histogram run in Mod. " << module << std::endl;
    m_runActive = false;
  }

  return 0;
}

/**
 * @details
 * Histogram data comes either from the module itself if running in online
 * mode or from the data generator.
 * @note The data generator is primarily used to debug plot elements, curve
 * fitting and display options. It will ignore any histogram settings (EMin,
 * BinFactor) from the DSP.
 */
int CPixieRunUtilities::ReadHistogram(int module, int channel) {
  // Allocate data structure for histogram and grab it or use the generator:
  try {
    int retval;
    if (m_useGenerator) {
      retval = m_pGenerator->GetHistogramData(m_genHistograms[channel].data(),
                                              m_histogramLength);
      m_histogram = m_genHistograms[channel];
    } else {
      retval = Pixie16ReadHistogramFromModule(
          m_histogram.data(), m_histogramLength, module, channel);
    }

    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieRunUtilities::ReadHistogram() failed to read histogram "
             "from module "
          << module << " channel " << channel;
      throw CXIAException(msg.str(), "Pixie16ReadHistogramFromModule()",
                          retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * Baseline acquisition is not a "run" in the same sense that histogram runs
 * or list mode data taking is a "run" to the API (no begin/end functions,
 * no run status change). However, in order for a user to accumulate enough
 * baseline statistics to make judgements about e.g. manually setting
 * baseline cuts, it needs to be treated as such in our manager. The active
 * run flag is set to true when taking a baseline "run."
 *
 * The baseline data itself is stored internally as a histogram of values in
 * [0, histLength). This data structure is reset on begin.
 */
int CPixieRunUtilities::BeginBaselineRun(int module, int nChannels) {
  std::cout << "Beginning baseline run in Mod. " << module << std::endl;

  SetHistogramLength(module);
  SetMaxBaselines(module);

  // Reset internal histogram data. We assume all channels on the module have
  // the same maximum baseline length.
  m_baselineHistograms.assign(nChannels,
                              std::vector<unsigned int>(m_histogramLength, 0));
  if (m_baseline.size() != m_histogramLength) {
    m_baseline.resize(m_histogramLength);
  }
  std::fill(m_baseline.begin(), m_baseline.end(), 0);

  m_runActive = true;

  return 0;
}

/**
 * @details
 * Really all we need to do here is set the active run flag to false.
 */
int CPixieRunUtilities::EndBaselineRun(int module) {
  m_runActive = false;
  std::cout << "Ended baseline run in Mod. " << module << std::endl;
  return 0;
}

/**
 * @details
 * Acquire baseline values for all channels on a module using
 * Pixie16AcquireBaselines() and update the internal storage for baseline data.
 * The single channel baseline data we want, specified by the input channel
 * parameter, is copied into a local variable which is accessible via a getter
 * function.
 *
 * @todo (ASC 7/14/23): Why not just have the getter take a channel as an
 * input parameter and return the correct baseline data. It seems unnecessary
 * to maintain a separate copy.
 */
int CPixieRunUtilities::ReadBaseline(int module, int channel) {
  try {
    // Fill internal DSP memory prior to trace read:
    int retval = Pixie16AcquireBaselines(module);

    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieRunUtilities::ReadBaseline() failed to"
          << " allocate memory for baselines in module " << module;
      throw CXIAException(msg.str(), "Pixie16AcquireBaselines()", retval);
    }

    // Baseline data is an array of baseline values, not a histogram.
    // To treat this like a run, make cumulative histogram of read values:

    UpdateBaselineHistograms(module);

    // Copy the single channel baseline data to m_baseline for getter access:
    std::copy(m_baselineHistograms[channel].begin(),
              m_baselineHistograms[channel].end(), m_baseline.begin());
  } catch (const CXIAException &e) {
    // Reset baseline data on failure:
    std::fill(m_baseline.begin(), m_baseline.end(), 0);
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * XIA API major verson > 3 does not guarantee a fixed-size statistics block. We
 * must use the API function `Pixie16GetStatisticsSize()` to determine the stats
 * block size and dynamically allocate memory for the statistics prior to
 * reading them.
 */
int CPixieRunUtilities::ReadModuleStats(int module) {
  try {
    // Where to read the statistics into, size may depend on revision.
    // Fetch the proper stats block size:
    std::vector<unsigned int> statistics(Pixie16GetStatisticsSize(), 0);

    int retval = Pixie16ReadStatisticsFromModule(statistics.data(), module);
    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieRunUtilities::ReadModuleStats() error accessing scaler "
             "statistics from module "
          << module;
      throw CXIAException(msg.str(), "Pixie16ReadStatisticsFromModule()",
                          retval);
    } else {
      module_config cfg;
      retval = PixieGetModuleInfo(module, &cfg);
      if (retval < 0) {
        std::stringstream msg;
        msg << "CPixieRunUtilities::ReadModuleStats() failed to read "
               "configuration info from module "
            << module;
        throw CXIAException(msg.str(), "PixieGetModuleInfo", retval);
      }
      double realTime = Pixie16ComputeRealTime(statistics.data(), module);
      for (int i = 0; i < cfg.number_of_channels; i++) {
        double inpRate =
            Pixie16ComputeInputCountRate(statistics.data(), module, i);
        double outRate =
            Pixie16ComputeOutputCountRate(statistics.data(), module, i);
        double liveTime = Pixie16ComputeLiveTime(statistics.data(), module, i);
        std::cout << "Module " << module << " channel " << i << " input "
                  << inpRate << " output " << outRate << " livetime "
                  << liveTime << " runtime " << realTime << std::endl;
      }
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * It is assumed that all channels on a module have the same histogram length.
 * Since this function cannot be called until after the system is booted, there
 * is no need to check for that condition here. The caller assumes responsibilty
 * for making sure the module number is valid.
 */
int CPixieRunUtilities::GetHistogramLength(int module) {
  unsigned int histLength = 0;
  try {
    int retval = PixieGetHistogramLength(module, 0, &histLength);
    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieRunUtilities::GetHistogramLength() failed to get histogram "
             "length for module "
          << module;
      throw CXIAException(msg.str(), "PixieGetHistogramLength()", retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return static_cast<int>(histLength);
}

/**
 * @details
 * It is assumed that all channels on a module have the same max baseline size.
 * Since this function cannot be called until after the system is booted, there
 * is no need to check for that condition here. The caller assumes responsibilty
 * for making sure the module number is valid.
 */
int CPixieRunUtilities::GetMaxBaselines(int module) {
  unsigned int maxBaselines = 0;
  try {
    int retval = PixieGetMaxNumBaselines(module, 0, &maxBaselines);
    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieRunUtilities::GetMaxBaselines() failed to get max "
             "baselines for module "
          << module;
      throw CXIAException(msg.str(), "PixieGetMaxNumBaselines()", retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return static_cast<int>(maxBaselines);
}

///
// Private functions
//

/**
 * @details
 * Update baseline histograms using data read from the module or the data
 * generator. Note that the internal histogram maintained by this class is the
 * max allowed histogram length for the module type, [0, nbins), 1 ADC unit/bin.
 * Values outside this range are dropped and not dispayed. This may result in
 * partial or no data being displayed for a baseline run depending on how the
 * baseline looks.
 * @todo (ASC 7/14/23): Handle out of range values better, at least warning
 * the user that something has been dropped.
 */
void CPixieRunUtilities::UpdateBaselineHistograms(int module) {
  module_config cfg;
  int retval = PixieGetModuleInfo(module, &cfg);
  if (retval < 0) {
    std::stringstream msg;
    msg << "CPixieRunUtilities::UpdateBaselineHistograms() failed to read "
           "configuration info from module "
        << module;
    throw CXIAException(msg.str(), "PixieGetModuleInfo", retval);
  }

  for (int i = 0; i < cfg.number_of_channels; i++) {
    std::vector<double> baselines(m_maxBaselines, 0);
    std::vector<double> timestamps(m_maxBaselines, 0);
    // Allocate data structure for baselines and grab them or use the
    // data generator to get data for testing:
    if (m_useGenerator) {
      retval = m_pGenerator->GetBaselineData(baselines.data(), m_maxBaselines);
    } else {
      retval = Pixie16ReadSglChanBaselines(baselines.data(), timestamps.data(),
                                           m_maxBaselines, module, i);
    }
    // Only the API call can fail, generator always returns 0:
    if (retval < 0) {
      std::stringstream msg;
      msg << "CPixieRunUtilities::UpdateBaselineHistograms() failed"
          << " to read baseline from module " << module << " channel " << i;
      throw CXIAException(msg.str(), "Pixie16ReadSglChanBaselines", retval);
    }

    // If we have the baseline, update its histogram for valid values:
    for (const auto &ele : baselines) {
      int bin = static_cast<int>(ele);
      if (bin >= 0 && bin < m_histogramLength) {
        m_baselineHistograms[i][bin]++;
      }
    }
  }
}
