/*
    
    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
    
*/

/** @file:  devtests.cpp
 *  @brief: Test suite for Dig2Device class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "Dig2Device.h"
#include <cmath>
#include <iostream>
// Connection parameters:
extern std::string connection;
extern bool        isUsb;

class devtest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(devtest);
  CPPUNIT_TEST(serial);    // Base parameter getting checks.
  CPPUNIT_TEST(getnum);
  CPPUNIT_TEST(getfloat);
  CPPUNIT_TEST(isPHA);

  CPPUNIT_TEST(devstring); // Convenience functions for
  CPPUNIT_TEST(devnum);    // device level parameters.
  CPPUNIT_TEST(devfloat);

  CPPUNIT_TEST(chanstring); // Convenience functions for
  CPPUNIT_TEST(channum);    // channel level params.
  CPPUNIT_TEST(chanfloat);

  // Set at top level (need full path).

  CPPUNIT_TEST(setclocksrc);
  CPPUNIT_TEST(setvetowidth);
  CPPUNIT_TEST(enclockFP);       // boolean
  
  // Set at device level:
  
  CPPUNIT_TEST(devsetclocksrc);
  CPPUNIT_TEST(devsetvetowidth);
  CPPUNIT_TEST(devenclockFP);
  CPPUNIT_TEST(devvolclkdelay);
  
  CPPUNIT_TEST(chsetstring);
  CPPUNIT_TEST(chsetint);
  CPPUNIT_TEST(chsetfloat);
  CPPUNIT_TEST(chsetbool);
  
  CPPUNIT_TEST(lvdsstring);
  //CPPUNIT_TEST(lvdsint);   There are no lvds ints.
  
  CPPUNIT_TEST_SUITE_END();
    
private:
    caen_nscldaq::Dig2Device* m_pConnection;
public:
    void setUp() {
        m_pConnection = new caen_nscldaq::Dig2Device(connection.c_str(), isUsb);
    }
    void tearDown() {
        delete m_pConnection;
    }
protected:
  void serial();
  void getnum();
  void getfloat();
  void isPHA();

  void devstring();
  void devnum();
  void devfloat();

  void chanstring();
  void channum();
  void chanfloat();

  void setclocksrc();
  void setvetowidth();
  void enclockFP();
  
  void devsetclocksrc();
  void devsetvetowidth();
  void devenclockFP();
  void devvolclkdelay();
  
  void chsetstring();
  void chsetint();
  void chsetfloat();
  void chsetbool();
  
  void lvdsstring();
  void lvdsint();
};

CPPUNIT_TEST_SUITE_REGISTRATION(devtest);

// Read serial - it's the same as the PID -- assumes
// a usb connection.

void devtest::serial()
{
  
    std::string ser;
    CPPUNIT_ASSERT_NO_THROW(
	 ser = m_pConnection->GetValue("/par/SerialNum")
    );
  if (isUsb)  EQ(connection, ser);
}

// Number of channels is an integer:

void devtest::getnum()
{
  int chans;
  CPPUNIT_ASSERT_NO_THROW(
     chans = m_pConnection->GetInteger("/par/NumCh")
  );
  EQ(64, chans);
}


// The volatile and permanent clock delays  is a float but
// damned if I know what to check it against.
void devtest::getfloat()
{
  float vol, perm;
  CPPUNIT_ASSERT_NO_THROW(
      vol = m_pConnection->GetReal("/par/VolatileClockOutDelay")
  );
  
}

// We're going to use dpp-pha parameter so we need to know
// that's the firmware type:

void devtest::isPHA()
{
  std::string fwType = m_pConnection->GetValue("/par/FwType");
  EQ(std::string("DPP_PHA"), fwType);
}

void devtest::devstring()
{
  std::string ser;
  CPPUNIT_ASSERT_NO_THROW(
    ser = m_pConnection->GetDeviceValue("SerialNum")
  );
  if (isUsb) EQ(connection ,ser);
}
void devtest::devnum() {
  int chans;
  CPPUNIT_ASSERT_NO_THROW(
     chans = m_pConnection->GetDeviceInteger("NumCh")
  );
  EQ(64, chans);
}
void devtest::devfloat()
{
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->GetDeviceReal("VolatileClockOutDelay"));
}
void devtest::chanstring()
{
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->GetChanValue(0,"EventTriggerSource"));
}

void devtest::channum()
{
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->GetChanInteger(0, "ADCVetoWidth"));
}

void devtest::chanfloat()
{
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->GetChanReal(0, "DCOffset"));
}

void devtest::setclocksrc()
{
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetValue("/par/ClockSource", "FPClkIn"));
  EQ(std::string("FPClkIn"), m_pConnection->GetDeviceValue("ClockSource"));

  // Set it to internal:

  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetValue("/par/ClockSource", "Internal"));
  EQ(std::string("Internal"), m_pConnection->GetDeviceValue("ClockSource"));
}
void devtest::setvetowidth()
{
  int original = m_pConnection->GetDeviceInteger("BoardVetoWidth");

  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetValue("/par/BoardVetoWidth", 0));
  EQ(0, m_pConnection->GetDeviceInteger("BoardVetoWidth"));
  m_pConnection->SetValue("/par/BoardVetoWidth", 1232); // granular
  EQ(1232, m_pConnection->GetDeviceInteger("BoardVetoWidth"));
  m_pConnection->SetValue("/par/BoardVetoWidth", original);
  EQ(original, m_pConnection->GetDeviceInteger("BoardVetoWidth"));
			  
  
}

void devtest::enclockFP()
{
  bool original = m_pConnection->GetDeviceBool("EnClockOutFP");
  
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetValue("/par/EnClockOutFP", true));
  EQ(true, m_pConnection->GetDeviceBool("EnClockOutFP"));
  
  m_pConnection->SetValue("/par/EnClockOutFP", false);
  EQ(false, m_pConnection->GetDeviceBool("EnClockOutFP"));
  
  m_pConnection->SetValue("/par/EnClockOutFP", original);
}


//
void devtest::devsetclocksrc()
{
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetDeviceValue("ClockSource", "FPClkIn"));
  EQ(std::string("FPClkIn"), m_pConnection->GetDeviceValue("ClockSource"));

  // Set it to internal:

  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetDeviceValue("ClockSource", "Internal"));
  EQ(std::string("Internal"), m_pConnection->GetDeviceValue("ClockSource"));
}
void devtest::devsetvetowidth()
{
  int original = m_pConnection->GetDeviceInteger("BoardVetoWidth");

  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetDeviceValue("BoardVetoWidth", 0));
  EQ(0, m_pConnection->GetDeviceInteger("BoardVetoWidth"));
  m_pConnection->SetDeviceValue("BoardVetoWidth", 1232); // granular
  EQ(1232, m_pConnection->GetDeviceInteger("BoardVetoWidth"));
  m_pConnection->SetDeviceValue("BoardVetoWidth", original);
  EQ(original, m_pConnection->GetDeviceInteger("BoardVetoWidth"));
			  
  
}

void devtest::devenclockFP()
{
  bool original = m_pConnection->GetDeviceBool("EnClockOutFP");
  
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetDeviceValue("EnClockOutFP", true));
  EQ(true, m_pConnection->GetDeviceBool("EnClockOutFP"));
  
  m_pConnection->SetDeviceValue("EnClockOutFP", false);
  EQ(false, m_pConnection->GetDeviceBool("EnClockOutFP"));
  
  m_pConnection->SetDeviceValue("EnClockOutFP", original);
}
void devtest::devvolclkdelay()
{
  double original = m_pConnection->GetDeviceReal("VolatileClockOutDelay");
  
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetDeviceValue("VolatileClockOutDelay", 12345.3));
  
  // there's granularity and it's not documented what that granularity is so
  // guessing below there's a 50ps granularity or less.  FOr these values
  // that's empirically seen to be true but....*sigh*
  
  double now = m_pConnection->GetDeviceReal("VolatileClockOutDelay");
  ASSERT(std::abs(now - 12345.3) < 50.0);
  
  m_pConnection->SetDeviceValue("VolatileClockOutDelay", 5000.7);
  now = m_pConnection->GetDeviceReal("VolatileClockOutDelay");
  ASSERT(std::abs(now - 5000.7) < 50.0);
  
  m_pConnection->SetDeviceValue("VolatileClockOutDelay", original);
}

void devtest::chsetstring()
{
  std::string original = m_pConnection->GetChanValue(0, "EventTriggerSource");
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetChanValue(0, "EventTriggerSource", "ITLA"));
  EQ(std::string("ITLA"), m_pConnection->GetChanValue(0, "EventTriggerSource"));
  
  m_pConnection->SetChanValue(0, "EventTriggerSource", "ChSelfTrigger");
  EQ(std::string("ChSelfTrigger"), m_pConnection->GetChanValue(0, "EventTriggerSource"));
  
  m_pConnection->SetChanValue(0, "EventTriggerSource", original.c_str());
                          
}
void devtest::chsetint()
{
  int original = m_pConnection->GetChanInteger(0, "ADCVetoWidth");
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetChanValue(0, "ADCVetoWidth", 0));
  EQ(0, m_pConnection->GetChanInteger(0, "ADCVetoWidth"));
  
  m_pConnection->SetChanValue(0, "ADCVetoWidth", 2000);
  EQ(2000, m_pConnection->GetChanInteger(0, "ADCVetoWidth"));
  
  m_pConnection->SetChanValue(0, "ADCVetoWidth", original);
}

void devtest::chsetfloat()
{
  double original = m_pConnection->GetChanReal(0, "DCOffset");
  
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetChanValue(0, "DCOffset", 0.0));
  EQ(0.0, m_pConnection->GetChanReal(0, "DCOffset"));
  
  m_pConnection->SetChanValue(0, "DCOffset", 50.0);
  ASSERT(std::abs(50.0 - m_pConnection->GetChanReal(0, "DCOffset")) < 1.0);
  
  m_pConnection->SetChanValue(original, "DCOffset", original);
}

void devtest::chsetbool()
{
  bool original = m_pConnection->GetChanBool(0, "ChEnable");
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetChanValue(0, "ChEnable", true));
  ASSERT(m_pConnection->GetChanBool(0, "ChEnable"));
  
  m_pConnection->SetChanValue(0, "ChEnable", false);
  ASSERT(!m_pConnection->GetChanBool(0, "ChEnable"));
  
  m_pConnection->SetChanValue(0, "ChEnable", original);
}

// lvds - group set - reads and writes:

void devtest::lvdsstring()
{
  std::string old = m_pConnection->GetLVDSValue(0, "LVDSMode");
  
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetLVDSValue(0, "LVDSMode", "IORegister"));
  EQ(std::string("IORegister"), m_pConnection->GetLVDSValue(0, "LVDSMode"));
  
  m_pConnection->SetLVDSValue(0, "LVDSMode", "SelfTriggers");
  EQ(std::string("SelfTriggers"), m_pConnection->GetLVDSValue(0, "LVDSMode"));
  
  m_pConnection->SetLVDSValue(0, "LVDSMode", old.c_str());
}
void devtest::lvdsint()
{
  // Set the mode to I/O register and outputs:
  
  m_pConnection->SetLVDSValue(0, "LVDSMode", "IORegister");
  m_pConnection->SetLVDSValue(0, "LVDSDirection", "Output");
  int old = m_pConnection->GetLVDSInteger(0, "LVDSIOReg");
  
  CPPUNIT_ASSERT_NO_THROW(m_pConnection->SetLVDSValue(0, "LVDSIOReg", 0x5555));
  EQ(0x5555, m_pConnection->GetLVDSInteger(0, "LVDSIOReg"));
  
  m_pConnection->SetLVDSValue(0, "LVDSIOReg", 0xaaaa);
  EQ(0xaaaa, m_pConnection->GetLVDSInteger(0, "LVDSIOReg"));
  
  m_pConnection->SetLVDSValue(0, "LVDSIOReg", old);
}