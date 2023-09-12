/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file: NSCLDAQLog.h
 *  @brief: Define logging interface for NSCLDAQ programs.
 */

#ifndef DAQLOG_H
#define DAQLOG_H
#include <string>

namespace daqlog {
    
    // Log severities for filtering:
    
    enum logLevel {
        Trace, Debug, Info, Warning, Error, Fatal
    };
    void setLogLevel(logLevel log);

    
    void trace(const char* msg);
    inline void trace(const std::string& msg) {
        trace(msg.c_str());
    }
    
    void debug(const char*  msg);
    inline void debug(const std::string& msg) {
        debug(msg.c_str());
    }
    
    void info(const char* msg);
    inline void info(const std::string& msg) {
        info(msg.c_str());
    }
    
    void warning(const char* msg);
    inline void warning(const std::string& msg) {
        warning(msg.c_str());
    }
    
    void error(const char* msg);
    inline void error(const std::string& msg) {
        error(msg.c_str());
    }
    
    void fatal(const char* msg);
    inline void fatal(const std::string& msg) {
        fatal(msg.c_str());
    }
    
    
    // Log sink (file).    
    
    void setLogFile(const char* filename);
    inline void setLogFile(const std::string& filename) {
        setLogFile(filename.c_str());
    }
    
};
#endif