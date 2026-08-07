/**
 * @file CDataGenerator.cpp
 * @brief Implementation of the offline data generation class.
 */

#include "CDataGenerator.h"

#include <cmath>

/**
 * @details
 * Create all distributions that are independent of the data size.
 */
CDataGenerator::CDataGenerator()
    : m_engine(std::random_device{}()), m_C(1000, 2000), m_A(100, 10000),
      m_rise(0.5, 0.05), m_decay(5.0, 0.05), m_baseline(4500, 5500),
      m_noise(0, 10) {}

/**
 * @details
 * Params are a pointer to the start of the data storage and a size, as is
 * done in the XIA API for easier integration/consistency.
 */
int CDataGenerator::GetTraceData(unsigned short *data, int dataSize,
                                 double binWidth) {
  // Depends on dataSize, must be local:
  std::uniform_real_distribution<double> dt0(0.05 * dataSize, 0.95 * dataSize);

  double C = m_C(m_engine);         // ADC units.
  double A = m_A(m_engine);         // ADC units.
  double t0 = dt0(m_engine);        // Sample number.
  double rise = m_rise(m_engine);   // Microseconds.
  double decay = m_decay(m_engine); // Microseconds.

  for (int i = 0; i < dataSize; i++) {
    data[i] = SinglePulse(C, A, t0, rise, decay, i, binWidth);
  }

  return 0;
}

/**
 * @details
 * Params are a pointer to the start of the data storage and a size, as
 * is done in the XIA API for easier integration/consistency. Data is
 * stored as a histogram, default binning 1 ADC unit per bin.
 */
int CDataGenerator::GetHistogramData(unsigned int *data, int dataSize) {
  // Depends on dataSize, must be local:
  std::normal_distribution<double> dgaus(dataSize / 4, 10); // Mean, stddev.
  int ene = 0;                                              // Event energy.

  for (int i = 0; i < 10000; i++) {
    ene = static_cast<int>(dgaus(m_engine));
    if (ene >= 0 && ene < dataSize) {
      data[ene]++;
    }
  }

  return 0;
}

/**
 * @details
 * Params are a pointer to the start of the data storage and a size, as is
 * done in the XIA API for easier integration/consistency.
 */
int CDataGenerator::GetBaselineData(double *data, int dataSize) {
  for (int i = 0; i < dataSize; i++) {
    data[i] = m_baseline(m_engine);
  }

  return 0;
}

unsigned short CDataGenerator::SinglePulse(double C, double A, double t0,
                                           double rise, double decay,
                                           int sample, double binWidth) {
  // Convert position to dt in us using the binWidth determined by the XDT
  // channel parameter value:

  double dt = (sample - t0) * binWidth;
  unsigned short pval = 0; // Pulse value for current sample

  if (sample < t0) {
    pval = static_cast<unsigned short>(C + m_noise(m_engine));
  } else {
    pval = static_cast<unsigned short>(
        C + A * (1 - std::exp(-dt / rise)) * std::exp(-dt / decay) +
        m_noise(m_engine));
  }

  return pval;
}
