#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#define private public           // White box testing.
#include "s800TriggerRegisters.h"
#undef private

#include <json/json.h>
#include <iostream>
// Some simple tests for the s800 register parsing.

// RegisteFile.json is in the SOURCE_DIR directory.
// 
static const char* jsonFile=SOURCE_DIR "/" "RegisterFile.json";

class RegTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(RegTests);
    CPPUNIT_TEST(construct);
    CPPUNIT_TEST(getreg_1);
    CPPUNIT_TEST(getreg_2);
    CPPUNIT_TEST(ts_0);
    CPPUNIT_TEST(ts_1);
    CPPUNIT_TEST(swclr);
    CPPUNIT_TEST(busyrst);
    CPPUNIT_TEST(run_low);
    CPPUNIT_TEST(run_hi);
    CPPUNIT_TEST(extclr_ena);
    CPPUNIT_TEST_SUITE_END();
protected:
    void construct();

    void getreg_1();
    void getreg_2();
    void ts_0();
    void ts_1();
    void swclr();
    void busyrst();
    void run_low();
    void run_hi();
    void extclr_ena();
private:
    S800TriggerRegisters* m_pRegs;
public:
    void setUp() {
        m_pRegs = new S800TriggerRegisters(0, jsonFile);
    }
    void tearDown() {
        delete m_pRegs;
        m_pRegs = 0;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RegTests);

// tests

void 
RegTests::construct() {
    // All the work is done in setUp.
}

void 
RegTests::getreg_1() {
    // Can find a register and get the correct value:

    std::uint32_t a;
    CPPUNIT_ASSERT_NO_THROW(a = m_pRegs->getRegister("TRGMASK"));
    CPPUNIT_ASSERT_EQUAL(std::uint32_t(4096), a);
}
void 
RegTests::getreg_2() {
    // Find nonexisting register throws Json::Exception.

    CPPUNIT_ASSERT_THROW(
        m_pRegs->getRegister("howdydoodybs"),
        Json::Exception
    );
}
void
RegTests::ts_0() {
    std::uint32_t a;
    CPPUNIT_ASSERT_NO_THROW(a = m_pRegs->timestampLowBits());
    CPPUNIT_ASSERT_EQUAL(std::uint32_t(4132), a);
}
void
RegTests::ts_1() {
    std::uint32_t a;
    CPPUNIT_ASSERT_NO_THROW(a = m_pRegs->timestampHighBits());
    CPPUNIT_ASSERT_EQUAL(std::uint32_t(4136), a);
}

void
RegTests::swclr() {
    std::uint32_t a;
    CPPUNIT_ASSERT_NO_THROW(a = m_pRegs->swClearRegister());
    CPPUNIT_ASSERT_EQUAL(std::uint32_t(18680), a);
}

void
RegTests::busyrst() {
    std::uint32_t a;
    CPPUNIT_ASSERT_NO_THROW(a = m_pRegs->busyResetRegister());
    CPPUNIT_ASSERT_EQUAL(std::uint32_t(4104), a);
}

void
RegTests::run_low() {
    std::uint32_t a;
    CPPUNIT_ASSERT_NO_THROW(a = m_pRegs->runNumberLowBits());
    CPPUNIT_ASSERT_EQUAL(std::uint32_t(18668), a);
}
void
RegTests::run_hi() {
    std::uint32_t a;
    CPPUNIT_ASSERT_NO_THROW(a = m_pRegs->runNumberHighBits());
    CPPUNIT_ASSERT_EQUAL(std::uint32_t(18672), a);
}

void 
RegTests::extclr_ena() {
    std::uint32_t a;
    CPPUNIT_ASSERT_NO_THROW(a = m_pRegs->externalClearEnableRegister());
    CPPUNIT_ASSERT_EQUAL(std::uint32_t(4108), a);
}