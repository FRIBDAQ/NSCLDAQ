/**
 * @file CMyTrigger.cpp
 * @brief Implement the DDAS trigger.
 */

#include "CMyTrigger.h"

#include <cstdlib>
#include <iostream>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

const int TRIGGER_TIMEOUT_SECS = 5; //!< Auto-trigger timeout in seconds.

/**
 * @details
 * If FIFO_THRESHOLD is defined and is a positive integer, it replaces the
 * default value of m_fifoThreshold. The FIFO threshold is the number of
 * 32-bit words that must be in the FIFO for the trigger to fire.
 */
CMyTrigger::CMyTrigger()
    : m_retrigger(false), m_fifoThreshold(EXTFIFO_READ_THRESH * 10),
      m_wordsInEachModule(nullptr) {
  const char *pThreshold = getenv("FIFO_THRESHOLD");
  if (pThreshold) {
    char *pEnd;
    unsigned long newThreshold = strtoul(pThreshold, &pEnd, 0);
    if (newThreshold && (pEnd != pThreshold)) {
      m_fifoThreshold = newThreshold;
    }
  }
  std::cerr << "Using a FIFO threshold of " << m_fifoThreshold << " words\n";
}

/**
 * @details
 * We need to deallocate memory used to store the FIFO words in each module.
 */
CMyTrigger::~CMyTrigger() { delete[] m_wordsInEachModule; }

void CMyTrigger::setup() { m_lastTriggerTime = time(nullptr); }

/**
 * @details
 * DDAS does not need any further signal as data taking end since this
 * function is also called on a pause of data taking on't even think
 * about desyncing modules here.
 */
void CMyTrigger::teardown() {}

/**
 * @details
 * Retrigger: always false.
 */
void CMyTrigger::Reset() { m_retrigger = false; }

/**
 * @details
 * Delete and recreate the FIFO words array based on the number of modules.
 */
void CMyTrigger::Initialize(int nummod) {
  NumberOfModules = nummod;
  m_retrigger = false;
  delete[] m_wordsInEachModule;
  m_wordsInEachModule = new unsigned int[NumberOfModules];
}

/**
 * @details
 * Defines the trigger logic. Trigger a read if the number of words in the
 * external FIFO of any Pixie-16 module in a crate exceeds a defined threshold.
 * - If the module is in the middle of processing a data buffer in the event
 * segment, continue processing the data buffer. Return a true trigger to pass
 * control back the event segment.
 * - If there are no buffers currently being processed in the event segment
 * look at the Pixie hardware to see if data currently needs to be read out.
 * Do so if the FIFO threshold is exceeded.
 * - If the trigger has timed out, trigger anyways.
 */
bool CMyTrigger::operator()() {
  try {
    // Currently processing a data buffer in the event segment.
    // Return a true trigger.
    if (m_retrigger) {
      m_lastTriggerTime = time(nullptr); // Reset the trigger timeout.
      return true;
    } else {
      // Read data from the FIFO on the module:
      int retval;
      bool thresholdMade(false);
      for (int i = 0; i < NumberOfModules; i++) {
        // Check how many words are stored in Pixie's readout FIFO:
        ModNum = i;
        nFIFOWords = 0;
        try {
          retval = Pixie16CheckExternalFIFOStatus(&nFIFOWords, ModNum);
          if (retval < 0) {
            std::string msg("Failed to read external FIFO status for module ");
            msg += ModNum;
            throw CXIAException(msg, "Pixie16CheckExternalFIFOStatus", retval);
            nFIFOWords = 0; // For safety.
          }
        } catch (const CXIAException &e) {
          std::cerr << e.ReasonText() << std::endl;
        }

        // Save the number of words in each module:
        m_wordsInEachModule[i] = nFIFOWords;

        // Trigger a read if the threshold is exceeded:
        if (nFIFOWords > m_fifoThreshold) {
#ifdef PRINTQUEINFO
          std::cout << "CTrigger: trigger satisfied in module " << i
                    << " nWords " << nFIFOWords << " threshold "
                    << m_fifoThreshold << std::endl;
#endif
          m_retrigger = true;
          thresholdMade = true; // Once polling is done, trigger.
        }
      } // End module loop.

      // Good trigger:
      if (thresholdMade) {
        m_lastTriggerTime = time(nullptr); // Record the trigger time.
        return true;                       // Some module was above threshold.
      }
    }

    // If the trigger has timed out, then trigger anyway:
    time_t now = time(nullptr);
    if ((now - m_lastTriggerTime) > TRIGGER_TIMEOUT_SECS) {
      m_lastTriggerTime = now;
      return true;
    }

    return false; // Currently not enough data to trigger
  } catch (...) {
    std::cout << "Unknown exception in trigger!" << std::endl;
  }

  // Poll the timeout again and trigger:
  time_t now = time(nullptr);
  if ((now - m_lastTriggerTime) > TRIGGER_TIMEOUT_SECS) {
    m_lastTriggerTime = now;
    return true;
  }

  return false;
}
