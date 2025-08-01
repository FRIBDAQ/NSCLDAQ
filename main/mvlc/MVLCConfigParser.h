/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief TCLCOnfigParser header with the VMUSB commands registered.alignas
*/

#ifndef MVLC_MVLCCONFIGPARSER_H
#define MVLC_MVLCCONFIGPARSER_H
#include "TCLConfigParser.h"



/**
 *  @class MVLCConfigParser
 *    This class is a TCLConfigParser with all of the commands supported
 * to generate configuration files for the mvlc.  To add a new
 * device you need to:
 * 
 * 1.  Write a CReadoutHardware for that device that emits the correct VME operations.a
 * 2.  Write a DeviceCommand that creates a CReadoutModule with an instance of
 * that device encapsulsted.
 * 3.  Extend our addExtensions method to register that command.
 * 
 * @note this is an irregular singleton so plugins, e.g. can find it.
 *   The constructor is public but throws logic error if it's been
 * multiply constructed.  getInstance returns the singleton pointer
 * -- which could be null if called too soon.  Note that, in general
 * this isn't a problem as plug ins get loaded from the config script...
 * which gets executed by this instance.
 */
class MVLCConfigParser : public TCLConfigParser {

public:
    MVLCConfigParser(const std::string& script);
    virtual ~MVLCConfigParser();
private:
    MVLCConfigParser(const MVLCConfigParser&);
    MVLCConfigParser& operator=(const MVLCConfigParser&);
    int operator==(const MVLCConfigParser&);
    int operator!=(const MVLCConfigParser&);
public:
    virtual void addExtensions();

    static MVLCConfigParser* getInstance() { 
        return reinterpret_cast<MVLCConfigParser*>(TCLConfigParser::getInstance()); 
    }
};


#endif
