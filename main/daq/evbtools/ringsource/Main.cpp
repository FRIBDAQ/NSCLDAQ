/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
* @file Main.cpp
* @brief  main program entry point.
*/

#include "rfcmdline.h"
#include "CRingFragmentSource.h"

#include <CEventOrderClient.h>
#include <Exception.h>
#include <CRemoteAccess.h>
#include <CRingBuffer.h>

#include <stdlib.h>
#include <iostream>
#include <list>


/**
 * Entry point:
 *  - Parse the command line.
 *  - Setup the connection to the event builder.
 *  - Connect as a consumer ot the ringbuffer.
 *  - Start the fragment source main application.
 */
int
main(int argc, char** argv)
{
    gengetopt_args_info args;
    cmdline_parser(argc, argv, &args);  // exits on error.
    
    // The connection to the event builder needs
    // the host, port (could be managed)
    // port name and info string, and the source list.
    
    std::string evbHost = args.evbhost_arg;
    std::string evbport = args.evbport_arg;
    std::string info    = args.info_arg;
    std::list<int> ids;
    for (int i =0; i < args.ids_given; i++) {
        ids.push_back(args.ids_arg[i]);
    }
    // If the port is "managed" we need to
    // translate it into a value:
    
    uint16_t port;
    try  {
        if (evbport == "managed") {
            if (!args.evbname_given) {
                std::cerr <<
                    "To lookup a managed port --evbname is required\n";
                exit(EXIT_FAILURE);
            }
            std::string evbname = args.evbname_arg;
            port = CEventOrderClient::Lookup(
                evbHost, evbname.c_str()
            );
        } else {
            port = atoi(evbport.c_str());
        }
    } catch(CException& e) {
        std::cerr << "Failed to get event builder port number";
        std::cerr << std::endl << e.ReasonText() << std::endl;
        exit(EXIT_FAILURE);
    }
    CEventOrderClient client(evbHost, port);
    try {
        client.Connect(info, ids);
    }
    catch (CException& e) {
        std::cerr << "Failed to connect to the event builder:";
        std::cerr << std::endl << e.ReasonText() << std::endl;
        exit(EXIT_FAILURE);
    }
    // To get a ring buffer connection we need the ring URI.
    

    std::string ringUri = args.ring_arg;
    CRingBuffer* pRing;
    try {
        pRing = CRingAccess::daqConsumeFrom(ringUri);
    }
    catch (CException& e) {
        std::cerr << "Unable to connect to the ring buffer " << ringUri << "\n";
        std::cerr << e.ReasonText() << std::endl;
    }
    // Validate the divisor value:

    if (args.ts_divisor_arg <= 0) {
        std::cerr << "Inavlid --ts-divisor value,  must be an integer > 0\n";
        exit(EXIT_FAILURE);
    }
    // Create our data source object and start it:
    
    CRingFragmentSource source(
        client, *pRing,
        ids, args.timestampextractor_arg, args.expectbodyheaders_flag,
        args.oneshot_arg, args.timeout_arg, 
        args.offset_arg, args.ts_divisor_arg, args.default_id_arg
    );
    source();
    
    client.disconnect();
    delete pRing;
    exit(EXIT_SUCCESS);    
}
