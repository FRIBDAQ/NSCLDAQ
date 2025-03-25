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

/** @file:  process.cpp
 *  @brief: cookbook that shows how to read ringbuffers from file and dispatch
 *          for ring specific processing.
 */


/**
 *  This program shows how to get data from a source of ring items.
 *  The ring item source can be either a file or a ringbuffer (local or remote).
 *
 *    The program accepts a single parameter, The URI of the ringbuffer.
 */

// header files:

// NSCLDAQ Headers:

#include <CDataSource.h>              // Abstract source of ring items.
#include <CDataSourceFactory.h>       // Turn a URI into a concrete data source
#include <CRingItem.h>                // Base class for ring items.
#include <DataFormat.h>                // Ring item data formats.
#include <Exception.h>                // Base class for exception handling.
#include <CRingItemFactory.h>         // creates specific ring item from generic.
#include <CRingScalerItem.h>          // Specific ring item classes.
#include <CRingStateChangeItem.h>     //                 |
#include <CRingTextItem.h>            //                 |
#include <CPhysicsEventItem.h>         //                |
#include <CRingPhysicsEventCountItem.h> //               |
#include <CDataFormatItem.h>          //                 |
#include <CGlomParameters.h>          //                 |
#include <CDataFormatItem.h>          //             ----+----

// Headers for other modules in this program:

#include "processor.h"

// standard run time headers:

#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>

static void
processRingItem(CRingItemProcessor& procesor, CRingItem& item);   // Forward definition, see below.

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
    o << "  process uri\n";
    o << "      uri - the file: or tcp: URI that describes where data comes from\n";
    std::exit(EXIT_FAILURE);
}


/**
 * The main program:
 *    - Ensures we have a URI parameter (and only a URI parameter).
 *    - Constructs a data source.
 *    - Reads items from the data source until the data source is exhausted.
 *
 *  @note Online ringbuffers are never exhausted.  The program will just block
 *        when the ringbuffer is empty until the ring has new data.
 *  @note Because of the note above, this function never exits for online sources.
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
    try {
        pDataSource =
        CDataSourceFactory::makeSource(argv[1], sample, exclude);
    }
    catch (CException& e) {
        usage(std::cerr, "Failed to open ring source");
    }
    // The loop below consumes items from the ring buffer until
    // all are used up.  The use of an std::unique_ptr ensures that the
    // dynamically created ring items we get from the data source are
    // automatically deleted when we exit the block in which it's created.
    
    CRingItem*  pItem;
    CRingItemProcessor processor;
    
    while ((pItem = pDataSource->getItem() )) {
        std::unique_ptr<CRingItem> item(pItem);     // Ensures deletion.
        processRingItem(processor, *item);
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
 *  @param processor - references the ring item processor that handles ringitems
 *  @param item - references the ring item we got.
 */
static void
processRingItem(CRingItemProcessor& processor, CRingItem& item)
{
    // Create a dynamic ring item that can be dynamic cast to a specific one:
    
    CRingItem* castableItem = CRingItemFactory::createRingItem(item);
    std::unique_ptr<CRingItem> autoDeletedItem(castableItem);
    
    // Depending on the ring item type dynamic_cast the ring item to the
    // appropriate final class and invoke the correct handler.
    // the default case just invokes the unknown item type handler.
    
    switch (castableItem->type()) {
        case PERIODIC_SCALERS:
            {    
                CRingScalerItem& scaler(dynamic_cast<CRingScalerItem&>(*castableItem));
                processor.processScalerItem(scaler);
                break;
            }
        case BEGIN_RUN:              // All of these are state changes:
        case END_RUN:
        case PAUSE_RUN:
        case RESUME_RUN:
            {
                CRingStateChangeItem& statechange(dynamic_cast<CRingStateChangeItem&>(*castableItem));
                processor.processStateChangeItem(statechange);
                break;
            }
        case PACKET_TYPES:                   // Both are textual item types
        case MONITORED_VARIABLES:
            {
                CRingTextItem& text(dynamic_cast<CRingTextItem&>(*castableItem));
                processor.processTextItem(text);
                break;
            }
        case PHYSICS_EVENT:
            {
                CPhysicsEventItem& event(dynamic_cast<CPhysicsEventItem&>(*castableItem));
                processor.processEvent(event);
                break;
            }
        case PHYSICS_EVENT_COUNT:
            {
                CRingPhysicsEventCountItem&
                    eventcount(dynamic_cast<CRingPhysicsEventCountItem&>(*castableItem));
                processor.processEventCount(eventcount);
                break;
            }
        case RING_FORMAT:
            {
                CDataFormatItem& format(dynamic_cast<CDataFormatItem&>(*castableItem));
                processor.processFormat(format);
                break;
            }
        case EVB_GLOM_INFO:
            {
                CGlomParameters& glomparams(dynamic_cast<CGlomParameters&>(*castableItem));
                processor.processGlomParams(glomparams);
                break;
            }
        default:
            {
                processor.processUnknownItemType(item);
                break;
            }
    }
}
