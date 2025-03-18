/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CTclCommand.cpp
 *  @brief: Implement the command base class.
 */
#include "CTclCommand.h"
#include <sstream>
#include <string>

/**
 * constructor
 *   @param interp - interpreter on which the command is registered.
 *   @param cmdName - Command name string.
 *   @note The client data will be a pointer to this object so that
 *        the static commandRelay method actually registered as the
 *        command handler can invoke operator().
 *   @note - we assume the registration works.
 */
CTclCommand::CTclCommand(Tcl_Interp* pInterp, const char* cmdName) :
    m_pInterp(pInterp), m_cmdName(cmdName)
{
    Tcl_CreateObjCommand(
        m_pInterp, m_cmdName.c_str(), commandRelay, this, nullptr
    );
}
/**
 * destructor unregisters the command
 */
CTclCommand::~CTclCommand() {
    Tcl_DeleteCommand(m_pInterp, m_cmdName.c_str());
}
////////////////////////////////////////////////////////////////
// Utilities for derived classes:

/**
 * requireExactly
 *    Require an exact parameter count.
 *  @param objv - the parameter vector.
 *  @param nParams - number required.
 *  @throw std::string error message if there's not a match.
 */
void
CTclCommand::requireExactly(std::vector<Tcl_Obj*>& objv, int nParams)
{
    if (objv.size() != nParams) {
        std::stringstream s;
        s << Tcl_GetString(objv[0]) << " requires " << nParams-1
          << " parameters but " << objv.size() -1 << " were given";
        std::string msg = s.str();
        throw msg;
    }
}
/**
 * requireAtMost
 *    Require at most a specified number of command words.
 * @param objv - the command words.
 * @param max  - maximum number of words allowed.
 * @throw std::string error message if there are too many params.
 */
void
CTclCommand::requireAtMost(std::vector<Tcl_Obj*>& objv, int max)
{
    if (objv.size() > max) {
        std::stringstream s;
        s << Tcl_GetString(objv[0]) <<  " requires at most " << max-1
          << " parameters but " << objv.size() -1 << " were given.";
        std::string msg = s.str();
        throw msg;
    }
}
/**
 * setResult (String)
 *   Given a string, sets the Tcl intepreter result to it.
 * @param result the string
 */
void
CTclCommand::setResult(const char* result)
{
    Tcl_Obj* resultObj  = Tcl_NewStringObj(result, -1);
    setResult(resultObj);
}
/**
 * setResult (Tcl_Obj*)
 */
void
CTclCommand::setResult(Tcl_Obj* obj)
{
    Tcl_IncrRefCount(obj);
    Tcl_SetObjResult(m_pInterp, obj);
    Tcl_DecrRefCount(obj);
}
/**
 * setREsult (double)
 */
void
CTclCommand::setResult(double value)
{
    Tcl_Obj* result = Tcl_NewDoubleObj(value);
    setResult(result);
}
/**
 * setResult(int)
 */
void
CTclCommand::setResult(int value)
{
    Tcl_Obj* result = Tcl_NewIntObj(value);
    setResult(result);
}
/**
 * getInteger
 *   Get an integer value from an object.
 * @param intObj - the object.
 * @return int   - the value.
 * @throw std::string - if the object does not have a valid integer.
 */
int
CTclCommand::getInteger(Tcl_Obj* obj)
{
    int result;
    int status = Tcl_GetIntFromObj(m_pInterp, obj, &result);
    if (status != TCL_OK) {
        throw std::string("Parameter cannot be converted from string to integer");
    }
    return result;
}
/**
 * getDouble
 *    Get a double value from an object.
 *  @param dblObj - the object.
 *  @return double - the value.
 *  @throw std::string if the object does not have double rep.
 */
double
CTclCommand::getDouble(Tcl_Obj* dblObj)
{
    double result;
    int status = Tcl_GetDoubleFromObj(m_pInterp, dblObj, &result);
    if (status != TCL_OK) {
        throw std::string("Paramter cannot be converted from string to double");
    }
    return result;
}
/////////////////////////////////////////////////////////////////
/**
 * commandRelay
 *   - Muster the objects passd to us into a vector and
 *   - invoke operator()
 * @param pObject - client data that's actually a pointer to the command
 *                  object who's operator() is called.
 * @param pInterp - Pointer to the intepreter executing the command
 *                 (ignored)
 * @param objc    - number of command line words.
 * @para objv     - vector of pointers to Tcl_Obj's that make up the
 *                  command.
 * @return int    - the return value from operator()
 * @note operator() is responsible for setting any intepreter result.
 * 
 */
int
CTclCommand::commandRelay(
    ClientData pObject, Tcl_Interp* pInterp,
    int objc, Tcl_Obj* const objv[]
)
{
    std::vector<Tcl_Obj*> args;
    for (int i =0; i < objc; i++) {
        args.push_back(objv[i]);
    }
    CTclCommand* p = reinterpret_cast<CTclCommand*>(pObject);
    return (*p)(args);
}
