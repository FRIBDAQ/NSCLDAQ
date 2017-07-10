/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CTCLServiceApiInstance.cpp
# @brief  Implementation of service api instance commands.
# @author <fox@nscl.msu.edu>
*/
#include "CTCLServiceApiInstance.h"
#include "CServiceApi.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include <iostream>


/**
 * constructor
 *   @param interp  - interpreter on which the command is registered.
 *   @param command - Command being registered
 *   @param uri     - URI to the database we are operating on.
 */
CTCLServiceApiInstance::CTCLServiceApiInstance(
    CTCLInterpreter& interp, const char* command, std::string uri
)  :
    CTCLObjectProcessor(interp, command, true),
    m_pApi(0)
{
    m_pApi = new CServiceApi(uri.c_str());        
}

/**
 * destructor:
 */
CTCLServiceApiInstance::~CTCLServiceApiInstance()
{
    delete m_pApi;
}

/**
 * operator()
 *    process the command.
 *
 * @param interp   - interpreter executing the command.
 * @param objv     - The command words.
 * @return int     - TCL_OK or TCL_ERROR.
 */
int
CTCLServiceApiInstance::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    bindAll(interp, objv);
    try {
        // Require that there at least be a subcommand.
        
        requireAtLeast(objv, 2, "Missing subcommand");
        std::string subcommand = objv[1];
        
        if (subcommand == "exists") {
            interp.setResult(m_pApi->exists() ? "1" : "0");
        } else if (subcommand == "create") {
            m_pApi->create();
        } else if (subcommand == "createprog") {
            createProg(interp, objv);
        } else if (subcommand == "setHost") {
            setHost(interp, objv);
        } else if (subcommand == "setCommand") {
            setProgram(interp, objv);
        } else if (subcommand == "setEditorPosition") {
            setEditorPosition(interp, objv);
        } else if (subcommand == "getEditorXPosition") {
            getEditorXPosition(interp, objv);
        } else if (subcommand == "getEditorYPosition") {
            getEditorYPosition(interp, objv);
        } else if (subcommand == "remove") {
            remove(interp, objv);
        } else if (subcommand == "listall") {
            listAll(interp, objv);
        } else if (subcommand == "list") {
            list(interp, objv);
        } else if (subcommand == "getProperty") {
            getProperty(interp, objv);
        } else if (subcommand == "setProperty") {
            setProperty(interp, objv);
        } else {
            throw std::logic_error("Invalid subcommand");
        }
        
    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult("Unexpected exception type caught in service creator");
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*---------------------------------------------------------------------------
 * utility methods
 */

/**
 * createProg
 *    Create a program.
 *
 * @param interp   - interpreter executing the command.
 * @param objv     - command line parameters.
 */
void
CTCLServiceApiInstance::createProg(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 5, "createprog needs name, path, host in addition to subcommand"
    );
    std::string name = objv[2];
    std::string path = objv[3];
    std::string host = objv[4];
    
    m_pApi->create(name.c_str(), path.c_str(), host.c_str());
}
/**
 * setHost
 *    changes the host for a program.
 * @param interp   - interpreter executing the command.
 * @param objv     - command line parameters.
 */
void
CTCLServiceApiInstance::setHost(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 4, "setHost needs name and a new host in addtion to subcommand"
    );
    std::string name = objv[2];
    std::string host = objv[3];
    m_pApi->setHost(name.c_str(), host.c_str());
    
}
/**
 * setProgram
 *    set a new program path.
 * @param interp   - interpreter executing the command.
 * @param objv     - command line parameters.
 */
void
CTCLServiceApiInstance::setProgram(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 4, "setCommand needs name and a new path in addtion to subcommand"
    );
    std::string name = objv[2];
    std::string path = objv[3];
    m_pApi->setCommand(name.c_str(), path.c_str());
}

/**
 * setEditorPosition
 *    Set a new position for a service with the editor.
 *
 * @param interp - interpreter running the command.
 * @param objv   - Command line parameters (api setEditorPosition name x y).
 */
void
CTCLServiceApiInstance::setEditorPosition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 5);
    std::string name = objv[2];
    int         x    = objv[3];
    int         y    = objv[4];
    
    m_pApi->setEditorPosition(name.c_str(), x, y);
}
/**
 * getEditorXPosition
 *    SEt the result with the x coordinate of the editor position of a
 *    service.
 *
 * @param interp - interpreter running the commabnd.
 * @param objv   - command words (api getEditorXPosition name)
 */
void
CTCLServiceApiInstance::getEditorXPosition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    std::string name = objv[2];
    
    CTCLObject result;
    result.Bind(interp);
    result = m_pApi->getEditorXPosition(name.c_str());
    
    interp.setResult(result);
}
/**
 * getEditorYPosition
 *    SEt the result with the x coordinate of the editor position of a
 *    service.
 *
 * @param interp - interpreter running the commabnd.
 * @param objv   - command words (api getEditorXPosition name)
 */
void
CTCLServiceApiInstance::getEditorYPosition(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 3);
    std::string name = objv[2];
    
    CTCLObject result;
    result.Bind(interp);
    result = m_pApi->getEditorYPosition(name.c_str());
    
    interp.setResult(result);
}
/**
 * remove
 *    remove a program from the database.
 *  @param interp   - interpreter executing the command.
 *  @param objv      - command line parameters.
 */
void
CTCLServiceApiInstance::remove(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 3, "remove command requires a program in addition to subcommand"
    );
    std::string name = objv[2];
    
    m_pApi->remove(name.c_str());
}
/**
 * listAll
 *    List all programs as a dictable.
 *
 *  @param interp   - interpreter executing the command.
 *  @param objv      - command line parameters.
 */
void
CTCLServiceApiInstance::listAll(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 2, "listall requires no additional parameters");
    
    std::map<std::string, std::pair<std::string, std::string>> listing =
        m_pApi->list();
    
    CTCLObject result;
    result.Bind(interp);
    
    std::map<std::string, std::pair<std::string, std::string>>::iterator p =
        listing.begin();
    while(p != listing.end()) {
        std::string progName = p->first;
        std::string path     = p->second.first;
        std::string host     = p->second.second;
        
        CTCLObject name, info;
        name.Bind(interp);
        info.Bind(interp);
        
        name = progName;
        info += path;
        info += host;
        
        result += progName;
        result += info;
        
        p++;
    }
    
    interp.setResult(result);    
}
/**
 * list
 *   return the path/host pair for a program.
 */
void
CTCLServiceApiInstance::list(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(
        objv, 3, "list requires only a program name in addtion to the subcommand"
    );
    std::string name=objv[2];
    
    std::pair<std::string, std::string> info = m_pApi->list(name.c_str());
    
    CTCLObject result;
    result.Bind(interp);
    result += info.first;
    result += info.second;
    
    interp.setResult(result);
}
/**
 * setProperty
 *    Sets the value of a service property.
 */
void
CTCLServiceApiInstance::setProperty(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireAtLeast(objv, 5);
    requireAtMost(objv, 6);
    
    int create = 1;
    
    std::string programName = objv[2];
    std::string propName    = objv[3];
    std::string propValue   = objv[4];
    
    if (objv.size() == 6) {
        create = objv[5];
    }

    
    m_pApi->setProperty(
        programName.c_str(), propName.c_str(), propValue.c_str(), create
    );
}
/**
 * getProperty
 *    Returns as the command value the value of a program property.
 */
void
CTCLServiceApiInstance::getProperty(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
)
{
    requireExactly(objv, 4);
    
    std::string programName = objv[2];
    std::string propName    = objv[3];
    
    interp.setResult(m_pApi->getProperty(programName.c_str(), propName.c_str()));
}

