// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <usb.h>
#include <CVMUSB.h>
#include <CVMUSBusb.h>
#include <vector>
#include <stdio.h>

using namespace std;

static Warning msg(string("regTests requires at least one VM-USB interface"));

class registerTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(registerTests);
  CPPUNIT_TEST(action);
  CPPUNIT_TEST(fwid);
  CPPUNIT_TEST(globmode);
  CPPUNIT_TEST(daqsettings);
  CPPUNIT_TEST(ledsrc);
  CPPUNIT_TEST(devsrc);
  CPPUNIT_TEST(dgg);
  CPPUNIT_TEST(vectors);
  CPPUNIT_TEST(bulksetup);
  CPPUNIT_TEST_SUITE_END();


private:
  struct usb_device*   m_dev;
  CVMUSB*  m_pInterface;
  const CVMUSB::ShadowRegisters* m_pShadow;

public:
  void setUp() {
    vector<struct usb_device*> devices = CVMUSB::enumerate();
    if (devices.size() == 0) {
      cerr << " NO USB interfaces\n";
      exit(0);
    }
    m_pInterface = new CVMUSBusb(devices[0]);
    m_pShadow = &m_pInterface->getShadowRegisters();
  }
  void tearDown() {
    delete m_pInterface;
    m_pShadow=0;
  }
protected:
  void action();
  void fwid();
  void globmode();
  void daqsettings();
  void ledsrc();
  void devsrc();
  void dgg();
  void vectors();
  void bulksetup();
};

CPPUNIT_TEST_SUITE_REGISTRATION(registerTests);

//write action register... can only determine that no throws happen.

void registerTests::action() {
  m_pInterface->writeActionRegister(0);	// this is really the only safe thing to write here.
				   
}

// Read firmware: can only output and hope no exceptions.

void registerTests::fwid()
{
  uint32_t fw = m_pInterface->readFirmwareID();
}
// Write 0's to the gobal mode register... all the 
// bits that count should match.. then turn them all  on too.
//
void registerTests::globmode()
{
  uint32_t validmask = 0x71ff;	// Bits that matter in the register.
  m_pInterface->writeGlobalMode(0);
  EQMSG("wrote shadow 0", (uint16_t)0, m_pShadow->globalMode);

  uint32_t mode = m_pInterface->readGlobalMode();
  EQMSG("wrote 0", 0U, mode & validmask);
  EQMSG("read 0", (uint16_t)(mode & validmask), m_pShadow->globalMode);

  m_pInterface->writeGlobalMode(0xffff);
  EQMSG("wrote shadow 0xffff", (uint16_t)0xffff, m_pShadow->globalMode);
  mode   = m_pInterface->readGlobalMode();
  EQMSG("wrote 1s", validmask, (uint32_t)(validmask & mode));
  EQMSG("read shadow 0xffff", validmask, (uint32_t)(validmask & m_pShadow->globalMode));

  // This left us with an invalid bus request level so:

  m_pInterface->writeGlobalMode((uint32_t)(4 << CVMUSB::GlobalModeRegister::busReqLevelShift));

}
// Daq settings can accept any bit patterns.

void registerTests::daqsettings()
{
  m_pInterface->writeDAQSettings(0xffffffff);
  EQMSG("wrote shadow ones", (uint32_t)0xffffffff, m_pShadow->daqSettings);
  uint32_t settings = m_pInterface->readDAQSettings();
  EQMSG("ones", (uint32_t)0xffffffff, settings);
  EQMSG("read shadow ones", (uint32_t)0xffffffff, m_pShadow->daqSettings);
 
  m_pInterface->writeDAQSettings(0);
  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->daqSettings);
  settings = m_pInterface->readDAQSettings();
  EQMSG("0's", (uint32_t)0, settings);
  EQMSG("read shadow zeroes", (uint32_t)0, m_pShadow->daqSettings);
}
// LEDSrc has somed dead bits.

void registerTests::ledsrc()
{
  uint32_t usedBits = 0x1f1f1f1f;

  m_pInterface->writeLEDSource(0xffffffff);
  EQMSG("wrote shadow ones", (uint32_t)0xffffffff, m_pShadow->ledSources);
  uint32_t value = m_pInterface->readLEDSource();
  EQMSG("ones", usedBits, (value & usedBits));
  EQMSG("read shadow ones", usedBits, m_pShadow->ledSources);

  m_pInterface->writeLEDSource(0);
  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->ledSources);
  value = m_pInterface->readLEDSource();

  EQMSG("0's", (uint32_t)0, (value & usedBits));
  EQMSG("read shadow zeroes", (uint32_t)0, m_pShadow->ledSources);

}
// Dev src register also has some dead bits.

void registerTests::devsrc()
{
  uint32_t usedBits =    0x77331f1f; // 7777 not 77ff since reset is momentary.

//  m_pInterface->writeDeviceSource(0x7fffffff);
  m_pInterface->writeDeviceSource(usedBits);
  EQMSG("wrote shadow ones", (uint32_t)usedBits, m_pShadow->deviceSources);
  uint32_t value = m_pInterface->readDeviceSource();
  EQMSG("ones", usedBits, (value & usedBits));
  EQMSG("read shadow ones", (uint32_t)usedBits, (usedBits & m_pShadow->deviceSources));

  m_pInterface->writeDeviceSource(0);
  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->deviceSources);
  value = m_pInterface->readDeviceSource();
  EQMSG("0's", (uint32_t)0, (value & usedBits));
  EQMSG("read shadow zeroes", (uint32_t)0, (usedBits & m_pShadow->deviceSources));
}
// Three registers for the gate and delay register use all 32 bits.
// dgga, dggb, dggextended.

void registerTests::dgg()
{

  // DGG A control register.

  m_pInterface->writeDGG_A(0xffffffff);
  EQMSG("wrote shadow ones", (uint32_t)0xffffffff, m_pShadow->dggA);
  uint32_t value = m_pInterface->readDGG_A();
  EQMSG("DGGA 1's", (uint32_t)0xffffffff, value);
  EQMSG("read shadow ones", (uint32_t)0xffffffff, m_pShadow->dggA);

  m_pInterface->writeDGG_A(0);
  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->dggA);
  value = m_pInterface->readDGG_A();
  EQMSG("DGGA 0's", (uint32_t)0, value);
  EQMSG("read shadow zeroes", (uint32_t)0, m_pShadow->dggA);

  // DGG B control register.

  m_pInterface->writeDGG_B(0xffffffff);
  EQMSG("wrote shadow ones", (uint32_t)0xffffffff, m_pShadow->dggB);
  value = m_pInterface->readDGG_B();
  EQMSG("DGGB 1's", (uint32_t)0xffffffff, value);
  EQMSG("read shadow ones", (uint32_t)0xffffffff, m_pShadow->dggB);

  m_pInterface->writeDGG_B(0);
  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->dggB);
  value = m_pInterface->readDGG_B();
  EQMSG("DGGB 0's", (uint32_t)0, value);
  EQMSG("read shadow zeroes", (uint32_t)0, m_pShadow->dggB);

  // DGG Extended control register.

  m_pInterface->writeDGG_Extended(0xffffffff);
  EQMSG("wrote shadow ones", (uint32_t)0xffffffff, m_pShadow->dggExtended);
  value = m_pInterface->readDGG_Extended();
  EQMSG("Extended 1's", (uint32_t)0xffffffff, value);
  EQMSG("read shadow ones", (uint32_t)0xffffffff, m_pShadow->dggExtended);

  m_pInterface->writeDGG_Extended(0);
  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->dggExtended);
  value = m_pInterface->readDGG_Extended();
  EQMSG("Extended 0's", (uint32_t)0, value);
  EQMSG("read shadow zeroes", (uint32_t)0, m_pShadow->dggExtended);
  
}


// Vectors: only some bits are used.
void registerTests::vectors()
{
  uint32_t usedBits = 0x77ff77ff;

  for (int vec =1; vec<=4; vec++) {
    char msg[100];
    
    sprintf(msg, "vector %d, 1's", vec);
    m_pInterface->writeVector(vec, 0xffffffff);
    EQMSG("wrote shadow ones", (uint32_t)0xffffffff, m_pShadow->interruptVectors.at(2*vec-1));
    uint32_t value = m_pInterface->readVector(vec);
    EQMSG(msg, (uint32_t)usedBits, (value & usedBits));
    EQMSG("read shadow ones", (uint32_t)0xffffffff, m_pShadow->interruptVectors.at(2*vec-1));

    sprintf(msg, "vector %d 0's", vec);
    m_pInterface->writeVector(vec, 0);
    EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->interruptVectors.at(2*vec-1));
    value = m_pInterface->readVector(vec);
    EQMSG(msg, (uint32_t)0, (value & usedBits));
    EQMSG("read shadow zeroes", (uint32_t)0, m_pShadow->interruptVectors.at(2*vec-1));

  }
}

// only some bits of the bulk transfer setup register are used.

void registerTests::bulksetup()
{
  uint32_t usedBits = 0xfff;
  m_pInterface->writeBulkXferSetup(0xffffffff);
  EQMSG("wrote shadow onees", (uint32_t)0xffffffff, m_pShadow->bulkTransferSetup);
  uint32_t value = m_pInterface->readBulkXferSetup();
  EQMSG("1's", usedBits, (value & usedBits));
  EQMSG("read shadow ones", usedBits, (usedBits & m_pShadow->bulkTransferSetup));

  m_pInterface->writeBulkXferSetup(0);
  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->bulkTransferSetup);
  value = m_pInterface->readBulkXferSetup();
  EQMSG("0's", (uint32_t)0, (value & usedBits));
  EQMSG("read shadow zeroes", (uint32_t)0, (usedBits & m_pShadow->bulkTransferSetup));
}


