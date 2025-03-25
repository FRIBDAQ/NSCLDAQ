/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
	     Aaron Chester
	     Facility for Rare Isotope Beams
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include <cmath>

#include "DDASDataSimulator.h"
#include "DDASHitUnpacker.h"
#include "DDASBitMasks.h"

#include "Asserts.h"

using namespace ddasfmt; // Format, unpackers, etc.
using namespace DAQ::DDAS; // Simulation framework.

/** @todo (ASC 11/6/24): Write tests for specifying only coarse TS. */

/**
 * @brief Test suite for the DDAS data simulation library.
 * @details
 * Pack a DDASHit into a Pixie data payload, unpack the payload and verify 
 * that the packed data has been set properly.
 */

class SimulatorTests : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(SimulatorTests);
    CPPUNIT_TEST(word0);
    CPPUNIT_TEST(word1and2_100);
    CPPUNIT_TEST(word1and2_250);
    CPPUNIT_TEST(word1and2_500);
    CPPUNIT_TEST(word3);
    CPPUNIT_TEST(extTS);
    CPPUNIT_TEST(energySums);
    CPPUNIT_TEST(qdcSums);
    CPPUNIT_TEST(trace);
    CPPUNIT_TEST(allOptions);
    CPPUNIT_TEST_SUITE_END();
    
private:
    DDASHitUnpacker   m_unpacker;
    DDASDataSimulator* m_pSimulator;

public:    
    void setUp()
	{
	    m_pSimulator = new DDASDataSimulator("dummyfile", 12);
	};
    void tearDown()
	{
	    delete m_pSimulator;
	};
    
protected:
    void word0();
    void word1and2_100();
    void word1and2_250();
    void word1and2_500();
    void word3();
    void extTS();
    void energySums();
    void qdcSums();
    void trace();
    void allOptions();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SimulatorTests);

void
SimulatorTests::word0()
{
    DDASHit hit, unpacked;
    
    hit.setModMSPS(250); 
    hit.setFinishCode(0);
    hit.setCrateID(0);
    hit.setSlotID(2);
    hit.setChannelID(0);
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);
    
    EQ(hit.getFinishCode(), unpacked.getFinishCode());
    EQ(hit.getCrateID(), unpacked.getCrateID());
    EQ(hit.getSlotID(), unpacked.getSlotID());
    EQ(hit.getChannelID(), unpacked.getChannelID());
}

void
SimulatorTests::word1and2_100()
{
    DDASHit hit, unpacked;

    hit.setModMSPS(100);    
    hit.setTime(1234.5678 + 10*static_cast<double>(std::pow(2,32)));
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);
    
    auto diff = hit.getTime() - 42949674194.5676;
    EQ(uint64_t(42949674190), unpacked.getCoarseTime());
    ASSERTMSG("100 MSPS reconstructed time", std::abs(diff) < 0.001);
}

void
SimulatorTests::word1and2_250()
{
    DDASHit hit, unpacked;
    
    hit.setModMSPS(250);
    hit.setTime(1234.5678 + 10*static_cast<double>(std::pow(2,32)));
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);
    
    double diff = unpacked.getTime() - 42949674194.5676;    
    EQ(uint64_t(42949674192), unpacked.getCoarseTime());
    EQ(uint32_t(0), unpacked.getCFDTrigSource());
    ASSERTMSG("250 MSPS reconstructed time", std::abs(diff) < 0.001);
}

void
SimulatorTests::word1and2_500()
{
    DDASHit hit, unpacked;
    
    hit.setModMSPS(500);
    hit.setTime(1234.5678 + 10*static_cast<double>(std::pow(2,32)));
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);
    
    double diff = unpacked.getTime() - 42949674194.5676;    
    EQ(uint64_t(42949674190), unpacked.getCoarseTime());
    EQ(uint32_t(3), unpacked.getCFDTrigSource());
    ASSERTMSG("500 MSPS reconstructed time", std::abs(diff) < 0.001);
}

void
SimulatorTests::word3()
{
    DDASHit hit, unpacked;
    
    hit.setModMSPS(250);
    hit.setEnergy(9876);
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);

    EQ(hit.getEnergy(), unpacked.getEnergy());
}

void
SimulatorTests::extTS()
{
    DDASHit hit, unpacked;
    
    hit.setModMSPS(250);
    hit.setExternalTimestamp(1234);
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);

    // Not specifiying a calibration for external TS is an error: 
    EXCEPTION(m_pSimulator->putHit(hit, 0, true), std::runtime_error);
    EQ(hit.getExternalTimestamp(), unpacked.getExternalTimestamp());
}

void
SimulatorTests::energySums()
{
    DDASHit hit, unpacked;
    std::vector<uint32_t> sums;
    for (int i = 0; i < SIZE_OF_ENE_SUMS; i++) {
	sums.push_back(i);
    }

    hit.setModMSPS(250);
    hit.setEnergySums(sums);
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);
    
    for (int i = 0; i < SIZE_OF_ENE_SUMS; i++) {
	EQ(hit.getEnergySums()[i], unpacked.getEnergySums()[i]);
    }
}

void
SimulatorTests::qdcSums()
{
    DDASHit hit, unpacked;
    std::vector<uint32_t> sums;
    for (int i = 0; i < SIZE_OF_QDC_SUMS; i++) {
	sums.push_back(i);
    }

    hit.setModMSPS(250);
    hit.setQDCSums(sums);
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);
    
    for (int i = 0; i < SIZE_OF_QDC_SUMS; i++) {
	EQ(hit.getQDCSums()[i], unpacked.getQDCSums()[i]);
    }
}

void
SimulatorTests::trace()
{
    DDASHit hit, unpacked;
    std::vector<uint16_t> trace;
    
    for (int i = 0; i < 10; i++) {
	trace.push_back(i);
    }
    
    hit.setModMSPS(250);   
    hit.setTrace(trace);
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);

    EQ(hit.getTraceLength(), unpacked.getTraceLength());
    for (int i = 0; i < 10; i++) {
	EQ(hit.getTrace()[i], unpacked.getTrace()[i]);
    }
}

void
SimulatorTests::allOptions()
{
    DDASHit hit, unpacked;
    std::vector<uint32_t> esums, qdcsums;
    std::vector<uint16_t> trace;
    
    for (int i = 0; i < SIZE_OF_ENE_SUMS; i++) {
	esums.push_back(i);
    }
    for (int i = 0; i < SIZE_OF_QDC_SUMS; i++) {
	qdcsums.push_back(i);
    }
    for (int i = 0; i < 10; i++) {
	trace.push_back(i);
    }
    
    hit.setModMSPS(250);
    hit.setExternalTimestamp(1234);
    hit.setEnergySums(esums);
    hit.setQDCSums(qdcsums);
    hit.setTrace(trace);
    
    m_pSimulator->setBuffer(hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), unpacked);

    // Ensure the event lengths are correct and the optional data are
    // set properly:
    
    uint32_t hdrLen = SIZE_OF_RAW_EVENT + SIZE_OF_EXT_TS
	+ SIZE_OF_ENE_SUMS + SIZE_OF_QDC_SUMS;
    uint32_t chanLen = hdrLen + std::ceil(trace.size()/2);
    EQ(hdrLen, unpacked.getChannelHeaderLength());
    EQ(chanLen, unpacked.getChannelLength());
    EQ(hit.getExternalTimestamp(), unpacked.getExternalTimestamp());
    for (int i = 0; i < SIZE_OF_ENE_SUMS; i++) {
	EQ(hit.getEnergySums()[i], unpacked.getEnergySums()[i]);
    }
    for (int i = 0; i < SIZE_OF_QDC_SUMS; i++) {
	EQ(hit.getQDCSums()[i], unpacked.getQDCSums()[i]);
    }
    for (int i = 0; i < 10; i++) {
	EQ(hit.getTrace()[i], unpacked.getTrace()[i]);
    }
}
