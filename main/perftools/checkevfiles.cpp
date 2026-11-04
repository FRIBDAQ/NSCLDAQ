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

/** @file:  checkevfiles.cpp
 *  @brief: Check that physics event timestamps increment by one across a run.
 */


/**
 *  Reads the event file segments of a run and verifies that the body-header
 *  timestamp of each PHYSICS_EVENT increments by one.  A mismatch is reported
 *  to stderr and timestamp tracking resynchronizes to the value just seen.
 *
 *    The program accepts a single parameter: the run number.  It opens the
 *    segments run-NNNN-SS.evt (SS = 00, 01, ...) in the current directory
 *    until a segment cannot be opened.
 */

// header files:

// NSCLDAQ Headers:

#include <CDataSource.h>              // Abstract source of ring items.
#include <CDataSourceFactory.h>       // Turn a URI into a concrete data source
#include <CRingItem.h>                // Base class for ring items.
#include <DataFormat.h>                // Ring item data formats.
#include <Exception.h>                // Base class for exception handling.

// standard run time headers:

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>

static void
processRingItem(CRingItem& item);   // Forward definition, see below.

static std::string makeUri(const char* runText, int segment)
{
    char uri[1000];
    int run = atoi(runText);
    sprintf(uri, "file://./run-%04d-%02d.evt", run, segment);
    return std::string(uri);
    
}

/**
 * Usage:
 *    This outputs an error message that shows how the program should be used
 *     and exits using std::exit().
 *
 * @param o   - references the stream on which the error message is printed.
 * @param msg - Error message that precedes the usage information.
 */
static void
usage(std::ostream& o, const char* msg)
{
    o << msg << std::endl;
    o << "Usage:\n";
    o << "  checkevfiles run\n";
    o << "      run - Number of the run - which must be in the cwd";
    std::exit(EXIT_FAILURE);
}


/**
 * The main program:
 *    - Ensures we have exactly one parameter (the run number).
 *    - Walks the run's segments (run-NNNN-SS.evt), constructing a data source
 *      for each in turn.
 *    - Reads items from each segment, checking PHYSICS_EVENT timestamps, until
 *      a segment cannot be opened (end of run).
 *
 *  @note The timestamp check assumes timestamps increment by one per event.
 */

int
main(int argc, char** argv)
{
    // Make sure we have enough command line parameters.
    
    if (argc != 2) {
        usage(std::cerr, "Not enough command line parameters");
    }
    // Create the data source.   Data sources allow us to specify ring item
    // types that will be skipped.  They also allow us to specify types
    // that we may only want to sample (e.g. for online ring items).
    
    std::vector<std::uint16_t> sample;     // Insert the sampled types here.
    std::vector<std::uint16_t> exclude;    // Insert the skippable types here.
    CDataSource* pDataSource;
    std::string uri;
    int segment(0);
    
    while (1) {
        uri = makeUri(argv[1], segment);
        std::cerr << "Processing segment " << segment << " (" << uri << ")\n";
        try {
            
            pDataSource =
                CDataSourceFactory::makeSource(uri, sample, exclude);
        }
            catch (CException& e) {
                std::cerr << "Failed to open event source: " << uri <<std::endl;
                exit(EXIT_SUCCESS);
        }
        // The loop below consumes items from the ring buffer until
        // all are used up.  The use of an std::unique_ptr ensures that the
        // dynamically created ring items we get from the data source are
        // automatically deleted when we exit the block in which it's created.
        
        CRingItem*  pItem;
        while ((pItem = pDataSource->getItem() )) {
            std::unique_ptr<CRingItem> item(pItem);     // Ensures deletion.
            processRingItem(*item);
        }
        delete pDataSource;
        pDataSource = nullptr;
        segment++;
    }
    // We can only fall through here for file data sources... normal exit
    
    std::exit(EXIT_SUCCESS);
}
/**
 * processRingItem.
 *    Modify this to put whatever ring item processing you want.
 *    In this case, we just output a message indicating when we have a physics
 *    event.  You  might replace this with code that decodes the body of the
 *    ring item and, e.g., generates appropriate root trees.
 *
 *  @param item - references the ring item we got.
 */
static void
processRingItem(CRingItem& item)
{
    static uint64_t ExpectedTimestamp(1);
    if (item.type() == PHYSICS_EVENT) {
        if (item.hasBodyHeader()) {
            uint64_t actualTs = item.getEventTimestamp();
            if (ExpectedTimestamp != actualTs) {
                std::cerr << "TS Mismatch. Expected "
                    << ExpectedTimestamp << " got " << actualTs << std::endl;
                ExpectedTimestamp = actualTs + 1;
            } else {
                ExpectedTimestamp++;
            }
        } else {
            std::cerr << "Expecting timestamp " << ExpectedTimestamp
                << " got a physics item with no body header\n";
            
        }
    }
}