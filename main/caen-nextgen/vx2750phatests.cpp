/*
   

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
        

/** @file:  vx2750phatests.cpp
 *  @brief: Test for VX2750Pha class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <string>

#include "Asserts.h"
#include "VX2750Pha.h"
#include <cstdint>
#include <iostream>
#include <vector>
#include <set>
extern std::string connection;
extern bool        isUsb;

using namespace caen_nscldaq;

class vx2750phatest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(vx2750phatest);
    CPPUNIT_TEST(getcup);
    CPPUNIT_TEST(getfpgafw);
    CPPUNIT_TEST(getfwType);
    CPPUNIT_TEST(getmodelCode);
    CPPUNIT_TEST(getpiggycode);
    CPPUNIT_TEST(getmodel);
    
    CPPUNIT_TEST(getform);
    CPPUNIT_TEST(getfamcode);
    CPPUNIT_TEST(getserial);
    CPPUNIT_TEST(getmotherrev);
    CPPUNIT_TEST(getpiggyrev);
    CPPUNIT_TEST(getlicense);
    CPPUNIT_TEST(isLicensed);
    
    CPPUNIT_TEST(timebomb);
    CPPUNIT_TEST(channelCount);
    CPPUNIT_TEST(bits);
    CPPUNIT_TEST(samplerate);
    CPPUNIT_TEST(inputRange);
    CPPUNIT_TEST(inputsDifferential);
    CPPUNIT_TEST(inputImpedance);
    CPPUNIT_TEST(ip);
    CPPUNIT_TEST(netmask);
    CPPUNIT_TEST(gateway);
    
    CPPUNIT_TEST(clocksource);
    CPPUNIT_TEST(clockout);
    CPPUNIT_TEST(startsource);
    CPPUNIT_TEST(gbltrigger);
    CPPUNIT_TEST(gblmulti);
    CPPUNIT_TEST(wavetrigger);
    CPPUNIT_TEST(wavetriggermulti);
    CPPUNIT_TEST(eventtrigger);
    CPPUNIT_TEST(eventtriggermulti);
    CPPUNIT_TEST(chtrigmask);
    CPPUNIT_TEST(tracerecordmode);
    CPPUNIT_TEST(trgoutmode);
    CPPUNIT_TEST(gpiomode);
    CPPUNIT_TEST(busyinsrc);
    
    CPPUNIT_TEST(syncout);
    CPPUNIT_TEST(bvetosrc);
    CPPUNIT_TEST(bvetowidth);
    CPPUNIT_TEST(bvetopolarity);
    CPPUNIT_TEST(chvetosrc);
    CPPUNIT_TEST(chvetowid);
    CPPUNIT_TEST(rundelay);
    CPPUNIT_TEST(autodisarm);
    CPPUNIT_TEST(getters);
    
    CPPUNIT_TEST(vclockdelay);
    CPPUNIT_TEST(pclockdelay);
    CPPUNIT_TEST(wdatasource);
    CPPUNIT_TEST(recl);
    CPPUNIT_TEST(wresolution);
    CPPUNIT_TEST(aprobe);   
    CPPUNIT_TEST(dprobe);
    CPPUNIT_TEST(pretrigger);
    CPPUNIT_TEST(tpulseperiod);
    CPPUNIT_TEST(tpulsewidth);
    CPPUNIT_TEST(tpulselow);
    CPPUNIT_TEST(tpulsehigh);
    CPPUNIT_TEST(iolevel);
    CPPUNIT_TEST(monitor);
    CPPUNIT_TEST(errorflags);
    CPPUNIT_TEST(itlab);
    CPPUNIT_TEST(pairlogic);
    CPPUNIT_TEST(itllevel);
    CPPUNIT_TEST(itlconnect);
    CPPUNIT_TEST(itlmask);
    CPPUNIT_TEST(itlwidth);
    
    // LVDS now mostly is understood. THe I/O register is not.
    
    CPPUNIT_TEST(lvdsmode);
    CPPUNIT_TEST(ldvsdir);
    CPPUNIT_TEST(ldvstrigmask);
    CPPUNIT_TEST(lvdsioreg);
    
    
    CPPUNIT_TEST(dac);
    
    // Only 2745's support vgagain so we don't test it.
    
    CPPUNIT_TEST(offcalib);
    CPPUNIT_TEST(chen);
    CPPUNIT_TEST(chanstat);
    CPPUNIT_TEST(dcoffset);
    CPPUNIT_TEST(chgain);
    CPPUNIT_TEST(threshold);
    CPPUNIT_TEST(polarity);
    CPPUNIT_TEST(egate);
    
    CPPUNIT_TEST(evsel);
    CPPUNIT_TEST(chcoincmask);
    CPPUNIT_TEST(clen);
    CPPUNIT_TEST(tfrise);
    CPPUNIT_TEST(tfretrig);
    CPPUNIT_TEST(efrise);
    CPPUNIT_TEST(efflat); //highly coupled with pk average.. hard to
    CPPUNIT_TEST(efpeak);
    CPPUNIT_TEST(efpeakavg); // figure out a restoring test but the functions work.
    CPPUNIT_TEST(efpole0);
    CPPUNIT_TEST(effg);
    CPPUNIT_TEST(lffilt);
    CPPUNIT_TEST(efblavg);
    CPPUNIT_TEST(efblguard);
    CPPUNIT_TEST(efpupguard);
    
    CPPUNIT_TEST(info);
    CPPUNIT_TEST(commands);
    CPPUNIT_TEST_SUITE_END();
    
private:
    VX2750Pha* m_pModule;
public:
    void setUp() {
        m_pModule = new VX2750Pha(connection.c_str(), isUsb);
        m_pModule->Reset();
    }
    void tearDown() {
        delete m_pModule;
    }
protected:
    void getcup();
    void getfpgafw();
    void getfwType();
    void getmodelCode();
    void getpiggycode();
    void getmodel();
    
    void getform();
    void getfamcode();
    void getserial();
    void getmotherrev();
    void getpiggyrev();
    void getlicense();
    void isLicensed();
    
    void timebomb();
    void channelCount();
    void bits();
    void samplerate();
    void inputRange();
    void inputsDifferential();
    void inputImpedance();
    void ip();
    void netmask();
    void gateway();
    
    void clocksource();
    void clockout();
    void startsource();
    void gbltrigger();
    void gblmulti();
    void wavetrigger();
    void wavetriggermulti();
    void eventtrigger();
    void eventtriggermulti();
    void tsresetsrc();
    void chtrigmask();
    void tracerecordmode();
    void trgoutmode();
    void gpiomode();
    void busyinsrc();
    
    void syncout();
    void bvetosrc();
    void bvetowidth();
    void bvetopolarity();
    void chvetosrc();
    void chvetowid();
    void rundelay();
    void autodisarm();
    void getters();
    
    void vclockdelay();
    void pclockdelay();
    void wdatasource();
    void recl();
    void wresolution();
    void aprobe();
    void dprobe();
    void pretrigger();
    void tpulseperiod();
    void tpulsewidth();
    void tpulselow();
    void tpulsehigh();
    void iolevel();
    void monitor();
    void errorflags();
    void itlab();
    void pairlogic();
    void itllevel();
    void itlconnect();
    void itlmask();
    void itlwidth();
    
    void lvdsmode();
    void ldvsdir();
    void ldvstrigmask();
    void lvdsioreg();
    
    void dac();
    
    void offcalib();
    void chen();
    void chanstat();
    void dcoffset();
    void chgain();
    void threshold();
    void polarity();
    void egate();
    
    void evsel();
    void chcoincmask();
    void clen();
    void tfrise();
    void tfretrig();
    void efrise();
    void efflat();
    void efpeak();
    void efpeakavg();
    void efpole0();
    void effg();
    void lffilt();
    void efblavg();
    void efblguard();
    void efpupguard();
    
    void info();
    void commands();
};


CPPUNIT_TEST_SUITE_REGISTRATION(vx2750phatest);

void vx2750phatest::getcup()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getCupVersion());
}
void vx2750phatest::getfpgafw()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getFpgaFWVersion());
}
void vx2750phatest::getfwType()
{
    VX2750Pha::FwType type;
    CPPUNIT_ASSERT_NO_THROW(type = m_pModule->getFirmwareType());
    ASSERT(type == VX2750Pha::DPP_PHA);
}
void vx2750phatest::getmodelCode()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getModelCode());
}
void vx2750phatest::getpiggycode()
{
    
    CPPUNIT_ASSERT_NO_THROW( m_pModule->getPiggyBackCode());
    
}
void vx2750phatest::getmodel()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getModelName());
}

void vx2750phatest::getform()
{
    VX2750Pha::FormFactor form;
    CPPUNIT_ASSERT_NO_THROW(form = m_pModule->getFormFactor());
    ASSERT(form == VX2750Pha::VME64X);
}
void vx2750phatest::getfamcode()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getFamilyCode());
}
void vx2750phatest::getserial()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getSerialNumber());
}
void vx2750phatest::getmotherrev()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getMotherboardRev());
}
void vx2750phatest::getpiggyrev()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getPiggybackRev());
}
void vx2750phatest::getlicense()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getLicense());
}
void vx2750phatest::isLicensed()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->isLicensed());    
}

void vx2750phatest::timebomb()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->remainingUnlicensedTime());
}
void vx2750phatest::channelCount()
{
    int nchan;
    CPPUNIT_ASSERT_NO_THROW(nchan = m_pModule->channelCount());
    EQ(64, nchan);
}
void vx2750phatest::bits()
{
    int bits;
    CPPUNIT_ASSERT_NO_THROW(bits = m_pModule->bitsOfResolution());
    EQ(16, bits);
}
void vx2750phatest::samplerate()
{
    int rate;
    CPPUNIT_ASSERT_NO_THROW(rate = m_pModule->sampleRate());
    EQ(125, rate);
}
void vx2750phatest::inputRange()
{
    int range;
    CPPUNIT_ASSERT_NO_THROW(range = m_pModule->inputRange());
    EQ(2, range);
}
void vx2750phatest::inputsDifferential()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->isDifferentialInput());
}
void vx2750phatest::inputImpedance()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->inputImpedance());
}

void vx2750phatest::ip()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->ipAddress());
}
void vx2750phatest::netmask()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->netMask());
}
void vx2750phatest::gateway()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->gateway());
}
/// get/set clock source:

void vx2750phatest::clocksource()
{
    // Gotta check 'em all.
    std::vector<VX2750Pha::ClockSource>  possibleSources =
        {VX2750Pha::Internal, VX2750Pha::FrontPanel 
        //, VX2750Pha::Clock_P0,   // Unimplemented.
        //VX2750Pha::DIPSelected
    };
    VX2750Pha::ClockSource original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getClockSource());
    
    for (auto source : possibleSources) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setClockSource(source));
        EQ(source, m_pModule->getClockSource());
    }
    m_pModule->setClockSource(original);
}


// Test get/set clock output (P0 and FP):
// P0 appears unimplemented?!?
void vx2750phatest::clockout()
{
    bool originalP0;
    bool originalFP;
    std::vector<bool> possible = {true, false};
    
    // CPPUNIT_ASSERT_NO_THROW(originalP0 = m_pModule->isClockOutOnP0());
    CPPUNIT_ASSERT_NO_THROW(originalFP = m_pModule->isClockOutOnFP());
    
    for (bool state : possible) {
        //CPPUNIT_ASSERT_NO_THROW(m_pModule->setClockOutOnP0(state));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setClockOutOnFP(state));
        //EQ(state, bool(m_pModule->isClockOutOnP0()));
        EQ(state, bool(m_pModule->isClockOutOnFP()));
    }
    // m_pModule->setClockOutOnP0(originalP0);
    m_pModule->setClockOutOnFP(originalFP);
}

// test get/set start source

void vx2750phatest::startsource()
{
    std::vector<VX2750Pha::StartSource> possible = {
        VX2750Pha::Start_EncodedClockIn, VX2750Pha::SINLevel, VX2750Pha::SINEdge,
        VX2750Pha::SWCommand, VX2750Pha::Start_LVDS, VX2750Pha::Start_P0
    };
    
    std::vector<VX2750Pha::StartSource> original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getStartSource());
    
    for (auto source : possible) {
        std::vector<VX2750Pha::StartSource> src;
        src.push_back(source);
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setStartSource(src));
        auto got = m_pModule->getStartSource();
        EQ(size_t(1), got.size());
        EQ(source, got[0]);
    }
    m_pModule->setStartSource(original);
}
// set/get global trigger source.

void vx2750phatest::gbltrigger()
{
    std::vector<VX2750Pha::GlobalTriggerSource> possibles = {
        VX2750Pha::GlobalTrigger_TriggerIn, VX2750Pha::GlobalTrigger_P0,
        VX2750Pha::GlobalTrigger_Software, VX2750Pha::GlobalTrigger_LVDS,
        VX2750Pha::GlobalTrigger_InternalA,
        VX2750Pha::GlobalTrigger_InternalB, VX2750Pha::GlobalTrigger_InternalAandB,
        VX2750Pha::GlobalTrigger_InternalAorB, VX2750Pha::GlobalTrigger_EncodedClockIn,
        VX2750Pha::GlobalTrigger_GPIO,
        VX2750Pha::GlobalTrigger_TestPulse
    };
    std::vector<VX2750Pha::GlobalTriggerSource> original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getGlobalTriggerSource());
    for (auto source : possibles) {
        std::vector<VX2750Pha::GlobalTriggerSource> value;
        std::vector<VX2750Pha::GlobalTriggerSource> receivedvalue;
        value.push_back(source);
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setGlobalTriggerSource(value));
        receivedvalue = m_pModule->getGlobalTriggerSource();
        EQ(size_t(1), receivedvalue.size());
        EQ(receivedvalue[0], value[0] );
    }
    m_pModule->setGlobalTriggerSource(original);
}
// we're allowed to set several global trigger conditions:

void vx2750phatest::gblmulti()
{
    std::vector<std::vector<VX2750Pha::GlobalTriggerSource>> multi = {
        {VX2750Pha::GlobalTrigger_TriggerIn, VX2750Pha::GlobalTrigger_P0},
        {VX2750Pha::GlobalTrigger_TriggerIn, VX2750Pha::GlobalTrigger_P0,
        VX2750Pha::GlobalTrigger_Software, VX2750Pha::GlobalTrigger_LVDS},
        {VX2750Pha::GlobalTrigger_InternalAorB, VX2750Pha::GlobalTrigger_EncodedClockIn,
        VX2750Pha::GlobalTrigger_GPIO,
        VX2750Pha::GlobalTrigger_TestPulse}
    };
    
    std::vector<VX2750Pha::GlobalTriggerSource> old;
    
    
    for (auto v : multi) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setGlobalTriggerSource(v));
        auto got = m_pModule->getGlobalTriggerSource();
        EQ(got.size(), v.size());    // Same number of sources.
        std::set<VX2750Pha::GlobalTriggerSource> sgot(got.begin(), got.end()); // sets are unordered:
        for (int i =0; i < v.size(); i++) {
            EQ(size_t(1), sgot.count(v[i]));
        }
    }
    
}



// set/get wave trigger source.

void vx2750phatest::wavetrigger()
{
    std::vector<VX2750Pha::WaveTriggerSource> possibles = {
        VX2750Pha::WaveTrigger_InternalA, VX2750Pha::WaveTrigger_InternalB,
        VX2750Pha::WaveTrigger_GlobalTriggerSource, VX2750Pha::WaveTrigger_TRGIN,
        VX2750Pha::ExternalInhibit,
        VX2750Pha::ADCUnderSaturation, VX2750Pha::ADCOverSaturation,
        VX2750Pha::WaveTrigger_Software, 
        VX2750Pha::WaveTrigger_ChannelSelfTrigger,
        VX2750Pha::WaveTrigger_AnyChannelSelfTrigger, VX2750Pha::WaveTrigger_Disabled
    };
    
    int nch = m_pModule->channelCount();
    std::vector<VX2750Pha::WaveTriggerSource> original;
    
    for (int i = 0; i < nch; i++) {
        std::vector<VX2750Pha::WaveTriggerSource> g;
        
        CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getWaveTriggerSource(i));
        for (auto source: possibles) {
            std::vector<VX2750Pha::WaveTriggerSource> v;
            v.push_back(source);
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setWaveTriggerSource(i, v));
            g = m_pModule->getWaveTriggerSource(i);
            EQ(size_t(1), g.size());
            EQ(v[0], g[0]);
        }
        m_pModule->setWaveTriggerSource(i, original);
    }
}
// Test multiple event trigger source:

void vx2750phatest::wavetriggermulti()
{
    std::vector<std::vector<VX2750Pha::WaveTriggerSource>> sources = {
        {VX2750Pha::WaveTrigger_InternalA, VX2750Pha::WaveTrigger_InternalB},
        { VX2750Pha::WaveTrigger_GlobalTriggerSource, VX2750Pha::WaveTrigger_TRGIN,
        VX2750Pha::ExternalInhibit,
        VX2750Pha::ADCUnderSaturation, VX2750Pha::ADCOverSaturation},
        {VX2750Pha::ADCUnderSaturation, VX2750Pha::ADCOverSaturation,
        VX2750Pha::WaveTrigger_Software, 
        VX2750Pha::WaveTrigger_ChannelSelfTrigger,
        VX2750Pha::WaveTrigger_AnyChannelSelfTrigger}
    };
    std::vector<VX2750Pha::WaveTriggerSource> original;
    int nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {                  // Test all channnels:
        original = m_pModule->getWaveTriggerSource(i);
        for (auto s : sources) {    // Representative vectors of values:
            m_pModule->setWaveTriggerSource(i, s);
            auto g = m_pModule->getWaveTriggerSource(i);
            EQ(s.size(), g.size());
            std::set<VX2750Pha::WaveTriggerSource> sb(s.begin(), s.end());
            for (auto item: g) {
                EQ(size_t(1), sb.count(item));         // Each item in g is in sb.
            }
        }
        
        m_pModule->setWaveTriggerSource(i, original);
    }
}

// Test set/get event trigger source.

void vx2750phatest::eventtrigger()
{
    std::vector<VX2750Pha::EventTriggerSource> possibles = {
        VX2750Pha::EventTrigger_InternalB, VX2750Pha::EventTrigger_InternalA,
        VX2750Pha::EventTrigger_GlobalTriggerSource, VX2750Pha::EventTrigger_TRGIN,
        VX2750Pha::EventTrigger_Software,
        VX2750Pha::EventTrigger_ChannelSelfTrigger,
        VX2750Pha::EventTrigger_AnyChannelSelfTrigger, VX2750Pha::EventTrigger_Disabled
    };
    // per channel parameter so:
    int nch = m_pModule->channelCount();
    std::vector<VX2750Pha::EventTriggerSource> original;
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getEventTriggerSource(i));
        std::vector<VX2750Pha::EventTriggerSource> g;
        for (auto source : possibles) {
            std::vector<VX2750Pha::EventTriggerSource> v;
            v.push_back(source);
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setEventTriggerSource(i, v));
            g = m_pModule->getEventTriggerSource(i);
            EQ(size_t(1), g.size());
            EQ(v[0], g[0]);
        }
        m_pModule->setEventTriggerSource(i, original);
    }
}

// Multiple values for the event trigger:

void vx2750phatest::eventtriggermulti()
{
    std::vector<std::vector<VX2750Pha::EventTriggerSource>> sources = {
        {VX2750Pha::EventTrigger_InternalB, VX2750Pha::EventTrigger_InternalA},
        {VX2750Pha::EventTrigger_InternalB, VX2750Pha::EventTrigger_InternalA,
        VX2750Pha::EventTrigger_GlobalTriggerSource, VX2750Pha::EventTrigger_TRGIN,
        VX2750Pha::EventTrigger_Software},
        {VX2750Pha::EventTrigger_GlobalTriggerSource, VX2750Pha::EventTrigger_TRGIN,
        VX2750Pha::EventTrigger_Software,
        VX2750Pha::EventTrigger_ChannelSelfTrigger,
        VX2750Pha::EventTrigger_AnyChannelSelfTrigger}
    };
    std::vector<VX2750Pha::EventTriggerSource>  original;
    
    // Test all channels
    
    int nch = m_pModule->channelCount();
    for (int i = 0; i < nch; i++) {
        original = m_pModule->getEventTriggerSource(i);
        
        // Test a few represntative combinations:
        
        for (auto src : sources) {
            m_pModule->setEventTriggerSource(i, src);
            auto got = m_pModule->getEventTriggerSource(i);
            EQ(src.size(), got.size());
            
            // Since order may change sets are easier to compare:
            
            std::set<VX2750Pha::EventTriggerSource> sb(src.begin(), src.end());
            for (auto g : got) {
                EQ(size_t(1), sb.count(g));
            }
        }
        
        m_pModule->setEventTriggerSource(i, original);
    }
}

// set/get per channel timestamp trigger mask.

void vx2750phatest::chtrigmask()
{
    int nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        uint64_t original;
        CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getChannelTriggerMask(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setChannelTriggerMask(i, 0));
        EQ(std::uint64_t(0), m_pModule->getChannelTriggerMask(i));
        
        m_pModule->setChannelTriggerMask(i, 0xffffffffffffffff);
        EQ(std::uint64_t(0xffffffffffffffff), m_pModule->getChannelTriggerMask(i));
        
        m_pModule->setChannelTriggerMask(i, original);
        
    }
    
}
// set/get trace record mode:

void vx2750phatest::tracerecordmode()
{
    std::vector<VX2750Pha::TraceRecordMode> modes = {
        VX2750Pha::Always, VX2750Pha::OnRequest
    };
    
    int nch = m_pModule->channelCount();   // per channel parameter.
    VX2750Pha::TraceRecordMode original;
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getTraceRecordMode(i));
        
        for (auto mode : modes) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setTraceRecordMode(i, mode));
            EQ(mode, m_pModule->getTraceRecordMode(i));
        }
        m_pModule->setTraceRecordMode(i, original);
    }
    
}


// set/get triggerout mode.

void vx2750phatest::trgoutmode()
{
    std::vector<VX2750Pha::TRGOUTMode> modes = {
        VX2750Pha::TriggerOut_TRGIN, VX2750Pha::TriggerOut_P0,
        VX2750Pha::TriggerOut_Software, VX2750Pha::TriggerOut_LVDS,
        VX2750Pha::TriggerOut_InternalA,
        VX2750Pha::TriggerOut_InternalB,
        VX2750Pha::TriggerOut_InternalAandInternalB,
        VX2750Pha::TriggerOut_InternalAorInternalB,
        VX2750Pha::TriggerOut_EncodedClockIn,
        VX2750Pha::TriggerOut_Run, VX2750Pha::TriggerOut_ReferenceClock,
        VX2750Pha::TriggerOut_TestPulse,
        VX2750Pha::TriggerOut_Busy, VX2750Pha::TriggerOut_Zero,
        VX2750Pha::TriggerOut_One, VX2750Pha::TriggerOut_SynchIn,
        VX2750Pha::TriggerOut_SIN, VX2750Pha::TriggerOut_GPIO,
        VX2750Pha::AcceptedTrigger, VX2750Pha::TriggerClock
    };
    VX2750Pha::TRGOUTMode original;

    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getTRGOUTMode());
    for (auto mode : modes) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setTRGOUTMode(mode));
        EQ(mode, m_pModule->getTRGOUTMode());
    }
    m_pModule->setTRGOUTMode(original);
}
// set/get/GPIO mode.

void vx2750phatest::gpiomode()
{
    std::vector<VX2750Pha::GPIOMode> modes = {
        VX2750Pha::GPIOMode_Disabled, VX2750Pha::GPIOMode_TriggerIn,
        VX2750Pha::GPIOMode_P0, VX2750Pha::GPIOMode_SIN, VX2750Pha::GPIOMode_LVDS,
        VX2750Pha::GPIOMode_InternalA,
        VX2750Pha::GPIOMode_InternalB, VX2750Pha::GPIOMode_InternalAandInternalB,
        VX2750Pha::GPIOMode_InternalAorInternalB, VX2750Pha::GPIOMode_EncodedClockIn,
        VX2750Pha::GPIOMode_SoftwareTrigger, VX2750Pha::GPIOMode_Run,
        VX2750Pha::GPIOMode_ReferenceClock,
        VX2750Pha::GPIOMode_TestPulse, VX2750Pha::GPIOMode_Busy,
        VX2750Pha::GPIOMode_Zero, VX2750Pha::GPIOMode_One
    };
    
    VX2750Pha::GPIOMode original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getGPIOMode());
    for (auto mode : modes) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setGPIOMode(mode));
        EQ(mode, m_pModule->getGPIOMode());
    }
}
// set/get busy input source.

void vx2750phatest::busyinsrc()
{
    std::vector<VX2750Pha::BusyInSource> sources = {
        VX2750Pha::BusyIn_SIN, VX2750Pha::BusyIn_GPIO,
        VX2750Pha::BusyIn_LVDS, VX2750Pha::BusyIn_Disabled 
    };
    
    VX2750Pha::BusyInSource original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getBusyInputSource());
    for (auto s : sources) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setBusyInputSource(s));
        EQ(s, m_pModule->getBusyInputSource());
    }
    m_pModule->setBusyInputSource(original);
}
// set/get sync out mode:

void vx2750phatest::syncout()
{
    std::vector<VX2750Pha::SyncOutMode> modes = {
        VX2750Pha::SyncOut_Disabled, VX2750Pha::SyncOut_SynchIn,
        VX2750Pha::SyncOut_TestPulse, VX2750Pha::InternalClock,
        VX2750Pha::SyncOut_Run
    };
    
    VX2750Pha::SyncOutMode original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getSyncOutMode());
    
    for (auto m : modes) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setSyncOutMode(m));
        EQ(m, m_pModule->getSyncOutMode());
    }
    m_pModule->setSyncOutMode(original);
}

// get/set board veto source:

void vx2750phatest::bvetosrc()
{
    std::vector<VX2750Pha::VetoSource> sources = {
        VX2750Pha::Veto_SIN, VX2750Pha::Veto_LVDS, VX2750Pha::Veto_GPIO,
        VX2750Pha::Veto_P0, VX2750Pha::Veto_EncodedClock,
        VX2750Pha::Veto_Disabled
    };
    VX2750Pha::VetoSource original;
    
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getBoardVetoSource());
    
    for (auto s : sources) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setBoardVetoSource(s));
        EQ(s, m_pModule->getBoardVetoSource());
    }
    m_pModule->setBoardVetoSource(original);
}
// board veto width - likely granular:
void vx2750phatest::bvetowidth()
{
    std::uint64_t original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getBoardVetoWidth());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setBoardVetoWidth(std::uint64_t(0)));
    EQ(std::uint64_t(0), m_pModule->getBoardVetoWidth());
    
    m_pModule->setBoardVetoWidth(std::uint64_t(34359738359));
    ASSERT(std::llabs(std::uint64_t(34359738359) - m_pModule->getBoardVetoWidth()) < 8);                        
    
    m_pModule->setBoardVetoWidth(original);
}
// board veto polarity:

void vx2750phatest::bvetopolarity()
{
    std::vector<VX2750Pha::VetoPolarity> pols = {
        VX2750Pha::ActiveHigh, VX2750Pha::ActiveLow
    };
    VX2750Pha::VetoPolarity original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getBoardVetoPolarity());
    for (auto p : pols) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setBoardVetoPolarity(p));
        EQ(p, m_pModule->getBoardVetoPolarity());
    }
    m_pModule->setBoardVetoPolarity(original);
}

// channel veto source

void vx2750phatest::chvetosrc()
{
    std::vector<VX2750Pha::ChannelVetoSource> sources = {
        VX2750Pha::BoardVeto, VX2750Pha::OverSaturation,
        VX2750Pha::UnderSaturation, VX2750Pha::ChanVeto_Disabled
    };
    VX2750Pha::ChannelVetoSource original;
    
    // per channel obviously:
    
    int nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getChannelVetoSource(i));
        for (auto s : sources) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setChannelVetoSource(i, s));
            EQ(s, m_pModule->getChannelVetoSource(i));
        }
        m_pModule->setChannelVetoSource(i, original);
    }
}
// ADC Veto width:

void vx2750phatest::chvetowid()
{
    std::uint32_t original;
    int nch = m_pModule->channelCount();
    int granularity = 32;
    for (int i = 0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getChannelVetoWidth(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setChannelVetoWidth(i, 0));
        EQ(std::uint32_t(0), m_pModule->getChannelVetoWidth(i));
        // maybe granular so:
        
        m_pModule->setChannelVetoWidth(i, 524279);
        
        ASSERT(std::labs(std::int32_t(524279) -  std::int32_t(m_pModule->getChannelVetoWidth(i))) < granularity);
        
        m_pModule->setChannelVetoWidth(i, original);
    }
    
}
// get/set run delay. 0 is probably 0 but other values might be granular.

void vx2750phatest::rundelay()
{
    std::uint32_t original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getRunDelay());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setRunDelay(0));
    EQ(std::uint32_t(0), m_pModule->getRunDelay());
    
    m_pModule->setRunDelay(524279);
    ASSERT(labs(std::int32_t(524279) - std::int32_t(m_pModule->getRunDelay())) < 16);
}

/// get/set auto disarm enable.

void vx2750phatest::autodisarm()
{
    bool original;
    CPPUNIT_ASSERT_NO_THROW(original = m_pModule->isAutoDisarmEnabled());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setAutoDisarmEnabled(true));
    ASSERT(m_pModule->isAutoDisarmEnabled());
    
    m_pModule->setAutoDisarmEnabled(false);
    ASSERT(!m_pModule->isAutoDisarmEnabled());
    
    m_pModule->setAutoDisarmEnabled(original);
}



// Geters we don't have much control over:

void vx2750phatest::getters()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getLEDStatus());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getAcquisitionStatus());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getMaxRawDataSize());
}
// clock delays

void vx2750phatest::vclockdelay()
{
    double max = 18888;
    double old;
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getVolatileClockDelay());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setVolatileClockDelay(-max));
    ASSERT(std::abs((-max) - m_pModule->getVolatileClockDelay()) < 1.0);
    
    m_pModule->setVolatileClockDelay(max);
    ASSERT(std::abs(max - m_pModule->getVolatileClockDelay()) < 1.0);
    
    m_pModule->setVolatileClockDelay(old);
}


void vx2750phatest::pclockdelay()
{
    double max = 18888;
    double old;
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getPermanentClockDelay());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setPermanentClockDelay(-max));
    ASSERT(std::abs((-max) - m_pModule->getPermanentClockDelay()) < 1.0);
    
    m_pModule->setPermanentClockDelay(max);
    ASSERT(std::abs(max - m_pModule->getPermanentClockDelay()) < 1.0);
    
    m_pModule->setPermanentClockDelay(old);
}

// wave data source
void vx2750phatest::wdatasource()
{
    std::vector<VX2750Pha::WaveDataSource> sources = {
        VX2750Pha::ADC_DATA, VX2750Pha::ADC_TEST_TOGGLE, VX2750Pha::ADC_TEST_RAMP,
        VX2750Pha::ADC_SIN, VX2750Pha::WaveSource_IPE,
        VX2750Pha::WaveSource_Ramp, VX2750Pha::SquareWave,
        VX2750Pha::ADC_TEST_PRBS
    };
    VX2750Pha::WaveDataSource old;
    int nch = m_pModule->channelCount();
    for  (int i = 0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getWaveDataSource(i));
        for (auto s: sources) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setWaveDataSource(i, s));
            EQ(s, m_pModule->getWaveDataSource(i));
        }
        m_pModule->setWaveDataSource(i, old);
    }
    
}
// Record length (samples/ns).

void vx2750phatest::recl()
{
    int original;
    int nch = m_pModule->channelCount();
    for (int i = 0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getRecordSamples(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setRecordSamples(i, 4));
        EQ(std::uint32_t(4), m_pModule->getRecordSamples(i));
        m_pModule->setRecordSamples(i, 8100);
        EQ(std::uint32_t(8100), m_pModule->getRecordSamples(i));
        m_pModule->setRecordSamples(i, original);
        
        // ns might be granular guessing 8's because of the multiplier
        // between Min samples/ns.
        
        CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getRecordNs(i));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setRecordNs(i, 32));
        ASSERT(std::labs(32 - m_pModule->getRecordNs(i)) < 8);
        m_pModule->setRecordNs(i, 64800);
        ASSERT(std::labs(64800 - m_pModule->getRecordNs(i)) < 8);
        m_pModule->setRecordNs(i, original);
    }
}
// wave resolution

void vx2750phatest::wresolution()
{
    std::vector<VX2750Pha::WaveResolution> options = {
        VX2750Pha::Res8, VX2750Pha::Res16, VX2750Pha::Res32, VX2750Pha::Res64
    };
    VX2750Pha::WaveResolution old;
    int nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getWaveResolution(i));
        for (auto o : options) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setWaveResolution(i, o));
            EQ(o, m_pModule->getWaveResolution(i));
        }
        m_pModule->setWaveResolution(i, old);
    }
}
// probes:

void vx2750phatest::aprobe()
{
    std::vector<VX2750Pha::AnalogProbe> options = {
        VX2750Pha::ADCInput, VX2750Pha::TimeFilter, VX2750Pha::EnergyFilter,
        VX2750Pha::EnergyFilterBaseline,
        VX2750Pha::EnergyFilterMinusBaseline

    };
    VX2750Pha::AnalogProbe original;
    int nch = m_pModule->channelCount();
    const int numProbes=2;
    
    for (int i =0; i < nch; i++) {
        for (int p = 0; p < numProbes; p++) {
            CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getAnalogProbe(i, p));
            for (auto o : options) {
                CPPUNIT_ASSERT_NO_THROW(m_pModule->setAnalogProbe(i, p, o));
                EQ(o, m_pModule->getAnalogProbe(i, p));
            }
            m_pModule->setAnalogProbe(i, p, original);
        }
    }
}
void vx2750phatest::dprobe()
{
    std::vector<VX2750Pha::DigitalProbe> options = {
        VX2750Pha::DProbe_Trigger, VX2750Pha::TimeFilterArmed,
        VX2750Pha::ReTriggerGuard,
        VX2750Pha::EneryFilterBaselineFreeze,
        VX2750Pha::EnergyFilterPeaking, VX2750Pha::EnergyFilterPeakReady,
        VX2750Pha::EnergyFilterPileupGuard,
        VX2750Pha::EventPileup, VX2750Pha::ADCSaturation,
        VX2750Pha::ADCSaturationProtection, VX2750Pha::PostSaturationEvent,
        VX2750Pha::EnergyFilterSaturation, VX2750Pha::AcquisitionInhibit
    };
    VX2750Pha::DigitalProbe original;
    int nch = m_pModule->channelCount();
    const int nprobes=4;
    
    for (int i = 0; i < nch; i++) {
        for (int p = 0; p < nprobes; p++) {
            CPPUNIT_ASSERT_NO_THROW(original = m_pModule->getDigitalProbe(i, p));
            for (auto o : options) {
                CPPUNIT_ASSERT_NO_THROW(m_pModule->setDigitalProbe(i, p, o));
                EQ(o, m_pModule->getDigitalProbe(i, p));
            }
            m_pModule->setDigitalProbe(i, p, original);
        }
    }
}
//Pretrigger (samples and ns).

void vx2750phatest::pretrigger()
{
    std::uint32_t minsamples = 4;
    std::uint32_t maxsamples = 4000;
    std::uint32_t minns      = 32;
    std::uint32_t maxns      = 3100;
    int granularity    = 32;    
    std::uint32_t old;
    int nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getPreTriggerSamples(i));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setPreTriggerSamples(i, minsamples));
        EQ(minsamples, m_pModule->getPreTriggerSamples(i));
        m_pModule->setPreTriggerSamples(i, maxsamples);
        EQ(maxsamples, m_pModule->getPreTriggerSamples(i));   // samples should be precise.
        m_pModule->setPreTriggerSamples(i, old);
        
        // time might be granular in 8 ns clumps?
        
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getPreTriggerNs(i));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setPreTriggerNs(i, minns));
        ASSERT(std::labs(std::int32_t(minns) - std::int32_t(m_pModule->getPreTriggerNs(i))) < granularity);
        m_pModule->setPreTriggerNs(i, maxns);
        ASSERT(std::labs(std::int32_t(maxns) - std::int32_t(m_pModule->getPreTriggerNs(i))) < granularity);
        m_pModule->setPreTriggerNs(i, old);
        
    }
}

// Test pulse properties:

void vx2750phatest::tpulseperiod()
{
    const std::uint64_t max = 34359738360ULL;
    
    // Granular in 8ns units.
    
    std::uint64_t old;
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getTestPulsePeriod());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setTestPulsePeriod(0));
    EQ(std::uint64_t(0), m_pModule->getTestPulsePeriod());
    m_pModule->setTestPulsePeriod(max);
    EQ(max, m_pModule->getTestPulsePeriod());
    m_pModule->setTestPulsePeriod(old);
}
void vx2750phatest::tpulsewidth()
{
    const std::uint64_t max = 34359738360;
    std::uint64_t old;
    
    // Granular in 8ns units but 0 is 0.
    
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getTestPulseWidth());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setTestPulseWidth(0));
    EQ(std::uint64_t(0), m_pModule->getTestPulseWidth());
    m_pModule->setTestPulseWidth(max);
    EQ(max, m_pModule->getTestPulseWidth());
    m_pModule->setTestPulseWidth(old);
}

void vx2750phatest::tpulselow()
{
    const std::uint32_t max = 65535;
    std::uint32_t old;
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getTestPulseLowLevel());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setTestPulseLowLevel(0));
    EQ(std::uint32_t(0), m_pModule->getTestPulseLowLevel());
    m_pModule->setTestPulseLowLevel(max);
    EQ(max, m_pModule->getTestPulseLowLevel());
    m_pModule->setTestPulseLowLevel(old);
}

void vx2750phatest::tpulsehigh()
{
    const std::uint32_t max = 65535;
    std::uint32_t old;
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getTestPulseHighLevel());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setTestPulseHighLevel(0));
    EQ(std::uint32_t(0), m_pModule->getTestPulseHighLevel());
    m_pModule->setTestPulseHighLevel(max);
    EQ(max, m_pModule->getTestPulseHighLevel());
    m_pModule->setTestPulseHighLevel(old);
}
// I/O level for lemmos

void vx2750phatest::iolevel()
{
    std::vector<VX2750Pha::IOLevel> levels = {
        VX2750Pha::TTL, VX2750Pha::NIM
    };
    VX2750Pha::IOLevel old;
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getIOLevel());
    for (auto l : levels) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setIOLevel(l));
        EQ(l, m_pModule->getIOLevel());
    }
    m_pModule->setIOLevel(old);
}

// Ensure we can get stuff from the various health monitors:

void vx2750phatest::monitor()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getAirInTemp());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getAirOutTemp());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getCoreTemp());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getFirstADCTemp());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getLastADCTemp());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getHottestADCTemp());
    for (int i = 0; i < 1; i++) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->getADCTemp(i));
    }
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getDCDCConverterTemp());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getDCDCConverterInputVoltage());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getDCDCConverterOutputVoltage());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getDCDCConverterAmps());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getDCDCConverterHz());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getDCDCConverterDutyCycle());
    
    // Evidently not actually implemented?
    //CPPUNIT_ASSERT_NO_THROW(m_pModule->getFanSpeed(0));
    //CPPUNIT_ASSERT_NO_THROW(m_pModule->getFanSpeed(1));
    
}
// Error flag masks etc... note there are 17 bits.

void vx2750phatest::errorflags()
{
    const std::uint32_t allbits = 0x1ffff;  // 17 bits set.
    std::uint32_t old;
    
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getErrorFlagMask());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setErrorFlagMask(0));
    EQ(std::uint32_t(0), m_pModule->getErrorFlagMask());
    m_pModule->setErrorFlagMask(allbits);
    EQ(allbits, m_pModule->getErrorFlagMask());
    m_pModule->setErrorFlagMask(old);
    
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getErrorFlagDataMask());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setErrorFlagDataMask(0));
    EQ(std::uint32_t(0), m_pModule->getErrorFlagDataMask());
    m_pModule->setErrorFlagDataMask(allbits);
    EQ(allbits, m_pModule->getErrorFlagDataMask());
    m_pModule->setErrorFlagDataMask(old);
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getErrorFlags());
    bool ready;
    CPPUNIT_ASSERT_NO_THROW(ready = m_pModule->isBoardReady());
    ASSERT(ready);
    
}
// ITL blocks (trigger logic).

void vx2750phatest::itlab()
{
    std::vector<VX2750Pha::IndividualTriggerLogic> modes = {
        VX2750Pha::ITL_OR,
        VX2750Pha::ITL_AND,
        VX2750Pha::Majority
    };
    VX2750Pha::IndividualTriggerLogic old;
    
    // Mainlogic:
    
    for (auto m : modes) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getITLAMainLogic());
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLAMainLogic(m));
        EQ(m, m_pModule->getITLAMainLogic());
        m_pModule->setITLAMainLogic(old);
        
        CPPUNIT_ASSERT_NO_THROW(old - m_pModule->getITLBMainLogic());
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLBMainLogic(m));
        EQ(m, m_pModule->getITLBMainLogic());
        m_pModule->setITLBMainLogic(old);
    }
    // Majority level a:
    
    unsigned oldmaj;
    CPPUNIT_ASSERT_NO_THROW(oldmaj = m_pModule->getITLAMajorityLevel());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLAMajorityLevel(0));
    EQ(0U, m_pModule->getITLAMajorityLevel());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLAMajorityLevel(63));
    EQ(63U, m_pModule->getITLAMajorityLevel());
    
    // Majority level b:
    
    CPPUNIT_ASSERT_NO_THROW(oldmaj = m_pModule->getITLBMajorityLevel());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLBMajorityLevel(0));
    EQ(0U, m_pModule->getITLBMajorityLevel());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLBMajorityLevel(63));
    EQ(63U, m_pModule->getITLBMajorityLevel());
    
    
}
// Pair trigger logic:

void vx2750phatest::pairlogic()
{
    std::vector<VX2750Pha::PairTriggerLogic> modes = {
        VX2750Pha::PTL_AND, VX2750Pha::PTL_OR, VX2750Pha::NONE
    };
    VX2750Pha::PairTriggerLogic olda;
    VX2750Pha::PairTriggerLogic oldb;
    
    CPPUNIT_ASSERT_NO_THROW(olda = m_pModule->getITLAPairLogic());
    CPPUNIT_ASSERT_NO_THROW(oldb = m_pModule->getITLBPairLogic());
    
    for (auto m : modes) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLAPairLogic(m));
        EQ(m, m_pModule->getITLAPairLogic());
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLBPairLogic(m));
        EQ(m, m_pModule->getITLBPairLogic());
    }
    m_pModule->setITLAPairLogic(olda);
    m_pModule->setITLBPairLogic(oldb);
    
}

void vx2750phatest::itllevel()
{
    bool now;
    CPPUNIT_ASSERT_NO_THROW(now = m_pModule->isITLAInverted());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLAInverted(true));
    ASSERT(m_pModule->isITLAInverted());
    m_pModule->setITLAInverted(false);
    ASSERT(!m_pModule->isITLAInverted());
    m_pModule->setITLAInverted(now);
    
    CPPUNIT_ASSERT_NO_THROW(now = m_pModule->isITLBInverted());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLBInverted(true));
    ASSERT(m_pModule->isITLBInverted());
    m_pModule->setITLBInverted(false);
    ASSERT(!m_pModule->isITLBInverted());
    m_pModule->setITLBInverted(now);
    
}
void vx2750phatest::itlconnect()
{
    std::vector<VX2750Pha::ITLConnect> modes = {
        VX2750Pha::ITL_Disabled, VX2750Pha::ITL_ITLA, VX2750Pha::ITL_ITLB
    };
    VX2750Pha::ITLConnect old;
    unsigned nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getITLConnect(i));
        for (auto m : modes) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLConnect(i, m));
            EQ(m, m_pModule->getITLConnect(i));
        }
        m_pModule->setITLConnect(i, old);
    }
}
void vx2750phatest::itlmask()
{
    // Setting the mask, ovewrites the ITLConnect settings according to the
    // manual so we need to save/restore those for all channels!!
    
    std::vector<VX2750Pha::ITLConnect> old;
    unsigned nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        old.push_back(m_pModule->getITLConnect(i));
    }
    // Play withthe ITL Masks:
    
    std::uint64_t oldmaska;
    std::uint64_t oldmaskb;
    CPPUNIT_ASSERT_NO_THROW(oldmaska = m_pModule->getITLAMask());
    CPPUNIT_ASSERT_NO_THROW(oldmaskb = m_pModule->getITLBMask());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLAMask(0));
    EQ(std::uint64_t(0), m_pModule->getITLAMask());
    m_pModule->setITLAMask(0xffffffffffffffff);
    EQ(std::uint64_t(0xffffffffffffffff), m_pModule->getITLAMask());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLBMask(0));
    EQ(std::uint64_t(0), m_pModule->getITLBMask());
    m_pModule->setITLBMask(0xffffffffffffffff);
    EQ(std::uint64_t(0xffffffffffffffff), m_pModule->getITLBMask());
    
    m_pModule->setITLAMask(oldmaska);
    m_pModule->setITLBMask(oldmaskb);
    
    // Restore the ITLConnect values:
    
    for (int i=  0; i < nch; i++) {
        m_pModule->setITLConnect(i, old[i]);
    }
}
void vx2750phatest::itlwidth()
{
    const std::uint32_t max = 524280;
    
    std::uint32_t olda;
    std::uint32_t oldb;
    
    CPPUNIT_ASSERT_NO_THROW(olda = m_pModule->getITLAGateWidth());
    CPPUNIT_ASSERT_NO_THROW(oldb = m_pModule->getITLBGateWidth());
    
    // 8ns granlarity:
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLAGateWidth(0));
    EQ(std::uint32_t(0), m_pModule->getITLAGateWidth());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLAGateWidth(max));
    ASSERT(std::labs(std::int32_t(max) - m_pModule->getITLAGateWidth()) < 8);
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLBGateWidth(0));
    EQ(std::uint32_t(0), m_pModule->getITLBGateWidth());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setITLBGateWidth(max));
    ASSERT(std::labs(std::int32_t(max) - std::int32_t(m_pModule->getITLBGateWidth())) < 8);
    
    
    m_pModule->setITLAGateWidth(olda);
    m_pModule->setITLBGateWidth(oldb);
}

void vx2750phatest::lvdsmode() {
    std::vector<VX2750Pha::LVDSMode> modes = {
        VX2750Pha::SelfTriggers, VX2750Pha::Sync, VX2750Pha::IORegister
    };
    VX2750Pha::LVDSMode prior;
    
    // there are four quartets to test:m_
    for (int q = 0; q < 4; q++) {
        CPPUNIT_ASSERT_NO_THROW(prior = m_pModule->getLVDSMode(q));
        for (auto m : modes) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setLVDSMode(q, m));
            EQ(m, m_pModule->getLVDSMode(q));
        }
        m_pModule->setLVDSMode(q, prior);
    }
    
}
void vx2750phatest::ldvsdir() {
    std::vector<VX2750Pha::LVDSDirection> dirs = {
        VX2750Pha::Input, VX2750Pha::Output
    };
    VX2750Pha::LVDSDirection prior;
    
    // Four quartets:
    
    for (int q = 0; q < 4; q++) {
        CPPUNIT_ASSERT_NO_THROW(prior = m_pModule->getLVDSDirection(q));
        for (auto d : dirs) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setLVDSDirection(q, d));
            EQ(d, m_pModule->getLVDSDirection(q));
        }
        m_pModule->setLVDSDirection(q, prior);
    }
    
    
}
void vx2750phatest::ldvstrigmask() {
    // We're going to assume we need  to be in trigger mode to set a trigger
    // mask (may not be so but...)
    
    VX2750Pha::LVDSMode prior;
    std::uint64_t oldmask;
    for (int q =0; q< 4; q++) {
        prior = m_pModule->getLVDSMode(q);
        m_pModule->setLVDSMode(q, VX2750Pha::SelfTriggers);
        
        CPPUNIT_ASSERT_NO_THROW(oldmask = m_pModule->getLVDSTriggerMask(q));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setLVDSTriggerMask(q, std::uint64_t(0xffffffffffffffff)));
        EQ(std::uint64_t(0xffffffffffffffff), m_pModule->getLVDSTriggerMask(q));
        
        m_pModule->setLVDSTriggerMask(q, 0);
        EQ(std::uint64_t(0), m_pModule->getLVDSTriggerMask(q));
        
        m_pModule->setLVDSTriggerMask(q, std::uint64_t(0x5555555555555555));
        EQ( std::uint64_t(0x5555555555555555), m_pModule->getLVDSTriggerMask(q));
        
        m_pModule->setLVDSTriggerMask(q, std::uint64_t(0xaaaaaaaaaaaaaaaa));
        EQ(std::uint64_t(0xaaaaaaaaaaaaaaaa), m_pModule->getLVDSTriggerMask(q));
        
        m_pModule->setLVDSMode(q, prior);
    }
    
}
void vx2750phatest::lvdsioreg() {
    
    VX2750Pha::LVDSMode prior[4];    // for each quartet.
    VX2750Pha::LVDSDirection dirs[4];
    
    // Set the mode to I/O register and the direction to output for all quartets:
    //
    for (int i =0; i < 4; i++) {
        prior[i]  = m_pModule->getLVDSMode(i);
        dirs[i]   = m_pModule->getLVDSDirection(i);
        
        m_pModule->setLVDSMode(i, VX2750Pha::IORegister);
        m_pModule->setLVDSDirection(i, VX2750Pha::Output);
    }
    
    // Now test the i/o reg don't bother saving:
    
    m_pModule->setLVDSIOReg(0xffff);
    EQ(std::uint16_t(0xffff), m_pModule->getLVDSIOReg());
    m_pModule->setLVDSIOReg(0x0);
    EQ(std::uint16_t(0), m_pModule->getLVDSIOReg());
    
    // Restore the mode and directions:
    
    for (int i =0; i < 4; i++) {
        m_pModule->setLVDSMode(i, prior[i]);
        m_pModule->setLVDSDirection(i, dirs[i]);
    }
}
// Test various DAC control functions.

void vx2750phatest::dac()
{
    std::vector<VX2750Pha::DACOutputMode> modes = {
        VX2750Pha::Static, VX2750Pha::DACOut_IPE, VX2750Pha::DACOut_ChInput,
        VX2750Pha::MemOccupancy, VX2750Pha::ChSum, VX2750Pha::OverThrSum,
        VX2750Pha::DACOut_Ramp, VX2750Pha::Sine5MHz, VX2750Pha::Square
    };
    VX2750Pha::DACOutputMode old;
    
    // Mode
    
    CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getDACOutMode());
    
    for (auto m : modes) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setDACOutMode(m));
        EQ(m, m_pModule->getDACOutMode());
    }
    
    m_pModule->setDACOutMode(old);
    
    // Value [static level]
    
    const std::uint16_t maxvalue = 16383;
    std::uint16_t oldlevel;
    CPPUNIT_ASSERT_NO_THROW(m_pModule->getDACOutValue());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setDACOutValue(0));
    EQ(std::uint16_t(0), m_pModule->getDACOutValue());
    m_pModule->setDACOutValue(maxvalue);
    EQ(maxvalue, m_pModule->getDACOutValue());
    
    m_pModule->setDACOutValue(oldlevel);
    
    // Channel the DAC outputs reproduce in that mode:
    
    unsigned ch;
    unsigned nch = m_pModule->channelCount();
    CPPUNIT_ASSERT_NO_THROW(ch = m_pModule->getDACChannel());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->setDACChannel(0));
    EQ(unsigned(0), m_pModule->getDACChannel());
    m_pModule->setDACChannel(nch-1);
    EQ(nch-1, m_pModule->getDACChannel());
    
    m_pModule->setDACChannel(ch);
    
    // Just in case mucking with levels and channels resets the mode:
    
    m_pModule->setDACOutMode(old);
}
// offset calibration on/off.

void vx2750phatest::offcalib()
{
    bool was;
   
    CPPUNIT_ASSERT_NO_THROW(was = m_pModule->isOffsetCalibrationEnabled());
    
    CPPUNIT_ASSERT_NO_THROW(m_pModule->enableOffsetCalibration(true));
    ASSERT(m_pModule->isOffsetCalibrationEnabled());
    m_pModule->enableOffsetCalibration(false);
    ASSERT(!m_pModule->isOffsetCalibrationEnabled());
    
    m_pModule->enableOffsetCalibration(was);

}
// Enable/disable channel

void vx2750phatest::chen()
{
    bool old;
    unsigned nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->isChannelEnabled(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->enableChannel(i, true));
        ASSERT(m_pModule->isChannelEnabled(i));
        m_pModule->enableChannel(i, false);
        ASSERT(!m_pModule->isChannelEnabled(i));
        
        m_pModule->enableChannel(i, old);
    }
}

// readonly channel status:

void vx2750phatest::chanstat() {
    unsigned nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->getChannelStatus(i));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->getSelfTriggerRate(i));
    }
}
// set/get the DC offset added to the input signal:
// Likely granular (1% level?)
void vx2750phatest::dcoffset() {
    double  oldpct;
    unsigned nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(oldpct = m_pModule->getDCOffset(i));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setDCOffset(i, 0.0)); // probably exact.
        EQ(0.0, m_pModule->getDCOffset(i));
        
        m_pModule->setDCOffset(i, 50.0);
        ASSERT(std::abs(50.0 - m_pModule->getDCOffset(i)) <= 1.0);
        
        m_pModule->setDCOffset(i, oldpct);
    }
}
// gain factors are readonly so:
void vx2750phatest::chgain() {
    unsigned nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->getGainFactor(i));
    }
}
// Trigger threshold.

void vx2750phatest::threshold() {
    unsigned nch = m_pModule->channelCount();
    std::uint32_t max = 8191;
    std::uint32_t old;
    
    for (int i = 0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getTriggerThreshold(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setTriggerThreshold(i, 1));
        EQ(std::uint32_t(1), m_pModule->getTriggerThreshold(i));
        
        m_pModule->setTriggerThreshold(i, max);
        EQ(max, m_pModule->getTriggerThreshold(i));
        
        m_pModule->setTriggerThreshold(i, old);
        
    }
}
void vx2750phatest::polarity() {
    std::vector<VX2750Pha::Polarity> pols = {
        VX2750Pha::Positive, VX2750Pha::Negative
    };
    VX2750Pha::Polarity old;
    unsigned nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getPulsePolarity(i));
        
        for (auto p : pols) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setPulsePolarity(i, p));
            EQ(p, m_pModule->getPulsePolarity(i));
        }
        
        m_pModule->setPulsePolarity(i, old);
    }
    
}
// energy skim is essentially an energy acceptance window.
void vx2750phatest::egate() {
    std::uint16_t max = 65534;
    std::uint16_t oldlow, oldhigh;
    
    unsigned nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(oldlow = m_pModule->getEnergySkimLowDiscriminator(i));
        CPPUNIT_ASSERT_NO_THROW(oldhigh = m_pModule->getEnergySkimHighDiscriminator(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergySkimLowDiscriminator(i, 1));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergySkimHighDiscriminator(i, 1));
        EQ(std::uint16_t(1), m_pModule->getEnergySkimLowDiscriminator(i));
        EQ(std::uint16_t(1), m_pModule->getEnergySkimLowDiscriminator(i));
        
        m_pModule->setEnergySkimLowDiscriminator(i, max);
        m_pModule->setEnergySkimHighDiscriminator(i, max);
        EQ(max, m_pModule->getEnergySkimLowDiscriminator(i));
        EQ(max, m_pModule->getEnergySkimHighDiscriminator(i));
        
        m_pModule->setEnergySkimHighDiscriminator(i, oldhigh);
        m_pModule->setEnergySkimLowDiscriminator(i, oldlow);
    }
}

// event and waveform selection:

void vx2750phatest::evsel()
{
    std::vector<VX2750Pha::EventSelection> selectors = {
        VX2750Pha::All, VX2750Pha::Pileup, VX2750Pha::EnergySkim
    };
    VX2750Pha::EventSelection old;
    
    
    
    int nch = m_pModule->channelCount();
    for (int i = 0; i < nch; i++) {
        // Event:
        
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getEventSelector(i));
        for (auto s : selectors) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setEventSelector(i, s));
            EQ(s, m_pModule->getEventSelector(i));        
        }
        m_pModule->setEventSelector(i, old);
        
        // wave:
        
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getWaveformSelector(i));
        for (auto s : selectors) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setWaveformSelector(i, s));
            EQ(s, m_pModule->getWaveformSelector(i));        
        }
        m_pModule->setWaveformSelector(i, old);
    }
    
}
// channel coincidence/anit-coincidence mask:

void vx2750phatest::chcoincmask()
{
    std::vector<VX2750Pha::CoincidenceMask> selections = {
        VX2750Pha::Coincidence_Disabled, VX2750Pha::Ch64Trigger,
        VX2750Pha::Coincidence_TRGIN, VX2750Pha::Coincidence_GlobalTriggerSource,
        VX2750Pha::Coincidence_ITLA, VX2750Pha::Coincidence_ITLB
    };
    
    VX2750Pha::CoincidenceMask oldc;
    VX2750Pha::CoincidenceMask olda;
    int nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(oldc = m_pModule->getCoincidenceMask(i));
        CPPUNIT_ASSERT_NO_THROW(olda = m_pModule->getAntiCoincidenceMask(i));
        
        for (auto s : selections) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setCoincidenceMask(i, s));
            EQ(s, m_pModule->getCoincidenceMask(i));
            
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setAntiCoincidenceMask(i, s));
            EQ(s, m_pModule->getAntiCoincidenceMask(i));
                                
        }
        
        m_pModule->setCoincidenceMask(i, oldc);
        m_pModule->setAntiCoincidenceMask(i, oldc);
    }
}
void vx2750phatest::clen()
{
    std::uint32_t mint = 8;
    std::uint32_t mins = 1;
    std::uint32_t maxt = 524280;
    std::uint32_t maxs = 65535;
    
    std::uint32_t olds;
    
    int nch = m_pModule->channelCount();
    for(int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(olds = m_pModule->getCoincidenceSamples(i));
    
        // Note we assume a granularity of 8ns for time:
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setCoincidenceNs(i, mint));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT((std::labs(std::int32_t(mint) - std::int32_t(m_pModule->getCoincidenceNs(i)))) <= 8)
        );
        m_pModule->setCoincidenceNs(i, maxt);
        ASSERT(std::labs(std::int32_t(maxt) - std::int32_t(m_pModule->getCoincidenceNs(i))) <= 8);
        
        // Samples is exact however:
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setCoincidenceSamples(i, mins));
        EQ(mins, m_pModule->getCoincidenceSamples(i));
        m_pModule->setCoincidenceSamples(i, maxs);
        EQ(maxs, m_pModule->getCoincidenceSamples(i));
        
        m_pModule->setCoincidenceSamples(i, olds);
    }
}
// Timing filter rise time:

void vx2750phatest::tfrise()
{
    std::uint32_t mint = 32;
    std::uint32_t maxt = 2000;
    std::uint32_t mins = 4;
    std::uint32_t maxs = 250;
    
    std::uint32_t olds;
    unsigned nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(olds = m_pModule->getTimeFilterRiseSamples(i));
        
        // Samples are exact
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setTimeFilterRiseSamples(i, mins));
        EQ(mins, m_pModule->getTimeFilterRiseSamples(i));
        m_pModule->setTimeFilterRiseSamples(i, maxs);
        EQ(maxs, m_pModule->getTimeFilterRiseSamples(i));
        
        // time has granularity of 8:
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setTimeFilterRiseTime(i, mint));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(std::int32_t(mint) - std::int32_t(m_pModule->getTimeFilterRiseTime(i))) < 8)
        );
        m_pModule->setTimeFilterRiseTime(i, maxt);
        ASSERT(std::labs(std::int32_t(maxt) - std::int32_t(m_pModule->getTimeFilterRiseTime(i))) < 8);
        
        m_pModule->setTimeFilterRiseSamples(i, olds);
    }
}
// Retrigger guard time/samples

void vx2750phatest::tfretrig()
{
    std::int32_t mins =0;
    std::int32_t maxs = 1000;
    std::int32_t mint = 0;
    std::int32_t maxt = 8000;
    
    std::uint32_t olds;
    unsigned nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(olds = m_pModule->getTimeFilterRetriggerGuardSamples(i));
        
         // Samples are exact:
         
         CPPUNIT_ASSERT_NO_THROW(m_pModule->setTimeFilterRetriggerGuardSamples(i, mins));
         EQ(mins, std::int32_t(m_pModule->getTimeFilterRetriggerGuardSamples(i)));
         CPPUNIT_ASSERT_NO_THROW(m_pModule->setTimeFilterRetriggerGuardSamples(i, maxs));
         EQ(maxs, std::int32_t(m_pModule->getTimeFilterRetriggerGuardSamples(i)));
         
         // ns has 8 ns granularity:
         
         CPPUNIT_ASSERT_NO_THROW(m_pModule->setTimeFilterRetriggerGuardTime(i, mint));
         CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(mint - std::int32_t(m_pModule->getTimeFilterRetriggerGuardTime(i))) < 8 )
         );
         CPPUNIT_ASSERT_NO_THROW(m_pModule->setTimeFilterRetriggerGuardTime(i, maxt));
         CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(maxt - std::int32_t(m_pModule->getTimeFilterRetriggerGuardTime(i))) < 8 )
         );                      
        
        m_pModule->setTimeFilterRetriggerGuardSamples(i, olds);
    }
}
// Energy filter rise time/samples.
void vx2750phatest::efrise()
{
    std::int32_t mins = 4;
    std::int32_t maxs = 1625;
    std::int32_t mint = 32;
    std::int32_t maxt = 13000;
    
    std::uint32_t old;      // THese are the same parameter actually just diff units.
    int nch  = m_pModule->channelCount();
    for (int i= 0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getEnergyFilterRiseSamples(i));
        
        // Samples are exact.
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterRiseSamples(i, mins));
        EQ(mins, std::int32_t(m_pModule->getEnergyFilterRiseSamples(i)));
        m_pModule->setEnergyFilterRiseSamples(i, maxs);
        EQ(maxs, std::int32_t(m_pModule->getEnergyFilterRiseSamples(i)));
        
        // Times are granular in 8ns units
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterRiseTime(i, mint));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(mint - std::int32_t(m_pModule->getEnergyFilterRiseTime(i))) < 8)
        );
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterRiseTime(i, maxt));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(maxt - std::int32_t(m_pModule->getEnergyFilterRiseTime(i))) < 8)
        );
        
        m_pModule->setEnergyFilterRiseSamples(i, old);
    }
}
// energy filter flattop time/samples.

void vx2750phatest::efflat()
{
    std::int32_t mins = 4;
    std::int32_t maxs = 375;
    std::int32_t mint = 32;
    std::int32_t maxt = 3000;
    
    
    std::int32_t old;    // Samples and T are just different units of the same thing.
    int nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        // Flattop averaging is coupled:
        
        auto oldpavg = m_pModule->getEnergyFilterPeakingAverage(i);
        m_pModule->setEnergyFilterPeakingAverage(i, VX2750Pha::Average1);
        
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getEnergyFilterFlatTopSamples(i));
        
        // Samples (exact)
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterFlatTopSamples(i, mins));
        EQ(mins, std::int32_t(m_pModule->getEnergyFilterFlatTopSamples(i)));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterFlatTopSamples(i, maxs));
        EQ(maxs, std::int32_t(m_pModule->getEnergyFilterFlatTopSamples(i)));
                
        // time (8ns steps)
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterFlatTopTime(i, mint));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(mint - std::int32_t(m_pModule->getEnergyFilterFlatTopTime(i))) < 8)
        );
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterFlatTopTime(i, maxt));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(maxt - std::int32_t(m_pModule->getEnergyFilterFlatTopTime(i))) < 8)
        );
        
        m_pModule->setEnergyFilterFlatTopSamples(i, old);
        m_pModule->setEnergyFilterPeakingAverage(i, oldpavg);
    }
}
// peaking pos.
void vx2750phatest::efpeak()
{
    // Good to 1%?
    // This is coupled to flattop...which must be set first.
    // and peaking positin + avg < flattop samplesis required.
    // I'm tired of ensuring that I restore stuff so. bag it.
    
    double old;
    int nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++)  {
        // A long energy flattop time should allow this stuff to work?
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterFlatTopSamples(i, 375));   // max.
        // Set averaging to 1 and all should work?
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPeakingAverage(i, VX2750Pha::Average1));
        
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getEnergyFilterPeakingPosition(i));
        // Maybe initial value can be out of range????
        // Or the whole parameter setting is just mis-implemented/
        if (old < 10) old = 10;
        if (old > 90) old = 90;
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPeakingPosition(i, 10.0));
        ASSERT(std::abs(10.0 - m_pModule->getEnergyFilterPeakingPosition(i)) < 1.0);
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPeakingPosition(i, 90.0));
        ASSERT(std::abs(90.0 - m_pModule->getEnergyFilterPeakingPosition(i)) < 1.0);
        
        m_pModule->setEnergyFilterPeakingPosition (i, old);
    }
}
// Averaging in peaking:

void vx2750phatest::efpeakavg()
{
    std::vector<VX2750Pha::EnergyPeakingAverage> options = {
        VX2750Pha::Average1, VX2750Pha::Average4, VX2750Pha::EPeakAvg_Average16,
        VX2750Pha::EPeakAvg_Average64
    };
    VX2750Pha::EnergyPeakingAverage old;
    std::uint32_t  oldflat;
    int nch = m_pModule->channelCount();
    // The peaking average samples must be longer than the flattop sample so:
    // and our biggest averaging is 64 so:
    for (int i =0; i < nch; i++) {
        // Similarly we need to set the flattop samples and the peaking position
        // small and flattop samples big to allow the full range of the
        // averaging to be tested:
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterFlatTopSamples(i, 375)); // largest allowed.
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPeakingPosition(i, 10.0));
        oldflat = m_pModule->getEnergyFilterFlatTopSamples(i);
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getEnergyFilterPeakingAverage(i));
        
        for (auto o : options) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPeakingAverage(i, o));
            EQ(o, m_pModule->getEnergyFilterPeakingAverage(i));
        }
        m_pModule->setEnergyFilterPeakingAverage(i, old);
        m_pModule->setEnergyFilterFlatTopSamples(i, oldflat);
    }
    
}

// pole zero correction:

void vx2750phatest::efpole0()
{
    std::int32_t mins = 4;
    std::int32_t maxs = 65500;
    std::int32_t mint = 32;
    std::int32_t maxt = 524000;
    
    std::int32_t olds;         // Samples times - just different units.
    int nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(olds = m_pModule->getEnergyFilterPoleZeroSamples(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPoleZeroSamples(i, mins));
        EQ(mins, std::int32_t(m_pModule->getEnergyFilterPoleZeroSamples(i)));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPoleZeroSamples(i, maxs));
        EQ(maxs, std::int32_t(m_pModule->getEnergyFilterPoleZeroSamples(i)));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPoleZeroTime(i, mint));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(mint - std::int32_t(m_pModule->getEnergyFilterPoleZeroTime(i))) < 8)
        );
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPoleZeroTime(i, maxt));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(maxt - std::int32_t(m_pModule->getEnergyFilterPoleZeroTime(i))) < 8)
        );
        
        m_pModule->setEnergyFilterPoleZeroSamples(i, olds);
    }
}
// Energy filter fine gain

void vx2750phatest::effg()
{
    double ming = 1.0;
    double maxg = 10.0;
    
    double old;
    int nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getEnergyFilterFineGain(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterFineGain(i, ming));
        ASSERT(std::abs(ming - m_pModule->getEnergyFilterFineGain(i)) < 1.0);
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterFineGain(i, maxg));
        ASSERT(std::abs(maxg - m_pModule->getEnergyFilterFineGain(i)) < 1.0);
        
        m_pModule->setEnergyFilterFineGain(i, old);
    }
}
// enable/disable low frequency filters:

void vx2750phatest::lffilt()
{
    bool old;
    int nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->isEnergyFilterFLimitationEnabled(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->enableEnergyFilterFLimitation(i, true));
        ASSERT(m_pModule->isEnergyFilterFLimitationEnabled(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->enableEnergyFilterFLimitation(i, false));
        ASSERT(!m_pModule->isEnergyFilterFLimitationEnabled(i));
        
        
        m_pModule->enableEnergyFilterFLimitation(i, old);
    }
}

// Energy filter baseline averaging:

void vx2750phatest::efblavg()
{
    std::vector<VX2750Pha::EnergyFilterBaselineAverage>  options = {
        VX2750Pha::Fixed, VX2750Pha::EFilterBlineAvg_Average16,
        VX2750Pha::EFilterBlineAvg_Average64,
        VX2750Pha::Average256, VX2750Pha::Average1024, VX2750Pha::Average4K,
        VX2750Pha::Average16K
    };
    VX2750Pha::EnergyFilterBaselineAverage old;
    int nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getEnergyFilterBaselineAverage(i));
        
        for (auto o : options) {
            CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterBaselineAverage(i, o));
            EQ(o, m_pModule->getEnergyFilterBaselineAverage(i));
        }
        
        m_pModule->setEnergyFilterBaselineAverage(i, old);
    }
}

// Energyfilter baseline guard

void vx2750phatest::efblguard()
{
    std::int32_t mins = 0;
    std::int32_t maxs = 1000;
    std::int32_t mint = 0;
    std::int32_t maxt = 8000;
    
    std::uint32_t old;
    int nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getEnergyFilterBaselineGuardSamples(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterBaselineGuardSamples(i, mins));
        EQ(mins, std::int32_t(m_pModule->getEnergyFilterBaselineGuardSamples(i)));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterBaselineGuardSamples(i, maxs));
        EQ(maxs, std::int32_t(m_pModule->getEnergyFilterBaselineGuardSamples(i)));
        
        // Times have a granularity of 8 ns:
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterBaselineGuardTime(i, mint));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::abs(mint - std::int32_t(m_pModule->getEnergyFilterBaselineGuardTime(i))) < 8)
        );
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterBaselineGuardTime(i, maxt));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::abs(maxt - std::int32_t(m_pModule->getEnergyFilterBaselineGuardTime(i))) < 8)
        );
        
        m_pModule->setEnergyFilterBaselineGuardSamples(i, old);
    }
}

//  energyfilter pileup guard

void vx2750phatest::efpupguard()
{
    std::int32_t mins = 0;
    std::int32_t maxs = 8000;
    std::int32_t mint = 0;
    std::int32_t maxt = 6400;
    
    std::uint32_t old;
    int nch = m_pModule->channelCount();
    
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(old = m_pModule->getEnergyFilterPileupGuardSamples(i));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPileupGuardSamples(i, mins));
        EQ(mins, std::int32_t(m_pModule->getEnergyFilterPileupGuardSamples(i)));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPileupGuardSamples(i, maxs));
        EQ(maxs, std::int32_t(m_pModule->getEnergyFilterPileupGuardSamples(i)));
        
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPileupGuardTime(i, mint));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(mint - std::int32_t(m_pModule->getEnergyFilterPileupGuardTime(i))) < 8);
        );
        CPPUNIT_ASSERT_NO_THROW(m_pModule->setEnergyFilterPileupGuardTime(i, maxt));
        CPPUNIT_ASSERT_NO_THROW(
            ASSERT(std::labs(maxt - std::int32_t(m_pModule->getEnergyFilterPileupGuardTime(i))) < 8);
        );
        
        m_pModule->setEnergyFilterPileupGuardSamples(i, old);
    }
}

// get info about the board and how it's running:

void vx2750phatest::info()
{
    unsigned nch = m_pModule->channelCount();
    for (int i =0; i < nch; i++) {
        CPPUNIT_ASSERT_NO_THROW(m_pModule->getEnergyBits(i));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->getRealtime(i));
        CPPUNIT_ASSERT_NO_THROW(m_pModule->getDeadtime(i));
    }
}
// Commands except start/stop/sw trigger.....

void vx2750phatest::commands()
{
    CPPUNIT_ASSERT_NO_THROW(m_pModule->Reset());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->Clear());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->Arm());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->Disarm());
    CPPUNIT_ASSERT_NO_THROW(m_pModule->ReloadCalibration());
}