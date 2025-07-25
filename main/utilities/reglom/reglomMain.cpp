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

/** @file:  reglom.cpp
 *  @brief: Main program for the reglom program.
 */

#include "reglomopts.h"
#include "CMerge.h"


#include <CDataSource.h>
#include <CDataSourceFactory.h>
#include <cstdlib>
#include <Exception.h>
#include <ErrnoException.h>
#include <URL.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdio.h>



/**
 * reglom for usage see reglomopts.ggo   Note that URI's are allowed
 *        (required actually) on the command line to operate.
 *        The options are esssentially the same as for glom:
 *        
 *     -  --dt - specifies the number of ticks that constitute and event.
 *     -  --timestamp-policy - Specifies the policy used to create the
 *               timestamp for built events.  See glom.
 *     -  --sourceid - Specifies the timestamp that is put in the
 *               output events.
 *     -  --output  - Specifies the final resting place (filename) for the
 *                data.
 *
 *   In addition to these options (which are actually used to configure glom
 *   on the other end of a pipeline) the command can have a number (two or more
 *   makes sense) URIS for input data.  These will be configured as data sources
 *   from which data are read.  While these can be ring buffers, the access
 *   pattern of the data sources is best suited for file URIS so a warning
 *   will be emitted if a tcp URI is encountered in the list.
 */

// Lookup table for timestamp-policy strings:

static const char* timestampPolicies[] = {
    "earliest", "latest", "average"
};

/**
 * glomCommand
 *    Create the command that will run glom from the argument information.
 *    Note that we assume that the DAQBIN env variable is defined.
 *    For the most part this is just reconstructing the options
 *    that are for glom on top of the actual glom command.
 *
 * @param arginfo - the process command line parameters.
 * @return std::string - the command string.
 */

static std::string
glomCommand(const gengetopt_args_info& arginfo)
{
  if (!std::getenv("DAQBIN")) {
    std::cerr << "DAQBIN is not defined.  Source a daqsetup script\n";
    std::exit(EXIT_FAILURE);
  }
    std::stringstream cmd;
    cmd << "${DAQBIN}/glom --dt=" << arginfo.dt_arg
        << " --timestamp-policy=" << timestampPolicies[arginfo.timestamp_policy_arg]
        << " --sourceid=" << arginfo.sourceid_arg
        << " > " << arginfo.output_arg;
    
    return cmd.str();
}

/**
 * main:
 *    Entry point.
 *      - Parse the command line parameters.
 *      - Setup glom as a subprocess on the other end of a pipe.
 *      - Open all the data sources.
 *      - Pass control over to the orderer
 *
 *  @param argc - number of coammand line parameters.
 *  @param argv - array of pointers to command line parameters.
 *  @return int (EXIT_SUCCESS or EXIT_FAILURE).  
 */
int
main(int argc, char** argv)
{
    // Process the command line parameters into the gengetopt struct:
    
    gengetopt_args_info arginfo;
    cmdline_parser(argc, argv, &arginfo);             // Exits with message on error.
    
    std::string glom = glomCommand(arginfo);                 // Create the glom command.
    
    // Now collect the data sources:
    
    if (!(arginfo.inputs_num >= 1)) {
        std::cerr << "reglomming only makes sense with at least one input source\n";
        std::exit(EXIT_FAILURE);
    }
    std::vector<CDataSource*> dataSources;
    std::vector<std::uint16_t> sample;
    std::vector<std::uint16_t> exclude;
    int i;                            // So it's visible in the catch blocks
    try {
        for (i = 0; i < arginfo.inputs_num; i++) {
            std::string source(arginfo.inputs[i]);
            URL url(source);                     // Throws if bad URL
            if (url.getProto() == "tcp")  {
                std::cerr << "Warning data source: " << source << " is a ringbuffer\n";
                std::cerr << "reglom is intended to run over files -- continuing anyway.\n";
            }
            dataSources.push_back(
                CDataSourceFactory::makeSource(source, sample, exclude)
            );
        }
    }
    catch (CException &e) {
        std::cerr << "Failed to open data source: "  << arginfo.inputs[i] << std::endl;
        std::cerr << e.ReasonText() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch(std::string msg) {
        
        std::cerr << "Failed to open data source: "  << arginfo.inputs[i] << std::endl;
        std::cerr << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch (const char* msg) {
        std::cerr << "Failed to open data source: "  << arginfo.inputs[i] << std::endl;
        std::cerr << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch (...) {
        std::cerr << "Failed to open data source: "  << arginfo.inputs[i] << std::endl;
        std::cerr << " Unanticipated exception type caught" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    // Start off glom to glue together our sorted fragments:
    
    std::cout << "Spawning off glom as follows: " << glom << std::endl;
    FILE* glomPipe;
    
    try {
        glomPipe = popen(glom.c_str(), "w");
        if (glomPipe == nullptr) {
            throw CErrnoException("popen failure");
        }
    }
    catch (CErrnoException& e) {
        std::cerr << "Unable to start GLOM subprocess: ";
        std::cerr << e.ReasonText() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    // Now run the merge out to glom:
    
    try {
        CMerge merger(glomPipe, dataSources);
        merger();
    }
    catch (CException& e) {
        std::cerr << "Error Merging data sources: " << e.ReasonText() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch (std::string msg) {
        std::cerr << "Error Merging data sources: " << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch (const char* msg) {
        std::cerr << "Error Merging data sources: " << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch (...) {
    }
        
    std::exit(EXIT_SUCCESS);
}
