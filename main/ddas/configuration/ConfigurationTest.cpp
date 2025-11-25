
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

#include <sstream>
#include <vector>
#include <string>

#include "Asserts.h"
#include "Configuration.h"

using namespace std;
using namespace ::DAQ::DDAS;

class ConfigurationTest : public CppUnit::TestFixture
{

public:
    CPPUNIT_TEST_SUITE(ConfigurationTest);
    CPPUNIT_TEST(print_0);
    CPPUNIT_TEST(setModEvtLength_0);
    CPPUNIT_TEST(setModEvtLength_1);
    CPPUNIT_TEST(setSlotMap_0);
    CPPUNIT_TEST(setSlotMap_1);
    CPPUNIT_TEST(setHardwareMap_0);
    CPPUNIT_TEST(setHardwareMap_1);
    CPPUNIT_TEST(setHardwareMap_2);
    CPPUNIT_TEST(setChannelMap_0);
    CPPUNIT_TEST(setChannelMap_1);
    CPPUNIT_TEST(setChannelMap_2);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() {}

    /** @brief Print out crate configuration. */
    void print_0()
	{
	    Configuration config;
	    config.setNumberOfModules(2);
	    config.setCrateId(123);
	    config.setModuleEventLengths({123,345});
	    config.setSlotMap({2,3});
	    config.setSettingsFilePath("/path/to/settings.file");
	    
	    std::stringstream stream;
	    config.print(stream);
	    std::string msg(
		"Crate number 123: 2 modules, in slots:2 3 DSPParFile: "
		"/path/to/settings.file"
		);
	    EQMSG("Print output", msg, stream.str());
	}

    /** 
     * @brief Setting module event lengths before setting number of modules 
     * is an error. 
     */
    void setModEvtLength_0()
	{
	    Configuration config;
	    CPPUNIT_ASSERT_THROW_MESSAGE(
		"Setting modevtlen before setNumberOfModules is an error",
		config.setModuleEventLengths({0}), std::runtime_error
		);
	}

    /** @brief Check setting event lengths. */
    void setModEvtLength_1()
	{
	    Configuration config;
	    config.setNumberOfModules(2);
	    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
		"settings modevtlen correctly succeeds",
		config.setModuleEventLengths({0, 2})
		);
	}

    /** 
     * @brief Setting a slot map before setting the number of modules is 
     * an error. 
     */
    void setSlotMap_0()
	{
	    Configuration config;
	    CPPUNIT_ASSERT_THROW_MESSAGE(
		"Setting a slot map before setNumberOfModules is an error",
		config.setSlotMap({0}), std::runtime_error
		);
	}

    /** @brief Check that we can correctly set slot maps. */
    void setSlotMap_1()
	{
	    Configuration config;
	    config.setNumberOfModules(2);
	    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
		"settings slot map correctly succeeds",
		config.setSlotMap({0, 2})
		);
	}

    /** @brief Check setting a hardware map. */
    void setHardwareMap_0()
	{
	    using namespace DAQ::DDAS::HardwareRegistry;
	    Configuration config;
	    config.setNumberOfModules(2);
	    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
		"Setting the hardware map correctly succeeds",
		config.setHardwareMap({RevB_100MHz_12Bit, RevF_250MHz_14Bit})
		);
	}

    /** 
     * @brief Setting a hardware map before setting the number of modules 
     * is an error. 
     */
    void setHardwareMap_1()
	{
	    using namespace DAQ::DDAS::HardwareRegistry;
	    Configuration config;
	    CPPUNIT_ASSERT_THROW_MESSAGE(
		"Setting hardware map before setNumberOfModules is an error",
		config.setHardwareMap({RevD_100MHz_12Bit, RevF_250MHz_14Bit}),
		std::runtime_error
		);
	}

    /** @brief Check that setting the hardware map modifies the map values. */
    void setHardwareMap_2()
	{
	    using namespace DAQ::DDAS::HardwareRegistry;
	    Configuration config;
	    config.setNumberOfModules(1);
	    std::vector<int> mapping = {RevC_100MHz_12Bit};
	    config.setHardwareMap(mapping);
	    ASSERTMSG(
		"setting hdwr map actually creates change in map",
		mapping == config.getHardwareMap()
		);

	}

    /** @brief Check that setting the channel map modifies the map values. */
    void setChannelMap_0()
	{
	    Configuration config;
	    config.setNumberOfModules(2);
	    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
		"Setting the channel map correctly succeeds",
		config.setChannelMap({16, 16})
		);
	}
    
    /** 
     * @brief Setting a channel map before setting the number of modules 
     * is an error. 
     */
    void setChannelMap_1()
	{
	    Configuration config;
	    CPPUNIT_ASSERT_THROW_MESSAGE(
		"Setting channel map before setNumberOfModules is an error",
		config.setChannelMap({16}),
		std::runtime_error
		);
	}

    /** @brief Check that setting the channel map modifies the map values. */
    void setChannelMap_2()
	{
	    using namespace DAQ::DDAS::HardwareRegistry;
	    Configuration config;
	    config.setNumberOfModules(1);
	    std::vector<unsigned short> mapping = {16};
	    config.setChannelMap(mapping);
	    ASSERTMSG(
		"setting channel map actually creates change in map",
		mapping == config.getChannelMap()
		);

	}
    
};

// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( ConfigurationTest);


