/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

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
 * @file CS800Triggernew.cpp
 * @brief Implementation of the new VME S800 trigger module 
 * @note When compiled for the MVLCGenerator the preprocessor symbol MVLC_GENERATOR is defined.
 */

#include <CS800TriggerNew.h>
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <s800TriggerRegisters.h>

#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
#endif



#include <iostream>

/** construction */

CS800TriggerNew::CS800TriggerNew() :
    m_pAPI(nullptr), m_pConfiguration(nullptr) {}

/** destructor: */

CS800TriggerNew::~CS800TriggerNew() {
    delete m_pAPI;             // Delete any remaining register object.
}

// Canonicals that are not defined for MVLC:
// and clone:
#ifndef MVLC_GENERATOR
CS800TriggerNew::CS800TriggerNew(const CS800TriggerNew& rhs) :
    m_pAPI(0), m_pConfiguration(nullptr)
{
    if (rhs.m_pConfiguration) {
        m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
    }
}
CReadoutHardware* 
CS800TriggerNew::clone() const {
    return new CS800TriggerNew(*this);
}

#endif
/**
 * onAttach
 *   Attach the configuration and register the configuration parameters.
 * Note that the preprocessor symbol DEFAULT_S800REGFILE is the path
 * to where the default S800 register JSON config file is installed.
 * This should only need to be modified in response to registers moving
 * in the module as a result of firmware updates that post-date the
 * installation.
 * 
 * @param configuration - References the configuration object.
 * 
 */
void
#ifdef MVLC_GENERATOR
CS800TriggerNew::onAttach(XXUSB::CConfigurableObject& configuration)
#else
CS800TriggerNew::onAttach(CReadoutModule& configuration)
#endif
{
    // Save the configuration pointer.
    // note that m_pAPI remains null until we Initialize at which point
    // we can resolve the -register-file option.

    m_pConfiguration = &configuration;

    m_pConfiguration->addParameter(
        "-register-file", nullptr, nullptr, std::string(DEFAULT_S800REGFILE)
    );                                     // Register def .json.
    m_pConfiguration->addIntegerParameter("-base", 0x0, 0xffffffff, 0); // Base address.
    m_pConfiguration->addBooleanParameter("-enable-extclear, true");
}
/**
 * Initialize:
 *    Initialize the module.  This is done after the configuration has been processed and before
 * the run actually starts taking data, the run number  has been set.
 * We need to create a register object to locate our stuff inside the module.
 * 
 * @param controller - controller (or for mvlc-generate a memorizor) for initialization operations.
 */
void
CS800TriggerNew::Initialize(CVMUSB& controller) {
    uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
    auto configFile = m_pConfiguration->cget("-register-file");

    delete m_pAPI;
    m_pAPI = nullptr;
    m_pAPI = new S800TriggerRegisters(base, configFile.c_str());   // This could throw...

    std::cout << "S800 trigger module: \n"
        << m_pAPI->describeJSON() << std::endl;

    // reset(?)  the module... hopefully does not clear the run number register.
    
    m_pAPI->swClear(controller);                  

    // Setting the run number here results in some nasty
    // circular build dependencies.  Therefore,
    // we'll do a slow controls module for it.

    // If desired, enable the external clear:

    m_pAPI->enableExternalClear(controller, m_pConfiguration->getBoolParameter("-enable-extclear"));
    
    // Clear the busy:

    m_pAPI->resetBusy(controller);

    // Now data taking can start.
}

/**
 * addReadoutList
 * 
 *    Add the stuff needed to readout the module to the 
 * readoutlist.  In order, we read the timestamp and the trigger mask.
 * 
 * @param list - readout list to modify.
 */
void
CS800TriggerNew::addReadoutList(CVMUSBReadoutList& list) {
    m_pAPI->addReadTimestamp(list);
    m_pAPI->addReadTriggerMask(list);
    
    // Do we need to clear the busy? If so, that has to happen after the last read....
}



// For the MVLC - the device command:

#ifdef MVLC_GENERATE
CS800TriggerNewCommand::CS800TriggerNewCommand(CTCLInterpreter& inerp, TCLConfigParser& parser) :
    DeviceCommand(interp, "s800trigger", parser) {}

CS800TriggerNewCommand::~CS800TriggerNewCommand() {}

ReadoutModule*
CS800TriggerNewCommand::createDevice(std::string  name) {
    auto dev = new CS800TriggerNew;
    auto result = new CReadoutModule;
    result->SetDriver(dev);
    return result;
}
#endif