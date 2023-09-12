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

/** @file:  NSCLDAQLog.cpp
 *  @brief: Thin wrappers around Boost logging for NSCLDAQ
 */

/* Note that config.h has set HAVE_BOOST_LOG if boot logging is installed.
 */
#define HAVE_BOOST_LOG   /// Cheat.

#include "NSCLDAQLog.h"
#include <stdexcept>
#include <CMutex.h>
#include <time.h>
#include <string.h>
#include <ios>

#ifdef HAVE_BOOST_LOG

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>

#endif

// The following static members are used in describing the
// filtering and logfile setup when the singleton private logging object
// is instantiated.  They must be set to valid values before the first
// logging call is initiated; else logic_error is thrown.

static daqlog::logLevel defaultLevel(daqlog::Info);
static daqlog::logLevel loggingLevel(defaultLevel);
static std::string logFile;

// Since no header is supplied, this class is only used by
// the api exported by NSCLDAQLog.h


/**
 *   Boost logging singleton wrapper class:
 */

namespace daqlog {
  void reset();
    class BoostLogWrapper {
#ifdef HAVE_BOOST_LOG
    private:
       boost::shared_ptr< boost::log::sinks::synchronous_sink< boost::log::sinks::text_file_backend > > m_pLogSink ;
#endif
    private:
        static BoostLogWrapper* m_pInstance;
        BoostLogWrapper();
    public:
        static BoostLogWrapper* getInstance();
        void Log(logLevel level, const char* msg);
        void Log(logLevel level, const std::string& msg) {
            Log(level, msg.c_str());
        }
    public:
#ifdef HAVE_BOOST_LOG
        static boost::log::trivial::severity_level mapSeverity(logLevel level);
#endif
    static const char* severityString(logLevel level);

    friend void daqlog::reset();
};                           // daqlog::BoostLogWrapper

};                           // daqlog

// Implementation of daqlog::BoostLogWrapper

daqlog::BoostLogWrapper* daqlog::BoostLogWrapper::m_pInstance(0);

/**
 * constructor
 *    - Require logfile to be non-empty else logic error is thrown.
 *    - If HAVE_BOOST_LOGGING setup the logging sink and severity based
 *      filter.
 *  @note this is private invoked by getInstance when m_pInstance is null.
 */
daqlog::BoostLogWrapper::BoostLogWrapper()
{
    if (logFile.empty()) {
        throw std::logic_error("daqlog::BoostLogWrapper - no log file set");
    }
#ifdef HAVE_BOOST_LOG
    m_pLogSink = boost::log::add_file_log(
        boost::log::keywords::file_name = logFile.c_str(),
        boost::log::keywords::open_mode = std::ios_base::app | std::ios_base::out
        );
    boost::log::core::get()->set_filter(
        boost::log::trivial::severity >= mapSeverity(loggingLevel)
    );
#endif
}
/**
 * getInstance
 *   Return the singleton instance of the logger object.
 *   Note this is threadsafe.
 *
 * @return daqlog::BoostLogWrapper*
 */
static CMutex instanceLock;
daqlog::BoostLogWrapper*
daqlog::BoostLogWrapper::getInstance()
{
    {
        CriticalSection lock(instanceLock);
        if (!m_pInstance) {
            m_pInstance = new BoostLogWrapper();
        }
    }
    // m_pInstance is now thread-safe stable
    
    return m_pInstance;
}
/**
 * Log
 *   The heart of the logging subsystem;
 *   If HAVE_BOOST_LOG Is set, then we can log via the BOOST_LOG_TRIVIAL macro
 *   we just need to map the loggingLevel to the boost severity level.
 *
 * @param level - the daqlog::logLevel at which to log.
 * @param msg   - the messgae to log.
 */
void
daqlog::BoostLogWrapper::Log(daqlog::logLevel level, const char*msg)
{
#ifdef HAVE_BOOST_LOG
    boost::log::trivial::severity_level sev = mapSeverity(level);
    time_t now = time(nullptr);
    char timebuf[256];
    ctime_r(&now, timebuf);
    timebuf[strlen(timebuf) - 1] = '\0';  // Kill the \n
    BOOST_LOG_STREAM_WITH_PARAMS(
        ::boost::log::trivial::logger::get(),
        (boost::log::keywords::severity = sev)
    )
     << "(" << timebuf << ")  " <<  severityString(level)  << " : "  << msg ;
    m_pLogSink->flush();
#endif
}
/**
 * severityString
 *    Return a const char* pointing at a string matching the severit passed in.
 *  @param s - severity level
 *  @return const char*
 */
const char*
daqlog::BoostLogWrapper::severityString(daqlog::logLevel s) {
    switch (s) {
    case Trace: return "Trace";
    case Debug: return "Debug";
    case Info:  return "Info";
    case Warning: return "Warning";
    case Error: return "Error";
    case Fatal: return "Fatal";
    default:
        return "*Invalid severity level*";
    }
}
/**
 * mapSeverity
 *    Map from a daqlog::logLevel value to a
 *    boost::log::trivial::severity value.
 *
 * @param level - the daqlog::logLevel
 * @return boost::log::trivial::severity  - corresponding boost severy level.
 * @note the entire method ifdefed because we can't declare it if boostlog isn't
 *       presrent.
 */
#ifdef HAVE_BOOST_LOG       
boost::log::trivial::severity_level
daqlog::BoostLogWrapper::mapSeverity(daqlog::logLevel level)
{

    boost::log::trivial::severity_level result;
    switch (level) {
    case Trace:
        result = boost::log::trivial::trace;
        break;
    case Debug:
        result = boost::log::trivial::debug;
        break;
    case Info:
        result = boost::log::trivial::info;
        break;
    case Warning:
        result = boost::log::trivial::warning;
        break;
    case Error:
        result = boost::log::trivial::error;
        break;
    case Fatal:
        result = boost::log::trivial::fatal;
        break;
    default:
        throw std::range_error("Invalid value for daqplog::loglevel");
    }
    
    return result;
}
#endif

/*-----------------------------------------------------------------------------
 * API implementation.
 *   Each API element just gets the singleton and calls the Log method.
 */

 void daqlog::trace(const char* msg)
 {
    daqlog::BoostLogWrapper* pInstance = daqlog::BoostLogWrapper::getInstance();
    pInstance->Log(daqlog::Trace, msg);
 }
 
 void daqlog::debug(const char* msg)
 {
    daqlog::BoostLogWrapper* pInstance = daqlog::BoostLogWrapper::getInstance();
    pInstance->Log(daqlog::Debug, msg);
 }
 
 void daqlog::info(const char* msg)
 {
    daqlog::BoostLogWrapper* pInstance = daqlog::BoostLogWrapper::getInstance();
    pInstance->Log(daqlog::Info, msg);
 }
 
 void daqlog::warning(const char* msg)
 {
    daqlog::BoostLogWrapper* pInstance = daqlog::BoostLogWrapper::getInstance();
    pInstance->Log(daqlog::Warning, msg);
 }
 
 void daqlog::error(const char*  msg)
 {
    daqlog::BoostLogWrapper* pInstance = daqlog::BoostLogWrapper::getInstance();
    pInstance->Log(daqlog::Error, msg);
 }
 
 void daqlog::fatal(const char* msg)
 {
    daqlog::BoostLogWrapper* pInstance = daqlog::BoostLogWrapper::getInstance();
    pInstance->Log(daqlog::Fatal, msg);
 }
 
 void daqlog::setLogFile(const char* filename)
 {
    logFile = filename;    
 }
 
 void daqlog::setLogLevel(daqlog::logLevel level)
 {
    loggingLevel = level;
 }
 
 // For testing.
 
 namespace daqlog {
 void reset()
 {
    loggingLevel = defaultLevel;
    logFile = "";
    
    delete daqlog::BoostLogWrapper::m_pInstance;
    daqlog::BoostLogWrapper::m_pInstance = nullptr;
 }

 }

