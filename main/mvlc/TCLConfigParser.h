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
#ifndef MVLC_TCLCONFIGPARSER_H
#define MVLC_TCLCONFIGPARSER_H

#include <string>
#include <vector>
#include <map>
// Forward class definitions:

class CTCLInterpreter;
class CStack;
class CReadoutModule;
class CTCLObjectProcessor;

/**
 *  @class TCLConfigParser
 *    This class uses an embedded, extended Tcl interpreter to process daqconfig files
 * used by VMUSBReadout into a form that allows them to be converted into .yaml config files
 * expected by fribdaq-readout (or Mesytec mdaq for that matter).
 * 
 * The interpreter is extended by command ensembles that are of the form:
 * 
 * \verbatim
 *    cmd create name ?configuration?
 *    cmd config name ?configuration?
 *    cmd cget name   ?config-option?  # slight extension to daqconfig.
 * \endverbatim
 * 
 * configuration in the command formats above are pairs of option-name/option-values.  e.g.
 * 
 *  -base 0x12340000  Might be an option that sets the base address of a module to 0x12340000
 * 
 * Key data:
 * 
 * *  m_pInterp is the Tcl interpreter that will be used to process the configuration file.
 * *  m_daqconfigFile is the daq configuration file that will be processed.
 * *  m_commandExtensions are the vector of command objects that implement the daqconfig commands.
 * *  m_pEventStack - is the stack that contain the set of modules that are to be read in 'event0' in
 * mvlc parlance - triggered by the event.
 * *  m_pScalerStack - is the stack that contains the modules that are periodically read by 'event1'
 * in mvlc parlance - these are typically scalers modules.
 * *  m_modules are all of the modules that have been created by the command extensions; indexed by their
 * names e.g. m_modules["adc"] will be the module created by e.g. cmd create adc. Note we call these
 * 'CReadoutHardware' in hopes of minimizing what we need to do to port code from usb/vmusb/daqconfig here.
 * 
 * In order to be testable, many functions are virtual that need not be so that they can be
 * overriden in test fixtures.
 * 
 * 
 * @note:  We use two step construction to improve testability. This is because virtual method dispatch 
 * does not operate in constructors.
 */


 
class TCLConfigParser {
    // Private data:
private:
    CTCLInterpreter*                   m_pInterp;
    std::string                        m_daqconfigFile;
    std::vector<CTCLObjectProcessor*>  m_commandExtensions;   //< commands added for modules.
    CStack*                            m_pEventStack;        //< Modules in the event stack.
    CStack*                            m_pScalerStack;       //< Modlues in the scaler stack.
    std::map <std::string, CReadoutModule*> m_modules;     //< Soup of modules.

    // Canonicals:
public:
    TCLConfigParser(const std::string& infile);
    virtual ~TCLConfigParser();                             // Virtual in case we need to derive for tests.

    // Forbidden canonicals:
private:
    TCLConfigParser(const TCLConfigParser& rhs);
    TCLConfigParser& operator=(const TCLConfigParser& rhs);
    int operator==(const TCLConfigParser& rhs);
    int operator!=(const TCLConfigParser& rhs);

    // Methods.  Some are virtual just because it makes testing possible.
public: 
    virtual void initialize();
    virtual void operator()();                                // Process the input file.

    // utilities -some might be virtual to support replacement in derived test objects.
protected:
    virtual void addExtensions();
    virtual void addExtension(CTCLObjectProcessor& cmdobj);       // Add one extension command.

};

#endif