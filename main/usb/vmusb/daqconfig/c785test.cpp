// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <C785.h>
#include <CReadoutModule.h>
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

using namespace std;


static const uint32_t base(0x80000000);	// change below if needed.
static const string   baseString("0x80000000");
static Warning needTDC("Need a 32 channel CAEN module at 0x80000000");


class c785test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(c785test);
  CPPUNIT_TEST(attach);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST(init);
  CPPUNIT_TEST(rdolist);
  CPPUNIT_TEST_SUITE_END();


private:
  C785*           m_pModule;
  CReadoutModule* m_pConfig;

public:
  void setUp() {
    m_pModule = new C785;
    m_pConfig = new CReadoutModule("test", *m_pModule);
  }
  void tearDown() {
    delete m_pConfig;
    delete m_pModule;
  }
protected:
  void attach();
  void config();
  void init();
  void rdolist();
};

CPPUNIT_TEST_SUITE_REGISTRATION(c785test);


// On creating a module, attaching it to a configuration should
// cause the right configuration entries and defaults to be made.
//
void c785test::attach() {
  string base                   = m_pConfig->cget("-base");
  string geo                    = m_pConfig->cget("-geo");
  string thresholds             = m_pConfig->cget("-thresholds");
  string issmall                = m_pConfig->cget("-smallthresholds");
  string ipl                    = m_pConfig->cget("-ipl");
  string vector                 = m_pConfig->cget("-vector");
  string highwater              = m_pConfig->cget("-highwater");
  string fastclear              = m_pConfig->cget("-fastclear");
  string suppress               = m_pConfig->cget("-supressrange");
  string suppressunderthreshold = m_pConfig->cget("-supressunderthreshold");
  string suppressoverflow       = m_pConfig->cget("-supressoverflow");

  // These don't have defaults and therefore should be blank:

  EQMSG("base", string("0"), base);
  EQMSG("geo",  string(""), geo);

  // These have defaults... be sure to change the code below if the
  // defaults change or assertions will fail.

  EQMSG("thresholds", 
	string("0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"),
	thresholds);
  EQMSG("issmall", string("false"), issmall);
  EQMSG("ipl", string("0"), ipl);
  EQMSG("vector", string("0"), vector);
  EQMSG("highwater", string("24"), highwater);
  EQMSG("fastclear", string("0"), fastclear);
  EQMSG("supress", string("false"), suppress);
  EQMSG("supressunderthreshold", string("true"), suppressunderthreshold);
  EQMSG("supressoverflow", string("false"), suppressoverflow);
}
// Configure the module: -base = baseString
//                       -geo  - 1
//
void c785test::config()
{
  m_pConfig->configure("-base", baseString);
  m_pConfig->configure("-geo",  "1");

  EQ(baseString, m_pConfig->cget("-base"));
  EQ(string("1"),          m_pConfig->cget("-geo"));
}

// Init the module:
//  and check the initialization.
void c785test::init()
{
  m_pConfig->configure("-base", baseString);
  m_pConfig->configure("-geo",  "1");
  m_pConfig->configure("-thresholds",
"0 1  2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31");
  m_pConfig->configure("-smallthresholds", "on");
  m_pConfig->configure("-fastclear", "12");
  m_pConfig->configure("-supressrange", true);
  m_pConfig->configure("-supressunderthreshold", true);
  m_pConfig->configure("-supressoverflow", false);

  vector<struct usb_device*> interfaces = CVMUSB::enumerate();
  CVMUSB vme(interfaces[0]);	// We assume there's at least one.

  m_pConfig->Initialize(vme);	// This should have done the device init.

  // Now bit by bit we need to check this.

  uint16_t value;
  vme.vmeRead16(base+0x1002, CVMUSBReadoutList::a32UserData, &value);
  // EQMSG("geo", 1, value & 0x1f);  // My device gets geo from the slot.

  // thresholds should all be initialized -> 0 & no kill bits.

  for (uint32_t chan =0; chan < 32; chan++) {
    vme.vmeRead16(base+0x1080+(chan*sizeof(uint16_t)), 
		  CVMUSBReadoutList::a32UserData, &value);
    EQMSG("Threshold", chan, (uint32_t)(value & 0x1ff));
  }
  // Reading the bitset2 register will tell us how the bits it sets
  // are programmed:

  vme.vmeRead16(base+0x1032, CVMUSBReadoutList::a32UserData, &value);
  EQMSG("Small thresholds", 0x100,  0x100 & value);

  EQMSG("supression", 0x10, value & 0x18);

  // IPL == vector == 0:

  vme.vmeRead16(base+0x100c, CVMUSBReadoutList::a32UserData, &value);
  EQMSG("Vector", 0, value & 0xff);
  vme.vmeRead16(base+0x100a,  CVMUSBReadoutList::a32UserData, &value);
  EQMSG("IPL", 0, value & 0x7);

  // Event trigger register has the high water mark number.

  vme.vmeRead16(base+0x1020, CVMUSBReadoutList::a32UserData, &value);
  EQMSG("trigger", 24, value & 0x1f);

  // fast clear window:

  vme.vmeRead16(base+0x102e, CVMUSBReadoutList::a32UserData, &value);
  EQMSG("fast clear", 12, value & 0x3ff);


  // Bitset 2 register should have all supression bits set.

  vme.vmeRead16(base+0x1032, CVMUSBReadoutList::a32UserData, &value);
  EQMSG("supression", 0x10, value & 0x18);

  // Control register must have 0x24:

  vme.vmeRead16(base+0x1010, CVMUSBReadoutList::a32UserData, &value);
  EQMSG("control1", 0x20, value & 0x74);
}
// Readout list executed immediately should return no data since there are
// no triggers.
//
void c785test::rdolist()
{
  vector<struct usb_device*> interfaces = CVMUSB::enumerate();
  CVMUSB vme(interfaces[0]);

  m_pConfig->configure("-base", baseString);
  m_pConfig->configure("-geo",  "1");
  m_pConfig->configure("-thresholds",
"0 1  2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31");
  m_pConfig->configure("-smallthresholds", "on");
  m_pConfig->configure("-fastclear", "12");


  m_pConfig->Initialize(vme);
  
  CVMUSBReadoutList list;
  m_pConfig->addReadoutList(list);


  uint32_t databuffer[34*32];
  size_t   readBytes;
  int status = vme.executeList(list, databuffer, sizeof(databuffer), &readBytes);

  // I would have thought I'd get readBytes == 0, status = 0.. but it appears
  // the BERR capability does not correclty work.

  EQMSG("status", 0, status);
}
