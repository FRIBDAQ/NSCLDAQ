/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2016.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
         NSCL
         Michigan State University
         East Lansing, MI 48824-1321
*/


#include <cppunit/extensions/HelperMacros.h>

#include "Asserts.h"

// Tests will pull the next two in from ../xiatest

#include <pixie16app_export.h>
#include <pixie16sys_export.h>

#include "HardwareRegistry.h"
#include "Configuration.h"
#include "SystemBooter.h"

#include <sstream>
#include <vector>
#include <string>

using namespace std;
namespace HR = ::DAQ::DDAS::HardwareRegistry;
using namespace ::DAQ::DDAS;

/*!
 * \brief Tests for the ModEvtFileParser class
 */
class SystemBooterTest : public CppUnit::TestFixture
{

  public:
    CPPUNIT_TEST_SUITE( SystemBooterTest );
    CPPUNIT_TEST( boot_0 );
    CPPUNIT_TEST( boot_1 );
    CPPUNIT_TEST( boot_2 );
    CPPUNIT_TEST( boot_3 );
    CPPUNIT_TEST( boot_4 );
    CPPUNIT_TEST( boot_5 );
    CPPUNIT_TEST( boot_6 );
    CPPUNIT_TEST( boot_7 );
    CPPUNIT_TEST( test
    CPPUNIT_TEST_SUITE_END();

    Configuration m_config;
    std::vector<std::string> m_activityLog;

  public:
    void setUpConfiguration()
    {
        m_config = Configuration();
        m_config.setSettingsFilePath("test.set");
        m_config.setFirmwareConfiguration(
	    HR::RevB_100MHz_12Bit, {"bcd_0", "bcd_1", "bcd_2", "bcd_3"}
	    );
        m_config.setFirmwareConfiguration(
	    HR::RevF_250MHz_14Bit, {"f250_0", "f250_1", "f250_2", "f250_3"}
	    );
        m_config.setFirmwareConfiguration(
	    HR::RevF_500MHz_12Bit, {"f500_0", "f500_1", "f500_2", "f500_3"}
	    );
        m_config.setNumberOfModules(3);
    }

    void setUp() {
        Test::Pixie16SetModuleType(0, HR::RevB_100MHz_12Bit);
        Test::Pixie16SetModuleType(1, HR::RevF_250MHz_14Bit);
        Test::Pixie16SetModuleType(2, HR::RevF_500MHz_12Bit);

        setUpConfiguration();

        SystemBooter booter;
        booter.setVerbose(false);
        booter.boot(m_config, SystemBooter::FullBoot);

        m_activityLog = Test::Pixie16GetActivityLog();
    }

    void tearDown() {
        Test::Pixie16ResetActivityLog();
    }

    void boot_0() {
        EQMSG("first module should be detected as revbcd",
	      int(HR::RevB_100MHz_12Bit),
              m_config.getHardwareMap()[0]);
    }

    void boot_1() {
        EQMSG("2nd module should be detected as revf_250MHz_14Bit",
	      int(HR::RevF_250MHz_14Bit),
              m_config.getHardwareMap()[1]);
    }

    void boot_2() {
        EQMSG("3rd module should be detected as revf_500MHz_12Bit",
	      int(HR::RevF_500MHz_12Bit),
              m_config.getHardwareMap()[2]);
    }

    void boot_3() {
        EQMSG("all three modules should have boot records",
              size_t(3), m_activityLog.size());
    }

    void boot_4 () {
        EQMSG("first boot should be correct",
              string("f0:bcd_0,f1:bcd_1,f2:bcd_2,f3:bcd_3,index:0,pattern:7f"),
              m_activityLog.at(0));
    }

    void boot_5 () {
        EQMSG("second boot should be correct",
              string("f0:f250_0,f1:f250_1,f2:f250_2,f3:f250_3,index:1,pattern:7f"),
              m_activityLog.at(1));
    }

    void boot_6 () {
        EQMSG("third boot should be correct",
              string("f0:f500_0,f1:f500_1,f2:f500_2,f3:f500_3,index:2,pattern:7f"),
              m_activityLog.at(2));
    }

    void boot_7 () {

        Test::Pixie16ResetActivityLog();
        setUpConfiguration();

        SystemBooter booter;
        booter.setVerbose(false);
        booter.boot(m_config, SystemBooter::SettingsOnly);

        m_activityLog = Test::Pixie16GetActivityLog();
        EQMSG("boot pattern should be 0x70",
              string("f0:bcd_0,f1:bcd_1,f2:bcd_2,f3:bcd_3,index:0,pattern:70"),
              m_activityLog.at(0));
    }
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( SystemBooterTest );


