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

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include "CMyEventSegment.h"
#undef private
#include <sstream>
#include <string.h>
#include <iostream>

class sanitytest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sanitytest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(empty);
    CPPUNIT_TEST(ok_1);
    CPPUNIT_TEST(ok_2);
    CPPUNIT_TEST(ok_3);
    CPPUNIT_TEST(ok_4);
    
    // Should report errors:
    
    CPPUNIT_TEST(wrongSlot_1);
    CPPUNIT_TEST(wrongSlot_2);
    CPPUNIT_TEST(wrongSlot_3);
    
    CPPUNIT_TEST(wronghdr_1);
    CPPUNIT_TEST(wronghdr_2);
    CPPUNIT_TEST_SUITE_END();
    
private:
    CMyEventSegment* m_pSeg;
public:
    void setUp() {
        m_pSeg = new CMyEventSegment;
    }
    void tearDown() {
        delete m_pSeg;
    }
protected:
    void construct_1();
    void empty();
    void ok_1();
    void ok_2();
    void ok_3();
    void ok_4();
    
    // Should report errors:
    
    void wrongSlot_1();
    void wrongSlot_2();
    void wrongSlot_3();
    
    void wronghdr_1();
    void wronghdr_2();
private:
    size_t makeOkHeader(
        CMyEventSegment::HitHeader* hdr, int crate, int slot, int chan
    );
    uint32_t makeId(
        int crate, int slot, int chan,
        int hdrlen, int evtlen
    );
};

CPPUNIT_TEST_SUITE_REGISTRATION(sanitytest);
//////////////////////////// Utilities ////////////////////////////////

/**
 *   Make an id word (uint32_t)
 *
 *   @param crate, slot, chan - channel in the system.
 *   @param hdrlen - Length of the header.
 *   @param evtlen - Length of the event.
 *   @return uint32_t - THe constructed id word.
 *   @note we assume no field overflows.   
 */
uint32_t
sanitytest::makeId(
    int crate, int slot, int chan,
    int hdrlen, int evtlen
)
{
    uint32_t result =
        chan | (slot << 4) | (crate << 8) | (hdrlen << 12) | (evtlen << 17);
    return result;
}


/**
 * make an event that consists only of an ok header.
 *
 * @param hdr - pointer to the buffer to hold the header.
 * @param crate - crate of  the event.
 * @param slot  - Slot of the event.
 * @param chan  - channel of the hit.
 * @return size_t - number of longs in the header (4).
 */
size_t
sanitytest::makeOkHeader(
    CMyEventSegment::HitHeader* hdr, int crate, int slot, int chan
)
{
    memset(hdr, 0, sizeof(CMyEventSegment::HitHeader));
    hdr->s_id = makeId(crate, slot, chan, 4, 4);
        
    return 4;
}


///////////////////////////////////// Tests /////////////////////////////
void sanitytest::construct_1()
{
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    EQ(std::iostream::pos_type(0), debugOutput.tellp());
}
// IF I pass an empty block of data there should be no output as well:

void sanitytest::empty()
{
    uint32_t* p = nullptr;
    m_pSeg->m_idToSlots.push_back(2);    // id 0 is slot 2
    
    m_pSeg->checkBuffer(p, 0, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    EQ(std::iostream::pos_type(0), debugOutput.tellp());
}
// 1 header correctly formed is ok:

void sanitytest::ok_1()
{
    CMyEventSegment::HitHeader hdr;
    size_t n = makeOkHeader(&hdr, 0, 2, 5);
    m_pSeg->m_idToSlots.push_back(2);
    m_pSeg->checkBuffer(reinterpret_cast<uint32_t*>(&hdr), n, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    EQ(std::iostream::pos_type(0), debugOutput.tellp());
}
// 2 good headers.

void sanitytest::ok_2()
{
    CMyEventSegment::HitHeader hdr[2];
    size_t n = makeOkHeader(hdr, 0, 2, 5);
    n +=       makeOkHeader(&hdr[1], 0, 2, 7);
    
    m_pSeg->m_idToSlots.push_back(2);
    m_pSeg->checkBuffer(reinterpret_cast<uint32_t*>(&hdr[0]), n, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    EQ(std::iostream::pos_type(0), debugOutput.tellp());
}
// headers with payload (e.g. trace).

void sanitytest::ok_3()
{
    uint32_t buffer[1000];
    CMyEventSegment::HitHeader* hdr =
        reinterpret_cast<CMyEventSegment::HitHeader*>(buffer);
    hdr->s_id = makeId(0, 2, 5, 4, 104);
    hdr->s_traceInfo = 200 << 16;     // 16 bit samples not 32.
    
    m_pSeg->m_idToSlots.push_back(2);
    m_pSeg->checkBuffer(buffer, 104, 0);
    
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    EQ(std::iostream::pos_type(0), debugOutput.tellp());
    
}
// Two hits with 200 sample traces.

void
sanitytest::ok_4()
{
    uint32_t buffer[1000];
    CMyEventSegment::HitHeader* hdr =
        reinterpret_cast<CMyEventSegment::HitHeader*>(buffer);
    hdr->s_id = makeId(0, 2, 5, 4, 104);
    hdr->s_traceInfo = 200 << 16;     // 16 bit samples not 32.
    
    hdr =
        reinterpret_cast<CMyEventSegment::HitHeader*>(&buffer[104]);
    hdr->s_id = makeId(0, 2, 5, 4, 104);
    hdr->s_traceInfo = 200 << 16;     // 16 bit samples not 32.
    
    m_pSeg->m_idToSlots.push_back(2);
    m_pSeg->checkBuffer(buffer, 208, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    EQ(std::iostream::pos_type(0), debugOutput.tellp());
}

// report an error if the slot does not match what we expected (one hit).

void
sanitytest::wrongSlot_1()
{
    CMyEventSegment::HitHeader hdr;
    auto n = makeOkHeader(&hdr, 0, 3, 0);
    
    m_pSeg->m_idToSlots.push_back(2);
    m_pSeg->checkBuffer(reinterpret_cast<uint32_t*>(&hdr), n, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    
    ASSERT(debugOutput.tellp() > 0);

#ifdef DEBUG_OUTPUT    
    std::stringstream* pS = dynamic_cast<std::stringstream*>(&debugOutput);
    
    std::string s = pS->str();
    std::cout << "supposed to be an error: \n";
    std::cout << s;
#endif
}

// Second of two hits has the wrong slot:

void
sanitytest::wrongSlot_2()
{
    CMyEventSegment::HitHeader hdr[2];
    auto n = makeOkHeader(hdr, 0, 2, 5);
    n+= makeOkHeader(&hdr[1], 0, 4, 0);
    m_pSeg->m_idToSlots.push_back(2);
    m_pSeg->checkBuffer(reinterpret_cast<uint32_t*>(hdr), n, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    
    ASSERT(debugOutput.tellp() > 0);

#ifdef DEBUG_OUTPUT    
    std::stringstream* pS = dynamic_cast<std::stringstream*>(&debugOutput);
    
    std::string s = pS->str();
    std::cout << "supposed to be an error: \n";
    std::cout << s;
#endif
}
// middle of three hits is wrong slot:

void
sanitytest::wrongSlot_3()
{
    CMyEventSegment::HitHeader hdr[3];
    auto n = makeOkHeader(hdr, 0, 2, 5);
    n+= makeOkHeader(&hdr[1], 0, 4, 0);
    n+= makeOkHeader(&hdr[2], 0, 2, 7);
    
    m_pSeg->m_idToSlots.push_back(2);
    m_pSeg->checkBuffer(reinterpret_cast<uint32_t*>(hdr), n, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    
    ASSERT(debugOutput.tellp() > 0);

#ifdef DEBUG_OUTPUT    
    std::stringstream* pS = dynamic_cast<std::stringstream*>(&debugOutput);
    
    std::string s = pS->str();
    std::cout << "supposed to be an error: \n";
    std::cout << s;
#endif
}

// Header size is wrong

void
sanitytest::wronghdr_1()
{
    CMyEventSegment::HitHeader h;
    h.s_id = makeId(0, 2, 0, 5, 4);
    m_pSeg->m_idToSlots.push_back(2);
    m_pSeg->checkBuffer(reinterpret_cast<uint32_t*>(&h), 4, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    
    ASSERT(debugOutput.tellp() > 0);

#ifdef DEBUG_OUTPUT    
    std::stringstream* pS = dynamic_cast<std::stringstream*>(&debugOutput);
    
    std::string s = pS->str();
    std::cout << "supposed to be an error: \n";
    std::cout << s;
#endif   
    
}
// Header has bad event length

void
sanitytest::wronghdr_2()
{
    CMyEventSegment::HitHeader h;
    h.s_id = makeId(0, 2, 0, 4, 5);
    m_pSeg->m_idToSlots.push_back(2);
    m_pSeg->checkBuffer(reinterpret_cast<uint32_t*>(&h), 4, 0);
    
    std::ostream& debugOutput(m_pSeg->getDebugStream());
    
    ASSERT(debugOutput.tellp() > 0);

#ifdef DEBUG_OUTPUT    
    std::stringstream* pS = dynamic_cast<std::stringstream*>(&debugOutput);
    
    std::string s = pS->str();
    std::cout << "supposed to be an error: \n";
    std::cout << s;
#endif   
}
