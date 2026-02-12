/**
 * @file CMyEventSegment.h
 * @brief Define a DDAS event segment.
 */

#ifndef CMYEVENTSEGMENT_H
#define CMYEVENTSEGMENT_H

#include <CEventSegment.h>

#include <vector>

#include <Configuration.h>
#include <SystemBooter.h>

/**
 * @todo (ASC 1/8/25): See Github issue #417: simplify readout and pass system
 * information around via the Configuation class.
 */

const int MAX_MODULES_PER_CRATE = 13; //!< A full crate is 13 modules.

class CMyTrigger;
class CExperiment;

/**
 * @class CMyEventSegment
 * @brief Derived class for DDAS event segments.
 * @details
 * The event segment reads out a logical chunk of an experiment. In the DDAS
 * case, data from a single crate (single source ID). An experiment may
 * consist of multiple crates arranged in a CCompoundEventSegment container.
 */

class CMyEventSegment : public CEventSegment {
private:
  size_t m_nModules;             //!< Number of modules in the crate.
  std::vector<int> m_modEvtLens; //!< Expected event lengths (32-bit words).
  /** Word to store rev, bit depth, and MSPS of module for insertion into
   * the data stream.*/
  unsigned int m_modRevBitMSPSWord[MAX_MODULES_PER_CRATE];
  /** Calibration constants for each module in the crate in nanoseconds per
   * clock tick. */
  double m_modClockCal[MAX_MODULES_PER_CRATE];
  DAQ::DDAS::Configuration m_config; //!< Configuration data for the segment.
  bool m_systemInitialized;          //!< Are modules are booted and online?
  bool m_firmwareLoadedRecently;     //!< True when loading FW on full boot.
  CMyTrigger *m_pTrigger;            //!< Trigger definition.
  CExperiment *m_pExperiment;        //!< Experiment we're reading data from.

  // Statistics:

  size_t m_nCumulativeBytes; //!< Cumulative bytes of data read.
  size_t m_nBytesPerRun;     //!< Bytes of data read this run.

public:
  /**
   * @brief Construct from trigger object and experiment
   * @param trig Pointer to the DDAS trigger.
   * @param exp Reference to the experiment the event segment comes from.
   */
  CMyEventSegment(CMyTrigger *trig, CExperiment &exp);
  /**
   * @brief Default constructor.
   * @note For unit testing purposes only!
   */
  CMyEventSegment(); // For unit testing only!!
  /** @brief Destructor. */
  ~CMyEventSegment();

  /** @brief Initialize the modules recording data in this segment. */
  virtual void initialize();
  /**
   * @brief Read data from the modules following a valid trigger.
   * @param[in,out] rBuffer  Read data into this buffer.
   * @param[in]     maxBytes Max bytes of data we can stuff in the buffer.
   * @return Number of 16-bit words in the ring item body.
   */
  virtual size_t read(void *rBuffer, size_t maxwords);
  /** @brief Nothing to disable. */
  virtual void disable();
  /** @brief Nothing to clear. */
  virtual void clear();

  /** @brief Manage run start operation. */
  virtual void onBegin();
  /** @brief Manage run resume operation. */
  virtual void onResume();
  /** @brief Just return. Sorting is offloaded into its own process. */
  virtual void onEnd(CExperiment *pExperiment) { return; };

  /**
   * @brief Get the number of modules in the crate.
   * @return Number of modules.
   */
  size_t getNumberOfModules() { return m_nModules; }
  /**
   * @brief Get the channel count in a given module.
   * @param mod The module index.
   * @return The channel count of that module.
   */
  unsigned short getChannelCount(unsigned int mod) {
    return m_config.getChannelCount(mod);
  }
  /**
   * @brief Get the crate ID value from the configuration.
   * @return The crate ID.
   */
  int getCrateID() const { return m_config.getCrateId(); }

  /**
   * @brief Perform clock synchronization.
   * @throw CDDASException If we fail to talk properly to the module while
   *   setting the clock synchronization parameters.
   */
  void synchronize();

  /**
   * @brief Load firmware and boot the modules.
   * @param type The boot type (boot mask) passed to the system booter
   *   (default = SystemBooter::FullBoot).
   * @throw CDDASException If the system is initialized and fails to exit
   *   before attempting to boot again.
   */
  void boot(
      DAQ::DDAS::SystemBooter::BootType = DAQ::DDAS::SystemBooter::FullBoot);

  /**
   * @brief Get the cumulative and current run statistics.
   * @return Cumulative and current run stats as a std::pair.
   */
  std::pair<size_t, size_t> getStatistics() {
    return std::pair<size_t, size_t>(m_nCumulativeBytes, m_nBytesPerRun);
  }
};

#endif
