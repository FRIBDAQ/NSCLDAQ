/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file CS800TriggerScalers.cpp
 * @brief Implementation for the new VME S800 trigger module scaler readout.
 * @note When compiled for the MVLCGenerator the preprocessor symbol MVLC_GENERATOR is defined.
 * @note The file depends on the $DAQBIN/s800scaler_gen file to prepare a list of what to read.
 */
#include <CS800TriggerScalers.h>
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <fstream>
#include <sstream>
#include <string>


#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
#endif


static const int DEFAULT_BLOCK_READ_THRESHOLD(8);  // Default for -block-read-threshold
 /**
  * constructor
  * 
  */
 CS800TriggerScalers::CS800TriggerScalers() :
    m_pConfiguration(nullptr) {}


/**
 * Destructor:
 */
CS800TriggerScalers::~CS800TriggerScalers() {}


// Canonicals only available to VMUSBReadout:

#ifndef MVLC_GENERATOR
// Copy construction

CS800TriggerScalers::CS800TriggerScalers(const CS800TriggerScalers& rhs) : m_pConfiguration(nullptr)
{
    if (rhs.m_pConfiguration) {
        m_pConfiguration = new CReadoutModule(*rhs.m_pConfiguration);
    }
}
// Assignment:

CS800TriggerScalers&
CS800TriggerScalers::operator=(const CS800TriggerScalers& rhs) {
    if ((&rhs != this)) {
        m_pConfiguration = nullptr;
        if (rhs.m_pConfiguration) {
            m_pConfiguration = new CReadoutModule(*rhs.m_pConfiguration);
        }
    }
    return *this;
}
#endif

/**
 *  onAttach - provide a configuration object to us and set it up.
 * 
 * @param configuration - references the configuration.
 */
void
CS800TriggerScalers::onAttach(
#ifdef MVLC_GENERATOR
    XXUSB::CConfigurableObject& 
#else
    CReadoutModule&
#endif
                            configuration
) {
    m_pConfiguration = &configuration;
    configuration.addIntegerParameter("-base");
    configuration.addParameter("-file", nullptr, nullptr, "no-file-set");
    configuration.addIntegerParameter("-block_read-threshold", 1, 128, DEFAULT_BLOCK_READ_THRESHOLD);
}

 /**
  * Initialize
  *    - initialize the module.  We just ensuer the file can be opened.
  * @param controller - references the VMUSB controller, which we ignore.
  */ 
  void
  CS800TriggerScalers::Initialize(CVMUSB& controller) {
    std::ifstream test(m_pConfiguration->cget("-file"));
    if (!test) {
        throwNoFile();
    }


  }
  /**
   *  addReadoutList
   *    Creates the readout list.  We read the -file and add individual reads to the stack
   * unless the count is >= the -block-read-threshold in which case we do a block read.
   * 
   * @param list - refers to the list that we will be modifying.
   */
  void
  CS800TriggerScalers::addReadoutList(CVMUSBReadoutList& list) {
    std::string file_name = m_pConfiguration->cget("-file");
    unsigned minblock = m_pConfiguration->getIntegerParameter("-block-read-threshold");
    uint32_t base_address = m_pConfiguration->getIntegerParameter("-base");
    std::ifstream file(file_name);
    if (!file) {
        throwNoFile();
    }

    // process the file:

    while (file) {
        uint32_t base;
        unsigned count;
        file >> base >> count;

        if (count < minblock) {
            for (int i =0; i < count; i++) {
                list.addRead32(base_address + base, CVMUSBReadoutList::a32UserData);
                base += sizeof(uint32_t);
            }
        } else {
            list.addBlockRead32(base_address + base, CVMUSBReadoutList::a32UserData, count);
        }
    }
  }

  #ifndef MVLC_GENERATOR
  CReadoutHardware* 
  CS800TriggerScalers::clone() const {
    return new CS800TriggerScalers(*this);
  }
  #endif


  // Utility to throw an exception if the file could not be opened:

  void
  CS800TriggerScalers::throwNoFile() {
    std::stringstream smsg;
    smsg << " Unable to open the s800 scaler configuration file: " << m_pConfiguration->cget("-file");
    std::string msg(smsg.str());
    throw msg;
  }

  #ifdef MVLC_GENERATOR
  /**
   *  The mvlc generator command class. 
   */
CS800TriggerScalerCommand::CS800TriggerScalerCommand(CTCLInterpreter& interp, TCLConfigParser& parser) :
  DeviceCommand(interp, "s800scaler", parser) {}

CS800TriggerScalerCommand::~CS800TriggerScalerCommand() {}


// Create the defvice.

CReadoutModule*
CS800TriggerScalerCommand::createDevice(std::string name) {
    auto driver = new CS800TriggerScalers;
    auto module = new CReadoutModule;

    module->SetDriver(driver);
    return module;
}

   #endif