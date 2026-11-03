/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  evbfilecheck.cpp
 *  @brief: Check correctness of eventbuilder files
 */

/**
 *  This program makes _big_ assumptions.  The assumption is that:
 *  The timestamp ticks increment by 1 for each event,
 *  The dt for glom is 1.
 *  There are two data sources,
 *  All data sources are always in coincidence  except at the end where
 *  one source may run longer than the others and produce singles.
 *
 *  This program,
 *    - Reports the number of events that don't match that description
 *      which are followed by an event that does follow that description.
 *    -  Writes to file the events  that are like that.
 
*/
 
#include <CDataSource.h>              // Abstract source of ring items.
#include <CDataSourceFactory.h>       // Turn a URI into a concrete data source
#include <CRingItem.h>                // Base class for ring items.
#include <CPhysicsEventItem.h>        // Physics item.
#include <DataFormat.h>                // Ring item data formats.
#include <Exception.h>                // Base class for exception handling.
#include <CRingItemFactory.h>

#include <FragmentIndex.h>        // From SpecTcl 5.x  e.g

// standard run time headers:

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstdint>
#include <set>

static void
processRingItem(CRingItem& item);   // Forward definition, see below.


// This is the list of items that are 'not good'.

static std::vector<CPhysicsEventItem> badItems;
static std::vector<std::string>       wrongness;

static std::ofstream badfile("badItems.txt");

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
    o << "  readrings uri\n";
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
    while ((pItem = pDataSource->getItem() )) {
        std::unique_ptr<CRingItem> item(pItem);     // Ensures deletion.
        processRingItem(*item);
    }
    // We can only fall through here for file data sources... normal exit
    
    std::exit(EXIT_SUCCESS);
}
/**
 * goodItem
 *    The item must:
 *    - Have a timestamp that is 2 greater than the last one.
 *    - Have two fragments with timestamp 1 greater than the last; one from
 *      event source 0, and one from event source 2 then two fragments
 *      with timestamp equal to the event timestamp, again  one from
 *      sid 0 and one from sid 2.
 *
 * @param item - references the ring itme.
 * @return bool - true if all criteria above are satisfied else false.
 */

static uint64_t lastTimestamp = 0;
bool goodItem(CPhysicsEventItem& item)
{
    uint64_t expectedTimestamp = lastTimestamp + 2;
    uint64_t eventTimestamp = item.getEventTimestamp();
    if (expectedTimestamp != eventTimestamp) {
        badItems.push_back(item);
        std::stringstream reason;
        reason << "Timestamp mismatch expected: " << expectedTimestamp
            << " got " << eventTimestamp;
        wrongness.push_back(reason.str());
        lastTimestamp = eventTimestamp;      // Try to resynch.
        return false;                        // No good timestamp.
    }
    lastTimestamp = eventTimestamp;          // do it here in case the goodness fails.
    
    // Get ready to examine the fragments:
    
    FragmentIndex frags(static_cast<uint16_t*>(item.getBodyPointer()));
    
    if (frags.getNumberFragments() != 4) {
        badItems.push_back(item);
        wrongness.push_back(std::string("There are not 4 fragments in this event"));
        return false;
    }
    // First two frags should have expectedTimestamp -1 and be from sources 0,2
    // though we don't know what order.
    
    std::set<uint32_t>   sids;
    
    FragmentInfo frag0 = frags.getFragment(0);
    FragmentInfo frag1 = frags.getFragment(1);
    uint64_t expectedFragTimestamp = expectedTimestamp -1;
    if (
        (frag0.s_timestamp != expectedFragTimestamp) ||
        (frag1.s_timestamp != expectedFragTimestamp)
    ) {
        badItems.push_back(item);
        std::stringstream reason;
        reason << " Fragment 1/0 has an unexpected timestamp:  Should be: "
            << expectedFragTimestamp << " frag 0 " << frag0.s_timestamp
            << " frag1 " << frag1.s_timestamp;
        wrongness.push_back(reason.str());
        return false;
    }
    sids.emplace(frag0.s_sourceId);
    sids.emplace(frag1.s_sourceId);
    if ((sids.count(0) == 0) || (sids.count(2) == 0)) {
        badItems.push_back(item);
        std::stringstream reason;
        reason << " First 2 frags should have sids 0, and 2 but got: "
            << " frag0: " << frag0.s_sourceId << " frag1: " << frag1.s_sourceId;
        wrongness.push_back(reason.str());
        return false;
    }
    
    // The last two frags (frag 2, 3) should both have the event timestamp
    // as we're using last for the policy.  they too should have sids 0, 2.
    
    sids.clear();
    FragmentInfo frag2 = frags.getFragment(2);
    FragmentInfo frag3 = frags.getFragment(3);
    if (
        (frag2.s_timestamp != eventTimestamp)  || (frag3.s_timestamp != eventTimestamp)
    ) {
        badItems.push_back(item);
        std::stringstream reason;
        reason << " Fragment 2/3 has an  unexpected timestamp.  Should be: "
            << eventTimestamp << " frag 2: " << frag2.s_timestamp
            << " frag 3 " << frag3.s_timestamp;
        wrongness.push_back(reason.str());
        return false;
    }
    sids.emplace(frag2.s_sourceId);
    sids.emplace(frag3.s_sourceId);
    if ((sids.count(0) == 0) || (sids.count(2) == 0)) {
        badItems.push_back(item);
        std::stringstream reason;
        reason << " Last 2 frags should have sids 0, and 2 but got: "
            << " frag2: " << frag2.s_sourceId << " frag3: " << frag3.s_sourceId;
        wrongness.push_back(reason.str());
        return false;
    }
    
    
    
    return true;
}
/**
 * dumpBadItem
 *    Dumps a single bad item to the file.
 *
 * @param i - the bad item index.
 */
void dumpBadItem(int i)
{
    badfile << "------------------------------------------------\n";
    badfile << wrongness[i] << std::endl;
    badfile << badItems[i].toString() << std::endl;

}
/**
 * dumpBadItems:
 *    Dump the bad items  and their reasons to the output file badfile.
 */
void dumpBadItems()
{
    if (badItems.size() != wrongness.size()) {
        std::cerr << "Can't dumpBadItems because there's a mismatch between \n";
        std::cerr << "the size of wrongness: " << wrongness.size()
            << " and the size of badItems: " << badItems.size() << std::endl;
        std::cerr << "This indicates an error in the goodItem() logic\n";
        std::exit(EXIT_FAILURE);
    }
    for (int i = 0; i < wrongness.size(); i++) {
        dumpBadItem(i);
    }
}
/**
 * At the end of the run, we _should_ have a sequence of bad items because
 * one source runs longer than the others.  These items should:
 * - differ in timestamp by 2 as before.
 * - Have two fragments.
 * - The fragments in all should be the same (from the longer  running source).
 *
 *  Any itesm which are not like this get dumped.
 */
void checkBadItemsAtEndRun()
{
    if (badItems.size() == 0) return;                  // Unlikely.
    badfile << "---------------- Items before end of run ----------------\n";
    
    // Establish the sourceid and base timestamp from the first bad item:
    
    CPhysicsEventItem first(badItems[0]);
    uint64_t expectedStamp = first.getEventTimestamp() - 2;  // set up for loop.
    FragmentIndex frags(static_cast<uint16_t*>(first.getBodyPointer()));
    if(frags.getNumberFragments() == 0) {
        badfile << "Checking end of run - first bad event has no fragments!!!!\n";
        dumpBadItem(0);
        std::exit(EXIT_FAILURE);
    }
    uint32_t expectedSid = frags.getFragment(0).s_sourceId;
    if (wrongness.size() != badItems.size()) {
        std::cerr << " End of run check: ";
        std::cerr << "Can't dumpBadItems because there's a mismatch between \n";
        std::cerr << "the size of wrongness: " << wrongness.size()
            << " and the size of badItems: " << badItems.size() << std::endl;
        std::cerr << "This indicates an error in the goodItem() logic\n";
        std::exit(EXIT_FAILURE);
    }
    
    // Now loop over all items.
    
    for (int i =0; i < badItems.size(); i++) {
        expectedStamp += 2;
        CPhysicsEventItem item(badItems[i]);
        if (expectedStamp != item.getEventTimestamp()) {
            badfile << "-------------------------- ERcheck\n";
            badfile << " Event timestamp mismatch - expected: "
                << expectedStamp  << " got " << item.getEventTimestamp()
                << std::endl << item.toString() << std::endl;
            expectedStamp = item.getEventTimestamp();
            continue;
        }
        FragmentIndex frags(static_cast<uint16_t*>(item.getBodyPointer()));
        if(frags.getNumberFragments() != 2) {
            badfile << "--------------------------- ERcheck\n";
            badfile << " Expected 2 fragments, but got " << frags.getNumberFragments()
                << std::endl << item.toString() << std::endl;
            continue;
        }
        FragmentInfo frag0 = frags.getFragment(0);
        FragmentInfo frag1 = frags.getFragment(1);
        uint32_t sid0 = frag0.s_sourceId;
        uint32_t sid1 = frag1.s_sourceId;
        
        if ((sid0 != expectedSid) || (sid1 != expectedSid)) {
            badfile << "------------------------- ERCheck\n";
            badfile << " Not all fragment source ids were as expected: " << expectedSid
              << " frag0: " << sid0 << " frag1: " << sid1 << std::endl;
            badfile << item.toString() << std::endl;
            continue;                    // In case we add tests later and forget.
        }
    }
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
    if (item.type() == PHYSICS_EVENT) {
        CPhysicsEventItem* pItem =
            dynamic_cast<CPhysicsEventItem*>(CRingItemFactory::createRingItem(item));
        if (goodItem(*pItem)) {
            if (badItems.size()) {
                dumpBadItems();
                badItems.clear();
                wrongness.clear();
            }
        }
        delete pItem;
    }
    // At the end of the run, we must ensure that all 'bad' items only have
    // contributions from one event source and that this source is the same for
    // all 'bad items'.
    
    if (item.type() == END_RUN) {
        checkBadItemsAtEndRun();
        badItems.clear();
        wrongness.clear();
        
    }
    
}

void* gpTCLApplication(nullptr);