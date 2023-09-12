/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             
*/

/** @file:  triggertests.cpp
 *  @brief: Tests for endpoint selection and getting an event with sw trigger.
 *  
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include "Dig2Device.h"
#include "Asserts.h"

extern bool isUsb;
extern std::string connection;

static const int TRIGGER_COUNT=100;
static const int DELAY_US=1;

#include "VX2750Pha.h"
using namespace caen_nscldaq;

class triggertests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(triggertests);
    CPPUNIT_TEST(raw_1);
    CPPUNIT_TEST(cooked_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    VX2750Pha* m_pModule;
public:
    void setUp() {
      caen_nscldaq::set_tracing(false);
        m_pModule = new VX2750Pha(connection.c_str(), isUsb);
        m_pModule->Reset();

    }
    void tearDown() {
        delete m_pModule;
    }
protected:
    void raw_1();
    void cooked_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(triggertests);

// Set up the module to
// -  Enable channel 0
// -  Allow Sw trigger on channel 0
// - select raw endpoint.
// Arm
// start
// SWtrigger
// read
// stop
// disarm.
void triggertests::raw_1()
{
    // Disable all channels except 0
    
    unsigned nch = m_pModule->channelCount();
    for (int c = 0; c < nch; c++) {
        m_pModule->enableChannel(c, true);
    }
    // I think this should give me what I want...
    
    
    std::vector<VX2750Pha::StartSource> start = {VX2750Pha::SWCommand};
    m_pModule->setStartSource(start);                         // ensure sw start is ok.
    m_pModule->setEventSelector(0, VX2750Pha::All);
    m_pModule->setWaveformSelector(0, VX2750Pha::All);
    std::vector<VX2750Pha::GlobalTriggerSource> gbltrig =
        {VX2750Pha::GlobalTrigger_Software};
    m_pModule->setGlobalTriggerSource(gbltrig);
    std::vector<VX2750Pha::WaveTriggerSource> wt = {VX2750Pha::WaveTrigger_GlobalTriggerSource};
    m_pModule->setWaveTriggerSource(0, wt);
    std::vector<VX2750Pha::EventTriggerSource> et = {VX2750Pha::EventTrigger_GlobalTriggerSource};
    m_pModule->setEventTriggerSource(0, et);
    
    
    uint32_t rawSize = m_pModule->getMaxRawDataSize();
    CPPUNIT_ASSERT_NO_THROW(m_pModule->selectEndpoint(VX2750Pha::Raw));
    CPPUNIT_ASSERT_NO_THROW(m_pModule->initializeRawEndpoint());
    
    std::uint8_t* pData = new std::uint8_t[rawSize];;
    size_t nRead;
    // From here on we encapsulatein a try/catch/rethrow block so that
    // our data block can be deleted if a test fails...as
    // test failures are exceptions.
    
    
    try {
        bool hasdata;
        m_pModule->Clear();
        m_pModule->Arm();
        m_pModule->Start();
        CPPUNIT_ASSERT_NO_THROW(m_pModule->Start());
        for (int i =0; i < TRIGGER_COUNT; i++) CPPUNIT_ASSERT_NO_THROW(m_pModule->Trigger());
        usleep(DELAY_US);
        CPPUNIT_ASSERT_NO_THROW(hasdata = m_pModule->hasData());
        ASSERT(hasdata);
        
        CPPUNIT_ASSERT_NO_THROW(nRead = m_pModule->readRawEndpoint(pData));
        ASSERT(nRead > 0);
        
        m_pModule->Stop();
        m_pModule->Disarm();
        m_pModule->Clear();
        
        
    }
    catch(...) {
        delete []pData;
        throw;
    }
    
     
}
// Default format for DPP readout
void
triggertests::cooked_1()
{
     // Disable all channels except 0
    
    unsigned nch = m_pModule->channelCount();
    for (int c = 0; c < nch; c++) {
        m_pModule->enableChannel(c, true);
    }
    // I think this should give me what I want...
    
    std::vector<VX2750Pha::StartSource> start = {VX2750Pha::SWCommand};
    m_pModule->setStartSource(start);                         // ensure sw start is ok.
    m_pModule->setEventSelector(0, VX2750Pha::All);
    m_pModule->setWaveformSelector(0, VX2750Pha::All);
    std::vector<VX2750Pha::GlobalTriggerSource> gbltrig =
        {VX2750Pha::GlobalTrigger_Software};
    m_pModule->setGlobalTriggerSource(gbltrig);
    std::vector<VX2750Pha::WaveTriggerSource> wt = {VX2750Pha::WaveTrigger_GlobalTriggerSource};
    m_pModule->setWaveTriggerSource(0, wt);
    std::vector<VX2750Pha::EventTriggerSource> et = {VX2750Pha::EventTrigger_GlobalTriggerSource};
    m_pModule->setEventTriggerSource(0, et);
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setDefaultFormat());
    
    m_pModule->selectEndpoint(VX2750Pha::PHA);
    CPPUNIT_ASSERT_NO_THROW(m_pModule->initializeDPPPHAReadout());
    
    
    
    VX2750Pha::DecodedEvent e;
    CPPUNIT_ASSERT_NO_THROW(m_pModule->initDecodedBuffer(e));
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setupDecodedBuffer(e));
    
    // At this point we need to ensure the data in the decoded buffer is freed:
    
    try {
        bool hasdata;
        m_pModule->Clear();
	
        m_pModule->Arm();
        m_pModule->Start();
	
        CPPUNIT_ASSERT_NO_THROW(m_pModule->Start());
        for (int i =0; i < TRIGGER_COUNT; i++) CPPUNIT_ASSERT_NO_THROW(m_pModule->Trigger());
        usleep(DELAY_US);
        CPPUNIT_ASSERT_NO_THROW(hasdata = m_pModule->hasData());
        ASSERT(hasdata);
        CPPUNIT_ASSERT_NO_THROW(m_pModule->readDPPPHAEndpoint(e));
    }
    catch (...) {
        m_pModule->freeDecodedBuffer(e);
        throw;
    }
    
    

    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->freeDecodedBuffer(e));
}
