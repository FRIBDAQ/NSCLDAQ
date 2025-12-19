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
# @file   CTclRingCommand.cpp
# @brief  Implement the ring Tcl command.
# @author <fox@nscl.msu.edu>
*/

#include "CTclRingCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CRingBuffer.h>
#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CDataFormatItem.h>
#include <CRingFragmentItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CGlomParameters.h>

#include <CRemoteAccess.h>
#include <CAllButPredicate.h>
#include <CDesiredTypesPredicate.h>
#include <CRingItemFactory.h>
#include <CAbnormalEndItem.h>
#include <CTimeout.h>

#include <tcl.h>

#include <limits>
#include <chrono>
#include <thread>
#include <iostream>
#include <utils.h>

using namespace std;

/**
 * construction
 *   @param interp - the interpreter on which we are registering the 'ring'
 *                   command.
 */
CTclRingCommand::CTclRingCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "ring", true) {}
    
/**
 * destruction:
 *    Kill off all the CRingItems in the m_attachedRings map.
 */
CTclRingCommand::~CTclRingCommand()
{
    while(! m_attachedRings.empty()) {
        CRingBuffer* pRing = (m_attachedRings.begin())->second;    // First item.
        delete pRing;
        m_attachedRings.erase(m_attachedRings.begin());
    }
}

/**
 * operator()
 *   Gains control when the command is executed.
 *
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @return int
 *   @retval TCL_OK - Normal return, the command was successful..
 *   @retval TCL_ERROR - Command completed in error.
 */
int
CTclRingCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // Put everything in a try/catch block. Errors throw strings that get put
    // in result:
    
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "Insufficient parameters");
        
        std::string subcommand = objv[1];
        if(subcommand == "attach") {
            attach(interp, objv);
            
        } else if (subcommand == "detach") {
            detach(interp, objv);
        } else if (subcommand == "get") {
            get(interp, objv);
        } else {
            throw std::string("bad subcommand");
        }
        
    } catch(std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    return TCL_OK;
}

/*----------------------------------------------------------------------------
 * Protected members (subcommand handlers)
 */

/**
 * attach
 *    Execute the ring attach command which attaches to a ring.
 *    *  Ensure there's a URI parameter.
 *    *  Connect to the ring creating a CRingBuffer object.
 *    *  Put the ring buffer object in the m_attachedRings map indexed by the
 *    *  ring URI.
 *
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @throw std::string error message to put in result string if TCL_ERROR
 *          should be returned from operator().
 */
void
CTclRingCommand::attach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    // need a URI:
    
    requireExactly(objv, 3, "ring attach needs a ring URI");
    std::string uri = objv[2];
    CRingBuffer* pRing(0);
    try {
        if (m_attachedRings.find(uri) != m_attachedRings.end()) {
            throw std::string("ring already attached");
        }
        pRing = CRingAccess::daqConsumeFrom(uri);
        m_attachedRings[uri] = pRing;
    }
    catch(std::string) {
        throw;
    }
    catch (...) {
        throw std::string("Failed to attach ring");
    }
}

/**
 * detach
 *    Execute the ring detach command:
 *    *  Ensure there's a URI parameter
 *    *  Use it to look up the ring in the m_attachedRings map (error if no match).
 *    *  delete the ring buffer object -- which disconnects from the ring.
 *    *  remove the map entry.
 *
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @throw std::string error message to put in result string if TCL_ERROR
 *          should be returned from operator().
 */
void
CTclRingCommand::detach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireExactly(objv, 3, "ring detach needs a URI");
    
    std::string uri = objv[2];
    std::map<std::string, CRingBuffer*>::iterator p = m_attachedRings.find(uri);
    if (p == m_attachedRings.end()) {
        throw std::string("ring is not attached");
    }
    CRingBuffer* pRing = p->second;
    m_attachedRings.erase(p);
    delete pRing;
}                             

/**
 * get
 *   Execute the ring get command (blocks until an item is available);
 *    * Ensure there's a ring URI parameter
 *    * Looks up the CRingBuffer in the map (error if no match).
 *    * Gets a CRingItem from the ring with the appropriate filter.
 *    * Produces a dict whose keys/contents will depend on the item type
 *      (which will always be in the -type key).  See the private formattting
 *      functions for more on what's in each dict.
 *   @param interp - reference to the interpreter that's executing the command.
 *   @param args   - The command line words.
 *
 *   @throw std::string error message to put in result string if TCL_ERROR
 *          should be returned from operator().
 */
void
CTclRingCommand::get(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    requireAtLeast(objv, 3, "ring get needs a URI");
    requireAtMost(objv, 6, "Too many command parameters");

    CAllButPredicate all;
    CDesiredTypesPredicate some;
    CRingSelectionPredicate* pred;
    pred = &all;                  // Unless otherwise specified don't restrict item types.
    
    // If there's a 4th parameter it must be a list of item types to select
    // from

    unsigned long timeout = std::numeric_limits<unsigned long>::max();

    size_t paramIndexOffset = 0;
    if (std::string(objv[2]) == "-timeout") {
        if (objv.size() >= 4) {
            CTCLObject object = objv[3];
            timeout = int(object.lindex(0));
        } else {
            throw std::string("Insufficient number of parameters");
        }
        paramIndexOffset = 2;
    }

    std::string uri  = std::string(objv[2+paramIndexOffset]);
    std::map<std::string, CRingBuffer*>::iterator p =  m_attachedRings.find(uri);
    if (p == m_attachedRings.end()) {
        throw std::string("ring is not attached");
    }

    if (objv.size() == 4+paramIndexOffset) {
        CTCLObject types = objv[3+paramIndexOffset];
        for (int i = 0; i < types.llength(); i++) {
            int type = int(types.lindex(i));
            some.addDesiredType(type);
        }
        pred = &some;                    //Restrict item types to desisred set.
    }
    
    // Get the item from the ring.
    
    
    CRingBuffer* pRing = p->second;
    auto pSpecificItem = getFromRing(*pRing, *pred, timeout);

    if (pSpecificItem == nullptr) {
        // oops... we timed out. return an empty string
        CTCLObject result;
        result.Bind(&interp);
        interp.setResult(result);
        return;
    }
    
    // Actual upcast depends on the type...and that describes how to format:
    
    CTCLObject result;
    result.Bind(interp);
    result += "type";
    result += pSpecificItem->typeName();
    
    switch(pSpecificItem->type()) {
        case BEGIN_RUN:
        case END_RUN:
        case PAUSE_RUN:
        case RESUME_RUN:
            formatStateChangeItem(interp, pSpecificItem, result);
            break;
        case PERIODIC_SCALERS:
            formatScalerItem(interp, pSpecificItem, result);
            break;
        case PACKET_TYPES:
        case MONITORED_VARIABLES:
            formatStringItem(interp, pSpecificItem, result);
            break;
        case RING_FORMAT:
            formatFormatItem(interp, pSpecificItem, result);
            break;
        case PHYSICS_EVENT:
            formatEvent(interp, pSpecificItem, result);
            break;
        case EVB_FRAGMENT:
        case EVB_UNKNOWN_PAYLOAD:
            formatFragment(interp, pSpecificItem, result);
            break;
        case PHYSICS_EVENT_COUNT:
            formatTriggerCount(interp, pSpecificItem, result);
            break;
        case EVB_GLOM_INFO:
            formatGlomParams(interp,  pSpecificItem, result);
            break;
        case ABNORMAL_ENDRUN:
            formatAbnormalEnd(interp, pSpecificItem, result);
        default:
            break;;
            // TO DO:
    }
    
    interp.setResult(result);
    delete pSpecificItem;
}

/*-----------------------------------------------------------------------------
 * Private utilities
 */


/**
 *  formatBodyHeader
 *  
 * Given that the object has a body header, this function creates the body header
 * dict for it
 *
 * @param interp - The interpreter to use when building the dict.
 * @parma p      - Pointer to the item.
 * @return CTCLObject - Really a list but in a format that can shimmer to a dict.
 */
CTCLObject
CTclRingCommand::formatBodyHeader(CTCLInterpreter& interp, CRingItem* p)
{
    CTCLObject subDict;
    subDict.Bind(interp);
    Tcl_Obj* tstamp = Tcl_NewWideIntObj(p->getEventTimestamp());
    subDict += "timestamp";
    subDict += CTCLObject(tstamp);
    
    subDict += "source";
    subDict += static_cast<int>(p->getSourceId());
    
    subDict += "barrier";
    subDict += static_cast<int>(p->getBarrierType());
    
    return subDict;        
}

/**
 *  format a ring state change item.  We're going to use the trick that a dict
 *  has a list rep where even elements are keys and odd elements their values.
 *  Users of the dict will shimmer into its dict rep. at first access.
 *
 *  @param interp - the interpreter.
 *  @param pItem  - Actually a CRingStateChangeItem.
 *
 *  Result is set with a dict with the keys:
 *
 *      type       - ring item type (textual)
 *      run        - run number
 *      timeoffset - Time offset.
 *      divisor    - time divisor
 *      offsetsec - time offset in seconds.
 *      realtime   - Time of day of the ring item (can use with [time format])
 *      title      - The run title.
 *      source     - the original source id of the data.
 *      bodyheader - only if there is a body header in the item.  That in turn is a dict with
 *                   timestamp, source, and barrier keys.
 */
 
void
CTclRingCommand::formatStateChangeItem(CTCLInterpreter& interp, CRingItem* pItem, CTCLObject& result)
{
    CRingStateChangeItem* p = reinterpret_cast<CRingStateChangeItem*>(pItem);
    
    result += "run";
    result += static_cast<int>(p->getRunNumber());
    
    setTiming(
        result,
        p->getElapsedTime(), p->getTimeDivisor(), p->computeElapsedTime(),
        p->getTimestamp()
    );
    
    result += "title";
    result += p->getTitle();
    
    result += "source";
    result += static_cast<int>(p->getOriginalSourceId());
    
    
    if (p->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, p);        
    }    
}
/**
 * formatScalerItem
 *    Formats a scaler item.  This creates a list that can be shimmered into a
 *    dict with the keys:
 *    - type - going to be Scaler
 *    - start - When in the run the start was.
 *    - startsec - Start time in seconds
 *    - end   - When in the run the end of the period was.
 *    - endsecs - end time in seconds.
 *    - realtime  Time emitted ([clock format] can take this)
 *    - divisor - What to divide start or end by to get seconds.
 *    - incremental - Bool true if this is incremental.
 *    - scalers     - List of scaler values.
 *    - source      - Original source id.
 *
 *    If there is a body header, the bodyheader key will be present and will be
 *    the body header dict.
 *
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - Actually a CRingScalerItem pointer.
 */
void
CTclRingCommand::formatScalerItem(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result)
{
    CRingScalerItem* pItem = reinterpret_cast<CRingScalerItem*>(pSpecificItem);
    
    // Stuff outside the body header
   
    
    result += "start";
    result += static_cast<int>(pItem->getStartTime());
    
    result += "startsec";
    result += static_cast<double>(pItem->computeStartTime());
    
    result += "end";
    result += static_cast<int>(pItem->getEndTime());
    
    result += "endsec";
    result += static_cast<double>(pItem->computeEndTime());
    
    result += "realtime";
    result += static_cast<int>(pItem->getTimestamp());
    
    result += "divisor";
    result += static_cast<int>(pItem->getTimeDivisor());
    
    result += "incremental";
    result += static_cast<int>(pItem->isIncremental() ? 1 : 0);
    
    result += "source";
    result += static_cast<int>(pItem->getOriginalSourceId());
    
    // Now the scaler vector itself:
    
    std::vector<uint32_t> scalerVec = pItem->getScalers();
    CTCLObject scalerList;
    scalerList.Bind(interp);
    for (int i=0; i < scalerVec.size(); i++) {
        scalerList += static_cast<int>(scalerVec[i]);
    }
    result += "scalers";
    result += scalerList;
    
    
    // If there is a body header add it too.
    
    if (pItem->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, pItem);
    }
}
/**
 * formatStringItem:
 *    Formats a ring item that contains a list of strings.   This produces a dict
 *    with the following keys:
 *    -   type - result of typeName.
*     -  timeoffset will have the offset time.
*     -  divisor will have the time divisor to get seconds.
*     -  offsetsec - time offset in seconds.
*     -  realtime will have something that can be given to [clock format] to get
*        when this was emitted
*     -  source - the original source id.
*     -  strings - list of strings that are in the ring item.
*     -  bodyheader - if the item has a body header.
*     @param interp - the intepreter object whose result will be set by this.
*     @param pSpecificItem - Actually a CRingTextItem pointer.
*     */
void
CTclRingCommand::formatStringItem(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result)
{
    CRingTextItem* p = reinterpret_cast<CRingTextItem*>(pSpecificItem);
    
    setTiming(
        result,
        p->getTimeOffset(), p->getTimeDivisor(), p->computeElapsedTime(),
        p->getTimestamp()
    );
    
    result += "source";
    result += static_cast<int>(p->getOriginalSourceId());

    
    CTCLObject stringList;
    stringList.Bind(interp);
    std::vector<std::string> strings = p->getStrings();
    for (int i =0; i < strings.size(); i++){
        stringList += strings[i];
    }
    result += "strings";
    result += stringList;
    
    if (p->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, p);
    }
}
/**
 * formatFormatitem
 *
 *    Formats a ring format item.  This will have the dict keys:
 *    *  type - what comes back from typeName()
 *    *  major - Major version number
 *    *  minor - minor version number.
 *
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - Actually a CDataFormatItem pointer.
*/ 
void
CTclRingCommand::formatFormatItem(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result)
{
    CDataFormatItem* p = reinterpret_cast<CDataFormatItem*>(pSpecificItem);
    

    result += "major";
    result += static_cast<int>(p->getMajor());
    
    result += "minor";
    result += static_cast<int>(p->getMinor());
}
/**
 * formatEvent
 *    Formats a physics event.  This is going ot have:
 *    *  type - "Event"
 *    *  bodyheader if one exists.
 *    *  size - number of bytes in the event
 *    *  body - byte array containing the event data.
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - The ring item.
*/ 

void
CTclRingCommand::formatEvent(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result)
{
    
    result += "size";
    result += static_cast<int>(pSpecificItem->getBodySize());
    
    result += "body";
    Tcl_Obj* body = Tcl_NewByteArrayObj(
        reinterpret_cast<const unsigned char*>(pSpecificItem->getBodyPointer()),
        static_cast<int>(pSpecificItem->getBodySize()));
    CTCLObject obj(body);
    obj.Bind(interp);
    result += obj;
    
    if (pSpecificItem->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, pSpecificItem);
    }
}

/**
 * formatFragment
 *
 *   Format an EVB_FRAGMENT ring item.
 *    *   type - "Event fragment"
 *    *   - timestamp - the 64 bit timestamp.
 *    *   - source    - The source id.
 *    *   - barrier   - The barrier type.
 *    *   - size      - payload size.
 *    *   - body      - Byte array containing the body.
 *
 *    @param interp - the intepreter object whose result will be set by this.
 *    @param pSpecificItem - The ring item.
 */
void
CTclRingCommand::formatFragment(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result)
{
    CRingFragmentItem* p = reinterpret_cast<CRingFragmentItem*>(pSpecificItem);
    
    result += "timestamp";
    Tcl_Obj* pTs = Tcl_NewWideIntObj(static_cast<Tcl_WideInt>(p->timestamp()));
    CTCLObject stamp(pTs);
    stamp.Bind(interp);
    result += stamp;
    
    result += "source";
    result += static_cast<int>(p->source());
    
    result += "barrier";
    result += static_cast<int>(p->barrierType());
    
    result += "size";
    result += static_cast<int>(p->payloadSize());
    
    result += "body";
    pEventBuilderFragment pFrag =
        reinterpret_cast<pEventBuilderFragment>(p->payloadPointer());
    Tcl_Obj* pBody =Tcl_NewByteArrayObj(
        pFrag->s_body,
        static_cast<int>(p->payloadSize()));
    CTCLObject body(pBody);
    body.Bind(interp);
    result += body;
}
/**
 * formatTriggerCount
 *
 *   Format dicts for PHYSICS_EVENT_COUNT items.
 *   this dict has:
 *
 *    *   type : "Trigger count"
*     *   bodyheader if the item has a body header present.
*     *   timeoffset - 123 (offset into the run).
*     *   divisor    - 1   divisor needed to turn that to seconds.
*     *   offsetsec  - offset in seconds.
*     *   triggers   - 1000 Number of triggers (note 64 bits).
*     *   realtime   - 0 time of day emitted.
*     *   source     - source id of the item.
*/
    
void
CTclRingCommand::formatTriggerCount(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result)
{
    CRingPhysicsEventCountItem* p =
        reinterpret_cast<CRingPhysicsEventCountItem*>(pSpecificItem);
    
    setTiming(
        result, p->getTimeOffset(),
        p->getTimeDivisor(), p->computeElapsedTime(),
        p->getTimestamp()
    );
        
    uint64_t ec = p->getEventCount();
    Tcl_Obj* eventCount = Tcl_NewWideIntObj(static_cast<Tcl_WideInt>(ec));
    CTCLObject eventCountObj(eventCount);
    eventCountObj.Bind(interp);
    result += "triggers";
    result += eventCountObj;

    result += "source";
    result += static_cast<int>(p->getOriginalSourceId());
    
    if (p->hasBodyHeader()) {
        result += "bodyheader";
        result += formatBodyHeader(interp, pSpecificItem);
    }
}
/**
 * formatGlomParams
 *     Format a glom parameters item.  This dict will have:
 *
 *     *  type - "Glom Parameters"
 *     *  bodyheader - never has this
 *     *  isBuilding - boolean True if building false otherwise.
 *     *  coincidenceWindow - Number of ticks in the coincidence window.
 *     *  timestampPolicy - one of "first", "last" or "average" indicating
 *        how the timestamp for the items are derived fromt he timestamps
 *        of their constituent fragments.
*/
void CTclRingCommand::formatGlomParams(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result)
{
    CGlomParameters *p = reinterpret_cast<CGlomParameters*>(pSpecificItem);
    
    result += "isBuilding";
    result += (p->isBuilding() ? 1 : 0);
    
    result += "coincidenceWindow";
    Tcl_Obj* dTicks = Tcl_NewWideIntObj(
        static_cast<Tcl_WideInt>(p->coincidenceTicks())
    );
    CTCLObject window(dTicks);
    window.Bind(interp);
    result += window;
    
    CGlomParameters::TimestampPolicy policy = p->timestampPolicy();
    result += "timestampPolicy";
    if (policy == CGlomParameters::first) {
        result += "first";
    } else if (policy == CGlomParameters::last) {
        result += "last";
    } else {
        result += "average";
    }
}
/**
 * formatAbnormalEnd
 *   We only provide  the type (Abnormal End).
 */
void
CTclRingCommand::formatAbnormalEnd(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result)
{

    // place holder for later stuff.

}

CRingItem*
CTclRingCommand::getFromRing(CRingBuffer &ring, CRingSelectionPredicate &predicate,
                             unsigned long timeout)
{

    CTimeout timer(timeout);

    CRingItem* pItem = nullptr;

    do {
        pItem = getFromRing(ring, timer);

        if (pItem) {
            if (!predicate.selectThis(pItem->type())
                    || (predicate.getNumberOfSelections() == 0) ) {
                break;
            }

        }

        delete pItem;
        pItem = nullptr;
    } while (! timer.expired());

    return pItem;
}

CRingItem*
CTclRingCommand::getFromRing(CRingBuffer &ring, CTimeout& timer)
{
    using namespace std::chrono;

    // look at the header, figure out the byte order and count so we can
    // create the item and fill it in.
    RingItemHeader header;

    // block with a timeout
    while ((ring.availableData() < sizeof(header)) && !timer.expired()) {
        std::this_thread::sleep_for(milliseconds(200));
    }

    // stop if we have no data and we timed out...
    // if we have timed out but there is data, then try to continue.
    if (timer.expired() && (ring.availableData() <= sizeof(header))) {
        return nullptr;
    }

    ring.peek(&header, sizeof(header));
    bool otherOrder(false);
    uint32_t size = header.s_size;
    if ((header.s_type & 0xffff0000) != 0) {
      otherOrder = true;
      size = swal(size);
    }

    // prevent a thrown range error caused by attempting to read more data
    // than is in the buffer.
    if (ring.availableData() < size) {
        return nullptr;
    }


    std::vector<uint8_t> buffer(size);
    size_t gotSize    = ring.get(buffer.data(),
                                 buffer.size(),
                                 buffer.size(),
                                 timer.getRemainingSeconds());// Read the item from the ring.
    if(gotSize  != buffer.size()) {
      if (gotSize == 0) {
        // operation timed out
        return nullptr;
      }

      std::cerr << "Mismatch in CRingItem::getItem required size: sb " << size << " was " << gotSize
                << std::endl;
    }

    return CRingItemFactory::createRingItem(buffer.data());
}

/**
 * setTiming
 *   Adds timing dict info to the  result
 * @param result -reference to the result being bult.
 * @param offset  offset value.
 * @param divisor - divisor -> seconds.
 * @param secs    - Seconds.
 * @param stamp   - time_t
 */
void CTclRingCommand::setTiming(
    CTCLObject& result, int offset, int divisor, double seconds,
    int stamp
)
{
result += "timeoffset";
    result += offset;
    result += "divisor";
    result += divisor;
    result += "offsetsec";
    result += seconds;
    result += "realtime";
    result += stamp;
}
/*-------------------------------------------------------------------------------
 * Package initialization:
 */

extern "C" 
int Tclringbuffer_Init(Tcl_Interp* pInterp)
{
    Tcl_PkgProvide(pInterp, "TclRingBuffer", "1.0");
    CTCLInterpreter* interp = new CTCLInterpreter(pInterp);
    CTclRingCommand* pCommand = new CTclRingCommand(*interp);
    
    return TCL_OK;
}


int gpTCLApplication = 0;
