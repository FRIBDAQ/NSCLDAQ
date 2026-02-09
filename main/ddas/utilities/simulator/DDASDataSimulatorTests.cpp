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
    CPPUNIT_TEST(idword);
    CPPUNIT_TEST(idword_revh);
    CPPUNIT_TEST(word0);
    CPPUNIT_TEST(word0_revh);
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
    DDASHit m_hit, m_unpacked;
    DDASHitUnpacker m_unpacker;
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
    void idword();
    void idword_revh();
    void word0();
    void word0_revh();
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
SimulatorTests::idword()
{
    m_hit.Reset();
    m_unpacked.Reset();

    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(15);
    m_hit.setADCResolution(16);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);
    
    EQ(m_hit.getHardwareRevision(), m_unpacked.getHardwareRevision());
    EQ(m_hit.getADCResolution(), m_unpacked.getADCResolution());
    EQ(m_hit.getModMSPS(), m_unpacked.getModMSPS());
}

void
SimulatorTests::idword_revh()
{
    m_hit.Reset();
    m_unpacked.Reset();
    
    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(17);
    m_hit.setADCResolution(16);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);
    
    EQ(m_hit.getHardwareRevision(), m_unpacked.getHardwareRevision());
    EQ(m_hit.getADCResolution(), m_unpacked.getADCResolution());
    EQ(m_hit.getModMSPS(), m_unpacked.getModMSPS());
}

void
SimulatorTests::word0()
{
    m_hit.Reset();
    m_unpacked.Reset();

    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(15);
    m_hit.setFinishCode(0);
    m_hit.setCrateID(0);
    m_hit.setSlotID(2);
    m_hit.setChannelID(0);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);
    
    // Unpacker should set the channel length and header length based on 
    // the fixed header size for the hit options, which in this case is 
    // just the raw event of 4 32-bit words:

    EQ(m_hit.getFinishCode(), m_unpacked.getFinishCode());
    EQ(uint32_t(4), m_unpacked.getChannelLength());
    EQ(uint32_t(4), m_unpacked.getChannelHeaderLength());
    EQ(m_hit.getCrateID(), m_unpacked.getCrateID());
    EQ(m_hit.getSlotID(), m_unpacked.getSlotID());
    EQ(m_hit.getChannelID(), m_unpacked.getChannelID());
}

void
SimulatorTests::word0_revh()
{
    m_hit.Reset();
    m_unpacked.Reset();

    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(17);
    m_hit.setFinishCode(0);
    m_hit.setCrateID(0);
    m_hit.setSlotID(2);
    m_hit.setChannelID(0);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);

    // Unpacker should set the channel length and header length based on 
    // the fixed header size for the hit options, which in this case is 
    // just the raw event of 4 32-bit words:

    EQ(m_hit.getFinishCode(), m_unpacked.getFinishCode());
    EQ(uint32_t(4), m_unpacked.getChannelLength());
    EQ(uint32_t(4), m_unpacked.getChannelHeaderLength());
    EQ(m_hit.getCrateID(), m_unpacked.getCrateID());
    EQ(m_hit.getSlotID(), m_unpacked.getSlotID());
    EQ(m_hit.getChannelID(), m_unpacked.getChannelID());
}

void
SimulatorTests::word1and2_100()
{
    m_hit.Reset();
    m_unpacked.Reset();

    m_hit.setModMSPS(100);
    m_hit.setHardwareRevision(15);
    m_hit.setTime(1234.5678 + 10*static_cast<double>(std::pow(2,32)));
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);
    
    auto diff = m_hit.getTime() - 42949674194.5676;
    EQ(uint64_t(42949674190), m_unpacked.getCoarseTime());
    ASSERTMSG("100 MSPS reconstructed time", std::abs(diff) < 0.001);
}

void
SimulatorTests::word1and2_250()
{
    m_hit.Reset();
    m_unpacked.Reset();

    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(15);
    m_hit.setTime(1234.5678 + 10*static_cast<double>(std::pow(2,32)));
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);
    
    double diff = m_unpacked.getTime() - 42949674194.5676;    
    EQ(uint64_t(42949674192), m_unpacked.getCoarseTime());
    EQ(uint32_t(0), m_unpacked.getCFDTrigSource());
    ASSERTMSG("250 MSPS reconstructed time", std::abs(diff) < 0.001);
}

void
SimulatorTests::word1and2_500()
{
    m_hit.Reset();
    m_unpacked.Reset();

    m_hit.setModMSPS(500);
    m_hit.setHardwareRevision(15);
    m_hit.setTime(1234.5678 + 10*static_cast<double>(std::pow(2,32)));
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);
    
    double diff = m_unpacked.getTime() - 42949674194.5676;    
    EQ(uint64_t(42949674190), m_unpacked.getCoarseTime());
    EQ(uint32_t(3), m_unpacked.getCFDTrigSource());
    ASSERTMSG("500 MSPS reconstructed time", std::abs(diff) < 0.001);
}

void
SimulatorTests::word3()
{
    m_hit.Reset();
    m_unpacked.Reset();

    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(15);
    m_hit.setEnergy(9876);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);

    EQ(m_hit.getEnergy(), m_unpacked.getEnergy());
}

void
SimulatorTests::extTS()
{
    m_hit.Reset();
    m_unpacked.Reset();

    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(15);
    m_hit.setExternalTimestamp(1234);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);

    // Not specifiying a calibration for external TS is an error: 
    EXCEPTION(m_pSimulator->putHit(m_hit, 0, true), std::runtime_error);
    EQ(m_hit.getExternalTimestamp(), m_unpacked.getExternalTimestamp());
}

void
SimulatorTests::energySums()
{
    m_hit.Reset();
    m_unpacked.Reset();
    
    std::vector<uint32_t> sums;
    for (int i = 0; i < SIZE_OF_ENE_SUMS; i++) {
    sums.push_back(i);
    }

    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(15);
    m_hit.setEnergySums(sums);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);
    
    for (int i = 0; i < SIZE_OF_ENE_SUMS; i++) {
	EQ(m_hit.getEnergySums()[i], m_unpacked.getEnergySums()[i]);
    }
}

void
SimulatorTests::qdcSums()
{
    m_hit.Reset();
    m_unpacked.Reset();
    
    std::vector<uint32_t> sums;
    for (int i = 0; i < SIZE_OF_QDC_SUMS; i++) {
	sums.push_back(i);
    }

    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(15);
    m_hit.setQDCSums(sums);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);
    
    for (int i = 0; i < SIZE_OF_QDC_SUMS; i++) {
	EQ(m_hit.getQDCSums()[i], m_unpacked.getQDCSums()[i]);
    }
}

void
SimulatorTests::trace()
{
    m_hit.Reset();
    m_unpacked.Reset();

    std::vector<uint16_t> trace;    
    for (int i = 0; i < 10; i++) {
	trace.push_back(i);
    }
    
    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(15);   
    m_hit.setTrace(trace);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);

    EQ(m_hit.getTraceLength(), m_unpacked.getTraceLength());
    for (int i = 0; i < 10; i++) {
	EQ(m_hit.getTrace()[i], m_unpacked.getTrace()[i]);
    }
}

void
SimulatorTests::allOptions()
{
    m_hit.Reset();
    m_unpacked.Reset();
    
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
    
    m_hit.setModMSPS(250);
    m_hit.setHardwareRevision(15);
    m_hit.setExternalTimestamp(1234);
    m_hit.setEnergySums(esums);
    m_hit.setQDCSums(qdcsums);
    m_hit.setTrace(trace);
    
    m_pSimulator->setBuffer(m_hit);
    auto buf = m_pSimulator->getBuffer();
    m_unpacker.unpack(buf.data(), buf.data() + buf.size(), m_unpacked);

    // Ensure the event lengths are correct and the optional data are
    // set properly:
    
    uint32_t hdrLen = SIZE_OF_RAW_EVENT + SIZE_OF_EXT_TS
	+ SIZE_OF_ENE_SUMS + SIZE_OF_QDC_SUMS;
    uint32_t chanLen = hdrLen + std::ceil(trace.size()/2);
    EQ(hdrLen, m_unpacked.getChannelHeaderLength());
    EQ(chanLen, m_unpacked.getChannelLength());
    EQ(m_hit.getExternalTimestamp(), m_unpacked.getExternalTimestamp());
    for (int i = 0; i < SIZE_OF_ENE_SUMS; i++) {
	EQ(m_hit.getEnergySums()[i], m_unpacked.getEnergySums()[i]);
    }
    for (int i = 0; i < SIZE_OF_QDC_SUMS; i++) {
	EQ(m_hit.getQDCSums()[i], m_unpacked.getQDCSums()[i]);
    }
    for (int i = 0; i < 10; i++) {
	EQ(m_hit.getTrace()[i], m_unpacked.getTrace()[i]);
    }
}
