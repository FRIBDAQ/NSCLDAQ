/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     Aaron Chester
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/**
 * @file ddasdumper.cpp
 * @brief Main program to use the format library to dump DDAS event files.
 * Based on the unified format library evtdump example code.
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <memory>
#include <algorithm>
#include <fstream>

#include <NSCLDAQFormatFactorySelector.h>
#include <DataFormat.h>
#include <RingItemFactoryBase.h>

// These are headers for the abstrct ring items we can get back from the
// factory. As new ring items are added this set of #include's must be
// updated as well as the switch statement in the dumpItem  method.

#include <CRingItem.h>
#include <CAbnormalEndItem.h>
#include <CDataFormatItem.h>
#include <CGlomParameters.h>
#include <CPhysicsEventItem.h>
#include <CRingFragmentItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CRingStateChangeItem.h>
#include <CUnknownFragment.h>

// End of ring item type headers

#include <URL.h>

#include "dumperargs.h"
#include "DataSource.h"
#include "FdDataSource.h"
#include "StreamDataSource.h"
#include "RootFileDataSink.h"

using namespace ufmt;

// Map of exclusion type strings to type integers:

static std::map<std::string, uint32_t> TypeMap = {
    {"BEGIN_RUN", BEGIN_RUN},
    {"END_RUN", END_RUN},
    {"PAUSE_RUN", PAUSE_RUN},
    {"RESUME_RUN", RESUME_RUN},
    {"ABNORMAL_ENDRUN", ABNORMAL_ENDRUN},
    {"PACKET_TYPES", PACKET_TYPES},
    {"MONITORED_VARIABLES", MONITORED_VARIABLES},
    {"RING_FORMAT", RING_FORMAT},
    {"PERIODIC_SCALERS", PERIODIC_SCALERS},
    {"INCREMENTAL_SCALERS", INCREMENTAL_SCALERS},
    {"TIMESTAMPED_NONINCR_SCALERS", TIMESTAMPED_NONINCR_SCALERS},
    {"PHYSICS_EVENT", PHYSICS_EVENT},
    {"PHYSICS_EVENT_COUNT", PHYSICS_EVENT_COUNT},
    {"EVB_FRAGMENT", EVB_FRAGMENT},
    {"EVB_UNKNOWN_PAYLOAD", EVB_UNKNOWN_PAYLOAD},
    {"EVB_GLOM_INFO", EVB_GLOM_INFO}
};

/**
 * @brief "Tokenize" a delimited string into a vector. Shamelessly stolen 
 * from https://www.techiedelight.com/split-string-cpp-using-delimiter/.
 * @param str   String to split up.
 * @param delim Delimimeter on which to split the string.
 * @return Tokenized vector of strings.
 */
static std::vector<std::string>
tokenize(std::string const &str, const char delim)
{
    std::vector<std::string> out;
    size_t start;
    size_t end = 0;
 
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
    
    return out;
}

/**
 * @brief Creates a vector of the ring item types to be excluded from the dump 
 * given a comma separated list of types.
 * @param exclusions - string containing the exclusion list.
 * @throw std::invalid_argument an exclusion item is not a string and is not 
 *   in the map of recognized item types.
 * @return Vector of items to exclude.
 * @details
 * A type can be a string or a positive number. If it is a string, it is 
 * translated to the type id using TypeMap. If it is a number, it is used 
 * as is.                
 */
std::vector<uint32_t>
makeExclusionList(const std::string& exclusions)
{
    std::vector<uint32_t> result;
    std::vector<std::string> words = tokenize(exclusions, ',');
    
    // Process the words into an exclusion list:
    for (auto s : words) {
        bool isInt(true);
        int intValue;
        try {
            intValue = std::stoi(s);
        }
        catch (...) { // Failed conversion to int
            isInt = false;
        }
        if (isInt) {
            result.push_back(intValue);
        } else {
            auto p = TypeMap.find(s);
            if (p != TypeMap.end()) {
                result.push_back(p->second);
            } else {
                std::string msg("Invalid item type in exclusion list: ");
                msg += s;
                throw std::invalid_argument(msg);
            }
        }
    }
    
    return result;
}

/**
 * @brief Map the version we get from the command line to a factory version.
 * @param fmtIn Format the user requested.
 * @throw std::invalid_argument Bad format version
 * @return Factory version ID (from the enum).
 * @note We should never throw because gengetopt will enforce the enum.
 */
static FormatSelector::SupportedVersions
mapVersion(enum_nscldaq_format fmtIn)
{
    switch (fmtIn) {
    case nscldaq_format_arg_12:
	return FormatSelector::v12;
    case nscldaq_format_arg_11:
	return FormatSelector::v11;
    case nscldaq_format_arg_10:
	return FormatSelector::v10;
    default:
	throw std::invalid_argument("Invalid DAQ format version specifier");
    }
}

/**
 * @brief Parse the URI of the source and based on the parse create the 
 * underlying connection. Create the correct concrete instance of DataSource 
 * given all that.
 * @param pFactory Pointer to the ring item factory to use.
 * @param strUrl   String URI of the connection.
 * @throw std::invalid_argument If a ringbuffer data source is requested.
 *   The unified format library is incorporated into the NSCLDAQ, but does
 *   not have NSCLDAQ support enabled as its installed first.
 * @return Dynamically allocated data source.
 */
DataSource*
makeDataSource(::ufmt::RingItemFactoryBase* pFactory, const std::string& strUrl)
{
    // Special case the url is just "-" then it's stdin, a file descriptor
    // data source:
    
    if (strUrl == "-") {
        return new FdDataSource(pFactory, STDIN_FILENO);   
    }
    
    // Parse the URI:    
    
    URL uri(strUrl);
    std::string protocol = uri.getProto();

    // URI could be for a ringbuffer or file:
    
    if ((protocol == "tcp") || (protocol == "ring")) {
	std::string msg(
	    "Ringbuffer support is not enabled for this version of "
	    "ddasdumper. To read data directly from a ringbuffer, "
	    "create a pipe to read from stdin: ringselector | ddasdumper -"
	    );
	throw std::invalid_argument(msg);
    } else {
        std::string path = uri.getPath();
	// Need it to last past block:
        std::ifstream& in(*(new std::ifstream(path.c_str())));
	if (!in.good()) {
	    std::string msg("Failed to create input stream from ");
	    msg += path;
	    throw std::invalid_argument(msg);	    
	}
        return new StreamDataSource(pFactory, in);
    }
}

/**
 * @brief Process PHYSICS_EVENT data and dump items.
 * @param pItem   Pointer to the item.
 * @param factory Reference to the factory appropriate to the format.
 * @param pSink   Pointer to the data sink we're writing to.
 * @throw std::logic_error If the wrong item type is specified.
 * @details
 * Steps performed by this function:
 * - Based on the item type, use the factory to get a new item using the same 
 *   data for the appropriate type.
 * - Process PHYSICS_EVENT items and write them to a data sink.
 * - Process selected event types and dump them to stdout using their 
 *   `toString()` method.
 * @note This method is rather long but this is only due to the switch 
 *   statement that must handle every possible ring item type in DataFormat.h. 
 *   The actual code is really quite simple (I think).
 * @note Use of std::unique_ptrs ensure that temporary, specific ring item 
 *   objects are automatically deleted.
 * @note EVB_FRAGMENT, EVB_UNKNOWN_PAYLOAD types are ignored.
 */
static void
dumpItem(
    ::ufmt::CRingItem* pItem, ::ufmt::RingItemFactoryBase& factory,
    RootFileDataSink* pSink
    )
{
    /** 
     * @todo (ASC 3/26/24): Dump quietly (without calling the various 
     * processXXX's).
     */
    
    // Note that the switch statement here assumes that if you have a ring
    // item type the factory can generate it... this fails if the wrong
    // version of the factory is used for the event file.
    
    switch(pItem->type()) {
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
    {
	std::unique_ptr<CRingStateChangeItem>
	    p(factory.makeStateChangeItem(*pItem));
	std::cout << p->toString() << std::endl;
    }
    break;
    case ABNORMAL_ENDRUN:
    {
    	std::unique_ptr<CAbnormalEndItem>
	    p(factory.makeAbnormalEndItem(*pItem));
	std::cout << p->toString() << std::endl;
    }
    break;
    case PACKET_TYPES:
    case MONITORED_VARIABLES:
    {
	std::unique_ptr<CRingTextItem> p(factory.makeTextItem(*pItem));
	std::cout << p->toString() << std::endl;
    }
    break;
    case RING_FORMAT:
    {
	try {
	    std::unique_ptr<CDataFormatItem>
		p(factory.makeDataFormatItem(*pItem));
	    std::cout << p->toString() << std::endl;
	}
	catch (std::bad_cast e) {
	    throw std::logic_error(
		"Unable to dump a data format item... likely you've "
		"specified the wrong --nscldaq-format"
		);
	}
    }
    break;
    case PERIODIC_SCALERS:
        // case INCREMENTAL_SCALERS: // Same value as PERIODIC_SCALERS.
    case TIMESTAMPED_NONINCR_SCALERS:
    {
	std::unique_ptr<CRingScalerItem> p(factory.makeScalerItem(*pItem));
	std::cout << p->toString() << std::endl;
    }
    break;
    case PHYSICS_EVENT:
    {
	// This item type gets written:
	std::unique_ptr<CPhysicsEventItem>
	    p(factory.makePhysicsEventItem(*pItem));
	pSink->putItem(*p.get());
    }
    break;
    case PHYSICS_EVENT_COUNT:
    {
	std::unique_ptr<CRingPhysicsEventCountItem>
	    p(factory.makePhysicsEventCountItem(*pItem));
	std::cout << p->toString() << std::endl;
    }
    break;
    case EVB_FRAGMENT:
    case EVB_UNKNOWN_PAYLOAD:
	break; // Ignored
    case EVB_GLOM_INFO:
    {
	std::unique_ptr<CGlomParameters> p(factory.makeGlomParameters(*pItem));
	std::cout << p->toString() << std::endl;
    }
    break;
    default:
	std::cout << pItem->toString() << std::endl;
	break;
    }
}

/**
 * @brief Setup, configure dumper settings and dump events.
 * @param argc, argv Argument count and argument vector.
 * @return EXIT_SUCCESS Successful event dump
 * @note All handled exceptions cause immidiate termination via 
 *   `std::exit(EXIT_FAILURE)`.
 */
int
main(int argc, char* argv[])
{
    try {
	gengetopt_args_info args;
        cmdline_parser(argc , argv, &args);

	// Figure out the parameters:

	std::string dataSource(args.source_arg); // Must be file:// or -
	std::string dataSink(args.fileout_arg);  // Not a URI
	int skipCount = args.skip_given ? args.skip_arg : 0;
	int dumpCount = args.count_given ? args.count_arg : 0;
	std::string excludeItems = args.exclude_arg;
        std::vector<uint32_t> exclusionList = makeExclusionList(excludeItems);
        int scalerBits = args.scaler_width_arg;
        FormatSelector::SupportedVersions version
	    = mapVersion(args.nscldaq_format_arg);

	// Factory for this format:
	
        auto& factory = FormatSelector::selectFactory(version);

	// Use the source name and factory to create a data source:
        
        std::unique_ptr<DataSource> pSource(
	    makeDataSource(&factory, dataSource)
	    );

	// Use the sink name and factory to create a ROOT file data sink:

	std::unique_ptr<RootFileDataSink> pSink(
	    new RootFileDataSink(&factory, dataSink.c_str())
	    );
        
        // Proces the scalerBits value into a
	// ::CRingScalerItem::m_ScalerFormatMask:
        
        uint64_t sbits = 1;
        sbits = sbits << scalerBits;
        sbits--;
        ::CRingScalerItem::m_ScalerFormatMask = sbits;

	// If there's a skip count skip exactly that many items:
	
	if (skipCount > 0) {
            for (int i = 0; i < skipCount; i++) {
                std::unique_ptr<::ufmt::CRingItem> p(pSource->getItem());
                if (!p.get()) { // End of source.
		    std::cout << "End of data source encountered while "
			      << "processing --skip items, exiting.\n";
		    return EXIT_SUCCESS;
                }
            }
        }

	// Now dump the items that are not excluded and if there's a dumpCount
        // only dump that many items... or until the end of the data source:

	int remaining = dumpCount;
	while (1) {
	    std::unique_ptr<::ufmt::CRingItem> pItem(pSource->getItem());
            if (!pItem.get()) { // End of source.
		return EXIT_SUCCESS;
            }

	    // Check the exclusion list and dump the item if allowed:
	    
	    if (std::find(
		    exclusionList.begin(), exclusionList.end(), pItem->type()
		    ) == exclusionList.end()) {

		// Dumpable:
		
                dumpItem(pItem.get(), factory, pSink.get());
                
                // Apply any limit to the count:
                
                if (args.count_given) {
                    remaining--;
                    if(remaining <= 0) {
                        return EXIT_SUCCESS;
                    }
                }
            }
	} // End of the processing loop
	
    }
    catch (const std::exception& e) {
	std::cerr << "ddasdumper main caught a C++ exception: " << e.what()
		  << std::endl;
	std::exit(EXIT_FAILURE);
    }
    catch (...) {
	std::cerr << "ddasdumper main caught an unexpected exception!\n";
	std::exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}
