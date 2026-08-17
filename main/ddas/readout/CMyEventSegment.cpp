/**
 * @file CMyEventSegment.cpp
 * @brief Implementation of the event segment class for DDAS systems.
 */

#include "CMyEventSegment.h"

#include <unistd.h>

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include <CExperiment.h>
#include <CReadoutMain.h>
#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

#include "CMyTrigger.h"
#include "HardwareRegistry.h"

using namespace DAQ::DDAS;

// These _happen_ to be the same for now, but they don't have to be.
// CSRA external clock bit comes from the Pixie channel parameters
// while the shift is defined by the first unused bit in the module
// data word that we write:

const uint32_t CCSRA_EXTTSENA_MASK(1 << 21);      //!< CSRA external clock bit.
const uint32_t MODREVBITMSPS_EXTCLK_BIT(1 << 21); //!< Shift in the clock word.

/**
 * @details
 * Initialize the system, load the configuration and expected event lengths
 * from the cfgPixie16.txt and modevtlen.txt files, boot the system and
 * initialize the trigger.
 *
 * In FRIBDAQ 12.0+, the external clock readout is merged into the standard
 * readout framework. The constructor determines whether or not the external
 * clock is enabled for each module the by checking the value of the
 * corresponding bit in the Pixie CSRA register in that module's channel 0.
 *
 * Failure to properly construct an event segment occurs if:
 * - The CSRA register cannot be read from channel 0 on any of the modules.
 * - An custom external timestamp clock calibration is <= 0.
 * - There are a mix of external and internal clocks enabled on the same crate.
 *
 * @todo (ASC 1/25/24): The assumption that the external timestamp bit for
 * channel 0 is the same as the rest of the module allows some obviously bad
 * configurations to be accepted. This _may_ be a QtScope issue too: users
 * should be prevented from enabling the external timestamp on a subset of
 * channels if the readout code doesn't support it.
 */
CMyEventSegment::CMyEventSegment(CMyTrigger *trig, CExperiment &exp)
    : m_modClockCal{}, m_config(), m_systemInitialized(false),
      m_firmwareLoadedRecently(false), m_pTrigger(trig), m_pExperiment(&exp),
      m_nCumulativeBytes(0), m_nBytesPerRun(0) {
  std::ios_base::sync_with_stdio(true);

  std::cout << "Trying to initialize Pixie" << std::endl << std::flush;

  const char *fwFile = getenv("FIRMWARE_FILE");
  if (fwFile) {
    m_config =
        *(Configuration::generate(fwFile, "cfgPixie16.txt", "modevtlen.txt"));
  } else {
    m_config =
        *(Configuration::generateManagedFW("cfgPixie16.txt", "modevtlen.txt"));
  }
  m_config.print(std::cout);
  std::cout.flush();

  // Conditionally load firmware and boot modules. The modules are only
  // booted if the env variable DDAS_BOOT_WHEN_REQUESTED is not defined.
  if (getenv("DDAS_BOOT_WHEN_REQUESTED") == nullptr) {
    boot();
  } else {
    boot(SystemBooter::SettingsOnly); // Load parameters only.
  }

  // Create the word to store the revision, bits, MSPS of the module for
  // insertion into the event segment.
  namespace HR = HardwareRegistry;
  std::vector<int> hdwrMap = m_config.getHardwareMap();
  int numInternalClock(0);
  int numExternalClock(0);
  for (unsigned int i = 0; i < m_config.getNumberOfModules(); i++) {
    auto type = hdwrMap.at(i);
    HR::HardwareSpecification specs = HR::getSpecification(type);
    // Bits 0-15: sampling frequency, Bits 16-23: bit depth,
    // Bits 24-31: revision number
    m_modRevBitMSPSWord[i] = (specs.s_hdwrRevision << 24) +
                             (specs.s_adcResolution << 16) +
                             specs.s_adcFrequency;
    m_modClockCal[i] = specs.s_clockCalibration;

    // Fold in the external clock - in our implemenation, all channels
    // save the external clock or none do. We'll determine if all do by
    // looking at the CCSRA_EXTSENA bit of channel control register A of
    // channel 0. We assume that resolution is limited to 16 bits max
    // making the resolution field of the m_modRevBitMSPSWord 5 bits wide,
    // leaving us 3 extra bits. So we'll put a 1 in bit 21 if the external
    // clock is used:

    double fCSRA; // Channel params are doubles, even the registers...
    try {
      int rv = Pixie16ReadSglChanPar("CHANNEL_CSRA", &fCSRA, i, 0);
      if (rv < 0) {
        std::string msg("Failed to read channel CSRA in module ");
        msg += i;
        throw CXIAException(msg, "Pixie16ReadSglChanPar", rv);
      }
    } catch (const CXIAException &e) {
      std::cerr << e.ReasonText() << std::endl;
      std::exit(EXIT_FAILURE); // Fatality!
    }

    // External clock mode:

    uint32_t csra = static_cast<uint32_t>(fCSRA); // A proper bitmask.
    if (csra & CCSRA_EXTTSENA_MASK) {
      m_modRevBitMSPSWord[i] |= MODREVBITMSPS_EXTCLK_BIT;
      numExternalClock++;

      // In external clock mode, the default clock scale factory is 1
      // but the DDAS_TSTAMP_SCALE_FACTOR environment variable can
      // override this. Note that our implementation doesn't well
      // support a mix of internal and external timestamps in a crate:

      m_modClockCal[i] = 1.0;
      const char *cal = std::getenv("DDAS_TSTAMP_SCALE_FACTOR");
      if (cal) {
        m_modClockCal[i] = atof(cal);
        if (m_modClockCal[i] <= 0) {
          std::stringstream msg;
          msg << "Invalid value for DDAS_TSTAMP_SCALE_FACTOR: '"
              << m_modClockCal[i] << "'";
          std::cerr << msg.str() << std::endl;
          std::exit(EXIT_FAILURE);
        }
      }

    } else {
      numInternalClock++;
    }

    // We don't really support both internal and external clocks in the
    // same readout at present:

    if ((numInternalClock > 0) && (numExternalClock > 0)) {
      std::cerr << "Some modules are set for internal clock "
                << "while others for external clock\n";
      std::cerr << "This is not a supported configuration!\n";
      std::exit(EXIT_FAILURE);
    }

    std::cout << "Module #" << i << ": module ID word=0x" << std::hex
              << m_modRevBitMSPSWord[i] << std::dec;
    std::cout << ", clock calibration = " << m_modClockCal[i] << std::endl;
    if (m_modRevBitMSPSWord[i] & MODREVBITMSPS_EXTCLK_BIT) {
      std::cout << "External clock timestamping will be used with "
                << " clock multiplier of " << m_modClockCal[i] << std::endl;
    }
  }

  m_pTrigger->Initialize(m_config.getNumberOfModules());
}

/**
 * @details
 * Initialize unless there is an INFINITY_CLOCK environment variable with the
 * value "YES" or the firmware has been loaded recently (no system exit).
 * @note This is not threadsafe as C++ does not require getenv to be
 * thread-safe.
 * @todo (ASC 1/25/24): An old comment from (I bet) RF: "paging through
 * the global **environ is probably thread-safe however I'm pretty sure at
 * this point in time there's no other thread doing a getenv()."
 */
void CMyEventSegment::initialize() {
  const char *envVal = std::getenv("INFINITY_CLOCK");
  if (!envVal || m_firmwareLoadedRecently) {
    synchronize();
    m_firmwareLoadedRecently = false;
  }
}

/**
 * @details
 * Pixie has triggered. There are greater than EXTFIFO_READ_THRESH words in
 * the output FIFO of a particular Pixie module. Read out all modules.
 *
 * This loop finds the first module that has at least one event in it since
 * the trigger fired. We read the minimum of all complete events and the number
 * of complete events that fit in that buffer (note that ) each buffer will
 * also contain the module type word. Note as well that modules count words
 * in uint32_t's but maxBytes is in uint16_t's.
 */
size_t CMyEventSegment::read(void *rBuffer, size_t maxBytes) {
  // memset(rBuffer, 0, maxBytes);// See what's been read (debugging).

  size_t maxLongs = maxBytes / sizeof(uint32_t); // Longs in the buffer.
  maxLongs = maxLongs - 128; // To be really sure we don't fill it.

  unsigned int *words = m_pTrigger->getWordsInModules();
  for (int i = 0; i < m_config.getNumberOfModules(); i++) {
    if (words[i] >= m_config.getModuleEventLengths().at(i)) {
      // Figure out if we fill the buffer or just take the complete
      // events from the module:
      uint32_t *p = static_cast<uint32_t *>(rBuffer);
      *p++ = m_modRevBitMSPSWord[i];
      maxLongs--; // Count that word.
      double *pd = reinterpret_cast<double *>(p);
      *pd++ = m_modClockCal[i];                        // Clock multiplier.
      maxLongs -= (sizeof(double) / sizeof(uint32_t)); // Count it.
      p = reinterpret_cast<uint32_t *>(pd);

      int readSize = words[i];
      if (readSize > maxLongs) {
        readSize = maxLongs;
      }

      // Read only complete events. Truncate readSize to the nearest
      // integer multiple of the module event length:

      readSize -= (readSize % m_config.getModuleEventLengths().at(i));

      // Read the data right into the ring item:

      auto prewords = words[i];
      unsigned int preread;
      try {
        int rv = Pixie16CheckExternalFIFOStatus(&preread, i);
        if (rv < 0) {
          std::string msg("Failed to read Pixie FIFO status!");
          throw CXIAException(msg, "Pixie16CheckExternalFIFOStatus", rv);
        }
      } catch (const CXIAException &e) {
        std::cerr << e.ReasonText() << std::endl;
      }

      try {
        int rv = Pixie16ReadDataFromExternalFIFO(
            reinterpret_cast<unsigned int *>(p), (unsigned int)readSize,
            (unsigned short)i);
        if (rv < 0) {
          std::string msg("Read failed from module ");
          msg += i;
          throw CXIAException(msg, "Pixie16ReadDataFromExternalFIFO", rv);
        }
      } catch (CXIAException &e) {
        std::cerr << e.ReasonText() << std::endl;
        m_pExperiment->haveMore();
        reject();
        return 0;
      }

      // Until we fall through the loop, count down the remaining words.
      m_pExperiment->haveMore();
      words[i] -= readSize;

      // Update stats. Add sizeof(double) and + 1 uint32_t for clock
      // multiplier and module ID words, respectively.
      size_t nBytes = sizeof(double) + (readSize + 1) * sizeof(uint32_t);
      m_nCumulativeBytes += nBytes;
      m_nBytesPerRun += nBytes;

      return nBytes / sizeof(uint16_t);
    }
  }

  // If we got here nobody had enough data left since the last trigger:

  m_pTrigger->Reset();
  reject();

  return 0;
}

void CMyEventSegment::disable() {
  // Nothing to disable right now
}

void CMyEventSegment::clear() {
  // Nothing to clear right now
}

/**
 * @details
 * Begin the list mode run with NEW_RUN (= 1) run mode. If the start fails,
 * display the return value of Pixie16StartListModeRun() and the error
 * code text.
 */
void CMyEventSegment::onBegin() {
  int rv = Pixie16StartListModeRun(m_config.getNumberOfModules(), LIST_MODE_RUN,
                                   NEW_RUN);
  try {
    if (rv < 0) {
      std::string msg("*ERROR* Failed to begin list mode run");
      throw CXIAException(msg, "Pixie16StartListModeRun", rv);
    } else {
      std::cout << "List mode run started OK " << rv << " mode " << std::hex
                << std::showbase << LIST_MODE_RUN << std::dec << " " << NEW_RUN
                << std::endl
                << std::flush;
    }
  } catch (const CXIAException &e) {
    std::cerr << e.ReasonText() << std::endl;
  }
  m_nBytesPerRun = 0; // New run presumably.
  usleep(100000);     // Delay for the DSP boot.
}

/**
 * @details
 * Resume the list mode run with RESUME_RUN (= 0) run mode. If the resume
 * fails, display the return value of Pixie16StartListModeRun and the error
 * code text.
 */
void CMyEventSegment::onResume() {
  int rv = Pixie16StartListModeRun(m_config.getNumberOfModules(), LIST_MODE_RUN,
                                   RESUME_RUN);
  try {
    if (rv < 0) {
      std::string msg("*ERROR* Failed to resume list mode run");
      throw CXIAException(msg, "Pixie16StartListModeRun", rv);

    } else {
      std::cout << "List mode run resumed OK " << rv << " mode " << std::hex
                << std::showbase << LIST_MODE_RUN << std::dec << " "
                << RESUME_RUN << std::endl
                << std::flush;
    }
  } catch (const CXIAException &e) {
    std::cerr << e.ReasonText() << std::endl;
  }
}

/**
 * @details
 * More or less straight from the XIA Pixie SDK docs: configure the system to
 * run synchronously through the backplane by setting Pixie module parameters.
 * Synchronous running means that the last module ready to take data starts
 * the run in all modules and the first module to end the run stops the run
 * in all modules (SYNCH_WAIT = 1). In synchronous mode, all run timers are
 * cleared at the start of a new run (IN_SYNCH = 0). Once the run has started,
 * IN_SYNCH is automatically set to 1.
 *
 * Removed from initialize() so that this can be called via a command.
 * See the CSyncCommand class for details.
 */
void CMyEventSegment::synchronize() {
  // Since in synch, we only have to set for the first module:

  int rv = Pixie16WriteSglModPar("SYNCH_WAIT", 1, 0);
  if (rv < 0) {
    throw CXIAException("Synch wait problem", "Pixie16WriteSglModPar", rv);
  } else {
    std::cout << "Synch Wait OK " << rv << std::endl;
  }

  rv = Pixie16WriteSglModPar("IN_SYNCH", 0, 0);
  if (rv < 0) {
    throw CXIAException("In-synch problem", "Pixie16WriteSglModPar", rv);
  } else {
    std::cout << "In Synch OK " << rv << std::endl;
  }
}

void CMyEventSegment::boot(SystemBooter::BootType type) {
  if (m_systemInitialized) {
    int rv = Pixie16ExitSystem(m_config.getNumberOfModules());
    if (rv < 0) {
      std::string msg("CMyEventSegment::boot() failed to exit system");
      throw CXIAException(msg, "Pixie16ExitSystem", rv);
    } // This is handled (or not) much higher up the stack...
    m_systemInitialized = false;
  }

  // DDAS exceptions when talking to modules, std::exception for other
  // stuff like errors in the hardware map:
  try {
    SystemBooter booter;
    booter.boot(m_config, type);

    m_systemInitialized = true;
    // Keep track of whether we loaded firmware... if we did, then we need
    // to sync next time around:
    m_firmwareLoadedRecently = (type == SystemBooter::FullBoot);
  } catch (const CXIAException &e) {
    m_systemInitialized = false;
    std::cerr << e.ReasonText() << std::endl;
  } catch (std::exception &e) {
    m_systemInitialized = false;
    std::cerr << e.what() << std::endl;
  }
}
