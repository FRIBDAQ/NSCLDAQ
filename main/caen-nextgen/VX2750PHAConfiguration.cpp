/*
*-------------------------------------------------------------
 
 CAEN SpA 
 Via Vetraia, 11 - 55049 - Viareggio ITALY
 +390594388398 - www.caen.it

------------------------------------------------------------

**************************************************************************
* @note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
*
* @file     VX2750PHAConfiguration.cpp
* @brief    Implement the PHA configuration class.
* @author   Ron Fox
*
*/

#include "VX2750PHAConfiguration.h"
#include "VX2750Pha.h"
#include <string>
#include <stdlib.h>
#include <map>
namespace caen_nscldaq {
// Local lookup tables:

static const std::map<unsigned, VX2750Pha::EnergyPeakingAverage> peakingAvgs =
{
  {1, VX2750Pha::Average1},
  {4, VX2750Pha::Average4},
  {16, VX2750Pha::EPeakAvg_Average16},
  {64, VX2750Pha::EPeakAvg_Average64}
};
static const std::map<unsigned, VX2750Pha::EnergyFilterBaselineAverage> blaverage=
{
   {0, VX2750Pha::Fixed},
   {16, VX2750Pha::EFilterBlineAvg_Average16},
   {64, VX2750Pha::EFilterBlineAvg_Average64},
   {256, VX2750Pha::Average256},
   {1024,VX2750Pha::Average1024},
   {4096, VX2750Pha::Average4K},
   {16384, VX2750Pha::Average16K}
};

    
/**
 *  constructor (default)
 * @param name - name of the object to hand off to the base class.
 *               this is used by various frameworks to look up the object.
 */
VX2750PHAModuleConfiguration::VX2750PHAModuleConfiguration(const char* name) :
  XXUSB::CConfigurableObject(name)
{
    defineReadoutOptions();
    defineGeneralOptions();
    defineAcqTriggerOptions();
    defineWfInspectionOptions();
    defineServiceOptions();
    defineITLOptions();
    defineLVDSOptions();
    defineDACOptions();
    defineInputConditioningOptions ();
    defineEventSelectionOptions();
    defineFilterOptions();
}

/**
 * constructor (copy)
 *    Since we have no data of our own, this is delegated fully to the base
 *    class
 *
 *  @param rhs - the object we're trying to copy construct.
 */
VX2750PHAModuleConfiguration::VX2750PHAModuleConfiguration(
    const VX2750PHAModuleConfiguration& rhs
) : XXUSB::CConfigurableObject(rhs)
{}

 /**
  * destructor
  *    Only need this for destructor chaining (to fill in the virtual
  *    method table).
  */
VX2750PHAModuleConfiguration::~VX2750PHAModuleConfiguration()
{}
    
 /**
  * assignment - again can delegate to the base class
  * @param rhs - the object being assigned to this.
  * @return VX2750PHAModuleConfiguration& (*this).
  */
VX2750PHAModuleConfiguration&
VX2750PHAModuleConfiguration::operator=(const VX2750PHAModuleConfiguration& rhs)
{
   XXUSB::CConfigurableObject::operator=(rhs);
   return *this;
}
/**
 * equality comparison
 *   @param rhs - object we're comparing to.
 *   @return int - nonzero if the two are equal.
 */
int
VX2750PHAModuleConfiguration::operator==(const VX2750PHAModuleConfiguration& rhs)
{
  return XXUSB::CConfigurableObject::operator==(rhs);
}
/**
 * inequality comparison.
 *   Rather than delegating we reuturn the boolean inverse of operator==
 *   in this way we're still good if we decide we need our own local data:
 * @param rhs - object we're comparing to.
 * @return int - nonzero if rhs is not equal to this.
 */
int
VX2750PHAModuleConfiguration::operator!=(const VX2750PHAModuleConfiguration& rhs)
{
   return !operator==(rhs);
}

/**
 * configureModule
 *    Configure the module in accordance with the current state of the configuration
 *    database.
 * @param module - module to configure.  The module object must already have
 *                 been connected to he physical hardware.
 */
void
VX2750PHAModuleConfiguration::configureModule(VX2750Pha& module)
{
  module.Reset();                   // Else we may get complaints about the default config.
  configureReadoutOptions(module);
  configureGeneralOptions(module);
  configureAcquisitionTriggerOptions(module);
  configureWfInspectionOptions(module);
  configureServiceOptions(module);
  configureITLOptions(module);
  configureLVDSOptions(module);
  configureDACOptions(module);
  configureInputConditioning(module);
  configureEventSelection(module);
  configureFilter(module);
}

 
////////////////////////////////////////////////////////////////////////////////
// Private methods define the various options:


/**
 * defineReadoutOptions
 *    Define options that control the readout.  These options default to false
 *    resulting in a minimal event if not otherwise configured.
 */
void
VX2750PHAModuleConfiguration::defineReadoutOptions()
{
    addBooleanParameter("readrawtimes", false);
    addBooleanParameter("readfinetimestamps", false);
    addBooleanParameter("readflags", false);
    addBooleanParameter("readtimedownsampling", false);
    addBoolListParameter("readanalogprobes", 2,2, false);
    addBoolListParameter("readdigitalprobes", 4,4, false);
    addBooleanParameter("readsamplecount", false);
    addBooleanParameter("readeventsize", false);
}
/**
 * configureReadoutOptions
 *    Configure the readout options in a module
 *  @param module - Te 
 */
void
VX2750PHAModuleConfiguration::configureReadoutOptions(VX2750Pha& module)
{
    // start from the default format:

    auto aprobeEnables = getBoolList("readanalogprobes");
    auto dprobeEnables = getBoolList("readdigitalprobes");
    module.setDefaultFormat();
    module.enableRawTimestamp(getBoolParameter("readrawtimes"));
    module.enableFineTimestamp(getBoolParameter("readfinetimestamps"));
    module.enableFlags(getBoolParameter("readflags"));
    module.enableDownsampling(getBoolParameter("readtimedownsampling"));
    module.enableAnalogProbes(aprobeEnables[0], aprobeEnables[1]);
    module.enableDigitalProbes(
       dprobeEnables[0], dprobeEnables[1], dprobeEnables[2], dprobeEnables[3]
    );
    module.enableSampleSize(getBoolParameter("readsamplecount"));
    module.enableRawEventSize(getBoolParameter("readeventsize"));
}

/**
 * defineGeneralOptions
 *    Define the general options.
 *
 */
void
VX2750PHAModuleConfiguration::defineGeneralOptions()
{
    const char* clockSources[] = {
       "Internal", "FPClkIn", // "P0ClkIn", "Link", "DIPswitchSel",
       nullptr
    };
    addEnumParameter("clocksource", clockSources, "Internal");
    // addBooleanParameter("outputp0clock", false);
    addBooleanParameter("outputfpclock", false);
    
    
}
/**
 * configureGeneralOptions
 *     Configure general options in a real module
 *   @param module -the module to configure.
 */
void
VX2750PHAModuleConfiguration::configureGeneralOptions(VX2750Pha& module)
{
  auto src = VX2750Pha::stringToClockSource.find(cget("clocksource"))->second;
  module.setClockSource(src);
  //module.setClockOutOnP0(getBoolParameter("outputp0clock"));
  module.setClockOutOnFP(getBoolParameter("outputfpclock"));
                                                      
                                                      
}
/**
 *  defineAcqTriggerOptions
 *  
 * define the options in the CAEN Acquisition Trigger and veto section
 */
void
VX2750PHAModuleConfiguration::defineAcqTriggerOptions()
{
     const char* startSrces[]  = {
         "EncodedClkIn", "SINlevel", "SINedge", "SWcmd", "LVDS", "P0", nullptr
     };
     addEnumListParameter("startsource", startSrces, "SWcmd", 1, 6, 1);
     
     const char* globalTriggerSrcs[] = {
         "TrgIn", "P0", "SwTrg", "LVDS", "ITLA", "ITLB",
         "ITLA_AND_ITLB", "ITLA_OR_ITLB", "EncodedClkIn", "GPIO", "TestPulse",
          nullptr
     };
     addEnumListParameter("gbltriggersrc", globalTriggerSrcs, "TrgIn", 1, 11, 1 );
     
     const char* waveTriggerSources[] = {
          "ITLB", "ITLA", "GlobalTriggerSource", "TRGIN", "ExternalInhibit",
          "ADCUnderSaturation", "ADCOverSaturation", "SWTrg", "ChSelfTrigger",
          "Ch64Trigger", "Disabled",
          nullptr
     };
     const char* eventTriggerSources[] = {
      "ITLB", "ITLA", "GlobalTriggerSource", "TRGIN", "SWTrg", "ChSelfTrigger",
      "Ch64Trigger", "Disabled", 
      nullptr
     };
     addListOfEnumLists("wavetriggersrc", waveTriggerSources, "TRGIN", 0,64,64);
     addListOfEnumLists("eventtriggersrc", eventTriggerSources, "TRGIN", 0,64,64);
     
     addIntListParameter("channeltriggermasks", 0, 64, 0, 64);

     const char* traceRecordModes[] = {
       "Always", "OnRequest", nullptr
     };
     addEnumListParameter("savetraces", traceRecordModes, "OnRequest", 0, 64, 64);
     
     const char* trgoutmodes[] = {
        "TRGIN", "P0", "SwTrg", "LVDS", "ITLA", "ITLB",
        "ITLA_AND_ITLB", "ITLA_OR_ITLB", "EncodedClkIn", "Run", "RefClk", "TestPulse",
        "Busy", "Fixed0", "Fixed1", "SyncIn", "SIN", "GPIO", "AcceptTrg", "TrgClk",
        nullptr
     };
     addEnumParameter("triggeroutmode", trgoutmodes, "TRGIN");
     
     const char* gpiomodes[] = {
       "Disabled", "TrgIn", "P0", "SIN", "LVDS", "ITLA", "ITLB",
       "ITLA_AND_ITLB", "ITLA_OR_ITLB", "EncodedClkIn", "SwTrg",
       "Run", "RefClk", "TestPulse", "Busy", "Fixed0", "Fixed1",
       nullptr
     };
     addEnumParameter("gpiomode", gpiomodes, "Disabled");
     
     const char* busyinSources[] = {
      "SIN", "GPIO", "LVDS", "Disabled",
        nullptr
     };
     addEnumParameter("busyinsrc", busyinSources, "Disabled");
     
     const char* syncoutmodes[] = {
        "Disabled", "SyncIn", "TestPulse", "IntClk", "Run",
        nullptr
     };
     addEnumParameter("syncoutmode", syncoutmodes, "Disabled");
     
     const char* boardvetosources[] = {
        "SIN", "LVDS", "GPIO", "P0", "EncodedClkIn", "Disabled",
        nullptr
     };
     addEnumParameter("boardvetosrc", boardvetosources, "Disabled");
     addIntegerParameter("boardvetowidth", 0, 34359738360, 200);
     const char* vetopolarities[] = {
      "ActiveHigh", "ActiveLow", nullptr
                    
     };
     addEnumParameter("boardvetopolarity", vetopolarities, "ActiveLow");
     
     
     const char* chanvetosources[ ] = {
        "BoardVeto", "ADCOverSaturation", "ADCUnderSaturation", "Disabled",
          nullptr
     };
     addEnumListParameter("chanvetosrc", chanvetosources, "Disabled", 0, 64, 64);
     addIntListParameter("chanvetowidth", 0, 524280, 0, 64, 64, 200);
     
     addIntegerParameter("rundelay", 0, 54280, 0);
     addBooleanParameter("autodisarm", true);
  
     addFloatParameter("volclkoutdelay", -18888.888, 18888.888, 0);
     addFloatParameter("permclkoutdelay", -18888.888, 18888.888, 0);
    
}
/**
 * configureAcquisitionTriggerOptions
 *    @param module -the module to configure.
 */
void
VX2750PHAModuleConfiguration::configureAcquisitionTriggerOptions(VX2750Pha& module)
{
  int nch = module.channelCount();
  
  auto startSources = getList("startsource");
  std::vector<VX2750Pha::StartSource>  startSourceVec;
  for (auto src :startSources) {
    startSourceVec.push_back(VX2750Pha::stringToStartSource.find(src)->second);
  }
  module.setStartSource(startSourceVec);
  auto globalTriggers = getList("gbltriggersrc");
  std::vector<VX2750Pha::GlobalTriggerSource> srcs;
  for (auto src: globalTriggers) {
    srcs.push_back(VX2750Pha::stringToGlobalTriggerSource.find(src)->second);
  }
  module.setGlobalTriggerSource(srcs);
  
  auto waveTriggers = getListOfLists("wavetriggersrc");
  auto evtTriggers  = getListOfLists("eventtriggersrc");
  auto triggerMasks = getUnsignedList("channeltriggermasks");   // since they're uint64_ts.
  auto saveTraces   = getList("savetraces");
  auto chanVetoSrcs = getList("chanvetosrc");
  auto chanVetoWidths = getIntegerList("chanvetowidth");
  
  for (int i =0; i < nch; i++) {
    if (waveTriggers.size() > i) {
      std::vector<VX2750Pha::WaveTriggerSource> srcs;
      for (auto s : waveTriggers[i]) {
        srcs.push_back(VX2750Pha::stringToWaveTrigger.find(s)->second);
      }
      module.setWaveTriggerSource(i, srcs);
    }
      if (evtTriggers.size() > i) {
        std::vector<VX2750Pha::EventTriggerSource> srcs;
        for (auto s : evtTriggers[i]) {
          srcs.push_back(VX2750Pha::stringToEventTrigger.find(s)->second);
        }
        module.setEventTriggerSource(i, srcs);
      }
      if (triggerMasks.size() > i)
        module.setChannelTriggerMask(i, triggerMasks[i]);
      if (saveTraces.size() > i) 
          module.setTraceRecordMode(i, (saveTraces[i] == "Always") ? VX2750Pha::Always : VX2750Pha::OnRequest);
      if (chanVetoSrcs.size() > i) 
        module.setChannelVetoSource(i, VX2750Pha::stringToChannelVeto.find(chanVetoSrcs[i])->second);
      if (chanVetoWidths.size() > i)
        module.setChannelVetoWidth(i, chanVetoWidths[i]);
  }
  
  module.setTRGOUTMode(VX2750Pha::stringToTRGOUT.find(cget("triggeroutmode"))->second);
  module.setGPIOMode(VX2750Pha::stringToGPIO.find(cget("gpiomode"))->second);
  module.setBusyInputSource(VX2750Pha::stringToBusyIn.find(cget("busyinsrc"))->second);
  module.setSyncOutMode(VX2750Pha::stringToSyncOut.find(cget("syncoutmode"))->second);
  module.setBoardVetoSource(VX2750Pha::stringToVeto.find(cget("boardvetosrc"))->second);
  module.setBoardVetoWidth(getIntegerParameter("boardvetowidth"));
  module.setBoardVetoPolarity(VX2750Pha::stringToVetoPolarity.find(cget("boardvetopolarity"))->second);
  module.setRunDelay(getIntegerParameter("rundelay"));
  module.setAutoDisarmEnabled(getBoolParameter("autodisarm"));
  
  module.setPermanentClockDelay(getFloatParameter("permclkoutdelay"));
  module.setVolatileClockDelay(getFloatParameter("volclkoutdelay"));
  
                                                 
  
}
/**
 * defineWfInspectionOptions
 *   Define options that control how waveform inspection (including
 *   digital probes) operate.
 *   See main class comments for more inforation.
 */
void
VX2750PHAModuleConfiguration::defineWfInspectionOptions()
{
  const char* wavesources[] = {
      "ADC_DATA", "ADC_TEST_TOGGLE", "ADC_TEST_RAMP", "ADC_TEST_SIN", "IPE",
      "Ramp", "SquareWave", "ADC_TEST_PRBS",
      nullptr
  };
  addEnumListParameter("wavesource", wavesources, "ADC_DATA", 0, 64, 64);
  addIntListParameter("recordsamples", 4, 8100,  0, 64, 64, 4);
  
  const char* wfresolutions[] = {
      "Res8", "Res16", "Res32", "Res64",
      nullptr
  };
  addEnumListParameter("waveresolutions", wfresolutions, "Res8", 0, 64, 64);
  
  const char* analogProbes[] = {
    "ADCInput", "TimeFilter", "EnergyFilter", "EnergyFilterBaseline",
    "EnergyFilterMinusBaseline",
    nullptr
  };
  addEnumListParameter("analogprobe1", analogProbes, "ADCInput", 0, 64, 64);
  addEnumListParameter("analogprobe2", analogProbes, "TimeFilter", 0, 64, 64);
  
  const char* digitalProbes[] = {
    "Trigger", "TimeFilterArmed", "ReTriggerGuard", "EnergyFilterBaselineFreeze",
    "EnergyFilterPeaking", "EnergyFilterPeakReady", "EnergyFilterPileUpGuard", "EventPileup", "ADCSaturation",
    "ADCSaturationProtection", "PostSaturationEvent", "EnergyFilterSaturation",
    "AcquisitionInhibit",
    nullptr
  };
  addEnumListParameter("digitalprobe1", digitalProbes, "Trigger", 0, 64, 64);
  addEnumListParameter("digitalprobe2", digitalProbes, "TimeFilterArmed", 0, 64, 64);
  addEnumListParameter("digitalprobe3", digitalProbes, "RetriggerGuard", 0, 64, 64);
  addEnumListParameter("digitalprobe4", digitalProbes, "EnergyFilterBaselineFreeze", 0, 64, 64);
  
  addIntListParameter("pretriggersamples", 4, 4000,  0, 64, 64, 100);
  
}
/**
 * configureWfInspectionOptions
 *    Configure the wave form inspection parameters from the internal configuration
 * @param module - reference to the module to configure.
 * @note these are all per channel configurations.
 */
 void
 VX2750PHAModuleConfiguration::configureWfInspectionOptions(VX2750Pha& module)
 {
    int nch = module.channelCount();            // # of channels in the module.
    auto wfsources = getList("wavesource");
    auto samples   = getIntegerList("recordsamples");
    auto resolutions = getList("waveresolutions");
    std::vector<std::string> analogProbes[2];
    analogProbes[0] = getList("analogprobe1");
    analogProbes[1] = getList("analogprobe2");
    std::vector<std::string> digitalProbes[4];
    digitalProbes[0] = getList("digitalprobe1");
    digitalProbes[1] = getList("digitalprobe2");
    digitalProbes[2] = getList("digitalprobe3");
    digitalProbes[3] = getList("digitalprobe4");
    auto pretrigger = getIntegerList("pretriggersamples");
    
    // Now loop over the channels setting the parameters:
    
    for (int i =0; i < nch; i++) {
      if (wfsources.size() > i)
        module.setWaveDataSource(
           i, VX2750Pha::stringToWaveDataSource.find((wfsources[i]))->second
        );
      if (samples.size() > i)
        module.setRecordSamples(i, samples[i]);
      if (resolutions.size() > i)
        module.setWaveResolution(
          i, VX2750Pha::stringToWaveResolution.find(resolutions[i])->second
        );
      
      for (int p = 0; p < 2; p++) {  // Constraints should make this <= 2
        auto& probes = analogProbes[p];
        if (probes.size() > i)
          module.setAnalogProbe(
            i , p, VX2750Pha::stringToAnalogProbe.find(probes[i])->second
          );
      }
      for (int p = 0; p < 4; p++) {
        auto& probes = digitalProbes[p];
        if (probes.size() > i)
            module.setDigitalProbe(
              i, p, VX2750Pha::stringToDigitalProbe.find(probes[i])->second
            );
      }
      module.setPreTriggerSamples(i, pretrigger[i]);
    }
 }
/**
 * defineServiceOptions
 *    Define options that control the writable digitizer service parameters.
 */
void
VX2750PHAModuleConfiguration::defineServiceOptions()
{
  addIntegerParameter("testpulseperiod", 0, 34359738360, 100000);
  addIntegerParameter("testpulsewidth", 0, 34359738360, 1000);
  addIntegerParameter("testpulselowlevel", 0, 65535, 0);
  addIntegerParameter("testpulsehighlevel", 0, 65535, 65535);
  
  const char* iolevels[] = {
    "NIM", "TTL", nullptr
  };
  addEnumParameter("iolevel", iolevels, "NIM");
  addIntegerParameter("errorflagmask", 0, 65535, 0);
  addIntegerParameter("errorflagdatamask", 0, 65535, 0);
}
/**
 * configureServiceOptions
 *    Configure the service options from the internal option database.
 * @param module - reference to the module being configured.
 */
void
VX2750PHAModuleConfiguration::configureServiceOptions(VX2750Pha& module)
{
    module.setTestPulsePeriod(getIntegerParameter("testpulseperiod"));
    module.setTestPulseWidth(getIntegerParameter("testpulsewidth"));
    module.setTestPulseLowLevel(getIntegerParameter("testpulselowlevel"));
    module.setTestPulseHighLevel(getIntegerParameter("testpulsehighlevel"));
    
    module.setIOLevel(cget("iolevel") == "NIM" ? VX2750Pha::NIM : VX2750Pha::TTL);
    module.setErrorFlagMask(getIntegerParameter("errorflagmask"));
    module.setErrorFlagDataMask(getIntegerParameter("errorflagdatamask"));
}
/**
 * defineITLOptions
 *    Define the options that control the two individual trigger logic
 *    blocks.
 */
void
VX2750PHAModuleConfiguration::defineITLOptions()
{
  const char* mainlogicoptions[] = {
    "OR", "AND", "Majority", nullptr
  };
  addEnumParameter("itlalogic", mainlogicoptions, "OR");
  addEnumParameter("itlblogic", mainlogicoptions, "OR");
  addIntegerParameter("itlamajoritylevel", 1, 63, 1);
  addIntegerParameter("itlbmajoritylevel", 1, 63, 1);
  
  const char* pairlogicoptions[] =  {
    "AND", "OR", "NONE", nullptr
  };
  addEnumParameter("itlapairlogic", pairlogicoptions, "None");
  addEnumParameter("itlbpairlogic", pairlogicoptions, "None");
  
  const char* itlpolarity[] = {
    "Direct", "Inverted", nullptr
  };
  addEnumParameter("itlapolarity", itlpolarity, "Direct");
  addEnumParameter("itlbpolarity", itlpolarity, "Direct");
  
  const char* itlconnection[] = {
    "Disabled", "ITLA", "ITLB", nullptr
  };
  //addEnumListParameter("itlconnect", itlconnection, "Disabled", 0, 64, 64);
  
  addIntegerParameter("itlamask", 0, 0xffffffffffffffff, 0xffffffffffffffff);
  addIntegerParameter("itlbmask", 0, 0xffffffffffffffff, 0xffffffffffffffff);
  addIntegerParameter("itlagatewidth", 0, 524280, 100);
  addIntegerParameter("itlbgatewidth", 0, 524280, 100);
  
}
/**
 * configureITLOptions
 *    Configure the module internal trigger logic from the internal configuration
 *    database.
 * @param module - references the module to be configured.
 */
void
VX2750PHAModuleConfiguration::configureITLOptions(VX2750Pha& module)
{
     module.setITLAMainLogic(VX2750Pha::stringToIndividualTriggerLogic.find(cget("itlalogic"))->second);
     module.setITLBMainLogic(VX2750Pha::stringToIndividualTriggerLogic.find(cget("itlblogic"))->second);
     module.setITLAMajorityLevel(getIntegerParameter("itlamajoritylevel"));
     module.setITLBMajorityLevel(getIntegerParameter("itlbmajoritylevel"));
     module.setITLAPairLogic(VX2750Pha::stringToPairLogic.find(cget("itlapairlogic"))->second);
     module.setITLBPairLogic(VX2750Pha::stringToPairLogic.find(cget("itlbpairlogic"))->second);
     module.setITLAInverted(cget("itlapolarity") == "Inverted" ? true : false);
     module.setITLBInverted(cget("itlbpolarity") == "Inverted" ? true : false);
     
     int nch= module.channelCount();
     //auto connections = getList("itlconnect");
     //for (int i = 0; i < nch; i++) {
     //   if (connections.size() > i) 
     //     module.setITLConnect(i, VX2750Pha::stringToITLConnect.find(connections[i])->second);
    //
     //   }
    module.setITLAMask(getUnsignedParameter("itlamask"));
    module.setITLBMask(getUnsignedParameter("itlbmask"));
    module.setITLAGateWidth(getUnsignedParameter("itlagatewidth"));
    module.setITLBGateWidth(getUnsignedParameter("itlbgatewidth"));
}
/**
 * defineLVDSOptions
 *     CReate settable options for the LVDS I/O pins.
 */
void
VX2750PHAModuleConfiguration::defineLVDSOptions()
{
  const char* lvdsmodes[] = {
    "SelfTriggers", "Sync", "IORegister", nullptr
  };
  addEnumListParameter("lvdsmode", lvdsmodes, "IORegister", 4, 4, 4);
  const char* lvdsdirections[] ={
    "Input", "Output", nullptr
  };
  addEnumListParameter("lvdsdirection", lvdsdirections, "Output", 4, 4, 4);
  addIntListParameter("lvdstrgmask", 0, 0xffffffffffffffff, 16, 16, 16, 0);
  addIntegerParameter("lvdsoutput", 0, 0xffff, 0);
}
/**
 * configureLVDSOptions
 *    COnfigures the LVDS options in a module from our internalconfiguration.
 * @param module -references the module to configure.
 */
void
VX2750PHAModuleConfiguration::configureLVDSOptions(VX2750Pha& module)
{
  auto modes = getList("lvdsmode");
  auto direction = getList("lvdsdirection");
  auto masks   = getUnsignedList("lvdstrgmask");
  
  // First set the direction and mode of each quartet.
  
  
  for (int i = 0; i <  modes.size(); i++) {
    module.setLVDSMode(i, VX2750Pha::stringToLVDSMode.find(modes[i])->second);
    module.setLVDSDirection(i, direction[i] == "Input" ? VX2750Pha::Input : VX2750Pha::Output);
  }
  
  // Now set the per pin trigger masks.
  
  for (int i =0; i < masks.size(); i++) {
    module.setLVDSTriggerMask(i, masks[i]);
  }
  module.setLVDSIOReg(getUnsignedParameter("lvdsoutput"));
}
/**
 * defineDACOptions
 *    Define the options for the digial to analog converter output.
 */
void
VX2750PHAModuleConfiguration::defineDACOptions()
{
  const char* dacoutmodes[]  = {
    "Static", "IPE", "ChInput", "MemOccupancy", "ChSum", "OverThrSum",
    "Ramp", "Sin5MHz", "Square",
    nullptr
  };
  addEnumParameter("dacoutmode", dacoutmodes, "ChSum");
  addIntegerParameter("dacoutputlevel", 0, 16383, 0);
  addIntegerParameter("dacoutchannel", 0, 63, 0);
}
/**
 * configureDACOptions
 *    @param module references a module that is to be configured.
 */
void
VX2750PHAModuleConfiguration::configureDACOptions(VX2750Pha& module)
{
  module.setDACOutMode(VX2750Pha::stringToDACOutMode.find(cget("dacoutmode"))->second);
  module.setDACOutValue(getIntegerParameter("dacoutputlevel"));
  module.setDACChannel(getIntegerParameter("dacoutchannel"));

}
/**
 * defineInputConditioningOptions
 *    See the header comments for these, and section 2.8 of the
 *    PHA parameter manual.
 */
void
VX2750PHAModuleConfiguration::defineInputConditioningOptions()
{
    addIntListParameter("vgagain", 0, 40, 0, 4, 4, 0);
    addBooleanParameter("offsetcalibrationenable", true);
    addBoolListParameter("channelenables", 0, 64, true, 64);
    addFloatListParameter("dcoffsets", 0.0, 100.0, 0, 64, 64, 50.0);
    addIntListParameter("triggerthresholds", 0, 8191, 0, 64, 64, 1023);
    
    const char* polarities[] = {"Positive", "Negative", nullptr};
    addEnumListParameter("inputpolarities", polarities, "Negative", 0, 64, 64);
    
}
/**
 * conifigureInputConditioning
 *    Configure how the analog inputs are conditioned prior to being
 *    presented to the FADC.
 * @param module - reference to module being configured.
 */
void
VX2750PHAModuleConfiguration::configureInputConditioning(VX2750Pha& module)
{
  
  
  // Sinc eonly 2745 digitizers have a variable gain amp:
  
  if (module.getFamilyCode() == 2745) {
    auto vgaGains = getIntegerList("vgagain");
    for (int i =0; i < 4; i++) {
         module.setVGAGain(i, vgaGains[i]); 
    }
  }
  // The remainder are all per channel parameters:
  
  auto enableOffsetCalibration = getBoolParameter("offsetcalibrationenable");
  auto channelEnables  = getBoolList("channelenables");
  auto dcOffsets  = getFloatList("dcoffsets");
  auto thresholds = getIntegerList("triggerthresholds");
  auto inputPolarities = getList("inputpolarities");
  
  module.enableOffsetCalibration(enableOffsetCalibration);
  
  int nch = module.channelCount();
  for (int i =0; i < nch; i++) {
    if (inputPolarities.size() > i)
      module.setPulsePolarity(
          i,
          (inputPolarities[i] == "Positive") ?
            VX2750Pha::Positive : VX2750Pha::Negative
      );
    if (channelEnables.size() > i) module.enableChannel(i, channelEnables[i]);
    if (dcOffsets.size() > i)      module.setDCOffset(i, dcOffsets[i]);
    if (thresholds.size() > i)     module.setTriggerThreshold(i, thresholds[i]);
    
  }
}
/**
 *  defineEventSelectionOptions
 *      Define the options for event selection and coincidences.
 *      see the header comments for the actual set of
 *      parameters.
 */
void
VX2750PHAModuleConfiguration::defineEventSelectionOptions()
{
  addIntListParameter("energyskimlow", 0, 65534, 0, 64, 64, 0);
  addIntListParameter("energyskimhigh", 0, 65534, 0, 64, 64, 65534);
  
  const char* selectors[] = {
    "All", "Pileup", "EnergySkim", nullptr
  };
  addEnumListParameter("eventselector", selectors, "All", 0, 64, 64);
  addEnumListParameter("waveselector", selectors, "All", 0, 64, 64);
  
  const char* masks[] = {
    "Disabled", "Ch64Trigger", "TRGIN", "GlobalTriggerSource", "ITLA", "ITLB",
    nullptr
  };
  addEnumListParameter("coincidencemask", masks, "Disabled", 0, 64, 64);
  addEnumListParameter("anticoincidencemask", masks, "Disabled", 0, 64, 64);
  
  addIntListParameter("coincidencelength", 8,  524280, 0, 64, 64, 100);
}
/**
 * configureEventSelection
 *   Apply the event selection configuration parameters to a module.
 * @param module -the module that will be configured.
 */
void
VX2750PHAModuleConfiguration::configureEventSelection(VX2750Pha& module)
{
  auto lowskims = getIntegerList("energyskimlow");
  auto hiskims  = getIntegerList("energyskimhigh");
  auto eventselectors = getList("eventselector");
  auto waveselectors = getList("waveselector");
  auto coincMasks = getList("coincidencemask");
  auto anticoincMasks = getList("anticoincidencemask");
  auto coincidencewindow = getIntegerList("coincidencelength");
  
  int nch = module.channelCount();
  for(int i =0; i < nch; i++) {
    if (lowskims.size() > i)
      module.setEnergySkimLowDiscriminator(i, lowskims[i]);
    if (hiskims.size() > i)
      module.setEnergySkimHighDiscriminator(i, hiskims[i]);
    if (eventselectors.size() > i)
      module.setEventSelector(i, VX2750Pha::stringToEventSelection.find(eventselectors[i])->second);
    if (waveselectors.size() > i)
       module.setWaveformSelector(i, VX2750Pha::stringToEventSelection.find(waveselectors[i])->second);
    if (coincidencewindow.size() > i)
      module.setCoincidenceNs(i, coincidencewindow[i]);
		if (coincMasks.size() > i)
      module.setCoincidenceMask(i, VX2750Pha::stringToCoincidenceMask.find(coincMasks[i])->second);
		if (anticoincMasks.size() > i)
      module.setAntiCoincidenceMask(i, VX2750Pha::stringToCoincidenceMask.find(anticoincMasks[i])->second);
  }
}
/**
 * defineFilterOptions
 *    Defines the time and energy filter configuration options.
 *    See header comments for parameter names.
 *    
 */
void
VX2750PHAModuleConfiguration::defineFilterOptions()
{
    addIntListParameter("tfrisetime", 4, 250, 0, 64, 64, 80);
    addIntListParameter("tfretriggerguard", 0, 1000, 0, 64, 64, 0);
    addIntListParameter("efrisetime", 4, 1625, 0, 64, 64, 80);
    addIntListParameter("efflattoptime", 4, 375, 0,64, 64, 80);
    addIntListParameter("efpeakingpos", 10, 90, 0, 64, 64, 50);
    
    const char* peakingaverages[] = {
      "1", "4", "16", "64", nullptr
    };
    addEnumListParameter("efpeakingavg", peakingaverages, "1", 0, 64, 64);
    addIntListParameter("efpolezero", 4, 65500, 0, 64, 64, 80);
    addFloatListParameter("effinegain", 0, 1.0, 10.0, 64, 64, 1.0);
    addBoolListParameter("eflflimitation", 0, 64, false, 64);
    
    const char* baselineaverages[] = {
      "0", "16", "64", "256", "1024", "4096", "16384", nullptr
    };
    addEnumListParameter("efbaselineavg", baselineaverages, "16", 0, 64, 64);
    addIntListParameter("efbaselineguardt", 0, 8000, 0, 64, 64, 0);
    addIntListParameter("efpileupguardt", 0, 8000, 0, 64, 64, 0);
    
}
/**
 * configureFilter
 *    Set the module filter options in accordance with our option database.
 * @parm module - module to configure.
 */
void
VX2750PHAModuleConfiguration::configureFilter(VX2750Pha& module)
{
    auto triggerRiseTimes = getIntegerList("tfrisetime");
    auto triggerRetriggerGuards = getIntegerList("tfretriggerguard");
    auto energyRiseTimes = getIntegerList("efrisetime");
    auto energyFlatTopTimes = getIntegerList("efflattoptime");
    auto energyPeakingPos = getIntegerList("efpeakingpos");
    auto peakingAverages  = getIntegerList("efpeakingavg");  // The enums all translate as integers.
    auto poleZeros       = getIntegerList("efpolezero");
    auto fineGains      =getFloatList("effinegain");
    auto lfEliminations = getBoolList("eflflimitation");
    auto blAveraging    = getIntegerList("efbaselineavg"); // again they're all integers
    auto blGuardTimes   = getIntegerList("efbaselineguardt");
    auto pupGuardTimes  = getIntegerList("efpileupguardt");
    
    int nch = module.channelCount();
    for (int i = 0; i < nch; i++) {
      if (triggerRiseTimes.size() > i)
        module.setTimeFilterRiseSamples(i, triggerRiseTimes[i]);
      if (triggerRetriggerGuards.size() > i)
        module.setTimeFilterRetriggerGuardSamples(i , triggerRetriggerGuards[i]);
      if (energyRiseTimes.size() > i)
        module.setEnergyFilterRiseSamples(i, energyRiseTimes[i]);
      if (energyFlatTopTimes.size() > i)
        module.setEnergyFilterFlatTopSamples(i, energyFlatTopTimes[i]);
      if (energyPeakingPos.size() > i)
        module.setEnergyFilterPeakingPosition(i, energyPeakingPos[i]);
      if (peakingAverages.size() > i)
        module.setEnergyFilterPeakingAverage(i, peakingAvgs.find(peakingAverages[i])->second);
      if (poleZeros.size() > i)
        module.setEnergyFilterPoleZeroSamples(i, poleZeros[i]);
      if (fineGains.size() > i)
        module.setEnergyFilterFineGain(i, fineGains[i]);
      if (lfEliminations.size() > i)
        module.enableEnergyFilterFLimitation(i, lfEliminations[i]);
      if (blAveraging.size() > i)
        module.setEnergyFilterBaselineAverage(i, blaverage.find(blAveraging[i])->second);
      if (blGuardTimes.size() > i)
        module.setEnergyFilterBaselineGuardTime(i, blGuardTimes[i]);
      if (pupGuardTimes.size() > i)
        module.setEnergyFilterPileupGuardTime(i, pupGuardTimes[i]);
    }
}

}                                   // caen_nscldaq namespace.



