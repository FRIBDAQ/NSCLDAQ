/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for the parser of VMUSB daqconfig.tcl files.
*/

#include "TCLConfigParser.h"
#include "TCLInterpreter.h"
#include "TCLObjectProcessor.h"
#include "CStack.h"
#include "CReadoutModule.h"
#include <stdexcept>
#include <iostream>
#include <tcl.h>

TCLConfigParser* TCLConfigParser::m_pInstance(0);

//////////////////////////////////////// implementing the canonical methods //////////////////////////
/**
 * constructor:
 *    @param[in] infile -name of the input file to interpret.
 */
TCLConfigParser::TCLConfigParser(const std::string& infile) :
    m_pInterp(new CTCLInterpreter),
    m_daqconfigFile(infile),
    m_pEventStack(0),
    m_pScalerStack(0)
{
    // Init the interpreter search paths etc:

    int status = Tcl_Init(m_pInterp->getInterpreter());
    if (status != TCL_OK) {
        std::cerr << "Failed to initialize Tcl interpreter paths.. attempting to continue\n";
    }
    // Logic error if we already exist:

    if (m_pInstance) {
        throw std::logic_error("Attempted duplicate TclConfig parser construction");
    } else {
        m_pInstance = this;
    }
}
/**
 *  destructor 
 *    Kill off all of the dynamic data:
 */
TCLConfigParser::~TCLConfigParser() {
    // Kill them modules in the map:
    
    for(auto& p : m_modules) {
        delete p.second;
    }
    m_modules.clear();                // not really needed.

    // Delete the stack - note they don't delete their modules
    // as they just hold pointer into modules in m_modules:
    // Still need to define a stack.

    //  Kill off the command extensions:

    for(auto p: m_commandExtensions) {
        delete p;
    }
    m_commandExtensions.clear();    // Not really needed.

    m_pInstance = nullptr;         // now there's not an instance.
    delete m_pInterp;                 

}
//////////////////////////////////  Implementing public operations /////////////////////////////////

/**
 *  initialize
 *     This is done separately from construction so that test overrides will be called.
 */
void
TCLConfigParser::initialize() {
    addExtensions();
}

/**
 *  operator()
 *      Source the configuration file.
 * @throw CExeption - from m_pInterp if there's an error in the script.
 * @throw std::logic_error - if there's more than one scaler or event tstack.
 */
void
TCLConfigParser::operator()() {
    m_pInterp->EvalFile(m_daqconfigFile);

    // See if wwe have an event stack and/or scaler stack...
    // Warn if we have stacks we can't trigger  (yet) and  error for more than one scaler
    // or more than one event stack:

    for (auto p : m_modules) {
        CStack* pStack = dynamic_cast<CStack*>(p.second->getDriver()); // null if module is not a stack.
        if (pStack) {
            auto trigger = pStack->getTriggerType();
            if (trigger == CStack::Nim1) {   // Event stack.
                if (m_pEventStack) {
                    throw std::logic_error("More than on event stack is specified!");
                } else {
                    m_pEventStack = pStack;
                }
            } else if (trigger == CStack::Scaler) {   // Periodic scaler stack.
                if (m_pScalerStack) {
                    throw std::logic_error("More than one scaler stack was specified");
                } else {
                    m_pScalerStack = pStack;
                }
            } else {     // UNsupported stack type:
                std::cerr << " Interrupt triggered stacks are not supported at this time\n";
            }
        }
    }
    // We'll catch no stacks in a derived class that does the actual code generation.
}

// Services for device commands:

/**
 * findDevice
 *     Locate a device by name in m_modules.
 * 
 * @param devname - name of the device to find.
 * @return CReadoutModule*
 * @retval nullptr - if there is no such device.
 */
CReadoutModule*
TCLConfigParser::findDevice(std::string devname) {
    CReadoutModule* result(nullptr);
    auto p = m_modules.find(devname);
    if (p != m_modules.end()) {
        result = p->second;
    }
    return result;
}
/**
 * addDevice
 *    Add a new device to the m_modules map - the caller is responslbe for ensuring this is
 * not a duplicate.
 * 
 * @param devname - name of the new devicde.
 * @param driver  - pointer to the device instance to have.  We assume ownership of this device.
 */
void
TCLConfigParser::addDevice(std::string devName, CReadoutModule* driver) {
    m_modules[devName]  = driver;                // Caller has to ensure this is not a duplicate name.
}

///////////////////////////////// private utilities ///////////////////////////////////////////



/**
 * addExtensions
 *     Adds all extension commands to the interpreter.  THis will just call addExtension for each of
 * the extension commands to be added (this supports better testing too).
 */
void
TCLConfigParser::addExtensions() {
    // Call addExtension oncde for each extension to add.
}

/**
 *  add a single extension to the m_commandExtensions array
 * 
 * @param cmdobj - command obhject to add.  
 */
void
TCLConfigParser::addExtension(CTCLObjectProcessor& cmdobj) {
    m_commandExtensions.push_back(&cmdobj);
}
/** getInstance
 * 
 * @return TCLConfigParser*  note could be null if called too soon.
 */
TCLConfigParser*
TCLConfigParser::getInstance() {
    return m_pInstance;
}