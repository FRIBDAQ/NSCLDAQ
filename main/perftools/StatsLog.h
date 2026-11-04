/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
        Aaron Chester
        FRIB
        Michigan State University
        East Lansing, MI 48824-1321
*/

/** @file  StatsLog.h
 *  @brief Accumulate per-pass throughput measurements and write a test summary.
 */

#ifndef STATSLOG_H
#define STATSLOG_H

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>

// Accumulates per-pass (bytes, nanoseconds) samples for a perftool and writes a
// summary when a timed test ends: the aggregate rate over the whole test
// (total bytes / total measured time), plus per-pass mean/min/max.
class StatsLog {
private:
  std::string m_name;
  uint64_t m_samples;
  long double m_totalBytes;
  long double m_totalNs;
  long double m_sumRate;
  double m_minRate;
  double m_maxRate;

public:
  StatsLog(const char *name)
      : m_name(name), m_samples(0), m_totalBytes(0.0L), m_totalNs(0.0L),
        m_sumRate(0.0L), m_minRate(0.0), m_maxRate(0.0) {}

  // Record one measurement pass.
  void add(uint64_t bytes, uint64_t ns) {
    double rate = ns ? double(bytes) / ns * 1.0e9 : 0.0;
    if (m_samples == 0) {
      m_minRate = m_maxRate = rate;
    } else {
      if (rate < m_minRate)
        m_minRate = rate;
      if (rate > m_maxRate)
        m_maxRate = rate;
    }
    m_samples++;
    m_totalBytes += bytes;
    m_totalNs += ns;
    m_sumRate += rate;
  }

  uint64_t samples() const { return m_samples; }

  // Append the summary to 'path' (empty => stderr only). Always echoes to
  // stderr.
  void writeSummary(const std::string &path, double wallSeconds) const {
    double agg =
        (m_totalNs > 0) ? double(m_totalBytes / m_totalNs) * 1.0e9 : 0.0;
    double mean = m_samples ? double(m_sumRate / m_samples) : 0.0;

    char when[64];
    time_t now = time(0);
    strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", localtime(&now));

    std::ostringstream os;
    os << "==== " << m_name << " summary " << when << " ====\n";
    os << "  samples          : " << m_samples << "\n";
    os << std::fixed << std::setprecision(3);
    os << "  wall elapsed (s) : " << wallSeconds << "\n";
    os << "  total bytes      : " << uint64_t(m_totalBytes) << "\n";
    os << std::scientific << std::setprecision(3);
    os << "  aggregate        : " << agg << " bytes/sec (" << std::fixed
       << std::setprecision(2) << agg / (1024.0 * 1024.0) << " MB/s)\n";
    os << std::scientific << std::setprecision(3);
    os << "  per-pass mean    : " << mean << " bytes/sec\n";
    os << "  per-pass min/max : " << m_minRate << " / " << m_maxRate
       << " bytes/sec\n";

    std::cerr << os.str();
    if (!path.empty()) {
      std::ofstream f(path.c_str(), std::ios::app);
      if (f)
        f << os.str();
      else
        std::cerr << "** StatsLog: cannot open log file '" << path << "'\n";
    }
  }
};
#endif
