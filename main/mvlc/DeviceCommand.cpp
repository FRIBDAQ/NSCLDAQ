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
@brief Implementation for the base class of commands that control devices.

*/

#include "DeviceCommand.h"
#include "TCLConfigParser.h"
#include <XXUSBConfigurableObject.h>
#include "CReadoutModule.h"

#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include <sstream>

/** construction
 *    @param interp - the TCL interpreter on which we will register our command
 *    @param cmd    - The name of the command e.g. "adc" will create add create etc.
 *    @param parser - TCLConfigParser object that contains the configuration we are producing.
 */
DeviceCommand::DeviceCommand(CTCLInterpreter& interp, const char* cmd, TCLConfigParser& parser) :
    CTCLObjectProcessor(interp, cmd, true),
    m_parser(parser) {

    }

/** 
 * destructor
 *   The real work is done by our base class destuctor.
 */
DeviceCommand::~DeviceCommand() {}

///////////////////////////////// public methods //////////////////////////////////////
/**
 *  operator()
 *     This is called whenever our command needs to be executed in a script.
 *     We ensure there is at least a subcommand and module name 
 *     and dispatch to the appropriate subcommand
 *     processing method.
 * 
 * @param interp - the interpreter executing the command.
 * @param objv   - vector of command words the first three must be the 
 *              -  Name of the command (cmd arg to constructor).
 *              -  Name of the subcommand (e.g. "create").
 *              - Name of the devices e.g. "my_digitizer"
 * @return  int
 * @retval TCL_OK - successful completion of the operation.
 * @retval TCL_ERROR - command processing resulted in an error which is described by the result.alignas
 * 
 * @note with the exception of the cget command, successful results will be the name of the device.
 *       e.g. puts [adc create my_digitizer]  will print my_digitizer.  cget, will set the result
 *       with the requested configuration information, if the command succeeds.
 * @note - implementation note. The subcommands can throw an exception to fail rather than
 *  and those will be converted in some way to a result and TCL_ERROR return code.
 */
int
DeviceCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    bindAll(interp, objv);                    // Bind all command words to the interpreter.
    try  {
        requireAtLeast(objv, 3,
            "This command requires at least a sub-command and a device name."
        );
        std::string subcommand = objv[1];
        std::string devName    = objv[2];

        if(subcommand == "create") {
            
            auto configuration = getConfigArray(objv);
            create(devName, configuration);
            interp.setResult(devName);
            interp.setResult(devName);
        } else if(subcommand == "config") {
            auto configuration = getConfigArray(objv);
            config(devName, configuration);
            interp.setResult(devName);
        } else if (subcommand == "cget") {
            requireAtMost(objv, 4, "At most one config parameter can be queried.");
            const char* pParam = nullptr;
            if (objv.size() == 4) {
                pParam = std::string(objv[3]).c_str();
            }
            cget(interp, devName, pParam);
        }
    }
    catch(std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch(CException & e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch(std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch(...) {
        interp.setResult("An unanticipated exception type was caught in DeviceCommaned::operator().");
        return TCL_ERROR;
    }
    // Nothing to catch means success.
    return TCL_OK;
}
/////////////////////////////// Protected methods - subcommands ////////////////////////////////////

/**
 *  create
 *     Called to process the create subcommand.  We verify there's not yet a device wit the same name
 * (logic error), Ask the strategy method to make new device and attach a configuration to it,
 * We then add the device to the parser and configure it via the config method - to handle and
 * trailing configuration options on the command line.
 * 
 * @note if there's an error in configuring the device, it will be in the parser but only configured
 * up to the option prior to the error.
 * 
 * @param name - name of the device to configure.
 * @param configParams - THe configuration option/value pairs - bound to the interpreter already.
 * 
 */
void 
DeviceCommand::create(std::string name, std::vector<CTCLObject>& configParams) {
    if (m_parser.findDevice(name)) {
        throwDeviceError(name, "Attempted to create a duplicate device");
    } else {
        CReadoutModule* pDevice = createDevice(name);
        pDevice->Attach(new XXUSB::CConfigurableObject(name));    // Set up the configuration options.
        m_parser.addDevice(name, pDevice);
        config(name, configParams);
    }
}

/**
 *  config   
 *    Called to configure a device we look it up in the parser, if it's not been created,
 * that's a logic error.   We ensure that there are an even number of config words (they come in
 * option/value pairs). If not that's also a logic error.  Then we get the devices configuration and
 * configure it -- any error will result in an exception that will bubble up to operator().
 * 
 * @param name - name of the device to configure.
 * @param config - the option/value pairs to configure
 * 
 */
void
DeviceCommand::config(std::string name, std::vector<CTCLObject>& config) {
    CReadoutModule* pModule = m_parser.findDevice(name);
    if (!pModule) {
        throwDeviceError(name, "Attempted to configure a non existent device ");
    } else {
        if (config.size() & 1) {
            throwDeviceError(name, "Configurations must consists of option/value pairs.");
        } else {
            XXUSB::CConfigurableObject* pConfig = pModule->getConfiguration();
            for (int i =0; i < config.size(); i += 2) {
                std::string option = config[i];
                std::string value = config[i+1];    // We ensured this is defined above.
                pConfig->configure(option, value);  // May throw if, option is invalid e.g.
            }
        }
    }
}

/**
 *  cget
 *     Gets either the entire configuration or the value of a single option.alignas
 *     If the option paramter is nullptr, the entire configuration is set in the result
 *     as a list of two element sublists, containing in order an option name and value e.g.
 * 
 * \verbatim
 *  {{-base 012435678} {-enable true} ... }
 * \endverbatim
 * 
 *    If option is not null and a valid option name the result is set with the value of that options.
 * 
 * @param interp  - the interpreter, needed to create list objects.
 * @param option  - If not null the name of the single option value desired.
 */
void 
DeviceCommand::cget(CTCLInterpreter& interp, std::string name,  const char* option) {
    CReadoutModule* pModule = m_parser.findDevice(name);
    if (!pModule) {
        throwDeviceError(name, "Attempting to query the configuration of a non-existent device");
    } else {
        XXUSB::CConfigurableObject* pConfig = pModule->getConfiguration();
        if (option) { 
            interp.setResult(pConfig->cget(std::string(option)));   // could also throw.
        } else {
            auto config = pConfig->cget();     // The entire config as name/value pairs.

            CTCLObject result; result.Bind(interp);
            for (auto item : config) {
                CTCLObject itemList; itemList.Bind(interp);
                CTCLObject name; name.Bind(interp); name = item.first;
                CTCLObject value; value.Bind(interp); value = item.second;

                itemList += name; itemList += value;
                result += itemList;
            }
            interp.setResult(result);
        }
    }
}



////////////////////////////// Private utiltiies. ////////////////////////////////////////////////

/*
    getConfigArray 
       Given the full set of command words returns those that are a configuration.
       

    @param objv - Full command word vector.
    @return std::vector<CTCLObject> - the tail that is configuration options and their values.
          This may be empty.
    @note - no attemp is made to ensure the resulting vector is even length.. that check is done
        in the config method, however.
    @note we can and do assume there are at least 3 words.
*/
std::vector<CTCLObject>
DeviceCommand::getConfigArray(std::vector<CTCLObject>& objv) const {
    auto first = objv.begin();   // command name.
    first++;                     // subcommand.
    first++;                     // Device name.
    first++;                     // end() or first option.

    return std::vector<CTCLObject>(first, objv.end());
}

/*
    throwDeviceError
       Build a nice error string for an error involving a device and reason
       and throws and std::logic_error with that message.

       @param name - name of the device.
       @param reason - reason the error is being thrown. 
*/
void
DeviceCommand::throwDeviceError(std::string name, const char* reason)  const{
    std::stringstream error;
    error << "Error involving the device named : " << name << " : " << reason;

    std::string message(error.str());
    throw std::logic_error(message);
}