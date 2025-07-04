// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

// The def/undefs let us look into the object's guts for white box testing.


#define private public
#define protected public
#include "CTclRingCommand.h"
#undef private
#undef protected

#include <tcl.h>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <CRingBuffer.h>
#include <DataFormat.h>
#include <CRingStateChangeItem.h>
#include <CRingTextItem.h>
#include <CDataFormatItem.h>
#include <CPhysicsEventItem.h>
#include <CRingFragmentItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CGlomParameters.h>
#include <CAbnormalEndItem.h>
#include <CRingScalerItem.h>

#include <iostream>
#include <unistd.h>

class RingTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RingTests);
  CPPUNIT_TEST(construct);
  
  // Tests for top level command processing
  
  CPPUNIT_TEST(needsubcommand);
  CPPUNIT_TEST(badsubcommand);
  
  // Tests for the attach command
  
  CPPUNIT_TEST(needURI);
  CPPUNIT_TEST(needvalidURI);
  CPPUNIT_TEST(validRing);
  CPPUNIT_TEST(alreadyAttached);
  
  // Tests for the detach command:
  
  CPPUNIT_TEST(detachNeedURI);
  CPPUNIT_TEST(detachNeedAttachedURI);
  CPPUNIT_TEST(detachOk);
  
  // Tests for the get command:
  
  CPPUNIT_TEST(getNeedUri);
  CPPUNIT_TEST(getNeedAttachedUri);
  CPPUNIT_TEST(getNoBodyHeaderBegin);
  CPPUNIT_TEST(getBodyHeaderBegin);
  CPPUNIT_TEST(getNoBodyHeaderScaler);
  CPPUNIT_TEST(getBodyHeaderScaler);
  CPPUNIT_TEST(getNoBodyHeaderPacketTypes);
  CPPUNIT_TEST(getBodyHeaderPacketTypes);
  CPPUNIT_TEST(getRingFormat);
  CPPUNIT_TEST(getNoBodyHeaderPhysics);
  CPPUNIT_TEST(getBodyHeaderPhysics);
  CPPUNIT_TEST(getEvbFragment);
  CPPUNIT_TEST(getPhysicsEventCount);
  CPPUNIT_TEST(getPhysicsEventCountBodyHeader);
  CPPUNIT_TEST(getGlomInfo);
  CPPUNIT_TEST(getWithPredicate);
  CPPUNIT_TEST(getTimeout_0);
  CPPUNIT_TEST(getTimeout_1);
  // Test for Abnormal End.
  
  CPPUNIT_TEST(getAbnormalEnd);

  
  CPPUNIT_TEST_SUITE_END();


private:
    CTCLInterpreter* m_pInterp;
    CTclRingCommand* m_pCommand;
public:
  void setUp() {
    m_pInterp   = new CTCLInterpreter();
    m_pCommand  = new CTclRingCommand(*m_pInterp);
    try {CRingBuffer::create("tcltestring");} catch(...) {}
  }
  void tearDown() {
    delete m_pCommand;
    delete m_pInterp;
    try {CRingBuffer::remove("tcltestring");} catch(...) {}
    unlink("/dev/shm/tcltestring");    // In case it lingered.
  }
protected:
  void construct();
  void needsubcommand();
  void badsubcommand();
  
  void needURI();
  void needvalidURI();
  void validRing();
  void alreadyAttached();
  
  void detachNeedURI();
  void detachNeedAttachedURI();
  void detachOk();
  
  void getNeedUri();
  void getNeedAttachedUri();
  void getNoBodyHeaderBegin();
  void getBodyHeaderBegin();
  void getNoBodyHeaderScaler();
  void getBodyHeaderScaler();
  void getNoBodyHeaderPacketTypes();
  void getBodyHeaderPacketTypes();
  void getNoBodyHeaderVars();
  void getBodyHeaderVars();
  void getRingFormat();
  void getNoBodyHeaderPhysics();
  void getBodyHeaderPhysics();
  void getEvbFragment();
  void getPhysicsEventCount();
  void getPhysicsEventCountBodyHeader();
  void getGlomInfo();
  void getWithPredicate();
  
  void getAbnormalEnd();
  void getTimeout_0();
  void getTimeout_1();

private:
    int tryCommand(const char* command);
    std::string getResult();
    void insertStateChange(int type, bool bodyHeader);
    void insertScalerItem(bool bodyHeader);
    void emitStringList(int type, bool bodyHeader);
    void emitFormat();
    void emitEvent(bool bodyHeader);
    void emitFragment();
    void emitEventCount(bool bodyHeader);
    void emitGlomParams();
    
    
    int getDictItem(Tcl_Obj* obj, const char* key, std::string& value);
    
};

CPPUNIT_TEST_SUITE_REGISTRATION(RingTests);


/*--------------------------------- Utility factorizations ---------------------------*/

// Factor out the stuff needed to run a command:
// returns the status.

int
RingTests::tryCommand(const char* cmd)
{
    Tcl_Interp* pInterp = m_pInterp->getInterpreter();
    return Tcl_GlobalEval(pInterp, cmd);
}
// Get interpreter result:

std::string
RingTests::getResult()
{
    Tcl_Interp* pInterp = m_pInterp->getInterpreter();
    return std::string(Tcl_GetStringResult(pInterp));
}

/**
 * get an item by key from an object that has a dict:
 *  @param obj  - the dict.
 *  @param key  - the key to retrieve.
 *  @param value - The value of the key (meaningful only if TCL_OK is returned)
 *  @return Resulst of Tcl_DictObjGet()
 */
int RingTests::getDictItem(Tcl_Obj* obj, const char* key, std::string& value)
{
    Tcl_Interp* pI = m_pInterp->getInterpreter();
    Tcl_Obj*    keyObj = Tcl_NewStringObj(key, -1);
    Tcl_Obj*    valObj;
    
    int status = Tcl_DictObjGet(pI, obj, keyObj, &valObj);
    if ((status == TCL_OK) && valObj) {
        value = Tcl_GetString(valObj);
    } else {
        status = TCL_ERROR;            // In case valObj was zero.
    }
    return status;
}
/**
 * insertStateChange - insert a state change item in the tcltestring
 * @param type - Actual state change type (e.g. BEGIN_RUN).
 * @param bheader - true if a body header is to be constructed.
 */
void RingTests::insertStateChange(int type, bool bheader)
{
    CRingBuffer ring("tcltestring", CRingBuffer::producer);
    if (bheader) {
        CRingStateChangeItem item(12345678, 1, 0, type, 123, 0, 0, std::string("A test title"));
        item.commitToRing(ring);
    } else {
        CRingStateChangeItem item(type, 123, 0, 0, std::string("A test title"));
        item.commitToRing(ring);
        
    }
}
/**
 * insertScalerItem - insert a scaler item in the tcltestring
 * @param bheader - true if the item must have a body header.
 */
void
RingTests::insertScalerItem(bool bheader)
{
    CRingBuffer ring("tcltestring", CRingBuffer::producer);
    std::vector<uint32_t> scalers;
    for (int i =0; i < 10; i++) {
        scalers.push_back(i);
    }
    if (bheader) {
        CRingScalerItem item(12345678, 1, 0, 0, 10, 0, scalers, 2);
        item.commitToRing(ring);
        
    } else {
        CRingScalerItem item(0, 10, 0, scalers);
        item.commitToRing(ring);
    }
}
void
RingTests::emitStringList(int type, bool bodyheader)
{
    std::vector<std::string> strings;
    strings.push_back("Type 1");
    strings.push_back("Type 2");
    strings.push_back("Type 3");
    CRingBuffer ring("tcltestring", CRingBuffer::producer);
    if (bodyheader) {
        CRingTextItem item(type, 1234, 1, 0, strings, 20, 1111, 1);
        item.commitToRing(ring);
    } else {
        
        CRingTextItem item(type, strings, 20,  1111);
        item.commitToRing(ring);
    }
}

void
RingTests::emitFormat()
{
    CRingBuffer ring("tcltestring", CRingBuffer::producer);
    CDataFormatItem item;
    item.commitToRing(ring);
}
void
RingTests::emitEvent(bool bodyHeader)
{
    CRingBuffer ring("tcltestring", CRingBuffer::producer);
    CPhysicsEventItem* pItem;
    if (bodyHeader) {
        pItem = new CPhysicsEventItem(1234, 2, 0);
    } else {
        pItem = new CPhysicsEventItem;
    }
    CPhysicsEventItem& item(*pItem);
    uint32_t* pN = reinterpret_cast<uint32_t*>(item.getBodyPointer());
    *pN++ = 12;
    uint16_t*  pd = reinterpret_cast<uint16_t*>(pN);
    for (int i = 0; i < 10; i++) {
        *pd++ = i;
    }
    item.setBodyCursor(pd);
    item.updateSize();
    
    item.commitToRing(ring);
    delete pItem;
}

void RingTests::emitFragment()
{
    CRingBuffer ring("tcltestring", CRingBuffer::producer);
    uint8_t payload[20];
    for (int i =0; i < 20; i++) {
        payload[i] = i;
    }
    CRingFragmentItem item(12345, 2,  sizeof(payload), payload, 0);
    item.commitToRing(ring);
}

void RingTests::emitEventCount(bool bodyHeader) {
    CRingBuffer ring("tcltestring", CRingBuffer::producer);
    if (bodyHeader) {
        CRingPhysicsEventCountItem item(1234, 2, 0, 1000, 123, 0);
        item.commitToRing(ring);
    } else {
        CRingPhysicsEventCountItem item(1000, 123, 0);
        item.commitToRing(ring);
        
    }
}
void RingTests::emitGlomParams()
{
    CRingBuffer ring("tcltestring", CRingBuffer::producer);
    CGlomParameters item(10, true, CGlomParameters::average);
    item.commitToRing(ring);
}
/*---------------------------------------------- TESTS ------------------------------------*/

// Construction should have created a 'ring' command.

void RingTests::construct() {
    Tcl_CmdInfo info;
    
    ASSERT(0 != Tcl_GetCommandInfo(m_pInterp->getInterpreter(), "ring", &info));
}
// Just the 'ring' command by itself returns an error because it needs a
// subcommand

void RingTests::needsubcommand() {
    int status = tryCommand("ring");
    EQ(TCL_ERROR, status);
    EQ(std::string("Insufficient parameters"), getResult() );
    
}

// invalid subcommand is an error too:

void RingTests::badsubcommand() {
    int status = tryCommand( "ring george");
    EQ(TCL_ERROR, status);
    EQ(std::string("bad subcommand"), getResult());
    
}

// The ring attach command should complain if there's no ring URI.

void RingTests::needURI() {
    
    int status = tryCommand( "ring attach");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring attach needs a ring URI"), getResult());
}
// The ring attach command should complain if there is no such ring:

void RingTests::needvalidURI() {
    try {
        CRingBuffer::remove("no-such-ring");
    } catch(...) {}                             /// Catch in case the ring never existed (likely).
    
    
    int status = tryCommand( "ring attach tcp://localhost/no-such-ring");
    EQ(TCL_ERROR, status);
    EQ(std::string("Failed to attach ring"), getResult());
}    

void RingTests::validRing() {
    
    int status = tryCommand( "ring attach tcp://localhost/tcltestring"); // See setUp().
    EQ(TCL_OK, status);
    
    // The map in m_pCommand should have this too:
    
    ASSERT(m_pCommand->m_attachedRings.end() !=
           m_pCommand->m_attachedRings.find("tcp://localhost/tcltestring"));
}

// An error to doubly attach a URI.

void RingTests::alreadyAttached() {
    
    
    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    status     = tryCommand("ring attach tcp://localhost/tcltestring");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring already attached"), getResult());
    
}


void RingTests::detachNeedURI() {
    
    int status = tryCommand("ring detach");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring detach needs a URI"), getResult());
}
void RingTests::detachNeedAttachedURI() {
    
    int status = tryCommand( "ring detach tcp://localhost/tcltestring");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring is not attached"), getResult());
    
}
void RingTests::detachOk() {
    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    status     = tryCommand("ring detach tcp://localhost/tcltestring");
    
    EQ(TCL_OK, status);
    ASSERT(m_pCommand->m_attachedRings.end() ==
           m_pCommand->m_attachedRings.find("tcp://localhost/tcltestring"));
    
}


  
void RingTests::getNeedUri(){
    
    int status = tryCommand("ring get ");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring get needs a URI"), getResult());
    
}
void RingTests::getNeedAttachedUri() {
    int status = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_ERROR, status);
    EQ(std::string("ring is not attached"), getResult());
}

// Attach, insert, then get should work to make us get a ring
// item.

void RingTests::getNoBodyHeaderBegin() {
    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    insertStateChange(BEGIN_RUN, false);
    status     = tryCommand("ring get    tcp://localhost/tcltestring");
    
    EQ(TCL_OK, status);
    
    auto cmdResult = getResult();
    std::cout << cmdResult << std::endl;

    /* We expect the result to be a dict with:
       type       - ring item type (textual)
       run        - run number
       timeoffset - Time offset in seconds.
       realtime   - Time of day of the ring item (can use with [time format])
       title      - The run title.
   */
    
    std::string item;
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    status = getDictItem(result, "type", item);
    EQ(TCL_OK, status);
    EQ(std::string("Begin Run"), item);
    
    status = getDictItem(result, "run", item);
    EQ(TCL_OK, status);
    EQ(std::string("123"), item);
    
    status = getDictItem(result, "timeoffset", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "realtime", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "title", item);
    EQ(TCL_OK, status);
    EQ(std::string("A test title"), item);
    
    // There should not be a bodyheader dict item:
    
    status = getDictItem(result, "bodyheader", item);
    EQ(TCL_ERROR, status);
    
    
}


void RingTests::getBodyHeaderBegin() {
    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    insertStateChange(BEGIN_RUN, true);
    status     = tryCommand("ring get    tcp://localhost/tcltestring");
    
    EQ(TCL_OK, status);
    
    /* We expect the result to be a dict with:
       type       - ring item type (textual)
       run        - run number
       timeoffset - Time offset in seconds.
       realtime   - Time of day of the ring item (can use with [time format])
       title      - The run title.
   */
    
    std::string item;
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    status = getDictItem(result, "type", item);
    EQ(TCL_OK, status);
    EQ(std::string("Begin Run"), item);
    
    status = getDictItem(result, "run", item);
    EQ(TCL_OK, status);
    EQ(std::string("123"), item);
    
    status = getDictItem(result, "timeoffset", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "realtime", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "title", item);
    EQ(TCL_OK, status);
    EQ(std::string("A test title"), item);
    
    /*
     * There should be a bodyheader which itself is a dict containing:
     * timestamp - The timstamp field.
     * source    - Id of the source.
     * barrier   - Barrier type
     */
    Tcl_Obj* bodyHeader;
    Tcl_Obj* key = Tcl_NewStringObj("bodyheader", -1);
    Tcl_DictObjGet(m_pInterp->getInterpreter(), result, key, &bodyHeader);
    ASSERT(bodyHeader);
    
    status = getDictItem(bodyHeader, "timestamp", item);
    EQ(TCL_OK, status);
    EQ(std::string("12345678"), item);
    
    status = getDictItem(bodyHeader, "source", item);
    EQ(TCL_OK, status);
    EQ(std::string("1"), item);
    
    status = getDictItem(bodyHeader, "barrier", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);

    
}

void RingTests::getNoBodyHeaderScaler()
{
    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    insertScalerItem(false);
    status     = tryCommand("ring get    tcp://localhost/tcltestring");
    
    EQ(TCL_OK, status);
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    // We expect a dict containing the following:
    //  - no bodyheader key.
    //  - type - going to be Scaler
    //  - start - When in the run the start was.
    //  - end   - When in the run the end of the period was.
    //  - realtime  Time emitted ([clock format] can take this)
    //  - divisor - What to divide start or end by to get seconds.
    //  - incremental - Bool true if this is incremental.
    //  - scalers     - List of scaler values.
    
    std::string item;
    status = getDictItem(result, "type", item);
    EQ(TCL_OK, status);
    EQ(std::string("Scaler"), item);
    
    status = getDictItem(result, "start", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "end", item);
    EQ(TCL_OK, status);
    EQ(std::string("10"), item);
    
    status = getDictItem(result, "divisor", item);
    EQ(TCL_OK, status);
    EQ(std::string("1"), item);
    
    status = getDictItem(result, "incremental", item);
    EQ(TCL_OK, status);
    EQ(std::string("1"), item);
    
    status = getDictItem(result, "realtime", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);
    
    Tcl_Obj* scalers;
    Tcl_Obj* key = Tcl_NewStringObj("scalers", -1);
    status = Tcl_DictObjGet(m_pInterp->getInterpreter(), result, key, &scalers);
    EQ(TCL_OK, status);
    ASSERT(scalers);
    
    // This must be a 10 element list with a counting pattern:
    
    CTCLObject scls(scalers);
    scls.Bind(m_pInterp);
    EQ(10, scls.llength());
    for(int i = 0; i < 10; i++) {
        EQ(i, int(scls.lindex(i)));
    }
    // No body header:
    
    status = getDictItem(result, "bodyheader", item);
    EQ(TCL_ERROR, status);
    
}


void RingTests::getBodyHeaderScaler() {

    int status = tryCommand("ring attach tcp://localhost/tcltestring");
    insertScalerItem(true);
    status     = tryCommand("ring get    tcp://localhost/tcltestring");
    
    EQ(TCL_OK, status);
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    // We expect a dict containing the following:
    //  - no bodyheader key.
    //  - type - going to be Scaler
    //  - start - When in the run the start was.
    //  - end   - When in the run the end of the period was.
    //  - realtime  Time emitted ([clock format] can take this)
    //  - divisor - What to divide start or end by to get seconds.
    //  - incremental - Bool true if this is incremental.
    //  - scalers     - List of scaler values.
    
    std::string item;
    status = getDictItem(result, "type", item);
    EQ(TCL_OK, status);
    EQ(std::string("Scaler"), item);
    
    status = getDictItem(result, "start", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);
    
    status = getDictItem(result, "end", item);
    EQ(TCL_OK, status);
    EQ(std::string("10"), item);
    
    status = getDictItem(result, "divisor", item);
    EQ(TCL_OK, status);
    EQ(std::string("2"), item);
    
    status = getDictItem(result, "incremental", item);
    EQ(TCL_OK, status);
    EQ(std::string("1"), item);
    
    status = getDictItem(result, "realtime", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);
    
    Tcl_Obj* scalers;
    Tcl_Obj* key = Tcl_NewStringObj("scalers", -1);
    status = Tcl_DictObjGet(m_pInterp->getInterpreter(), result, key, &scalers);
    EQ(TCL_OK, status);
    ASSERT(scalers);
    
    // This must be a 10 element list with a counting pattern:
    
    CTCLObject scls(scalers);
    scls.Bind(m_pInterp);
    EQ(10, scls.llength());
    for(int i = 0; i < 10; i++) {
        EQ(i, int(scls.lindex(i)));
    }
    // Now there should be a body:
    
   /*
     * There should be a bodyheader which itself is a dict containing:
     * timestamp - The timstamp field.
     * source    - Id of the source.
     * barrier   - Barrier type
     */
    Tcl_Obj* bodyHeader;
    key = Tcl_NewStringObj("bodyheader", -1);
    Tcl_DictObjGet(m_pInterp->getInterpreter(), result, key, &bodyHeader);
    ASSERT(bodyHeader);
    
    status = getDictItem(bodyHeader, "timestamp", item);
    EQ(TCL_OK, status);
    EQ(std::string("12345678"), item);
    
    status = getDictItem(bodyHeader, "source", item);
    EQ(TCL_OK, status);
    EQ(std::string("1"), item);
    
    status = getDictItem(bodyHeader, "barrier", item);
    EQ(TCL_OK, status);
    EQ(std::string("0"), item);    
    
}

void RingTests::getNoBodyHeaderPacketTypes()
{
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitStringList(PACKET_TYPES, false);
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    Tcl_Obj* pResult = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    
    /* pResult should have a dict and:
    *  -  There won't be a bodyheader key.
    *  -  type will be the item type e.g. 
    *  -  timeoffset will have the offset time.
    *  -  divisor will have the time divisor to get seconds.
    *  -  realtime will have something that can be given to [clock format] to get
    *     when this was emitted
    *  -  strings - list of strings that are in the ring item.
    */
    
    std::string item;
    
    stat  = getDictItem(pResult, "type", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("Packet types"));
    
    stat = getDictItem(pResult, "timeoffset", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("20"));
    
    stat = getDictItem(pResult, "divisor", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("1"));
    
    stat = getDictItem(pResult, "realtime", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("1111"));
    
    Tcl_Obj* strings;
    Tcl_Obj* key = Tcl_NewStringObj("strings", -1);
    stat = Tcl_DictObjGet(m_pInterp->getInterpreter(), pResult, key, &strings);
    
    CTCLObject stringList(strings);
    stringList.Bind(m_pInterp);
    EQ(3, stringList.llength());
    EQ(std::string("Type 1"), std::string(stringList.lindex(0)));
    EQ(std::string("Type 2"), std::string(stringList.lindex(1)));
    EQ(std::string("Type 3"), std::string(stringList.lindex(2)));
    
    // NO body header:
    
    EQ(TCL_ERROR, getDictItem(pResult, "bodyheader", item));
    
    
}
void RingTests::getBodyHeaderPacketTypes(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitStringList(PACKET_TYPES, true);
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    Tcl_Obj* pResult = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    
    /* pResult should have a dict and:
    *  -  There will be a bodyheader key.
    *  -  type will be the item type e.g. 
    *  -  timeoffset will have the offset time.
    *  -  divisor will have the time divisor to get seconds.
    *  -  realtime will have something that can be given to [clock format] to get
    *     when this was emitted
    *  -  strings - list of strings that are in the ring item.
    */
    
    std::string item;
    
    stat  = getDictItem(pResult, "type", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("Packet types"));
    
    stat = getDictItem(pResult, "timeoffset", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("20"));
    
    stat = getDictItem(pResult, "divisor", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("1"));
    
    stat = getDictItem(pResult, "realtime", item);
    EQ(TCL_OK, stat);
    EQ(item, std::string("1111"));
    
    Tcl_Obj* strings;
    Tcl_Obj* key = Tcl_NewStringObj("strings", -1);
    stat = Tcl_DictObjGet(m_pInterp->getInterpreter(), pResult, key, &strings);
    
    CTCLObject stringList(strings);
    stringList.Bind(m_pInterp);
    EQ(3, stringList.llength());
    EQ(std::string("Type 1"), std::string(stringList.lindex(0)));
    EQ(std::string("Type 2"), std::string(stringList.lindex(1)));
    EQ(std::string("Type 3"), std::string(stringList.lindex(2)));
    
    //  body header:  By now we believe the contents.
    
    EQ(TCL_OK, getDictItem(pResult, "bodyheader", item));
    
}

void RingTests::getRingFormat(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitFormat();
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    /*
     * Format items have:
     *    - no body header
     *    - type ("Ring item format version")
     *    - major - Major version number.
     *    - minor - Minor version number.
     */
    
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    std::string item;
    
    EQ(TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Ring Item format version"), item);
    
    EQ(TCL_OK, getDictItem(result, "major", item));
    EQ(std::string("11"), item);
    
    EQ(TCL_OK, getDictItem(result, "minor", item));
    EQ(std::string("0"), item);
    
    EQ(TCL_ERROR, getDictItem(result, "bodyheader", item));
    
    
}

void RingTests::getNoBodyHeaderPhysics(){

    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitEvent(false);
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    /* This dict contains:
     *    - type - "Event"
     *    - size - Number of bytes in the event.
     *    - body - The event data as a bytearray (Use [binary scan] to pick it apart)
     *    - There shouild be no bodyheader.
     */
    
    std::string item;
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    EQ(TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Event"), item);
    
    EQ(TCL_OK, getDictItem(result, "size", item));
    EQ(std::string("24"), item);
    
    Tcl_Obj*    key = Tcl_NewStringObj("body", -1);
    Tcl_Obj*    byteArray;
    stat = Tcl_DictObjGet(m_pInterp->getInterpreter(), result, key, &byteArray);
    EQ(TCL_OK, stat);
    ASSERT(byteArray);              // Null if no such key.
    
    int size;
    struct _eventBody {
        uint32_t size;
        uint16_t body[10];
    } *event = reinterpret_cast<struct _eventBody*>(Tcl_GetByteArrayFromObj(byteArray, &size));
    EQ(24, size);
    EQ(static_cast<int>(24/sizeof(uint16_t)), static_cast<int>(event->size));
    for (int i =0; i < 10; i++) {
        EQ(i, static_cast<int>(event->body[i]));
    }
    
    EQ(TCL_ERROR, getDictItem(result, "bodyheader", item));
    

    
}

void RingTests::getBodyHeaderPhysics(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitEvent(true);
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    /* This dict contains:
     *    - type - "Event"
     *    - size - Number of bytes in the event.
     *    - body - The event data as a bytearray (Use [binary scan] to pick it apart)
     *    - There shouild be no bodyheader.
     */
    
    std::string item;
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    EQ(TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Event"), item);
    
    EQ(TCL_OK, getDictItem(result, "size", item));
    EQ(std::string("24"), item);
    
    Tcl_Obj*    key = Tcl_NewStringObj("body", -1);
    Tcl_Obj*    byteArray;
    stat = Tcl_DictObjGet(m_pInterp->getInterpreter(), result, key, &byteArray);
    EQ(TCL_OK, stat);
    ASSERT(byteArray);              // Null if no such key.
    
    int size;
    struct _eventBody {
        uint32_t size;
        uint16_t body[10];
    } *event = reinterpret_cast<struct _eventBody*>(Tcl_GetByteArrayFromObj(byteArray, &size));
    EQ(24, size);
    EQ(static_cast<int>(24/sizeof(uint16_t)), static_cast<int>(event->size));
    for (int i =0; i < 10; i++) {
        EQ(i, static_cast<int>(event->body[i]));
    }
    
    EQ(TCL_OK, getDictItem(result, "bodyheader", item));
        
}

void RingTests::getEvbFragment(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitFragment();
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    /**
     * fragments are funky.  They have:
     *   - type      - "Event fragment"
     *   - timestamp - the 64 bit timestamp.
     *   - source    - The source id.
     *   - barrier   - The barrier type.
     *   - size      - payload size.
     *   - body      - Byte array containing the body.
     */
    
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    std::string item;
    
    EQ(TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Event fragment"), item);
    
    EQ(TCL_OK, getDictItem(result, "timestamp", item));
    EQ(std::string("12345"), item);
    
    EQ(TCL_OK, getDictItem(result, "source", item));
    EQ(std::string("2"), item);
    
    EQ(TCL_OK, getDictItem(result, "barrier", item));
    EQ(std::string("0"), item);
    
    EQ(TCL_OK, getDictItem(result, "size", item));
    EQ(std::string("20"), item);
    
    Tcl_Obj* byteArray;
    Tcl_Obj* key = Tcl_NewStringObj("body", -1);
    
    EQ(TCL_OK, Tcl_DictObjGet(m_pInterp->getInterpreter(), result, key, &byteArray));
    int length;
    unsigned char*  payload = Tcl_GetByteArrayFromObj(byteArray, &length);
    
    EQ(20, length);
    for (int i = 0; i < length; i++) {
        EQ(i, static_cast<int>(*payload++));
    }
    
    
    
}

void RingTests::getPhysicsEventCount(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitEventCount(false);
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    std::string item;

    /* result should be a dict with:
     * *   type : "Trigger count"
     * *   no bodyheader item.
     * *   timeoffset - 123 (offset into the run).
     * *   divisor    - 1   divisor needed to turn that to seconds.
     * *   triggers   - 1000 Number of triggers.
     * *   realtime   - 0 time of day emitted.
     */
    
    EQ(TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Trigger count"), item);
    
    EQ(TCL_ERROR, getDictItem(result, "bodyheader", item));
    
    EQ(TCL_OK, getDictItem(result, "timeoffset", item));
    EQ(std::string("123"), item);
    
    EQ(TCL_OK, getDictItem(result, "divisor", item));
    EQ(std::string("1"), item);
    
    EQ(TCL_OK, getDictItem(result, "triggers", item));
    EQ(std::string("1000"), item);
    
    EQ(TCL_OK, getDictItem(result, "realtime", item));
    EQ(std::string("0"), item);
}
void RingTests::getPhysicsEventCountBodyHeader() {
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitEventCount(true);
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    std::string item;

    /* result should be a dict with:
     * *   type : "Trigger count"
     * *    bodyheader item.
     * *   timeoffset - 123 (offset into the run).
     * *   divisor    - 1   divisor needed to turn that to seconds.
     * *   triggers   - 1000 Number of triggers.
     * *   realtime   - 0 time of day emitted.
     */
    
    EQ(TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Trigger count"), item);
    
    
    EQ(TCL_OK, getDictItem(result, "timeoffset", item));
    EQ(std::string("123"), item);
    
    EQ(TCL_OK, getDictItem(result, "divisor", item));
    EQ(std::string("1"), item);
    
    EQ(TCL_OK, getDictItem(result, "triggers", item));
    EQ(std::string("1000"), item);
    
    EQ(TCL_OK, getDictItem(result, "realtime", item));
    EQ(std::string("0"), item);
    
    EQ(TCL_OK, getDictItem(result, "bodyheader", item));
    
}
void RingTests::getGlomInfo(){
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    emitGlomParams();
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    
    std::string item;
    Tcl_Obj* result = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    /*  We should have:
     *   * type  "Glom Parameters"
     *   * Never a body header.
     *   * isBuilding  1
     *   * timestampPolicy "average"
     *   * coincidenceWindow 10
     *
     */
    
    EQ(TCL_OK, getDictItem(result, "type", item));
    EQ(std::string("Glom Parameters"), item);
    
    EQ(TCL_ERROR, getDictItem(result, "bodyheader", item));
    
    EQ(TCL_OK, getDictItem(result, "isBuilding", item));
    EQ(std::string("1"), item);
    
    EQ(TCL_OK, getDictItem(result, "timestampPolicy", item));
    EQ(std::string("average"), item);
    
    EQ(TCL_OK, getDictItem(result, "coincidenceWindow", item));
    EQ(std::string("10"), item);
    
}
void RingTests::getWithPredicate()
{
    std::cout << "getWithPredicate" << std::endl;
    // We're going to request an item using a predicate that only allows
    // BEGIN_RUN and END_RUN items.  Then we'll emit a begin run and a bunch
    // of events and an end run.  We should only see the begin and
    // end runs.
    
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    insertStateChange(BEGIN_RUN, false);
    usleep(100);

    // Timing issue with this loop leads to test failure. Emit fewer events.
    // --ASC 6/2/23
    // Pause after commiting to ring. Not sure how much an effect it has,
    // but seems more stable. Trying to handle exceptions to figure out
    // the cause of the failure has proven unsuccessful. Probably worth
    // revisiting at some point and identifying an actual underlying cause.
    // --ASC 8/20/24
    
    for (int i =0; i < 10; i++) {
        emitEvent(false);
        usleep(100);
    }

    insertStateChange(END_RUN, false);
    usleep(100);

    stat = tryCommand("ring get tcp://localhost/tcltestring [list 1 2]");
    Tcl_Obj* event1 = Tcl_GetObjResult(m_pInterp->getInterpreter());
   
    
    std::string item;
    getDictItem(event1, "type", item);
    EQ(std::string("Begin Run"), item);
    
    stat = tryCommand("ring get tcp://localhost/tcltestring [list 1 2]");
    Tcl_Obj* event2 = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    getDictItem(event2, "type", item);
    EQ(std::string("End Run"), item);
    
    
}
void RingTests::getAbnormalEnd()
{
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    CRingBuffer ring("tcltestring", CRingBuffer::producer);
    CAbnormalEndItem item;
    item.commitToRing(ring);
    
    stat = tryCommand("ring get tcp://localhost/tcltestring");
    EQ(TCL_OK, stat);
    Tcl_Obj* received = Tcl_GetObjResult(m_pInterp->getInterpreter());
    
    std::string itemValue;
    getDictItem(received, "type", itemValue);
    EQ(std::string("Abnormal End"), itemValue);
}

void RingTests::getTimeout_0()
{
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");
    CRingBuffer ring("tcltestring", CRingBuffer::producer);

    stat = tryCommand("ring get -timeout 0 tcp://localhost/tcltestring ");

    EQ(TCL_OK, stat);
    EQ(std::string(""), getResult());
}

void RingTests::getTimeout_1()
{
    int stat = tryCommand("ring attach tcp://localhost/tcltestring");

    insertStateChange(BEGIN_RUN, false);

    stat = tryCommand("ring get -timeout 0 tcp://localhost/tcltestring 1");

    EQ(TCL_OK, stat);

    Tcl_Obj* event2 = Tcl_GetObjResult(m_pInterp->getInterpreter());

    std::string itemValue;
    getDictItem(event2, "type", itemValue);
    EQ(std::string("Begin Run"), itemValue);
}
