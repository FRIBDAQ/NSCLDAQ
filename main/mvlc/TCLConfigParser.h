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
#inlcude <map>
// Forward class definitions:

class CTCLInterpreter;
class CStack;
class CReadoutHardware
class CTCLObjectProcessor;

class TCLConfigParser {
    // Private data:
private:
    CTCLInterpreter*                  m_pInterp;
    std::string                       m_daqconfigFile;
    std::vector<CTCLObjectProcessor>  m_commandExtensions;   //< commands added for modules.
    CStack*                            m_pEventStack;        //< Modules in the event stack.
    CStack*                            m_pScalerStack;       //< Modlues in the scaler stack.
    std::map <std::string, CReadoutHardware*> m_modules;     //< Soup of modules.

    // Canonicals:
public:
    TCLConfigParser(const std::string infile);
    virtual ~TCLConfigParser);                             // Virtual in case we need to derive for tests.

    // Forbidden canonicals:
private:
    TCLConfigParser(const TCLConfigParser& rhs);
    TCLConfigParser& operator=(const TCLConfigParser& rhs);
    int operator==(const TCLConfigParser& rhs);
    int operator!=(const TCLConfigParser& rhs);

    // Methods.  Some are virtual just because it makes testing possible.

    virtual operator();                                // Process the input file.

    // utilities -some might be virtual to support replacement in derived test objects.
private:

};

#endif