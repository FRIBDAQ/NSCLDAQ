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
# @file   ringStatusDaemon.cpp
# @brief  Daemon that periodically pushes ring status information to the status aggregator.
# @author <fox@nscl.msu.edu>
*/

#include "ringstatus.h"
#include <zmq.hpp>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include <cstdlib>
#include <chrono>
#include <thread>

#include <CPortManager.h>
#include <Exception.h>
#include "CStatusMessage.h"
#include "CRingStatusDaemon.h"
#include <nsclzmq.h>


static const unsigned MAX_SERVICE_CHECKS(10);
static const unsigned SERVICE_CHECK_INTERVAL(2);
static const unsigned SERVICE_STARTUP_DELAY(5);
/**
 * translateService
 *    Given a service name use the port manager to figure out the port
 *    on which it's listening.
 *
 * @param serviceName - Name of the service to check out
 * @return unsigned   - port number.
 * @note exits on failure.  See MAX_SERVICE_CHECKS and SERVICE_CHECK_INTERVAL
 *       for failure criteria.
 */
static unsigned translateService(const char* serviceName)
{
    // See MAX_SERVICE_CHECKS and SERVICE_CHECK_INTERVAL above.
    
    CPortManager manager;
    for (int i = 0; i < MAX_SERVICE_CHECKS; i++) {
        std::vector<CPortManager::portInfo> info = manager.getPortUsage();
        
        // for_each and lambdas don't give an easy early loop termination when
        // we find the service so:
        
        for (size_t s = 0; s < info.size(); s++) {
            if (info[s].s_Application == serviceName) {
                return info[s].s_Port;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(SERVICE_CHECK_INTERVAL));
    }
    std::cerr << "Unable to translate service: " << serviceName << " giving up\n";
    std::exit(EXIT_FAILURE);
    
}
/**
 * main
 *    Program entry point.  This just parses parameters, figures out the port
 *    on which the aggregator is listening and
 *
 *  @param argc - number of command words.
 *  @param argv - the command words.
 */
int main(int argc, char** argv)
{
    
    gengetopt_args_info parsedArgs;    
    if (cmdline_parser(argc,argv, &parsedArgs)) {
        std::exit(EXIT_FAILURE);            // cmdline_parser says why and actually exits.
    }
    // Before doing anything interesting, wait for the local port manager
    // to fire up:
    
    if (!CPortManager::waitPortManager(10)) {
        std::cerr << "Local port manager is not running\n";
        std::exit(EXIT_FAILURE);
    }
    // There can be a macroscopic time between the port being advertised and
    // a listener available on it.  We'll delay another bit for that
    // to happen:
    
    
    // Figure out the port
    
    try {
        unsigned port = translateService(parsedArgs.service_arg);
	std::this_thread::sleep_for(std::chrono::seconds(SERVICE_STARTUP_DELAY));
        ZmqSocket&  pusher(*ZmqObjectFactory::createSocket(ZMQ_PUSH));
        
        std::stringstream uri;
        uri << "tcp://localhost:" << port;
        pusher->connect(uri.str().c_str());
        
        // here's where we start the application:
        
        CRingStatusDaemon app(pusher, parsedArgs.update_rate_arg);
        app();
    }
    catch(CException& e) {
        std::cerr << e.ReasonText();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
