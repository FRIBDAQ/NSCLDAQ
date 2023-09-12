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
* @file     VX2750TclConfig.cpp
* @brief    Implementation of caen_nscldaq::VX2750PHAConfiguration class.
* @author   Ron Fox
*
*/
#include "VX2750TclConfig.h"
#include "VX2750PHAConfiguration.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <tcl.h>
#include <algorithm>
#include <stdexcept>
namespace caen_nscldaq {
/**
 * constructor
 *    Register the command with the interpreter:
 *  @param interp - interpreter on which the command will be registered.
 *  @param pName  - Name of the command.
 */
VX2750TclConfig::VX2750TclConfig(CTCLInterpreter& interp, const char* pName) :
    CTCLObjectProcessor(interp, pName, TCLPLUS::kfTRUE)
{}

/**
 * destructor
 *   - We need to get rid of the modules we've already gotten as they get
 *     new'd into existence.
 */
VX2750TclConfig::~VX2750TclConfig() {
    std::for_each(
        m_modules.begin(), m_modules.end(),
        [](std::pair<std::string, VX2750PHAModuleConfiguration*> item) {  //Lambda that
            delete item.second;                               // Deletes the module.
        }
    );
}

/**
 * operator()
 *    Called when the command is executed.
 *  @param interp -interpreter running the command.
 *  @param objv  - command words.
 *  @return int TCL_OK - successs, TCL_ERROR failure.
 *  @note the command result may be set. See the create, config, cget, destroy and
 *        list comments.
 */
int
VX2750TclConfig::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        requireAtLeast(objv, 2, "Insufficient command parameters");
        std::string subcommand = objv[1];
        
        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "config") {
            config(interp, objv);
        } else if (subcommand == "cget") {
            cget(interp, objv);
        } else if (subcommand == "delete") {
            cget(interp, objv);
        } else if (subcommand == "list") {
            list(interp, objv);
        } else {
            std::string msg = "Unrecognized subcommand: ";
            msg += subcommand;
            throw msg;
        }
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 * getModule
 *   Given a module name return the pointer to its configuration or throws a
 *   string exception if not found.
 * @param pName name of the module
 * @return VX2750PHAModuleConfiguration*
 */
VX2750PHAModuleConfiguration*
VX2750TclConfig::getModule(const char* pName)
{
    std::string name(pName);
    return getConfigOrThrow(name);
}
/**
 * listModules
 *    @return std::vector<std::string>  list of module names.
 */
std::vector<std::string>
VX2750TclConfig::listModules()
{
    std::vector<std::string> result;
    std::for_each(m_modules.begin(), m_modules.end(),
        [&result](std::pair<std::string, VX2750PHAModuleConfiguration*> m) {
           result.push_back(m.first);
        }
    );
    return result;
}
//////////////////////////////////////////////////////////////////////////////
/**
 * create
 * 
 * create a new module:
 *     vx27x0 create name
 *
 *   -   Require exactly three command line words.
 *   -   Require a unique name.
 *   -   Create a new VX2750PHAConfiguration and save it in the map
 *       indexed by its name.
 *       
 * @param interp - references the interpreter running the command.
 * @param obvj   - the encapsulated command words.
 * @throw Various appropriate exceptions on faiulre.
*/
void VX2750TclConfig::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "Incorrect number of command line parameters");
    
    std::string name = objv[2];
    if (m_modules.count(name) > 0) {
        throw std::string("Duplicate configuration name");
    }
    
    m_modules[name] = new VX2750PHAModuleConfiguration(name.c_str());
}
/**
 * config
 *    Configure a module.
 *    -  Require at least 5 parameters
 *    -  Require an odd number of parameters.
 *    -  Look up the name, throwing if it can't be found.
 *    -  For each config name/value pair, attempt to configure that parameter
 *
 *  
 *    
 * @param interp - references the interpreter running the command.
 * @param obvj   - the encapsulated command words.
 * @throw Various appropriate exceptions on faiulre.
 * @note Partial configuration is possible:  If a configuration attempt fails,
 *       all of the configurations up to that failure will be done.
*/
void VX2750TclConfig::config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireAtLeast(objv, 5, "Insufficient command line parameters");
    if ((objv.size() % 2) != 1) {
        throw std::string("Each configuration parameter name must be paired with a value");
    }
    std::string name = objv[2];
    auto pConfig = getConfigOrThrow(name);
    
    // Note that failures will throw too:
    
    for (int i = 3; i < objv.size(); i += 2) {
        std::string param =objv[i];
        std::string value = objv[i+1];
        pConfig->configure(param, value);
    }
}
/**
 * cget
 *    Put the value of a configuration parameter in the command result.
 *    - Must have exactly three or four
 *    - The last word must be an existing object name.
 *    - The result of cget is put in the intepreter result.
 * @param interp - references the interpreter running the command.
 * @param obvj   - the encapsulated command words.
 * @throw Various appropriate exceptions on faiulre.
 */
void VX2750TclConfig::cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{

    
    if (objv.size() != 3 && objv.size() != 4) {
        throw std::string("Incorrect Number of command line parameters");
    }    
    std::string name = objv[2];
    auto pConfig = getConfigOrThrow(name);

    if (objv.size() == 4) {
        // Single item:
        
        std::string param = objv[3];
        interp.setResult(pConfig->cget(param));
    } else {
        // Full config:
        
        auto config = pConfig->cget();
        CTCLObject result;
        result.Bind(interp);
        
        // Each name value pair makes a 2 element list which is an element
        // of result.
        
        for (int i = 0; i < config.size(); i++) {
            CTCLObject item;
            item.Bind(interp);
            std::string name = config[i].first;
            std::string value = config[i].second;
            item += name;
            item +=  value;
            result += item;
        }
        interp.setResult(result);
    }
    
}
/**
 * destroy
 *    Destroys a module configuration.
 *    - Must be 3 words on the command.
 *    - The last word must be a valid module.
 *    - The module is deleted.
 *    - The map entry is erased.
 * @param interp - references the interpreter running the command.
 * @param obvj   - the encapsulated command words.
 * @throw Various appropriate exceptions on faiulre.
 */
void VX2750TclConfig::destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "Incorrect number of command parameters");
    std::string name = objv[2];
    auto pConfig = getConfigOrThrow(name);
    m_modules.erase(name);
    delete pConfig;
}
/**
 * list
 *   List the names of the modules in the configuration.
 *
 * @param interp - references the interpreter running the command.
 * @param obvj   - the encapsulated command words.
 * @throw Various appropriate exceptions on faiulre.
 */
void VX2750TclConfig::list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 2, "Incorrect number of command parameters");
    
    CTCLObject result;
    result.Bind(interp);
    std::for_each(m_modules.begin(), m_modules.end(),
        [&result](std::pair<std::string, VX2750PHAModuleConfiguration*> item) {
            result += item.first;             
        }
    );
    interp.setResult(result);
}

///////////////////////////////////////////////////////////////////////////////
// Private utils.

/**
 * getConfigOrThrow
 *    Look up a configuration object by name and throw if it does not exist.
 * @param name - name of config to find.
 */
VX2750PHAModuleConfiguration*
VX2750TclConfig::getConfigOrThrow(std::string name)
{
    auto ptr = m_modules.find(name);
    if (ptr == m_modules.end()) {
        std::string message="No such module name: ";
        message += name;
        throw message; 
    }
    return ptr->second;
}
}                                    // caen_nscldaq namespace.
