/*
*-------------------------------------------------------------
 
 CAEN SpA 
 Via Vetraia, 11 - 55049 - Viareggio ITALY
 +390594388398 - www.caen.it

------------------------------------------------------------

**************************************************************************
* @note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
*
* @file     TclConfiguredReadout.cpp
* @brief    Implement the TclConfiguredReadout class.
* @author   Ron Fox
*
*/
#include "TclConfiguredReadout.h"
#include "DynamicMultiTrigger.h"
#include <VX2750EventSegment.h>
#include <VX2750MultiTrigger.h>
#include <CAENVX2750PhaTrigger.h>
#include <VX2750MultiModuleEventSegment.h>
#include <VX2750TclConfig.h>
#include <VX2750TclConfig.h>

#include <TCLInterpreter.h>
#include <TCLVariable.h>
#include <Exception.h>

#include <stdexcept>
#include <iostream>
#include <set>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

using namespace caen_nscldaq;

/**
 * constructor:
 *    @param configFile - name of the configuration file to process.  This can
 *                        be any path that's specified relative to the cwd in
 *                        which the Readout is run.
 *    @param pExperiment - Pointer to the experiment object that controls the
 *                       readout program.
 */
TclConfiguredReadout::TclConfiguredReadout(
    const char* configFile, CExperiment* pExperiment
) :  m_pExperiment(pExperiment), m_pTrigger(new DynamicMultiTrigger),
    m_pCurrentTrigger(nullptr),
    m_pCurrentConfiguration(nullptr),
    m_pCurrentEventSegment(nullptr),
    m_configFile(configFile)
{
    memset(m_priorDigest, 0, sizeof(m_priorDigest));    // Force initial configuration.        
}

/**
 * destructor
 *    We maintain quite a bit of dynamic data but, if we get this wrong it's
 *    not that big a deal because the typical lifetime of this object is the same
 *    as the program lifetime.
 */
TclConfiguredReadout::~TclConfiguredReadout() {
    delete m_pTrigger;
    deleteTrigger();
    delete m_pCurrentConfiguration;
    delete m_pCurrentEventSegment;
    
}
//////////////////////////////////////////////////////////////////////////////
// Define modules:

/**
 *  addModule
 *      Adds a module definition to the set of modules that will be created
 *      and, if all goes well, configured when the run begins.
 *  @param name - module name - must match the name of the module created in the
 *      configuration file.
 *   @param connectionString - connection string that's passed to the Dig2Device
 *      constructor when the module is created (e.g. stringified IP address).
 *   @param sid - is the unique source id to be assigned to hits from this module.
 *   @param isUsb - true if the module is connected via USB false if by Ethernet.
 *      This parameter defaults to false (Ethernet connected module).
 */
void
TclConfiguredReadout::addModule(
    const char* name, const char* connectionString, std::uint32_t sid, bool isUsb
) {
    ModuleInfo info = {
        .s_name             = name,
        .s_ConnectionString = connectionString,
        .s_sourceId        = sid,
        .s_isUsb            = isUsb
    };
    m_modules.push_back(info);
}

/**
* getTrigger
*    Returns a pointer to the trigger object we created.
*    This is actually the DynamicMultiTrigger.
*    This allows the Skeleton code to instantiate
*    and register this event segment and then get its
*    trigger and register that.
* @return CEventTrigger*
*/
CEventTrigger*
TclConfiguredReadout::getTrigger() {
    return m_pTrigger;
}
////////////////////////////////////////////////////////////////////////////////
// Event segment interface.
//

/**
 * onBegin
 *    Here's where all the heavy lifting is done.  Since we are beginning a run,
 *    we also know that we are not yet taking data.
 *    -   Delete the prior configuration, trigger, event segment -- if there is one
 *        (note that deleting a nullptr is a harmless no-op by the C++ standard).
 *    -  In order to fail early, we process the configuration file to create a new
 *       configuration -- failure -- we abort the begin by throwing an exception.
 *    -  We create modules for each entry in m_modules.
 *    -  We create a trigger for each module and add it to a new VX2750Event segment
 *       specific event trigger...we make that trigger what they dynamic trigger uses.
 *    -  We create a multimodule event segment.
 *    -  We invoke the onBegin for the multimodule event segment.
 *   All of this sets up the infrastructure to allow us to simply delegate
 *   the remainder of our event acquisitino methods to the multmodule event segment.
 *   
 */
void
TclConfiguredReadout::onBegin()
{
    
    m_pCurrentEventSegment->onBegin();
}


/**
 * initialize
 *    Called just prior to data taking to do hardware initialization....
 *    which is called prior to beginning the run.
 */
void
TclConfiguredReadout::initialize() {
    // Kill off the previous readout stuff.
    
  if (configChanged()) {
    m_pTrigger->clear();
    delete m_pCurrentConfiguration;
    m_pCurrentConfiguration = nullptr;
    deleteTrigger();
    delete m_pCurrentEventSegment;
    m_pCurrentEventSegment = nullptr;
    
    readConfiguration();         // Won't return on error
    checkModuleConfiguration();  // won't return on error.
    createTrigger();             // also create the modules in the trigger.
    
    m_pCurrentEventSegment = new VX2750MultiModuleEventSegment(m_pExperiment, m_pCurrentTrigger);
    m_pTrigger->addTrigger(m_pCurrentTrigger);
    
    
    m_pCurrentEventSegment->setConfigChanged();
  }
    m_pCurrentEventSegment->initialize();
    
}
/**
 * disable
 *    turns off hardware just prior to ending the run.
 */
void
TclConfiguredReadout::disable() {
    if (m_pCurrentEventSegment) {
        m_pCurrentEventSegment->disable();
    }
}
/**
 * onPause
 *    Called in respose to the pause command.
 */
void
TclConfiguredReadout::onPause() {
    if (m_pCurrentEventSegment) {
        m_pCurrentEventSegment->onPause();
    }
}
/**
 * onResume
 *    Called in response to a resume command:
 */
void
TclConfiguredReadout::onResume()
{
    if (m_pCurrentEventSegment) {
        m_pCurrentEventSegment->onResume();
    }
}
/**
 * read
 *    Called to actually read data.
 *  @param pBuffer -  pointer to the buffer.
 *  @param maxwords- Maximum number of 16 bit words the buffer can hold.
 */
size_t
TclConfiguredReadout::read(void* pBuffer, size_t maxwords) {
    return m_pCurrentEventSegment->read(pBuffer, maxwords);
}

//////////////////////////////////////////////////////////////////////////////
// Private utilities:

/**
 * deleteTrigger
 *    Deleteing the trigger also implies deleting the modules it holds.
 *    @note this may be called when thee is no current trigger
 *
 */
void
TclConfiguredReadout::deleteTrigger()  {
    if (m_pCurrentTrigger) {
        auto modules = m_pCurrentTrigger->getModules();
        for (auto m: modules) {
            delete m;
        }
        delete m_pCurrentTrigger;
        m_pCurrentTrigger = nullptr;
    }
    
    
}
/**
 * readConfiguration
 *    - Create a CTCLInterpreter.
 *    - Create the current cofiguration (on the interpreter).
 *    - Source the file to create the configuration.
 *
 * @note we do this all in a try/catch block and, if an error/exception
 *      occurs, we catch and delete anything we've made so far and
 *      rethrow
 */
void
TclConfiguredReadout::readConfiguration() {
    CTCLInterpreter interp;
    try {
        
        m_pCurrentConfiguration = new VX2750TclConfig(interp, "v27xxpha");
        interp.EvalFile(m_configFile);
        
    }
    catch (CException& e) {
        delete m_pCurrentConfiguration;
        m_pCurrentConfiguration = nullptr;
        std::cerr << e.ReasonText() << std::endl;
        std::cerr << getTclTraceback(interp) << std::endl;
        throw;
    }
    catch (std::string s) {
        delete m_pCurrentConfiguration;
        m_pCurrentConfiguration = nullptr;
        std::cerr << s << std::endl;
        std::cerr << getTclTraceback(interp) << std::endl;
        throw;
    }
    catch (std::exception& e) {
        delete m_pCurrentConfiguration;
        m_pCurrentConfiguration = nullptr;
        std::cerr << e.what() << std::endl;
        std::cerr << getTclTraceback(interp) << std::endl;
        throw;
    }
    catch (...) {
        delete m_pCurrentConfiguration;
        m_pCurrentConfiguration = nullptr;
        std::cerr << "Unanticipated exception type caught while processing the configuration file.\n";
        std::cerr << getTclTraceback(interp) << std::endl;
        throw;
    }
}
/**
 * checkModuleConfiguration
 *    It's going to be a fatal error if not all of our modules have a configuration.
 *    This method checks that m_pCurrentConfiguration has module names
 *    defined for each of the modules in m_modules.  If that is not the case
 *    we output a message to stderr and throw and std::exception.  If we
 *    have to throw the exception, we also kill off the configuration.
 */
void
TclConfiguredReadout::checkModuleConfiguration() {
    auto configuredNames = m_pCurrentConfiguration->listModules();
    std::set<std::string> configuredSet(configuredNames.begin(), configuredNames.end());
    
    // easiest to check in the set  We'll figure out all missing modules
    // before we throw so:
    
    bool missing(false);
    for (auto m : m_modules) {
        if (configuredSet.count(m.s_name) == 0) {
            std::cerr << m_configFile <<
                " Does not have a configurationfor module "
                << m.s_name << std::endl;
            missing = true;
        }
    }
    // If there were missing modules then we throw:
    
    if (missing) {
        delete m_pCurrentConfiguration;
        m_pCurrentConfiguration = nullptr;
        throw std::string("Configuration file is invalid");
    }
    
}
/**
 * createTrigger
 *    At this point, we have a configuration, it has a stanza for each
 *    module in m_modules.  We can confidently build the readout infrastructure
 *    knowing that evertying glues together properly.
 *    - We create a new mutlmodule trigger.
 *    - We create a VX2750MultiModuleEventSegment
 *    - For each module in m_modules we make a VX2750EventSegment
 *    - We create a trigger for that module and add it to the multimodule trigger.
 *
 */
void
TclConfiguredReadout::createTrigger() {
    m_pCurrentTrigger = new VX2750MultiTrigger;
    m_pCurrentEventSegment =
        new VX2750MultiModuleEventSegment(m_pExperiment, m_pCurrentTrigger);
    for (auto m : m_modules) {
        auto pSegment =
            new VX2750EventSegment(
                m_pExperiment, m.s_sourceId, m.s_name.c_str(),
                m_pCurrentConfiguration, m.s_ConnectionString.c_str(), m.s_isUsb
        );
        auto pModuleTrigger = new CAENVX2750PhaTrigger(*pSegment);
        m_pCurrentTrigger->addTrigger(pModuleTrigger);
    }
    
}


/**
 * getTclTraceback
 *    Pulls the value of the global errorInfo variable which is the traceback
 *    for any Tcl error thrown in evaluating a script.
 *  @param interp - interpreter whose traceback we're trying to get
 *  @return std::string - value of errorInfo.
 *  @retval "" if there's somehow no errorInfo var.
 */
std::string
TclConfiguredReadout::getTclTraceback(CTCLInterpreter& interp) {
    std::string result;
    
    CTCLVariable info(&interp, "errorInfo", false);
    const char* pTrace = info.Get();
    if (pTrace) result = pTrace;
    
    return result;
}
/**
 * configChanged
 * 
 * @return bool true - if the md5 sum of the configuration file does not match
 *                     m_priorDigest indicating the file was edited.
 * @note m_priorDigest is updated with the current MD5 hash  of the configuration file.
 */
bool
TclConfiguredReadout::configChanged()
{
    std::uint8_t currentDigest[MD5_DIGEST_LENGTH];
    computeConfigDigest(currentDigest);
    bool result = 
      memcmp(m_priorDigest,currentDigest, sizeof(m_priorDigest)) != 0;
    memcpy(m_priorDigest, currentDigest, sizeof(m_priorDigest));
    
    return result;
}
/**
 * computeConfigDigest
 *    Compute the md5 hash of the configuration file with thanks to
 *    https://stackoverflow.com/questions/3395690/md5sum-of-file-in-linux-c
 *
 * @param[out] pResult - pointer to a buffer of MD5_DIGEST_LENGTH bytes
 *     into which the hash is computed.
 *  m_configFile is the name of the file whose hash is computed.
 *
 * @throw std::invalid_argument - if the config file can't be opened for read.
 * @throw std::runtime_error - if the config file read fails.
 */
void
TclConfiguredReadout::computeConfigDigest(std::uint8_t* pResult) {
    MD5_CTX c;                    // Hash context.
    ssize_t bytes;                // bytes read per gulp.
    const int readSize = 512;     // Gulp size for file read.
    std::uint8_t buffer[readSize];  // File read buffer.
    
    int fd = open(m_configFile.c_str(), O_RDONLY);
    if (fd < 0) {
        throw std::invalid_argument("Config file could not be opened.");
    }
    
    bytes = ::read(fd, buffer, readSize);
    if (bytes < 0) {
        throw std::runtime_error("Config file read failed during MD5 computation");
    }
    MD5_Init(&c);
    while (bytes > 0) {
        MD5_Update(&c, buffer, bytes);
        bytes = ::read(fd, buffer, readSize);
        if (bytes < 0) {
            throw std::runtime_error("Config file read failed during MD5 computation");
        }
    }
    // End file encountered.
    
    MD5_Final(pResult, &c);
    
    
}
