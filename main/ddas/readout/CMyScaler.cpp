/**
 * @file CMyScaler.cpp
 * @brief Implement the DDAS scaler class.
 */

#include "CMyScaler.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

CMyScaler::CMyScaler(unsigned short crate, unsigned short mod,
                     unsigned short nchan)
    : m_crate(crate), m_module(mod), m_nChannels(nchan), m_prevIC(nchan, 0),
      m_prevOC(nchan, 0) {
  clearCounters(m_statistics.s_cumulative);
  clearCounters(m_statistics.s_perRun);

  std::cout << "Scalers know crate ID = " << m_crate << std::endl;
}

CMyScaler::~CMyScaler() {}

/**
 * @details
 * Called by CExperiment::Start() prior to starting a new run.
 */
void CMyScaler::initialize() {
  std::fill(m_prevIC.begin(), m_prevIC.end(), 0);
  std::fill(m_prevOC.begin(), m_prevOC.end(), 0);

  clearCounters(m_statistics.s_perRun);
}

/**
 * @details
 * Now we need to calculate the # of events from the last read of the scalers.
 * NSCL scaler buffers just expect the # events since the last read. However,
 * Pixie-16 statistics cannot be cleared, so we need to do some math and store
 * the counts from our previous read.
 *
 * Input counts (IC) and rate (ICR) are fast triggers. Output counts (OC) and
 * rate (OCR) are accepted triggers.
 */
std::vector<uint32_t> CMyScaler::read() {
  try {
    std::vector<unsigned int> statistics(Pixie16GetStatisticsSize(), 0);
    int retval = Pixie16ReadStatisticsFromModule(statistics.data(), m_module);

    if (retval < 0) {
      std::string msg("Error accessing scalar statistics from module ");
      msg += m_module;
      throw CXIAException(msg, "Pixie16ReadStatisticsFromModule", retval);
    }

    // 2n+1: n for input, n for output, 1 for crateid
    double inputCounts[m_nChannels] = {0};
    double outputCounts[m_nChannels] = {0};
    unsigned long scalerData[2 * m_nChannels + 1] = {0};
    scalerData[0] = (unsigned long)m_crate;

    for (int i = 0; i < m_nChannels; i++) {
      // Raw input counts (number of fast triggers seen by FPGA):
      inputCounts[i] =
          Pixie16ComputeRawInputCount(statistics.data(), m_module, i);

      // Raw output counts (validated events handled by DSP,
      // "live" counts):
      outputCounts[i] =
          Pixie16ComputeRawOutputCount(statistics.data(), m_module, i);

      // Finally compute the events since the last scaler read:
      scalerData[(2 * i + 1)] = (unsigned long)(inputCounts[i] - m_prevIC[i]);
      scalerData[(2 * i + 2)] = (unsigned long)(outputCounts[i] - m_prevOC[i]);

      // Reset counters since last read:
      m_prevIC[i] = inputCounts[i];
      m_prevOC[i] = outputCounts[i];
    }

    // Copy scaler information into the output vector
    m_scalers.clear();
    m_scalers.insert(m_scalers.end(), scalerData,
                     scalerData + (2 * m_nChannels + 1));

    // Figure out the statistics by summing over the scalerData (it's
    // incremental so we can just add it all in). Channel data come in
    // pairs starting at 1:

    int idx = 1;
    for (int i = 0; i < m_nChannels; i++) {
      m_statistics.s_cumulative.s_nTriggers += scalerData[idx];
      m_statistics.s_perRun.s_nTriggers += scalerData[idx];

      m_statistics.s_cumulative.s_nAcceptedTriggers += scalerData[idx + 1];
      m_statistics.s_perRun.s_nAcceptedTriggers += scalerData[idx + 1];

      idx += 2;
    }

    return m_scalers;

  } catch (const CXIAException &e) {
    std::cerr << e.ReasonText() << std::endl;
    return m_scalers;
  } catch (...) {
    std::cerr << "Unexpected exception encountered in CMyScaler::read!\n";
    return m_scalers;
  }
}

void CMyScaler::clearCounters(Counters &c) { memset(&c, 0, sizeof(Counters)); }
