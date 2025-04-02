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


// Stub classes - remove them as the real classes get defined:

class CReadoutModule {};
class CStack {};

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
 */
void
TCLConfigParser::operator()() {
    m_pInterp->EvalFile(m_daqconfigFile);
}

// Services for device commands:



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