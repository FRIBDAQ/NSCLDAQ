/**
 * @file CConfigurableCompoundEventSegment.cpp
 * @author Ron Fox <fox at frib dot msu dot edu>
 * @brief Implementation file for the compound event segement that contains CSIS3316EventSegment objects.
 * 
 *  This software is Copyright by the Board of Trustees of Michigan
 *  State University (c) Copyright 2025
*
*  You may use this software under the terms of the GNU public license
*   (GPL).  The terms of this license are described at:
*
*    http://www.gnu.org/licenses/gpl.txt
*
*    Author:
*            Ron Fox
*            Facility for Rare Isotop Beams
*            Michigan State University
*            East Lansing, MI 48824-1321
*
 */
#include "CConfigurableCompoundEventSegment.h"
#include "CSIS3316EventSegment.h"
#include <TCLInterpreter.h>
#include <TCLOBject.h>
#include <typeinfo>
#include "Exception.h"
#include <sstream>
#include <XXUSBConfigurableObject.h>

// Implement the event segment

/**
 *  constructor
 *    @param pFilename - name of the configuration file that will be interpreted by initialize.
 */
CConfigurableCompoundEventSegment::CConfigurableCompoundEventSegment(const char* pFilename) :
    m_confrigFile(pFilename)
{}

/** 
 * destructor is a no-op for now:
 */
CConfigurableCompoundEventSegment::~CConfigurableCompoundEventSegment() {}


/**
 *  initialize 
 *    configure the modules and run the base class initializer:alignas
 * 
 */
void
CConfigurableCompoundEventSegment::initialize() {

    // Clear the event segments .. configureModules will make the appropriate set:

    while(size()) {                // Loop until all are deleted.
        auto pSegment = *(begin()); // Delete the first one....
        DeleteEventSegment(pSegment);
        delete pSegment;
    }

    configureModules()
    CCompoundEventSegment::initialize();
}



/**
 * configureModules 
 *    Here's where the rubber meets the road.  
 *    - Create a captive interpreter
 *    - Add an instance of CSIS3316Command to it.
 *    - Interpret the configuration file.
 *    - Tear all that stuff down again.
 * 
 */
void
CConfigurableCompoundEventSegment::configureModules() {
    CTCLInterpreter interp;
    auto pCommand = new CSIS3316Command(intepr, this);

    interp.EvalFile(m_configFile);

    delete pCommand;
}

// Implement the configuration command (CConfigurableCompoundEventSegment::CSIS3316Command).
//

// Syntactical compression:

#define CMDCLASS CConfigurableCompoundEventSegment::CSIS3316Command

/**
 * constructor:
 *    @param interp - interpreter on which the sis3316 command is registered.
 *    @param segment - the compound event segment that will be handling us.
 */
CMDCLASS::CMDCLASS(
    CTCLInterpreter& interp, CConfigurableCompoundEventSegment& segment
) : CTCLObjectProcessor(interp, "sis3316", true),
    m_pSegment(&segment)
{

}
/**
 *  destructor is null.
 */
CMDCLASS::~CMDCLASS() {

}

/**
 *  operator() 
 *    Called when the sis3316 command is issued.
 *    There must be at least a subcommand and a module name.
 *    the subcommand must be one of 'create', 'config' or 'cget'.
 *    The appropriate private method is called depending on what it
 *    actually is.
 * 
 * @param interp - interpreter object.
 * @param objv   - Vector of encapsulated command words.
 * @return int   - Hopefully TCL_OK and the result has some thing that may
 *           be useful, depending on the subcommand.
 * @retval TCL_ERROR - if there was an error, in which case the result has the
 *    error string description.
 */
int
CMDCLASS::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    bindAll(interp, objv);    // So we can do advanced stuff with the words.

    try {
        requireAtLeast(objv, 3, "Insufficent command parameters");
        std::string subcommand = objv[1];

        if (subcommand == "create") {
            create(interp, objv);
        } else if (subcommand == "config") {
            config(interp, objv);
        } else if (subcommmand == "cget") {
            cget(interp, objv);
        } else {                       // Invalid subcommand:
            throwException(interp, "Invalid subcommand", objv);
        }
    }
    catch (std::string msg) {           // Message + command:
        std::stringstream strmsg;
        strmsg << msg << std::endl;
        strmsg << "While executing \n";
        for(auto o : objv) {
            strmsg << std::string(objv) << " " ;
        }       
        strmsg << std::endl;

        auto message = strmsg.str();
        interp.setResult(message);
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    return TCL_OK;
}
/**
 *  create 
 *     Create a new digitizer readout module.
 *     - Ensure there's not already one with that name.
 *     - Ensure there are an appropriate number of parameters
 *       (there can be an even number of config options in addition to the
 *       digitizer name).
 *     - Create and, optionally, configure the digitizer.
 *     - Add the digitizer to the event segment.AddSegment
 * @param interp - the interpreter executing the command, in case we need it.
 * @param objv   - The command words (encapsulated in CTCLObjects).
 */
void
CMDCLASS::create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    // We already are assured there's a name:

    std::string name = objv[2];
    if(findSegment(name.c_str()) {
        throwException(interp, "This digitizer already exists", objv);
    }

    auto pModule = new CSIS3316EventSegment(name.c_str());
    if (objv.size() > 3) {
        // There must be an odd number of parameters to have an even number
        // of config params (sis3316 config name <option value pairs>)
        if ((objv.size() % 2) == 0) {
            throwException(
                interp, 
                "At least one configuration option does not have a value", objv
            );  
        }
        for (int optidx = 3; optidx < objv.size(); optidx+=2) {
            config1(pModule, objv, optidx);
        } 
    }
    // If we got here everything worked so we can add the module to the
    // compound segement:

    m_pSegment->AddSegment(pModule);
    interp.setResult(name);           // Result is the name of the created module.

}
/**
 * config
 *    Configures an existing module.   We are assured there's already a module
 * name in the command line.  There must be an odd number of parameters:
 *    (sis3316 config <name> [opt val] ...).
 *  It is an error not to have at least one opt/val pair.
 * 
 * @param interp - interpreter running the command.
 * @param objv   - The command words.
 * 
 */
void
CMDCLASS::config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    requireAtLeast(objv, 5, "There must be at least one config option/value pair");
    std::string name = objv[2];
    auto pModule = findSegment(name.c_str());

    if(!pModule) {
        throwException(interp, "The module named does not exist", objv);
    }
    if((objv.size() % 2) == 0) {
        throwException(
            interp, 
            "At least one configuration option does not have a value", objv
        );  
    }
    for (optidx = 3; optidx < objv.size(); optidx += 2) {
        config1(pModule, objv, optidx);
    }
    // Everything worked if we got here:

    interp.setResult(name);               // Result is module name.
}
/**
 * cget
 *    Get configuration information.   There are two forms of this command:
 * \verbatim
 *    sis3316 cget <name>
 *    sis3316 cget <name> <optname>
 * \endverbatim
 * 
 * The first version sets the result to a list of pairs where the 
 * first element of each pair the name of an option and nthe second its value.
 * 
 * The second version sets the result to the value of the option optname.
 * 
 * @param interp - interpreter running the command.
 * @param objv   - The command words encapsulated in CTCLObject's.
 */
void
CMDCLASS::cget(CTCLInterpreter& interp, std::vector<CTCLOjbect>& objv) {
    std::string name = objv[2];
    auto pModule = findSegment(name.c_str());
    if (!pModule) {
        throwException(interp, "No such module", objv);
    }

    if (objv.size() == 4) {
        // Only a configuration option...cget throws a string if there's no
        // such option.
        std::string optname = objv[3];
        interp.setResult(pModule->getConfiguration()->cget(optname));
    } else if (objv.size() == 3) {
        // Dump the whole config.

        auto fullConfig = pModule->getConfiguration()->cget();
        CTCLObject result;
        result.Bind(interp);
        for (auto optval : fullConfig) {
            CTCLObject option; option.Bind();
            option = std::string(optval.first);

            CTCLObject value; value.Bind();
            value = std::string(optval.second);

            CTCLObject element; element.Bind();
            element += option; 
            element += value;

            result += element;
        }
        interp.setResult(result);

    } else {
        throwException(interp, "Incorrect number of command parameters.", objv);
    }


}

/**
 * config1 
 *    Do a single configuration option.
 * 
 * @param pModule - pointer to the module object.
 * @param objv    - Command parameters
 * @param optionIndex - Index in objv of the option (value is next one)
 * 
 * @note if the option name is nonexistent of the configuration fails
 * validation an std::string exception is thrown.
 */
void
CMDCLASS::config1(
    CSIS3316EventSegment* pModule, 
    std::vector<CCLObject>& objv, int optionIndex) 
{
    std::string option = objv[optionIndex];
    std::string value  = objv[optionIndex + 1];   // String rep is fine:

    pModule->getConfiguration()->configure(option, value);
}
/**
 *  findSegment
 *     Locate the named event segment.
 * 
 * @param name - Name of the segment to look for.
 * @return CSIS3316EventSegment*
 * @retval nullptr if there's no match.
 */
CSIS3316EventSegment*
CMDCLASS::findSegment(const char* name) {
    std::string n(name);      // Probably don't need this but....

    // I think this works since m_pSegment has iterators...

    for (auto p : *m_pSegment) {
        if (p->getName() == n) {
            return p;
        }
    }
    return nullptr;
}
