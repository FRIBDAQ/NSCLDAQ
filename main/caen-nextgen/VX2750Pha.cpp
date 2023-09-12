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
* @file     VX2750Pha.cpp
* @brief    Implements the PHA class layered on Dig2Device.cpp
* @author   Ron Fox
* @todo     ITLConnect can support multiple connections.
*/
#include "VX2750Pha.h"
#include <stdexcept>
#include <sstream>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>

namespace caen_nscldaq {

static const std::uint64_t FPGA_NS_PER_CLOCK = 8;
static const unsigned DPP_MAX_PARAMS=30;    // sizes argv for DPP-PHA endpoint.

// Enumerator mappings.  Readonly have string->enum maps.  R/W have
// both a string to enum and enum to string map.  

static const std::map<std::string, VX2750Pha::FwType> stringToFwType = {
    { "DPP_PHA", VX2750Pha::DPP_PHA},
    { "DPP_ZLE", VX2750Pha::DPP_ZLE},
    { "DPP_PSD", VX2750Pha::DPP_PSD},
    { "DPP_DAW", VX2750Pha::DPP_DAW},
    { "DPP_OPEN", VX2750Pha::DPP_OPEN},
    {"Scope", VX2750Pha::Scope}
};

static const std::map<std::string, VX2750Pha::FormFactor> stringToFormFactor = {
    {"0", VX2750Pha::VME},
    {"1", VX2750Pha::VME64X},
    {"2", VX2750Pha::DT}
};

const std::map<std::string, VX2750Pha::ClockSource> VX2750Pha::stringToClockSource = {
    {"Internal", VX2750Pha::Internal},
    {"FPClkIn", VX2750Pha::FrontPanel},
    {"P0ClkIn",   VX2750Pha::Clock_P0},
    {"Link",   VX2750Pha::Link},
    {"DIPswitchSel", VX2750Pha::DIPSelected}
};
static const std::map<VX2750Pha::ClockSource, std::string> clockSourceToString = {
    {VX2750Pha::Internal, "Internal"},
    {VX2750Pha::FrontPanel, "FPClkIn"},
    {VX2750Pha::Clock_P0, "P0ClkIn"},
    {VX2750Pha::Link, "Link"},
    {VX2750Pha::DIPSelected, "DIPswitchSel"}
};

const std::map<std::string, VX2750Pha::StartSource> VX2750Pha::stringToStartSource = {
    {"EncodedClkIn", VX2750Pha::Start_EncodedClockIn},
    {"SINlevel", VX2750Pha::SINLevel},
    {"SINedge", VX2750Pha::SINEdge},
    {"SWcmd", VX2750Pha::SWCommand},
    {"LVDS", VX2750Pha::Start_LVDS},
    {"P0",   VX2750Pha::Start_P0}
};

static const std::map<VX2750Pha::StartSource, std::string> startSourceToString = {
    {VX2750Pha::Start_EncodedClockIn, "EncodedClkIn"},
    {VX2750Pha::SINLevel, "SINlevel"},
    {VX2750Pha::SINEdge, "SINedge"},
    {VX2750Pha::SWCommand, "SWcmd"},
    {VX2750Pha::Start_LVDS, "LVDS"},
    {VX2750Pha::Start_P0, "P0"}
};

const std::map<std::string, VX2750Pha::GlobalTriggerSource> VX2750Pha::stringToGlobalTriggerSource = {
    {"TrgIn", VX2750Pha::GlobalTrigger_TriggerIn},
    {"P0", VX2750Pha::GlobalTrigger_P0},
    {"SwTrg", VX2750Pha::GlobalTrigger_Software},
    {"LVDS", VX2750Pha::GlobalTrigger_LVDS},
    {"ITLA", VX2750Pha::GlobalTrigger_InternalA},
    {"ITLB", VX2750Pha::GlobalTrigger_InternalB},
    {"ITLA_AND_ITLB", VX2750Pha::GlobalTrigger_InternalAandB},
    {"ITLA_OR_ITLB", VX2750Pha::GlobalTrigger_InternalAorB},
    {"EncodedClkIn", VX2750Pha::GlobalTrigger_EncodedClockIn},
    {"GPIO", VX2750Pha::GlobalTrigger_GPIO},
    {"TestPulse", VX2750Pha::GlobalTrigger_TestPulse}
};
static const std::map<VX2750Pha::GlobalTriggerSource, std::string> globalTriggerSourceToString = {
    {VX2750Pha::GlobalTrigger_TriggerIn, "TrgIn"},
    {VX2750Pha::GlobalTrigger_P0, "P0"},
    {VX2750Pha::GlobalTrigger_Software, "SwTrg"},
    {VX2750Pha::GlobalTrigger_LVDS, "LVDS"},
    {VX2750Pha::GlobalTrigger_InternalA, "ITLA"},
    {VX2750Pha::GlobalTrigger_InternalB, "ITLB"},
    {VX2750Pha::GlobalTrigger_InternalAandB, "ITLA_AND_ITLB"},
    {VX2750Pha::GlobalTrigger_InternalAorB, "ITLA_OR_ITLB"},
    {VX2750Pha::GlobalTrigger_EncodedClockIn, "EncodedClkIn"},
    {VX2750Pha::GlobalTrigger_GPIO, "GPIO"},
    {VX2750Pha::GlobalTrigger_TestPulse, "TestPulse"}    
};

const std::map<std::string, VX2750Pha::WaveTriggerSource> VX2750Pha::stringToWaveTrigger= {
     {"ITLA", VX2750Pha::WaveTrigger_InternalA},
     {"ITLB", VX2750Pha::WaveTrigger_InternalB},
     {"GlobalTriggerSource", VX2750Pha::WaveTrigger_GlobalTriggerSource},
     {"TRGIN", VX2750Pha::WaveTrigger_TRGIN},
     {"ExternalInhibit", VX2750Pha::ExternalInhibit},
     {"ADCUnderSaturation", VX2750Pha::ADCUnderSaturation},
     {"ADCOverSaturation", VX2750Pha::ADCOverSaturation},
     {"SWTrg", VX2750Pha::WaveTrigger_Software},
     {"ChSelfTrigger", VX2750Pha::WaveTrigger_ChannelSelfTrigger},
     {"Ch64Trigger", VX2750Pha::WaveTrigger_AnyChannelSelfTrigger},
     {"Disabled", VX2750Pha::WaveTrigger_Disabled}
};
static const std::map<VX2750Pha::WaveTriggerSource, std::string> waveTriggerToString= {
     {VX2750Pha::WaveTrigger_InternalA, "ITLA"},
     {VX2750Pha::WaveTrigger_InternalB, "ITLB"},
     {VX2750Pha::WaveTrigger_GlobalTriggerSource, "GlobalTriggerSource"},
     {VX2750Pha::WaveTrigger_TRGIN, "TRGIN"},
     {VX2750Pha::ExternalInhibit, "ExternalInhibit"},
     {VX2750Pha::ADCUnderSaturation, "ADCUnderSaturation"},
     {VX2750Pha::ADCOverSaturation, "ADCOverSaturation"},
     {VX2750Pha::WaveTrigger_Software, "SWTrg"},
     {VX2750Pha::WaveTrigger_ChannelSelfTrigger, "ChSelfTrigger"},
     {VX2750Pha::WaveTrigger_AnyChannelSelfTrigger, "Ch64Trigger"},
     {VX2750Pha::WaveTrigger_Disabled, "Disabled"}
};

const std::map<std::string, VX2750Pha::EventTriggerSource> VX2750Pha::stringToEventTrigger = {
    {"ITLB", VX2750Pha::EventTrigger_InternalB},
    {"ITLA", VX2750Pha::EventTrigger_InternalA},
    {"GlobalTriggerSource", VX2750Pha::EventTrigger_GlobalTriggerSource},
    {"TRGIN", VX2750Pha::EventTrigger_TRGIN},
    {"SWTrg", VX2750Pha::EventTrigger_Software},
    {"ChSelfTrigger", VX2750Pha::EventTrigger_ChannelSelfTrigger},
    {"Ch64Trigger", VX2750Pha::EventTrigger_AnyChannelSelfTrigger},
    {"Disabled", VX2750Pha::EventTrigger_Disabled}
};
static const std::map<VX2750Pha::EventTriggerSource, std::string> eventTriggerToString = {
    {VX2750Pha::EventTrigger_InternalB, "ITLB"},
    {VX2750Pha::EventTrigger_InternalA, "ITLA"},
    {VX2750Pha::EventTrigger_GlobalTriggerSource, "GlobalTriggerSource"},
    {VX2750Pha::EventTrigger_TRGIN, "TRGIN"},
    {VX2750Pha::EventTrigger_Software, "SWTrg"},
    {VX2750Pha::EventTrigger_ChannelSelfTrigger, "ChSelfTrigger"},
    {VX2750Pha::EventTrigger_AnyChannelSelfTrigger, "Ch64Trigger"},
    {VX2750Pha::EventTrigger_Disabled, "Disabled"}    
};



static const std::map<std::string, VX2750Pha::TraceRecordMode> stringToTraceRecord = {
    {"Always", VX2750Pha::Always},
    {"OnRequest", VX2750Pha::OnRequest}
};
static const std::map<VX2750Pha::TraceRecordMode, std::string> traceRecordToString = {
    {VX2750Pha::Always, "Always"},
    {VX2750Pha::OnRequest, "OnRequest"}
};

const std::map<std::string, VX2750Pha::TRGOUTMode> VX2750Pha::stringToTRGOUT = {
    {"TrgIn", VX2750Pha::TriggerOut_TRGIN},
    {"P0", VX2750Pha::TriggerOut_P0},
    {"SwTrg", VX2750Pha::TriggerOut_Software},
    {"LVDS", VX2750Pha::TriggerOut_LVDS},
    {"ITLA", VX2750Pha::TriggerOut_InternalA},
    {"ITLB", VX2750Pha::TriggerOut_InternalB},
    {"ITLA_AND_ITLB", VX2750Pha::TriggerOut_InternalAandInternalB},
    {"ITLA_OR_ITLB", VX2750Pha::TriggerOut_InternalAorInternalB},
    {"EncodedClkIn", VX2750Pha::TriggerOut_EncodedClockIn},
    {"Run", VX2750Pha::TriggerOut_Run},
    {"RefClk", VX2750Pha::TriggerOut_ReferenceClock},
    {"TestPulse", VX2750Pha::TriggerOut_TestPulse},
    {"Busy", VX2750Pha::TriggerOut_Busy},
    {"Fixed0", VX2750Pha::TriggerOut_Zero},
    {"Fixed1", VX2750Pha::TriggerOut_One},
    {"SyncIn", VX2750Pha::TriggerOut_SynchIn},
    {"SIN", VX2750Pha::TriggerOut_SIN},
    {"GPIO", VX2750Pha::TriggerOut_GPIO},
    {"AcceptTrg", VX2750Pha::AcceptedTrigger},
    {"TrgClk", VX2750Pha::TriggerClock}
};

static const std::map<VX2750Pha::TRGOUTMode, std::string> TRGOUTToString = {
    {VX2750Pha::TriggerOut_TRGIN, "TrgIn"},
    {VX2750Pha::TriggerOut_P0, "P0"},
    {VX2750Pha::TriggerOut_Software, "SwTrg"},
    {VX2750Pha::TriggerOut_LVDS, "LVDS"},
    {VX2750Pha::TriggerOut_InternalA, "ITLA"},
    {VX2750Pha::TriggerOut_InternalB, "ITLB"},
    {VX2750Pha::TriggerOut_InternalAandInternalB, "ITLA_AND_ITLB"},
    {VX2750Pha::TriggerOut_InternalAorInternalB, "ITLA_OR_ITLB"},
    {VX2750Pha::TriggerOut_EncodedClockIn, "EncodedClkIn"},
    {VX2750Pha::TriggerOut_Run, "Run"},
    {VX2750Pha::TriggerOut_ReferenceClock, "RefClk"},
    {VX2750Pha::TriggerOut_TestPulse, "TestPulse"},
    {VX2750Pha::TriggerOut_Busy, "Busy"},
    {VX2750Pha::TriggerOut_Zero, "Fixed0"},
    {VX2750Pha::TriggerOut_One, "Fixed1"},
    {VX2750Pha::TriggerOut_SynchIn, "SyncIn"},
    {VX2750Pha::TriggerOut_SIN, "SIN"},
    {VX2750Pha::TriggerOut_GPIO, "GPIO"},
    {VX2750Pha::AcceptedTrigger, "AcceptTrg"},
    {VX2750Pha::TriggerClock, "TrgClk"}
};

const std::map<std::string, VX2750Pha::GPIOMode>  VX2750Pha::stringToGPIO = {
    {"Disabled", VX2750Pha::GPIOMode_Disabled},
    {"TrgIn", VX2750Pha::GPIOMode_TriggerIn},
    {"P0", VX2750Pha::GPIOMode_P0},
    {"SIN", VX2750Pha::GPIOMode_SIN},
    {"LVDS", VX2750Pha::GPIOMode_LVDS},
    {"ITLA", VX2750Pha::GPIOMode_InternalA},
    {"ITLB", VX2750Pha::GPIOMode_InternalB},
    {"ITLA_AND_ITLB", VX2750Pha::GPIOMode_InternalAandInternalB},
    {"ITLA_OR_ITLB", VX2750Pha::GPIOMode_InternalAorInternalB},
    {"EncodedClkIn", VX2750Pha::GPIOMode_EncodedClockIn},
    {"SwTrg", VX2750Pha::GPIOMode_SoftwareTrigger},
    {"Run", VX2750Pha::GPIOMode_Run},
    {"RefClk", VX2750Pha::GPIOMode_ReferenceClock},
    {"TestPulse", VX2750Pha::GPIOMode_TestPulse},
    {"Busy", VX2750Pha::GPIOMode_Busy},
    {"Fixed0", VX2750Pha::GPIOMode_Zero},
    {"Fixed1", VX2750Pha::GPIOMode_One}
};
static const std::map<VX2750Pha::GPIOMode,std::string> GPIOToString = {
    {VX2750Pha::GPIOMode_Disabled, "Disabled"},
    {VX2750Pha::GPIOMode_TriggerIn, "TrgIn"},
    {VX2750Pha::GPIOMode_P0, "P0"},
    {VX2750Pha::GPIOMode_SIN, "SIN"},
    {VX2750Pha::GPIOMode_LVDS, "LVDS"},
    {VX2750Pha::GPIOMode_InternalA, "ITLA"},
    {VX2750Pha::GPIOMode_InternalB, "ITLB"},
    {VX2750Pha::GPIOMode_InternalAandInternalB, "ITLA_AND_ITLB"},
    {VX2750Pha::GPIOMode_InternalAorInternalB, "ITLA_OR_ITLB"},
    {VX2750Pha::GPIOMode_EncodedClockIn, "EncodedClkIn"},
    {VX2750Pha::GPIOMode_SoftwareTrigger, "SwTrg"},
    {VX2750Pha::GPIOMode_Run, "Run"},
    {VX2750Pha::GPIOMode_ReferenceClock, "RefClk"},
    {VX2750Pha::GPIOMode_TestPulse, "TestPulse"},
    {VX2750Pha::GPIOMode_Busy, "Busy"},
    {VX2750Pha::GPIOMode_Zero, "Fixed0"},
    {VX2750Pha::GPIOMode_One, "Fixed1"}
};

const std::map<std::string, VX2750Pha::BusyInSource> VX2750Pha::stringToBusyIn = {
    {"SIN", VX2750Pha::BusyIn_SIN},
    {"GPIO", VX2750Pha::BusyIn_GPIO},
    {"LVDS", VX2750Pha::BusyIn_LVDS},
    {"Disabled", VX2750Pha::BusyIn_Disabled }
};
static const std::map<VX2750Pha::BusyInSource, std::string> busyInToString = {
    {VX2750Pha::BusyIn_SIN, "SIN"},
    {VX2750Pha::BusyIn_GPIO, "GPIO"},
    {VX2750Pha::BusyIn_LVDS, "LVDS"},
    {VX2750Pha::BusyIn_Disabled, "Disabled"}
};

const std::map<std::string, VX2750Pha::SyncOutMode> VX2750Pha::stringToSyncOut = {
    {"Disabled", VX2750Pha::SyncOut_Disabled},
    {"SyncIn", VX2750Pha::SyncOut_SynchIn},
    {"TestPulse", VX2750Pha::SyncOut_TestPulse},
    {"IntClk", VX2750Pha::InternalClock},
    {"Run", VX2750Pha::SyncOut_Run}
};
static const std::map<VX2750Pha::SyncOutMode, std::string> syncOutToString = {
    {VX2750Pha::SyncOut_Disabled, "Disabled"},
    {VX2750Pha::SyncOut_SynchIn, "SyncIn"},
    {VX2750Pha::SyncOut_TestPulse, "TestPulse"},
    {VX2750Pha::InternalClock, "IntClk"},
    {VX2750Pha::SyncOut_Run, "Run"}
};

const std::map<std::string , VX2750Pha::VetoSource>  VX2750Pha::stringToVeto = {
    {"SIN", VX2750Pha::Veto_SIN},
    {"LVDS", VX2750Pha::Veto_LVDS},
    {"GPIO", VX2750Pha::Veto_GPIO},
    {"P0", VX2750Pha::Veto_P0},
    {"EncodedClkIn", VX2750Pha::Veto_EncodedClock},
    {"Disabled", VX2750Pha::Veto_Disabled}
};
static const std::map<VX2750Pha::VetoSource, std::string>  vetoToString = {
    {VX2750Pha::Veto_SIN, "SIN"},
    {VX2750Pha::Veto_LVDS, "LVDS"},
    {VX2750Pha::Veto_GPIO, "GPIO"},
    {VX2750Pha::Veto_P0, "P0"},
    {VX2750Pha::Veto_EncodedClock, "EncodedClkIn"},
    {VX2750Pha::Veto_Disabled, "Disabled"}
};

const std::map<std::string, VX2750Pha::VetoPolarity> VX2750Pha::stringToVetoPolarity = {
    {"ActiveHigh", VX2750Pha::ActiveHigh},
    {"ActiveLow", VX2750Pha::ActiveLow}
};
static const std::map<VX2750Pha::VetoPolarity, std::string> vetoPolarityToString = {
    {VX2750Pha::ActiveHigh, "ActiveHigh"},
    {VX2750Pha::ActiveLow, "ActiveLow"}
};

const std::map<std::string, VX2750Pha::ChannelVetoSource> VX2750Pha::stringToChannelVeto = {
    {"BoardVeto", VX2750Pha::BoardVeto},
    {"ADCOverSaturation", VX2750Pha::OverSaturation},
    {"ADCUnderSaturation", VX2750Pha::UnderSaturation},
    {"Disabled", VX2750Pha::ChanVeto_Disabled}
};
static const std::map<VX2750Pha::ChannelVetoSource, std::string> channelVetoToString = {
    {VX2750Pha::BoardVeto, "BoardVeto"},
    {VX2750Pha::OverSaturation, "ADCOverSaturation"},
    {VX2750Pha::UnderSaturation, "ADCUnderSaturation"},
    {VX2750Pha::ChanVeto_Disabled, "Disabled"}
};

const std::map<std::string, VX2750Pha::WaveDataSource> VX2750Pha::stringToWaveDataSource = {
    {"ADC_DATA", VX2750Pha::ADC_DATA},
    {"ADC_TEST_TOGGLE", VX2750Pha::ADC_TEST_TOGGLE},
    {"ADC_TEST_RAMP", VX2750Pha::ADC_TEST_RAMP},
    {"ADC_TEST_SIN", VX2750Pha::ADC_SIN},
    {"IPE", VX2750Pha::WaveSource_IPE},
    {"Ramp", VX2750Pha::WaveSource_Ramp},
    {"SquareWave", VX2750Pha::SquareWave},
    {"ADC_TEST_PRBS", VX2750Pha::ADC_TEST_PRBS}
};
static const std::map<VX2750Pha::WaveDataSource, std::string> waveDataSourceToString = {
    {VX2750Pha::ADC_DATA, "ADC_DATA"},
    {VX2750Pha::ADC_TEST_TOGGLE, "ADC_TEST_TOGGLE"},
    {VX2750Pha::ADC_TEST_RAMP, "ADC_TEST_RAMP"},
    {VX2750Pha::ADC_SIN, "ADC_TEST_SIN"},
    {VX2750Pha::WaveSource_IPE, "IPE"},
    {VX2750Pha::WaveSource_Ramp, "Ramp"},
    {VX2750Pha::SquareWave, "SquareWave"},
    {VX2750Pha::ADC_TEST_PRBS, "ADC_TEST_PRBS"}
};

const std::map<std::string, VX2750Pha::WaveResolution> VX2750Pha::stringToWaveResolution = {
    {"Res8", VX2750Pha::Res8},
    {"Res16", VX2750Pha::Res16},
    {"Res32", VX2750Pha::Res32},
    {"Res64", VX2750Pha::Res64} 
};
static const std::map<VX2750Pha::WaveResolution, std::string> waveResolutionToString = {
    {VX2750Pha::Res8, "Res8",},
    {VX2750Pha::Res16, "Res16"},
    {VX2750Pha::Res32, "Res32"},
    {VX2750Pha::Res64, "Res64"} 
};

const std::map<std::string, VX2750Pha::AnalogProbe> VX2750Pha::stringToAnalogProbe = {
    {"ADCInput", VX2750Pha::ADCInput},
    {"TimeFilter", VX2750Pha::TimeFilter},
    { "EnergyFilter", VX2750Pha::EnergyFilter},
    {"EnergyFilterBaseline", VX2750Pha::EnergyFilterBaseline},
    {"EnergyFilterMinusBaseline", VX2750Pha::EnergyFilterMinusBaseline}
};
static const std::map<VX2750Pha::AnalogProbe, std::string> analogProbeToString = {
    {VX2750Pha::ADCInput, "ADCInput"},
    {VX2750Pha::TimeFilter,"TimeFilter"},
    {VX2750Pha::EnergyFilter, "EnergyFilter"},
    {VX2750Pha::EnergyFilterBaseline, "EnergyFilterBaseline"},
    {VX2750Pha::EnergyFilterMinusBaseline, "EnergyFilterMinusBaseline"} 
};

const std::map<std::string, VX2750Pha::DigitalProbe> VX2750Pha::stringToDigitalProbe = {
    {"Trigger", VX2750Pha::DProbe_Trigger},
    {"TimeFilterArmed", VX2750Pha::TimeFilterArmed},
    {"ReTriggerGuard", VX2750Pha::ReTriggerGuard},
    {"EnergyFilterBaselineFreeze", VX2750Pha::EneryFilterBaselineFreeze},
    {"EnergyFilterPeaking", VX2750Pha::EnergyFilterPeaking},
    {"EnergyFilterPeakReady", VX2750Pha::EnergyFilterPeakReady},
    {"EnergyFilterPileUpGuard", VX2750Pha::EnergyFilterPileupGuard},
    {"EventPileup", VX2750Pha::EventPileup},
    {"ADCSaturation", VX2750Pha::ADCSaturation},
    {"ADCSaturationProtection", VX2750Pha::ADCSaturationProtection},
    {"PostSaturationEvent", VX2750Pha::PostSaturationEvent},
    {"EnergyFilterSaturation", VX2750Pha::EnergyFilterSaturation},
    {"AcquisitionInhibit", VX2750Pha::AcquisitionInhibit}
};
static const std::map<VX2750Pha::DigitalProbe, std::string> digitalProbeToString = {
    {VX2750Pha::DProbe_Trigger, "Trigger"},
    {VX2750Pha::TimeFilterArmed, "TimeFilterArmed"},
    {VX2750Pha::ReTriggerGuard, "ReTriggerGuard"},
    {VX2750Pha::EneryFilterBaselineFreeze, "EnergyFilterBaselineFreeze"},
    {VX2750Pha::EnergyFilterPeaking, "EnergyFilterPeaking" },
    {VX2750Pha::EnergyFilterPeakReady, "EnergyFilterPeakReady"},
    {VX2750Pha::EnergyFilterPileupGuard, "EnergyFilterPileUpGuard"},
    {VX2750Pha::EventPileup, "EventPileup"},
    {VX2750Pha::ADCSaturation, "ADCSaturation"},
    {VX2750Pha::ADCSaturationProtection, "ADCSaturationProtection"},
    {VX2750Pha::PostSaturationEvent, "PostSaturationEvent"},
    {VX2750Pha::EnergyFilterSaturation, "EnergyFilterSaturation" },
    {VX2750Pha::AcquisitionInhibit, "AcquisitionInhibit"}
};

static const std::map<std::string, VX2750Pha::IOLevel> stringToIOLevel = {
    {"TTL", VX2750Pha::TTL},
    {"NIM", VX2750Pha::NIM}
};
static const std::map<VX2750Pha::IOLevel, std::string> ioLevelToString = {
    {VX2750Pha::NIM, "NIM"},
    {VX2750Pha::TTL, "TTL"}
};

const std::map<std::string, VX2750Pha::IndividualTriggerLogic> VX2750Pha::stringToIndividualTriggerLogic = {
    {"OR", VX2750Pha::ITL_OR},
    {"AND", VX2750Pha::ITL_AND},
    {"Majority", VX2750Pha::Majority}
};
static const std::map<VX2750Pha::IndividualTriggerLogic, std::string> individualTriggerLogicToString = {
    {VX2750Pha::ITL_OR, "OR"},
    {VX2750Pha::ITL_AND, "AND"},
    {VX2750Pha::Majority, "Majority"}
};

const std::map<std::string, VX2750Pha::PairTriggerLogic> VX2750Pha::stringToPairLogic = {
    {"AND", VX2750Pha::PTL_AND},
    {"OR", VX2750Pha::PTL_OR},
    {"NONE", VX2750Pha::NONE}
};
static const std::map<VX2750Pha::PairTriggerLogic, std::string> pairLogicToString = {
    {VX2750Pha::PTL_AND, "AND"},
    {VX2750Pha::PTL_OR, "OR"},
    {VX2750Pha::NONE, "NONE"}
};

const std::map<std::string, VX2750Pha::ITLConnect> VX2750Pha::stringToITLConnect = {
    {"Disabled", VX2750Pha::ITL_Disabled},
    {"ITLA", VX2750Pha::ITL_ITLA},
    {"ITLB", VX2750Pha::ITL_ITLB}
};
static const std::map<VX2750Pha::ITLConnect, std::string> ITLConnectToString = {
    {VX2750Pha::ITL_Disabled, "Disabled"},
    {VX2750Pha::ITL_ITLA, "ITLA"},
    {VX2750Pha::ITL_ITLB, "ITLB"}
};

const std::map<std::string, VX2750Pha::LVDSMode> VX2750Pha::stringToLVDSMode = {
     {"SelfTriggers", VX2750Pha::SelfTriggers},
     {"Sync", VX2750Pha::Sync},
     {"IORegister", VX2750Pha::IORegister}
};
static const std::map<VX2750Pha::LVDSMode, std::string> LVDSModeToString = {
     {VX2750Pha::SelfTriggers, "SelfTriggers"},
     {VX2750Pha::Sync, "Sync"},
     {VX2750Pha::IORegister, "IORegister"}    
};

static const std::map<std::string, VX2750Pha::LVDSDirection> stringToLVDSDirection = {
    {"Input", VX2750Pha::Input},
    {"Output", VX2750Pha::Output}
};
static const std::map<VX2750Pha::LVDSDirection, std::string> LVDSDirectionToString = {
    {VX2750Pha::Input, "Input"},
    {VX2750Pha::Output, "Output"}    
};

const std::map<std::string, VX2750Pha::DACOutputMode> VX2750Pha::stringToDACOutMode = {
    {"Static", VX2750Pha::Static},
    {"IPE", VX2750Pha::DACOut_IPE},
    {"ChInput", VX2750Pha::DACOut_ChInput},
    {"MemOccupancy", VX2750Pha::MemOccupancy},
    {"ChSum", VX2750Pha::ChSum},
    {"OverThrSum", VX2750Pha::OverThrSum},
    {"Ramp", VX2750Pha::DACOut_Ramp},
    {"Sin5MHz", VX2750Pha::Sine5MHz},
    {"Square", VX2750Pha::Square}
};
static const std::map<VX2750Pha::DACOutputMode, std::string> DACOutModeToString = {
    {VX2750Pha::Static, "Static"},
    {VX2750Pha::DACOut_IPE, "IPE"},
    {VX2750Pha::DACOut_ChInput, "ChInput"},
    {VX2750Pha::MemOccupancy, "MemOccupancy"},
    {VX2750Pha::ChSum, "ChSum"},
    {VX2750Pha::OverThrSum, "OverThrSum"},
    {VX2750Pha::DACOut_Ramp, "Ramp"},
    {VX2750Pha::Sine5MHz, "Sin5MHz"},
    {VX2750Pha::Square, "Square"}
};

static const std::map<std::string, VX2750Pha::Polarity> stringToPolarity = {
    {"Positive", VX2750Pha::Positive},
    {"Negative", VX2750Pha::Negative}
};
static const std::map<VX2750Pha::Polarity, std::string> polarityToString = {
    {VX2750Pha::Positive, "Positive"},
    {VX2750Pha::Negative, "Negative"}
};

const std::map<std::string, VX2750Pha::EventSelection> VX2750Pha::stringToEventSelection = {
    {"All", VX2750Pha::All},
    {"Pileup", VX2750Pha::Pileup},
    {"EnergySkim", VX2750Pha::EnergySkim}
};
static const std::map<VX2750Pha::EventSelection, std::string> eventSelectionToString = {
    {VX2750Pha::All, "All"},
    {VX2750Pha::Pileup, "Pileup"},
    {VX2750Pha::EnergySkim, "EnergySkim"}
};

const std::map<std::string, VX2750Pha::CoincidenceMask> VX2750Pha::stringToCoincidenceMask = {
    {"Disabled", VX2750Pha::Coincidence_Disabled},
    {"Ch64Trigger", VX2750Pha::Ch64Trigger},
    {"TRGIN", VX2750Pha::Coincidence_TRGIN},
    {"GlobalTriggerSource", VX2750Pha::Coincidence_GlobalTriggerSource},
    {"ITLA", VX2750Pha::Coincidence_ITLA},
    {"ITLB", VX2750Pha::Coincidence_ITLB}
};
static const std::map<VX2750Pha::CoincidenceMask, std::string> coincidenceMaskToString = {
    {VX2750Pha::Coincidence_Disabled, "Disabled"},
    {VX2750Pha::Ch64Trigger, "Ch64Trigger"},
    {VX2750Pha::Coincidence_TRGIN, "TRGIN"},
    {VX2750Pha::Coincidence_GlobalTriggerSource, "GlobalTriggerSource"},
    {VX2750Pha::Coincidence_ITLA, "ITLA"},
    {VX2750Pha::Coincidence_ITLB, "ITLB"}
};

static const std::map<std::string, VX2750Pha::EnergyPeakingAverage> stringToPeakingAverage = {
    {"OneShot", VX2750Pha::Average1},
    {"LowAVG", VX2750Pha::Average4},
    {"MediumAVG", VX2750Pha::EPeakAvg_Average16},
    {"HighAVG", VX2750Pha::EPeakAvg_Average64}
};
static const std::map<VX2750Pha::EnergyPeakingAverage, std::string> peakingAverageToString = {
    {VX2750Pha::Average1, "OneShot"},
    {VX2750Pha::Average4, "LowAVG"},
    {VX2750Pha::EPeakAvg_Average16, "MediumAVG"},
    {VX2750Pha::EPeakAvg_Average64, "HighAVG"}
};

static const std::map<std::string, VX2750Pha::EnergyFilterBaselineAverage> stringToBLAverage = {
    {"Fixed", VX2750Pha::Fixed},
    {"VeryLow", VX2750Pha::EFilterBlineAvg_Average16},
    {"Low", VX2750Pha::EFilterBlineAvg_Average64},
    {"MediumLow", VX2750Pha::Average256},
    {"Medium", VX2750Pha::Average1024},
    {"MediumHigh", VX2750Pha::Average4K},
    {"High", VX2750Pha::Average16K}
};
static const std::map<VX2750Pha::EnergyFilterBaselineAverage, std::string> BLAverageToString = {
    {VX2750Pha::Fixed, "Fixed"},
    {VX2750Pha::EFilterBlineAvg_Average16, "VeryLow"},
    {VX2750Pha::EFilterBlineAvg_Average64, "Low"},
    {VX2750Pha::Average256, "MediumLow"},
    {VX2750Pha::Average1024, "Medium"},
    {VX2750Pha::Average4K, "MediumHigh"},
    {VX2750Pha::Average16K, "High"}
};

static const std::map<std::string, VX2750Pha::Endpoint> stringToEndpoint = {
    {"raw", VX2750Pha::Raw},
    {"dpppha", VX2750Pha::PHA}
};
static const std::map<VX2750Pha::Endpoint, std::string> endpointToString = {
    {VX2750Pha::Raw, "raw"},
    {VX2750Pha::PHA, "dpppha"}
};
////////////////////////////////////////////////////////////////////////////////
// Public methods

    /**
     * constructor
     *    @param hostOrPid - Host or PID specification
     *    @param isUsb     - True if the connection is via direct USB.
     */
    VX2750Pha::VX2750Pha(const char* hostOrPid, bool isUsb) :
        Dig2Device(hostOrPid, isUsb)
    {
        
    }
    /**
     * destructor - ensures chaining to the base class.
     */
    VX2750Pha::~VX2750Pha() {}
    
    /**
     * getCupVersion
     *    Return the CupVersion string (I wonder is this supposed to be CPU?) const;.
     * @return std::string
     */
    std::string
    VX2750Pha::getCupVersion() const
    {
        return GetDeviceValue("CupVer");
    }
    /**
     * getFPGA_FwVer
     *     Return firmware version type.
     *  @return std::string
     */
    std::string
    VX2750Pha::getFpgaFWVersion() const
    {
        return GetDeviceValue("FPGA_FwVer");
    }
    /**
     * Get the firmware type. Note that if the firmware type is anything other
     * than DPP_PHA this class is worthless.
     *
     * @return FWType
     */
    VX2750Pha::FwType
    VX2750Pha::getFirmwareType() const
    {
        std::string strFwType = GetDeviceValue("FwType");
        return stringToEnum(stringToFwType, strFwType);
    }
    /**
     * getModelCode
     *     Return the module model code.
     * @return std::string
     */
    std::string
    VX2750Pha::getModelCode() const
    {
        return GetDeviceValue("ModelCode");
    }
    /**
     * getPiggyBackCode
     *    @return std::string - the piggy back card type code.
     */
    std::string
    VX2750Pha::getPiggyBackCode() const
    {
        return GetDeviceValue("PBCode");
    }
    /**
     * getModelName
     *    @return std::string - module model code.
     *    
     */
    std::string
    VX2750Pha::getModelName() const
    {
        return GetDeviceValue("ModelName");
    }
    /**
     * getFormFactor
     * 
     * @return FormFactor - The form factor of the module.
     */
    VX2750Pha::FormFactor
    VX2750Pha::getFormFactor() const
    {
        return stringToEnum(stringToFormFactor, GetDeviceValue("FormFactor"));
    }
    /**
     * getFamilyCode
     *   @return int - family code number.
     */
    int
    VX2750Pha::getFamilyCode() const
    {
        return GetDeviceInteger("FamilyCode");
    }
    /**
     * getSerialNumber
     *    @return std::string - serial string.
     */
    std::string
    VX2750Pha::getSerialNumber() const
    {
        return GetDeviceValue("SerialNum");
    }
    /**
     *   getMotherboardRev
     * @return int - motherboard revision level
     */
    int
    VX2750Pha::getMotherboardRev() const
    {
        return GetDeviceInteger("PCBrev_MB");
    }
    /**
     * getPiggybackRev
     *    @return int - daughterboard revision level
     */
    int
    VX2750Pha::getPiggybackRev() const
    {
        return GetDeviceInteger("PCBrev_PB");
    }
    /**
     *  getLicense
     *  @return std::string - license code.
     */
    std::string
    VX2750Pha::getLicense() const
    {
        return GetDeviceValue("License");    
    }
    /**
     * isLicensed
     * @return bool  True if this board is licensed.
     * @throw std::runtime_error - if the value is not recognized. 
     */
    bool
    VX2750Pha::isLicensed() const
    {
        auto value = GetDeviceValue("LicenseStatus");
        if (value == "Licensed") {
            return true;
        } else if (value == "NotLicensed") {
            return false;
        } else {
            std::stringstream s;
            s << "isLicensed - invalid value from device: '" << value << "'";
            std::string msg = s.str();
            throw std::runtime_error(msg);
        }
    }
    /**
     * remainingUnlicensedTime
     *   @return int - number of minutes remaining to run on an unlicensed board.
     */
    int
    VX2750Pha::remainingUnlicensedTime() const
    {
        return GetDeviceInteger("LicenseRemainingTime");
    }
    /**
     * channelCount
     * @return int - number of channels the board has available.
     */
    int
    VX2750Pha::channelCount() const
    {
        return GetDeviceInteger("NumCh");
    }
    /**
     * bitsOfResolution
     *    @return int -number of bits of resolution th adc has.
     */
    int
    VX2750Pha::bitsOfResolution() const
    {
        return GetDeviceInteger("ADC_Nbit");
    }
    /**
     * sampleRate
     *   @return int The ADC sample rate in MHz
     */
    int
    VX2750Pha::sampleRate() const
    {
        return GetDeviceInteger("ADC_SamplRate");
    }
    /**
     * inputRange
     *    @return int   - Volts peak-to-peak of input range.
     */
    int
    VX2750Pha::inputRange() const
    {
        return GetDeviceInteger("InputRange"); 
    }
    /**
     *  isDifferential
     *     @return bool - true of inputs are diff if single ended, false.
     *     @throw std::runtime_error if the value of the InputType parameter is
     *     unrecognizable.
     */
    bool
    VX2750Pha::isDifferentialInput() const
    {
        auto value = GetDeviceInteger("InputType");
        switch (value) {
        case 0:
            return false;
        case 1:
            return true;
        default:
            {
                std::stringstream s;
                s << "isDifferential unrecognized value from digitizer: '" << value << "'";
                std::string msg = s.str();
                throw std::runtime_error(msg);
            }
        }
        
    }
    /**
     * inputImpedance
     *    @return int - input impetance in ohms.
     */
    int
    VX2750Pha::inputImpedance() const
    {
        return GetDeviceInteger("Zin");
    }
    
    /**
     * ipAddress
     *   @return int  - uint32_t actually with IP address.
     */
    int
    VX2750Pha::ipAddress() const
    {
        std::string ip = GetDeviceValue("IPAddress");
        return dottedToInt(ip);
    }
    /**
     * netMask
     *  @return int  Network mask.
     */
    int
    VX2750Pha::netMask() const
    {
        std::string netmask = GetDeviceValue("Netmask");
        return dottedToInt(netmask);
    }
    /**
     * gateway
     *    @return int - gateway IP
     */
    int
    VX2750Pha::gateway() const
    {
        std::string gw = GetDeviceValue("Gateway");
        return dottedToInt(gw);
    }
    /**
     *  getClockSource
     *     @return V2750Pha::ClockSource - The clock source.
     */
    VX2750Pha::ClockSource
    VX2750Pha::getClockSource() const
    {
        std::string strSource = GetDeviceValue("ClockSource");
        return stringToEnum(stringToClockSource, strSource);
    }
    /**
     * setClockSou8rce
     *    Set the source of the unit's clock:
     *  @param source - a VX2750Pha::ClockSource value selecting the source.
     */
    void
    VX2750Pha::setClockSource(ClockSource selection) const
    {
        std::string src = enumToString(clockSourceToString, selection);
        SetDeviceValue("ClockSource", src.c_str());
    }
    /**
     * isClockOutOnP0
     *    @return bool - true if the clock output is enabled on the backplane P0 connector.
     */
    bool
    VX2750Pha::isClockOutOnP0() const
    {
        return  GetDeviceBool("EnClockOutP0");
        

    }
    /**
     * setClockOutOnP0
     *    Enable/disable the clock to be output on the P0 connector.
     *  @param bool - Desired state, true to enable.
     */
    void
    VX2750Pha::setClockOutOnP0(bool state) const
    {
        SetDeviceValue("EnClockOutP0", state );
    }
    /**
     * isClockOutOnP0
     *    @return bool - true if the clock is enabled to be output on the front
     *                   panel.
     */
    bool
    VX2750Pha::isClockOutOnFP() const
    {
        return GetDeviceBool("EnClockOutFP");
        
    }
    /**
     * setCLockoutOnFP
     *     Enable/disable the clock to be output to the front panel.
     * @param bool  - desired state - true means output false don't.
     */
    void
    VX2750Pha::setClockOutOnFP(bool state) const
    {
        
        SetDeviceValue("EnClockOutFP", state);
    }
    /**
     * getStartSource
     *   @return std::vector<VX2750Pha::StartSource> - describes the sources selected to start
     *          digitizer runs.
     */
    std::vector<VX2750Pha::StartSource>
    VX2750Pha::getStartSource() const
    {
        std::string strValue = GetDeviceValue("StartSource");
        // this is a | separated list of start sources:
        //
        auto stringList = orListToStringList(strValue);
        std::vector<StartSource> result;
        for (auto s : stringList)  {
            result.push_back(stringToEnum(stringToStartSource, s));
        }
        return result;
    }
    /**
     * setStartSource
     *    @param src - std::Vector<VX2750Pha::StartSource> that selects what will start
     *       data acquisition on a digitizer.
     */
    void
    VX2750Pha::setStartSource(const std::vector<StartSource>& src) const
    {
        if (src.size() == 0) {
            throw std::logic_error("setStartSource requires at least one source");
        }
        std::vector<std::string> sources;
        for (auto s : src) {
            sources.push_back(enumToString(startSourceToString, s));
        }        
        std::string strValue = stringListToOrList(sources);

        SetDeviceValue("StartSource", strValue.c_str());
    }
    /**
     * getGLobalTriggerSource
     *   @return std::vector<GlobalTriggerSource>  Returns a vector that
     *      contains all of the global trigger sources that have been
     *      selected.
     */
    std::vector<VX2750Pha::GlobalTriggerSource>
    VX2750Pha::getGlobalTriggerSource() const
    {
        std::string strValue = GetDeviceValue("GlobalTriggerSource");
        
        // This string is a | separated list of trigger sources.
        // so first we get a list of strings using orListToStringList
        // Then we look up each string in the list:
        
        auto stringList = orListToStringList(strValue);
        std::vector<GlobalTriggerSource> result;
        for (auto s : stringList) {
            result.push_back(stringToEnum(stringToGlobalTriggerSource, s));
        }        
        return result;
    }
    /**
     * setGlobalTriggerSource
     *    @param sel - vector of VX2750Pha::GlobalTriggerSource that selects all
     *    of the things that can fire the global trigger.  The actual trigger
     *    is a logical or of all those sources.
     */
    void
    VX2750Pha::setGlobalTriggerSource(const std::vector<GlobalTriggerSource>& sel) const
    {
        if (sel.size() == 0) {
            throw std::logic_error("setGlobalTriggerSource, requires at least one trigger source");
        }
        // First turn the sources into a list of strings, then
        // we can use stringListToOrList to construct what has to be sent to the
        // device:
        
        std::vector<std::string> sources;
        for (auto s : sel) {
            sources.push_back(enumToString(globalTriggerSourceToString, s));
        }
        
        
        std::string strValue = stringListToOrList(sources);
        SetDeviceValue("GlobalTriggerSource", strValue.c_str());
    }
    /**
     * getWaveTriggerSource
     *     @param ch  - A digitizer channel number
     *     @return vector<VX2750Pha::WaveTriggerSource> - the wave trigger sources of ch.
     */
    std::vector<VX2750Pha::WaveTriggerSource>
    VX2750Pha::getWaveTriggerSource(unsigned ch) const
    {
        
        std::string strValue = GetChanValue(ch, "WaveTriggerSource");
        auto stringList      = orListToStringList(strValue);
        std::vector<WaveTriggerSource> result;
        for (auto s: stringList ) {
            result.push_back(stringToEnum(stringToWaveTrigger, s));
        }
        return result;
    }
    /**
     * setWaveTriggerSource
     *
     *    @param ch - channel number
     *    @param selection - vector WaveTriggerSource  to set as thee selected value.
     *                       the or of all of the triggers sources in the vector
     *                       is the actual trigger condition.
     *    @note - no attempt is made to remove duplicates.
     *    @note - At least one element must be in the selection.
     */
    void
    VX2750Pha::setWaveTriggerSource(unsigned ch, const std::vector<WaveTriggerSource>& selection) const
    {
        if (selection.size() == 0) {
            throw std::logic_error("At least one wave trigger source is required");
        }
        // conver the selection vector to a vector of enum values:
        
        std::vector<std::string> stringList;
        for (auto s : selection) {
            stringList.push_back(enumToString(waveTriggerToString, s));
        }
        
        std::string value = stringListToOrList(stringList);
        SetChanValue(ch, "WaveTriggerSource", value.c_str());
    }
    /**
     * getEventTriggerSource
     *    @param ch   - a digitizer channel
     *    @return std::vector<VX2750Pha::EventTriggerSource> - sources of ch's event trigger.
     */
    std::vector<VX2750Pha::EventTriggerSource>
    VX2750Pha::getEventTriggerSource(unsigned ch) const
    {
        std::string strValue = GetChanValue(ch, "EventTriggerSource");
        auto stringList = orListToStringList(strValue);
        std::vector<VX2750Pha::EventTriggerSource> result;
        for (auto s : stringList) {
            result.push_back(stringToEnum(stringToEventTrigger, s)); 
        }
        return result;
    }
    /**
     * setEventTriggerSource
     *    @param ch - a digitizer channel number.
     *    @param selection - the VX2750Pha::EventTriggerSources selected for that channel.
     *          The actual trigger source is the or of the sources in the vector.
     *          At least one trigger source must be present.
     */
    void
    VX2750Pha::setEventTriggerSource(unsigned ch, const std::vector<EventTriggerSource>& selection) const
    {
        // Make a vector of strings and then convert that to an orlist:
        
        std::vector<std::string> stringList;
        for (auto s : selection) {
            stringList.push_back(enumToString(eventTriggerToString, s));
        }
        
        
        std::string sel = stringListToOrList(stringList);;
        SetChanValue(ch, "EventTriggerSource", sel.c_str());
    }
        /**
     *    getChannelTriggerMask
     *  @param ch - Digitizer channel
     *  @return std::uint64_t - trigger mask fr the channel.
     *  @note the base class does not provide for 64 bit integer values --
     */
    std::uint64_t
    VX2750Pha::getChannelTriggerMask(unsigned ch) const
    {
        std::string strValue = GetChanValue(ch, "ChannelsTriggerMask");
        std::stringstream s(strValue);
        std::uint64_t result;
        s >> result;
        return result;
    }
    /**
     * setChannelTriggerMask
     *  @param ch   - The channel to set.
     *  @param mask - the trigger mask to set.
     */
    void
    VX2750Pha::setChannelTriggerMask(unsigned ch, std::uint64_t mask) const
    {
        std::stringstream s;
        s << mask;
        std::string value = s.str();
        SetChanValue(ch, "ChannelsTriggerMask", value.c_str());
    }
    /** getTraceRecordMode
     *    @param ch - digitizer channel.
     *    @return VX2750Pha::TraceRecordMode - the trace record mode for that channel.
     */
    VX2750Pha::TraceRecordMode
    VX2750Pha::getTraceRecordMode(unsigned chan) const
    {
        std::string strValue = GetChanValue(chan, "WaveSaving");
        return stringToEnum(stringToTraceRecord, strValue);
    }
    /**
     * setTraceRecordMode
     *    @param ch - digitizer channel.
     *    @param mode - desired trace recording mode for that channel.
     */
    void
    VX2750Pha::setTraceRecordMode(unsigned ch, TraceRecordMode mode) const
    {
        std::string value =  enumToString(traceRecordToString, mode);
        SetChanValue(ch, "WaveSaving", value.c_str());
    }
    /**
     * getTRGOUTMode
     *    Get what's selected to be output on the TRGOUT connector.
     * @return TRGOUTMode
     */
    VX2750Pha::TRGOUTMode
    VX2750Pha::getTRGOUTMode() const
    {
        std::string strMode = GetDeviceValue("TrgOutMode");
        return stringToEnum(stringToTRGOUT, strMode);
    }
    /**
     * setTRGOUTMode
     *    Select what's on the TRGOUT output.
     * @param select - VX2750Pha::TRGOutMode selecting what's output.
     */
    void
    VX2750Pha::setTRGOUTMode(TRGOUTMode select) const
    {
        std::string strMode = enumToString(TRGOUTToString, select);
        SetDeviceValue("TrgOutMode", strMode.c_str());
    }
    /**
     * getGPIOMode
     *    @return VX2750Pha::GPIOMode - value indicating what is signaled on the GPIO output.
     */
    VX2750Pha::GPIOMode
    VX2750Pha::getGPIOMode() const
    {
        std::string value = GetDeviceValue("GPIOMode");
        return stringToEnum(stringToGPIO, value);
    }
    /**
     * setGPIOMode
     *    @param mode  - the desired output on the GPIO output.
     */
    void
    VX2750Pha::setGPIOMode(GPIOMode mode) const
    {
        std::string value = enumToString(GPIOToString, mode);
        SetDeviceValue("GPIOMode", value.c_str());
    }
    /**
     * getBusyInputSource
     *   @return VX2750Pha::BusyInSource - The current selection for the board busy.
     */
    VX2750Pha::BusyInSource
    VX2750Pha::getBusyInputSource() const
    {
        std::string value = GetDeviceValue("BusyInSource");
        return stringToEnum(stringToBusyIn, value);
    }
    /** setBusyInputSource
     *  @param src - desired busy source.
     */
    void
    VX2750Pha::setBusyInputSource(VX2750Pha::BusyInSource src) const
    {
        std::string value = enumToString(busyInToString, src);
        SetDeviceValue("BusyInSource", value.c_str());
    }
    /**
     *  getSyncOutMode
     *     @return VX2750Pha::SyncOutMode - the current selection of the CLKOUT connector
     */
    VX2750Pha::SyncOutMode
    VX2750Pha::getSyncOutMode() const
    {
        std::string value = GetDeviceValue("SyncOutMode");
        return stringToEnum(stringToSyncOut, value);
    }
    /**
     * setSyncOutMode
     *    @param mode - desired output on the CLKOUT
     */
    void
    VX2750Pha::setSyncOutMode(SyncOutMode mode) const
    {
        std::string value =  enumToString(syncOutToString, mode);
        SetDeviceValue("SyncOutMode", value.c_str());
        
    }
    /**
     * getBoardVetoSource
     *    @return V2750::VetoSource - condition used to veto acquisitiontriggers.
     */
    VX2750Pha::VetoSource
    VX2750Pha::getBoardVetoSource() const
    {
        std::string src = GetDeviceValue("BoardVetoSource");
        return stringToEnum(stringToVeto, src);
    }
    /**
     * setBoardVetoSource
     *   @param src  - Specifies where the board VETO comes from.
     */
    void
    VX2750Pha::setBoardVetoSource(VetoSource src) const
    {
        std::string value = enumToString(vetoToString, src);
        SetDeviceValue("BoardVetoSource", value.c_str());
    }
    /**
     * getBoardVetoWidth
     *     Gets amount of strecth in the board veto in ns.  Note that 0 means
     *     the veto is only active as long as the source is asserted
     *  @return int - nanoseconds of stretch
     */
    std::uint64_t
    VX2750Pha::getBoardVetoWidth() const
    {
        // Longer than an integer so:
        
        std::string strValue = GetDeviceValue("BoardVetoWidth");
        std::stringstream s(strValue);
        std::uint64_t result;
        s >> result;
        return result;
    }
    /**
     * setBoardVetoWidth
     *    @param ns - number of nanoseconds the veto should be stretched after
     *                the source is deasserted.
     */
    void
    VX2750Pha::setBoardVetoWidth(std::uint64_t ns) const
    {
        // since SetDeviceValue overloads to in ints and this can be longer:
        
        std::stringstream strValue;
        strValue << ns;
        std::string value = strValue.str();
        SetDeviceValue("BoardVetoWidth", value.c_str());
    }
    /**
     * getBoardVetoPolarity
     *    @return VX2750Pha::VetoPolarity  - the selected veto source poliarity.
     */
    VX2750Pha::VetoPolarity
    VX2750Pha::getBoardVetoPolarity() const
    {
        std::string strValue = GetDeviceValue("BoardVetoPolarity");
        return stringToEnum(stringToVetoPolarity, strValue);
    }
    /**
     * setVetoPolarity
     *    @param pol  - Which polarity makes the veto source asserted.
     */
    void
    VX2750Pha::setBoardVetoPolarity(VetoPolarity pol) const
    {
        std::string strValue = enumToString(vetoPolarityToString, pol);
        SetDeviceValue("BoardVetoPolarity", strValue.c_str());
    }
    /**
     * getChannelVetoSource
     *    Return the source of a per channel veto.
     *
     *  @param ch - channel number
     *  @return VX2750Pha::ChannelVetoSource - veto source for ch.
     */
    VX2750Pha::ChannelVetoSource
    VX2750Pha::getChannelVetoSource(unsigned ch) const
    {
        std::string strValue = GetChanValue(ch,  "ChannelVetoSource");
        return stringToEnum(stringToChannelVeto, strValue);
    }
    /**
     * setChannelVetoSource
     *    @param ch - channel number
     *    @param src - Source of the veto for that channel.
     */
    void
    VX2750Pha::setChannelVetoSource(unsigned ch, ChannelVetoSource src) const
    {
        std::string strValue = enumToString(channelVetoToString, src);
        SetChanValue(ch,  "ChannelVetoSource", strValue.c_str());
    }
    /**
     *     getChanneVetoWidth
     *   Returns the number of nanoseconds for a channel veto (if the source
     *   is ADC under or over saturation).
     *   the deassertion of the source.
     *
     *  @param chan - the channel.
     *  @return std::uint32_t - the stretch.
     */
    std::uint32_t
    VX2750Pha::getChannelVetoWidth(unsigned chan) const
    {
        return GetChanInteger(chan, "ADCVetoWidth");
    }
    /**
     * setChannelVetoWidth
     *   Sets the adc channel veto width for a channel if the source is
     *   over/under saturation.
     * @param chan - channel number.
     * @param ns   - nanoseconds.
     */

    void
    VX2750Pha::setChannelVetoWidth(unsigned chan, std::uint32_t ns) const
    {
        SetChanValue(chan, "ADCVetoWidth", static_cast<int>(ns));
    }
    /**
     * getRunDelay
     *    Reads the delay between electronic (not software) run start and
     *    actual run start.  This is intended to compensate for propagation delays
     *    on a start daisy chain (earlier modules delay longer), and provides
     *    a fully synchronized start of data acquisition in multiple modules.
     *    
     *  @return std::uint32_t nanoseconds delay
     */
    std::uint32_t
    VX2750Pha::getRunDelay() const
    {
        return GetDeviceInteger("RunDelay");
    }
    /**
     * setRunDelay
     *    Set the run delay (see above).
     *  @param ns - nanoseconds of run delay requested.
     */
    void
    VX2750Pha::setRunDelay(std::uint32_t ns) const
    {
        SetDeviceValue("RunDelay", static_cast<int>(ns));
    }
    /**
     * isAutoDisarmEnabled
     *    @return bool - If true, the digitizer is set to disarm at the end of
     *                   a run.  If false, the digitizer remains armed when the run end.
     */
    
    bool
    VX2750Pha::isAutoDisarmEnabled() const
    {
        return textToBool(GetDeviceValue("EnAutoDisarmAcq"));
        
    }
    /**
     * setAutoDisarmEnabled
     *    Set the state of the auto disarm
     *  @param state - desired state.
     */
    void
    VX2750Pha::setAutoDisarmEnabled(bool state) const
    {
        std::string stringValue = state ? "True" : "False";
        SetDeviceValue("EnAutoDisarmAcq", stringValue.c_str());
    }
    /**
     * isMultiWindowRunEnabled
     * @return bool - state: if true, runs last for the duration of the RUN input
     *                 signal. If false, runs can last for the duration of several
     *                 RUN Input transitions.  See the methods that control thye
     *                 timestamp pause parameter.
     */
    bool
    VX2750Pha::isMultiWindowRunEnabled() const
    {
        return textToBool(GetDeviceValue("EnMultiWindowRun"));
    }
    /**
     * setMultiWindowRunEnable
     *    @param state - desired state.  See above.
     */
    void
    VX2750Pha::setMultiWindowRunEnable(bool state) const {
        std::string strValue = state ? "True" : "False";
        SetDeviceValue("EnMultiWindowRun", strValue.c_str());
    }
    /**
     * isTimestampHoldEnabled
     *    @bool - True if timestamp increments are suspended when the run is paused
     */
    bool
    VX2750Pha::isPauseTimestampHoldEnabled() const
    {
        std::string value = GetDeviceValue("PauseTimeStamp");
        // Hold -> true, Run -> False.
        
        if (value == "Hold") {
            return true;
        } else if (value == "Run") {
            return false;
        } else {
            std::stringstream strError;
            strError << "isTimestampHoldEnabled got an unrecognized value: '"
                << value << "'";
            std::string msg = strError.str();
            throw std::runtime_error(msg);
        }
    }
    /**
     * setPauseTimestampHold
     *    @param bool - true if the timestamp should not increment (Hold)
     *                  when runs are paused.
     */
    void
    VX2750Pha::setPauseTimestampHold(bool enable) const
    {
        std::string value = enable ? "Hold" : "Run";
        SetDeviceValue("PauseTimestamp", value.c_str());
    }
    /**
     * getLEDStatus
     *    Return a 32 bit value bit bits set indicating the LED
     *    status.  Symbolic bitmask definitions are in VX2750Pha::LED_*
     * @return std::uint32
     */
    std::uint32_t
    VX2750Pha::getLEDStatus() const
    {
        return GetDeviceInteger("LedStatus");
        
    }
    /**
     * getAcquisitionStatus
     *   @return std::uint32_t - bit mask indicating acquisition status.
     *          The VX2750Pha::ACQ_* symbols provide symbolic definitions for the
     *          bit masks for each git in the response
     */
    std::uint32_t
    VX2750Pha::getAcquisitionStatus() const
    {
        return GetDeviceInteger("AcquisitionStatus");
    }
    /**
     * getMaxRawDataSize
     *    @return std::uint32_t The largest number of bytes of data that can be
     *                          returned in reads from the raw endpoint.
     */
    std::uint32_t
    VX2750Pha::getMaxRawDataSize() const
    {
        return GetDeviceInteger("MaxRawDataSize");
    }
    /**
     *  getVolatileClockDelay
     *       Gets the current delay in clock output relative to the input reference
     *       clock.
     * @return double
     */
    double
    VX2750Pha::getVolatileClockDelay() const
    {
        return GetDeviceReal("VolatileClockOutDelay");
    }
    /**
     *  setVolatileClockDelay
     *     @param ps - picoseconds of delay between input and output clock.
     */
    void
    VX2750Pha::setVolatileClockDelay(double ps) const
    {
        SetDeviceValue("VolatileClockOutDelay", ps);
    }
    /**
     * getPermanentClockOutDelay
     *    @return double -ps - clock delay loaded into the hardware on power
     *                         up.
     */
    double
    VX2750Pha::getPermanentClockDelay() const
    {
        return GetDeviceReal("PermanentClockOutDelay");
    }
    /**
     * setPermanentCLockDelay
     *     @param double ps - picoseconds delay loaded on power up into the clock
     *                        delay.
     */
    void
    VX2750Pha::setPermanentClockDelay(double ps) const
    {
        SetDeviceValue("PermanentClockOutDelay", ps);
    }
    /**
     * getWaveDataSource
     *     @param ch - channelnumber.
     *     @return VX2750Pha::WaveDataSource - enumerated value that describes
     *              the source of waveform data sent to pulse processing
     *              for that channel.
     */
    VX2750Pha::WaveDataSource
    VX2750Pha::getWaveDataSource(unsigned ch) const
    {
        return stringToEnum(stringToWaveDataSource, GetChanValue(ch, "WaveDataSource"));
    }
    /**
     * setWaveDataSource
     *    @param ch - channel to set.
     *    @param selection - select the waveform data source sent to DPP
     */
    void
    VX2750Pha::setWaveDataSource(unsigned chan, WaveDataSource selection) const
    {
        std::string value = enumToString(waveDataSourceToString, selection);
        SetChanValue(chan, "WaveDataSource", value.c_str());
    }
    /**
     * getRecordSamples
     *    @param ch - channel number.
     *    @return int - number of samples of waveform that will be acquired.
     */
    std::uint32_t
    VX2750Pha::getRecordSamples(unsigned ch) const
    {
        return GetChanInteger(ch, "ChRecordLengthS");
    }
    /**
     * setRecorcdSamples
     *    @param ch - channel number
     *    @param samples - number of samples to acquire.
     */
    void
    VX2750Pha::setRecordSamples(unsigned ch, std::uint32_t samples) const
    {
        SetChanValue(ch, "ChRecordLengthS", static_cast<int>(samples));
    }
    /**
     * getRecordNs
     *   @param ch - channel number.
     *   @return std::uint32_t - number of nanoseconds of waveform that will be recorded.
     */
    std::uint32_t
    VX2750Pha::getRecordNs(unsigned ch) const
    {
        return GetChanInteger(ch, "ChRecordLengthT");
    }
    /**
     * setRecordNs
     *    @param ch - channel number.
     *    @param ns - Desired of captured waveform.  Note that the actual
     *                set value may differ if ns is not an exact number of samples
     */
    void
    VX2750Pha::setRecordNs(unsigned ch, std::uint32_t ns) const
    {
        SetChanValue(ch, "ChRecordLengthT", static_cast<int>(ns));
    }
    /**
     * getWaveResolution
     *     Return the actual time covered by a waveform sample.
     *     This supports saveform decimation.
     * @param ch - channel.
     * @return WaveResolution - Enum indication the selected resolution.
     */
    VX2750Pha::WaveResolution
    VX2750Pha::getWaveResolution(unsigned ch) const
    {
        std::string value = GetChanValue(ch, "WaveResolution");
        return stringToEnum(stringToWaveResolution, value);
    }
    /**
     * setWaveResolution
     *    @param ch - channel to set.
     *    @param sel - selector.
     */
    void
    VX2750Pha::setWaveResolution(unsigned ch, WaveResolution sel) const
    {
        std::string value = enumToString(waveResolutionToString, sel);
        SetChanValue(ch, "WaveResolution", value.c_str());
    }
    /**
     * getAnalogProbe
     *   @param ch  - channel number.
     *   @param probeNum - probe number , 0 or 1.
     *   @return VX2750Pha::AnalogProbe.
     */
    VX2750Pha::AnalogProbe
    VX2750Pha::getAnalogProbe(unsigned ch, unsigned probeNum) const
    {
        checkInclusiveRange(0, 1, probeNum);         // Good probe number.
        std::string param = appendNumber("WaveAnalogProbe", probeNum);
        
        std::string value = GetChanValue(ch, param.c_str());
        return stringToEnum(stringToAnalogProbe, value);
    }
    /**
     * setAnalogProbe
     *   @param ch - channel number,
     *   @param probeNum -probe number.
     *   @param selection - selected prob3e.
     */
    void
    VX2750Pha::setAnalogProbe(unsigned ch, unsigned probeNum, AnalogProbe selection) const
    {
        checkInclusiveRange(0, 1, probeNum);
        std::string param = appendNumber("WaveAnalogProbe", probeNum);
        
        std::string value = enumToString(analogProbeToString, selection);
        SetChanValue(ch, param.c_str(), value.c_str());
    }
    /**
     * getDigitalProbe
     *   @param ch - channel
     *   @param probe - number of the probe to get.
     *   @return VX2750Pha::DigitalProbe
     */
    VX2750Pha::DigitalProbe
    VX2750Pha::getDigitalProbe(unsigned ch, unsigned probe) const
    {
        checkInclusiveRange(0, 3, probe);
        std::string param = appendNumber("WaveDigitalProbe", probe);
        std::string value = GetChanValue(ch, param.c_str());
        return stringToEnum(stringToDigitalProbe, value);
    }
    /**
     * setDigitalProbe
     *   @param ch - channel to set.
     *   @param probe - number of probe to set [0-3]
     *   @param selection  - The probe to select
     */
    void
    VX2750Pha::setDigitalProbe(unsigned ch, unsigned probe, DigitalProbe selection) const
    {
        checkInclusiveRange(0, 3, probe);
        std::string param = appendNumber("WaveDigitalProbe", probe);
        std::string value = enumToString(digitalProbeToString, selection);
        SetChanValue(ch, param.c_str(), value.c_str());
    }
    /**
     * getPreTriggerSamples
     *    @param chan  - channel numbger.
     *    @return std::uint32_t nsamples -samples in the pretrigger for that channel.
     */
    std::uint32_t
    VX2750Pha::getPreTriggerSamples(unsigned chan) const
    {
        return GetChanInteger(chan, "ChPreTriggerS");
    }
    /**
     * setPreTriggerSamples
     *    @param ch - channel number.
     *    @param samples - number of samples to have in the channels' pretrigger window.
     */
    void
    VX2750Pha::setPreTriggerSamples(unsigned ch, std::uint32_t samples) const
    {
        SetChanValue(ch, "ChPreTriggerS", static_cast<int>(samples));
    }
    /**
     * getPreTriggerNs
     *    Get the pretrigger time window in nanoseconds.
     *  @param ch - channel
     *  @return std::uint32_t - nanoseconds.
     */
    std::uint32_t
    VX2750Pha::getPreTriggerNs(unsigned chan) const
    {
        return GetChanInteger(chan, "ChPreTriggerT");
    }
    /**
     * setPreTriggerNs
     *    Set the pretrigger time window in nanoseconds.
     *  @param ch - channel
     *  @param ns -desired pretrigger window.
     */
    void
    VX2750Pha::setPreTriggerNs(unsigned chan, std::uint32_t ns) const
    {
        SetChanValue(chan, "ChPreTriggerT", static_cast<int>(ns));
    }
    
    /**
     * getTestPulsePeriod
     *    @return std::uint32_t = nanoseconds between repetitions of the test pulse.
     */
    std::uint64_t
    VX2750Pha::getTestPulsePeriod() const
    {
        return GetDeviceULong("TestPulsePeriod");
    }
    /**
     * setTestPulsePeriod.
     * @param ns - desired nanoseconds between repetitions of the test pulse.
     */
    void
    VX2750Pha::setTestPulsePeriod(std::uint64_t ns) const
    {
        SetDeviceValue("TestPulsePeriod", ns);
    }
    /**
     * getTestPulseWidth
     *    @return std::uint32_t test pulse width in ns.
     */
    std::uint64_t
    VX2750Pha::getTestPulseWidth() const
    {
        return GetDeviceULong("TestPulseWidth");
    }
    /**
     * setTestPulseWidth
     *    @param ns - desired nanonseconds for which the test pulse is high.
     */
    void
    VX2750Pha::setTestPulseWidth(std::uint64_t ns) const
    {
        SetDeviceValue("TestPulseWidth", ns);
    }
    /**
     * getTestPulseLowLevel
     * @return std::uint32_t  ADC Counts for the low value of the test pulse.
     */
    std::uint32_t
    VX2750Pha::getTestPulseLowLevel() const
    {
        return GetDeviceInteger("TestPulseLowLevel");
    }
    /**
     * setTestPulseLowLevel
     *   @param counts - adc counts for the test pulse low level.
     */
    void
    VX2750Pha::setTestPulseLowLevel(std::uint32_t counts) const
    {
        SetDeviceValue("TestPulseLowLevel", static_cast<int>(counts));
    }
    /**
     * getTestPulseHigHlevel
     *    @return std::uint32_t  - adc counts when the test pulser is high
     */
    std::uint32_t
    VX2750Pha::getTestPulseHighLevel() const
    {
        return GetDeviceInteger("TestPulseHighLevel");
    }
    /**
     * setTestPulseHighLevel
     *    @param counts - Desired ADC counts fo the high level of the test pulser.
     */
    void
    VX2750Pha::setTestPulseHighLevel(std::uint32_t counts) const
    {
         SetDeviceValue("TestPulseHighLevel", static_cast<int>(counts));
    }
    /**
     * getIOLevel
     *   @return VX2750Pha::IOLevel - I/O signalling level for the TRGIN, SIN, TRGOUT
     *                                 I/Os.
     */
    VX2750Pha::IOLevel
    VX2750Pha::getIOLevel() const
    {
        return stringToEnum(stringToIOLevel, GetDeviceValue("IOLevel"));
    }
    /**
     * setIOLevel
     *   @param level - desired I/O signalling level.
     */
    void
    VX2750Pha::setIOLevel(IOLevel level) const
    {
        std::string value = enumToString(ioLevelToString, level);
        SetDeviceValue("IOLevel", value.c_str());
    }
    /**
     * getAirInTemp
     *    @return double - the temperature in C at the air intakes
     */
    double
    VX2750Pha::getAirInTemp() const
    {
        return GetDeviceReal("TempSensAirIn");
    }
    /**
     * getAirOutTemp
     *    Get the temperature of the board exhaust
     * @return double
     */
    double
    VX2750Pha::getAirOutTemp() const
    {
        return GetDeviceReal("TempSensAirOut");
    }
    /**
     * getCoreTemp
     *    Get the temperature at the FPGA (board core).
     * @return double
     */
    double
    VX2750Pha::getCoreTemp() const
    {
        return GetDeviceReal("TempSensCore");
    }
    /**
     * getFirstADCTemp
     *   @return double the temp in C at the first ADC sensor.
     */
    double
    VX2750Pha::getFirstADCTemp() const
    {
        return GetDeviceReal("TempSensFirstADC");
    }
    /**
     * getLastADCTemp
     *  @return double the temp in C at the last ADC sensor.
     */
    double
    VX2750Pha::getLastADCTemp() const
    {
        return GetDeviceReal("TempSensLastADC");
    }
    /**
     * getHottestADCTemp
     *    @return double - The temp in C at the hottest ADC.
     */
    double
    VX2750Pha::getHottestADCTemp() const
    {
        return GetDeviceReal("TempSensHottestADC");
    }
    /**
     * getADCTemp
     *    Get the temperature at a specific adc chip temperature sensor.
     *  @param chip - ADC number [0-7].
     *  @return double - Temperature in C at that chip's sensor.
     */
    double
    VX2750Pha::getADCTemp(unsigned chip) const
    {
        checkInclusiveRange(0,7, chip);
        std::string paramName = appendNumber("TempSensADC", chip);
        
        return GetDeviceReal(paramName.c_str());
    }
    /**
     * getDCDCConverterTemp
     *    @return double - Temperature in C at the DC DC voltage converter.
     */
    double
    VX2750Pha::getDCDCConverterTemp() const
    {
        return GetDeviceReal("TempSensDCDC");
    }
    /**
     * getDCDCConverterInputVoltage
     *   @return double - Volts at the input sensor to the DC-DC converter.
     */
    double
    VX2750Pha::getDCDCConverterInputVoltage() const
    {
        return GetDeviceReal("VInSensDCDC");
    }
    /*
     * getDCDCConverterOutputVoltage
     *  @return double - volts at the output sensor of the DC-DC converter.
     */
    double
    VX2750Pha::getDCDCConverterOutputVoltage() const
    {
        return GetDeviceReal("VOutSensDCDC");
    }
    /**
     * getDCDCConverterAmps
     *     @return double - DCDC converter current in amps.
     */
    double
    VX2750Pha::getDCDCConverterAmps() const
    {
        return GetDeviceReal("IOutSensDCDC");
    }
    /**
     * getDCDCCOnverterHz
     *    @return double - frequence of the DCDC Coverter in Hz.
     */
    double
    VX2750Pha::getDCDCConverterHz() const
    {
        return GetDeviceReal("FreqSensCore");
    }
    /**
     * getDCDCConverterDutyCycle
     *   @return double - percentage duty cycle of the DCDC converter.
     */
    double
    VX2750Pha::getDCDCConverterDutyCycle() const
    {
        return GetDeviceReal("DutyCycleSensDCDC");
    
    }
    /**
     * getFanSpeed
     *    @param fan - Fan whose speed we want in the range [0,1].
     *    @return std::uint32_t - RPMs of the specified fan.
     *    @note this is only supported for the desktop form factor devices.
     */
    std::uint32_t
    VX2750Pha::getFanSpeed(unsigned fan) const
    {
        checkInclusiveRange(0,1, fan);
        std::string paramName = appendNumber("SpeedSensFan", fan+1);  // called 1/2 by CAEN.
        return GetDeviceInteger(paramName.c_str());
    }
    /**
     * getErrorFlagMask
     *    Fetches the error bits that drive the LEDS on the device front panel.
     *    @return std::uint32_t - Bit encoded see VX2750Pha::ERR_* definitions.
     */
    std::uint32_t
    VX2750Pha::getErrorFlagMask() const
    {
        return GetDeviceInteger("ErrorFlagMask");
    }
    /**
     * setErrorFlagMask
     *    Not sure why we're allowed to set this but the manual says this is
     *    a writable parameter.
     * @param mask - desired error flag mask.
     */
    void
    VX2750Pha::setErrorFlagMask(std::uint32_t mask) const
    {
        SetDeviceValue("ErrorFlagMask", static_cast<int>(mask));
    }
    /**
     * getErrorFlagDataMask
     *    @return std::uint32_t the bit encoded error mask that will be included
     *               in the data.
     */
    std::uint32_t
    VX2750Pha::getErrorFlagDataMask() const
    {
        return GetDeviceInteger("ErrorFlagDataMask");
    }
    /**
     * setErrorFlagDataMask
     *    Again this, for reasons uknown to me is somehow writable.
     * @param mask - the Desired error data mask.
     */
    void
    VX2750Pha::setErrorFlagDataMask(std::uint32_t mask) const
    {
      SetDeviceValue("ErrorFlagDataMask", static_cast<int>(mask));
    }
    /**
     * getErrorFlags
     *    @return std::uint32_t - the error flags.  These are also, I think,
     *         encoded via the VX2750Pha::ERR_ bitmasks.
     */
    std::uint32_t
    VX2750Pha::getErrorFlags() const
    {
        return GetDeviceInteger("ErrorFlags");
    }
    /**
     * isBoardReady
     *    @return bool - True if there are no error flags set else false
     */
    bool
    VX2750Pha::isBoardReady() const
    {
        return textToBool(GetDeviceValue("BoardReady"));
    }
    /**
     * getITLAMainLogic
     *     See the figure in 2.5.1 - Returns the main logic selected for individual
     *     trigger A:
     *  @return VX2750Pha::IndividualTriggerLogic - the selection
    */
    VX2750Pha::IndividualTriggerLogic
    VX2750Pha::getITLAMainLogic() const
    {
        std::string value = GetDeviceValue("ITLAMainLogic");
        return stringToEnum(stringToIndividualTriggerLogic, value);
    }
    /*v*
     * getITLBMainLogic.
     *   @return VX2750Pha::IndividualTriggerLogic - same as above but for trigger B.
     */
    VX2750Pha::IndividualTriggerLogic
    VX2750Pha::getITLBMainLogic() const
    {
        std::string value = GetDeviceValue("ITLBMainLogic");
        return stringToEnum(stringToIndividualTriggerLogic, value);
    }
    /**
     * setITLAMainLogic
     *    Select the desired condition for the A logic trigger.
     * @param selection - The desired logic selection.
     */
    void
    VX2750Pha::setITLAMainLogic(IndividualTriggerLogic selection) const
    {
        std::string value = enumToString(individualTriggerLogicToString, selection);
        SetDeviceValue("ITLAMainLogic", value.c_str());
    }
    /**
     * setITLBMainLogic
     *    Select the desired condition for the A logic trigger.
     * @param selection - The desired logic selection.
     */
    void
    VX2750Pha::setITLBMainLogic(IndividualTriggerLogic selection) const
    {
        std::string value = enumToString(individualTriggerLogicToString, selection);
        SetDeviceValue("ITLBMainLogic", value.c_str());
    }
    /**
     * getITLAMajorityLevel
     *   @return unsigned - Majority level for trigger logic A. only meaningful if
     *                      ITLB's main logic has been set to majority.
     */
    unsigned
    VX2750Pha::getITLAMajorityLevel() const
    {
        return GetDeviceInteger("ITLAMajorityLev");
    }
    /**
     * getITLBMajorityLevel
     *   @return unsigned - same as above but for trigger logic B.
     */
    unsigned
    VX2750Pha::getITLBMajorityLevel() const
    {
        return GetDeviceInteger("ITLBMajorityLev");
    }
    /**
     * setITLAMajoritylevel
     *   @param level - the number of channels that must be triggering in coincidence
     *                  to make ITLA trigger fire if it is in majority mode.
     */
    void
    VX2750Pha::setITLAMajorityLevel(unsigned level) const
    {
      SetDeviceValue("ITLAMajorityLev", static_cast<int>(level));
    }
    /**
     *  setITLBMajorityLevel
     *    @param level - same as above but for ITLB
     */
    void
    VX2750Pha::setITLBMajorityLevel(unsigned level) const
    {
      SetDeviceValue("ITLBMajorityLev", static_cast<int>(level));
    }
    /**
     * getITLAPairLogic
     *    Returns how channel pairs are combined and fed into the trigger logic
     *    blocks.
     * @return PairTriggerLogic - see 2.5.3 of the PHA Parameter manual for
     *        a full description of this part of the trigger logic but note that
     *        None and OR have not so subtle differences.
     */
    VX2750Pha::PairTriggerLogic
    VX2750Pha::getITLAPairLogic() const
    {
        return stringToEnum(stringToPairLogic, GetDeviceValue("ITLAPairLogic"));
    }
    /**
     * getITLBPairLogic
     *    @return PairTriggerLogic - same as above but for the B trigger block.
     */
    VX2750Pha::PairTriggerLogic
    VX2750Pha::getITLBPairLogic() const
    {
        return stringToEnum(stringToPairLogic, GetDeviceValue("ITLBPairLogic"));
    }
    /**
     * setITLAPairLogic
     *    Selects the logic fed to the ITLA trigger logic by channel self triggers
     *    for adjacent pairs.
     *  @param sel - the pair logic selection
     */
    void
    VX2750Pha::setITLAPairLogic(PairTriggerLogic sel) const
    {
        std::string value = enumToString(pairLogicToString, sel);
        SetDeviceValue("ITLAPairLogic", value.c_str());
    }
    /**
     * setITLBPairLogic
     *    Same as above but for the B trigger logic logic block.
     */
    void
    VX2750Pha::setITLBPairLogic(PairTriggerLogic sel) const
    {
        std::string value = enumToString(pairLogicToString, sel);
        SetDeviceValue("ITLBPairLogic", value.c_str());
    }
    /**
     * isITLAInverted
     *    @bool - true if the output of ITLA is inverted.
     */
    bool
    VX2750Pha::isITLAInverted() const
    {
        std::string value = GetDeviceValue("ITLAPolarity");
        if (value == "Direct") return false;
        if (value == "Inverted") return true;
        std::stringstream msg;
        msg << "VX2750Pha::isITLAInverted - unrecognized return string : ";
        msg << value;
	std::string m = msg.str();
        throw std::runtime_error(m);
    }
    bool
    VX2750Pha::isITLBInverted() const
    {
        std::string value = GetDeviceValue("ITLBPolarity");
        if (value == "Direct") return false;
        if (value == "Inverted") return true;
        std::stringstream msg;
        msg << "VX2750Pha::isITLAInverted - unrecognized return string : ";
        msg << value;
	std::string m = msg.str();
        throw std::runtime_error(m);
    }
    /**
     * setITLAInverted
     *     @param invert - true to request inverted output.
     */
    void
    VX2750Pha::setITLAInverted(bool inverted) const
    {
        std::string value = inverted? "Inverted" : "Direct";
        SetDeviceValue("ITLAPolarity", value.c_str());
    }
    /**
     * setITLBInverted
     *    @param invert true to request inverted
     */
    void
    VX2750Pha::setITLBInverted(bool inverted) const
    {
        std::string value = inverted? "Inverted" : "Direct";
        SetDeviceValue("ITLBPolarity", value.c_str());
    }
    /**
     * getITLConnect
     *    @param ch  - channel number.
     *    @return ITLConnect
     */
    VX2750Pha::ITLConnect
    VX2750Pha::getITLConnect(unsigned ch) const
    {
        return stringToEnum(stringToITLConnect, GetChanValue(ch, "ITLConnect"));
    }
    /**
     * setITLConnect
     *    @param ch - channel.
     *    @param selection - the selected connection mode.
     */
    void
    VX2750Pha::setITLConnect(unsigned ch, ITLConnect selection) const
    {
        std::string value = enumToString(ITLConnectToString, selection);
        SetChanValue(ch, "ITLConnect", value.c_str());
    }
    /**
     * getITLAMask
     *    get the mask of channels that are inputs to the ITLA logic.
     *    Use either the mask or ITLConnect but not both.
     *    @return uint64_t
     */
    std::uint64_t
    VX2750Pha::getITLAMask() const
    {
        return GetDeviceULong("ITLAMask");
    }
    /**
     * getITLBMask
     *    same as above but for the B trigger logic block.
     */
    std::uint64_t
    VX2750Pha::getITLBMask() const
    {
        return GetDeviceULong("ITLBMask");
    }
    /**
     * setITLAMask
     *    @param mask - new ITLA mask.
     */
    void
    VX2750Pha::setITLAMask(std::uint64_t mask) const
    {
        SetDeviceValue("ITLAMask", mask);
    }
    /**
     * setITLBMask
     *    same as above but for the b mask.
     */
    void
    VX2750Pha::setITLBMask(std::uint64_t mask) const
    {
        SetDeviceValue("ITLBMask", mask);
    }
    /**
     * getITLAGateWidth
     *    @return std::uint32_t - width of the A gate in 8ns steps.
     */
    std::uint32_t
    VX2750Pha::getITLAGateWidth() const
    {
        return GetDeviceInteger("ITLAGateWidth");
    }
    /**
     * getITLBGateWidth
     *    same as above but for the B gat width.
     */
    std::uint32_t
    VX2750Pha::getITLBGateWidth() const
    {
        return GetDeviceInteger("ITLBGateWidth");
    }
    /**
     * setITLAGateWidth
     *    @param width - desired gatewidth in 8ns steps.
     */
    void
    VX2750Pha::setITLAGateWidth(std::uint32_t width) const
    {
      SetDeviceValue("ITLAGateWidth", static_cast<int>(width));
    }
    /**
     * setITLBGateWidth
     *   same as above but for the b gate.
     */
    void
    VX2750Pha::setITLBGateWidth(std::uint32_t width) const
    {
      SetDeviceValue("ITLBGateWidth", static_cast<int>(width));
    }
    /**
     * getLVDSMode
     *   @param quartet - which of the quartets.
     *   @return LVDSMode - the mode that quartet is set in.
     */
    VX2750Pha::LVDSMode
    VX2750Pha::getLVDSMode(unsigned quartet) const
    {
        return stringToEnum(stringToLVDSMode, GetLVDSValue(quartet, "LVDSMode"));
    }
    /**
     * setLVDSMode
     *    @param quartet -the quartet to set.
     *    @param mode    - The LVDSMode desired for that quartet.
     */
    void
    VX2750Pha::setLVDSMode(unsigned quartet, LVDSMode mode) const
    {
        std::string value = enumToString(LVDSModeToString, mode);
        SetLVDSValue(quartet, "LVDSMode", value.c_str());
    }
    /**
     * getLVDSDirection
     * @param quartet - which group of 4 to interrogate.
     * @return LVDSDirection - Is the quartet composed of inputs or outputs.
     */
    VX2750Pha::LVDSDirection
    VX2750Pha::getLVDSDirection(unsigned quartet) const
    {
        return stringToEnum(stringToLVDSDirection, GetLVDSValue(quartet, "LVDSDirection"));
    }
    /**
     * setLVDSDirection
     *   @param quartet to set.
     *   @param direction - desired signal direction.n
     */
    void
    VX2750Pha::setLVDSDirection(unsigned quartet, LVDSDirection direction) const
    {
        std::string value = enumToString(LVDSDirectionToString, direction);
        SetLVDSValue(quartet, "LVDSDirection", value.c_str());
    }
    /**
     * getLVDSIOReg
     *    If an LVDS register is set to IORegister mode, this will
     *    return the value of the LVDS I/O states.  This will be correct if the
     *    register has been programmed for input or output.
     *    Not sure what the bits will be of the quartets not set as i/o register
     *    will be?
     * @return std::uint16_t - Bit mask of the I/O register.
     */
    std::uint16_t
    VX2750Pha::getLVDSIOReg() const
    {
        return GetDeviceInteger("LVDSIOReg");
    }
    /**
     * setLVDSIOReg
     *    Sets the value of an LVDS I/O register.  This is only meaningful if
     *    the mode is IORegister and the direction in output.  In that case,
     *    this sets the values of the LVDS pins per the mask value passed in.
     *    presumably only the bits of the quartets that have been set to
     *    to outputs matter.
     * @param mask    - Bit mask of LVDS outputs.
     */
    void
    VX2750Pha::setLVDSIOReg(std::uint16_t mask) const
    {
        SetDeviceValue("LVDSIOReg", mask);
    }
    /**
     * getLVDSTriggerMask
     *    Reads the self trigger mask that would be sent to the LVDS outputs
     *    if the mode is self trigger and the mode SelfTrigger
     * @param masknum - Mask number.
     * @return std::uint64_t
     */
    std::uint64_t
    VX2750Pha::getLVDSTriggerMask(unsigned masknum) const
    {
        // This mask number bit is handled with pure insantiy -- see the manual
        // rather than encoding it into the path it's the initial value of the
        // buffer.
        
        return GetLVDSTriggerMask(masknum);
       
    }
    /**
     * setLVDSTriggerMask
     *    @param masknum - mask number.
     *    @param mask - the trigger mask.
     */
    void
    VX2750Pha::setLVDSTriggerMask(unsigned masknum, std::uint64_t mask) const
    {
        SetLVDSTriggerMask(masknum, mask);
    
    }
    /**
     * getDACOutMode
     *    @return DACOutMode - the signal assignment to the DACOut connector.
     */
    VX2750Pha::DACOutputMode
    VX2750Pha::getDACOutMode() const
    {
        return stringToEnum(stringToDACOutMode, GetDeviceValue("DACoutMode"));
    }
    /**
     * setDACOutMode
     *    Assign the function of the DACOut connector
     *
     *    @para mode - desired mode.
     */
    void
    VX2750Pha::setDACOutMode(DACOutputMode mode) const
    {
        std::string value = enumToString(DACOutModeToString, mode);
        SetDeviceValue("DACoutMode", value.c_str());
    }
    /**
     * getDACOutValue
     *    If the output has been set to Static, this value is the value sent to
     *    the DAC for output on the DACOut connector
     * @return std::uint16_t
     */
    std::uint16_t
    VX2750Pha::getDACOutValue() const
    {
        return GetDeviceInteger("DACoutStaticLevel");
    }
    /**
     * setDACOutValue
     *    If the output has been set to Static, this sends a value to the DAC
     *    and the converted output is sent to the DACOut connector.
     *   @param value - DAC value to set.
     */
    void
    VX2750Pha::setDACOutValue(std::uint16_t value) const
    {
        SetDeviceValue("DACoutStaticLEvel", value);
    }
    /**
     * getDACChannel
     *   @return unsigned - If a channel is gated to the DAQout connector,
     *                     this fetches the channel number.\
     */
    unsigned
    VX2750Pha::getDACChannel() const
    {
        return GetDeviceInteger("DACoutChSelect");
    }
    /**
     * setDACChannel
     *    @param chan - number of the channel to send to the DACout if the
     *                  channel mode is selected.
    */
    void
    VX2750Pha::setDACChannel(unsigned chan) const
    {
      SetDeviceValue("DACoutChSelect", static_cast<int>(chan));
    }
    /**
     * getVGAGain
     *    Return the VGA gain for a set of 16 channels.
     *  @param group - the set to retrieve for.
     *  @return double - The gain in dB
     *  @note assume that the names are:  /vga/group/par/VGAGain.
     */
    double
    VX2750Pha::getVGAGain(unsigned group) const
    {
        std::stringstream strParName;
        strParName << "/vga/" << group << "/par/VGAGain";
        std::string parname = strParName.str();
        
        return GetReal(parname.c_str());
    }
    /**
     * setVGAGain
     *   Set the gain common to a gropu of 16 channels.
     * @param group - the group to set.
     * @param value - the value to set.
     * @note see above about contructing the path.
     */
    void
    VX2750Pha::setVGAGain(unsigned group, double value) const
    {
        std::stringstream strParName;
        strParName << "/vga/" << group << "/par/VGAGain";
        std::string parname = strParName.str();
        SetValue(parname.c_str(), value);
    }
    
    /**
     * isOffsetCalibrationEanbled
     *    @param chan - the cannel to check.
     *    @return bool - true if the selected channel's offset calibration
     *                    is enabled.
     */
    bool
    VX2750Pha::isOffsetCalibrationEnabled() const
    {
        return GetDeviceBool("EnOffsetCalibration");
    }
    /**
     * enableOffsetCalibration
     *    @param chan - the channel.
     *    @param enable  - True to enable, false to disable.
     */
    void
    VX2750Pha::enableOffsetCalibration(bool enable) const
    {
        SetDeviceValue("EnOffsetCalibration", enable);
    }
    /**
     * isChannelEnabled
     *    @param chan - channel to check.
     *    @return bool - True if the channel is enable.
     */
    bool
    VX2750Pha::isChannelEnabled(unsigned chan) const
    {
       return GetChanBool(chan, "ChEnable");
    }
    /**
     * enableChannel
     *   @param chan  - number of channel to enable/disable.
     *   @param enable - true to enable false to disable.
     */
    void
    VX2750Pha::enableChannel(unsigned chan, bool enable) const
    {
        SetChanValue(chan, "ChEnable", enable);
    }
    /**
     * getSelfTriggerRate
     *   @param chan - channel to check.
     *   @return int  - Channel self trigger rate in Hz
     */
    int
    VX2750Pha::getSelfTriggerRate(unsigned chan) const
    {
        return GetChanInteger(chan, "SelfTrgRate");
    }
    /**
     * getChannelStatus
     *    @param chan - the channel to query.
     *    @return std::uint32_t - a bitmask providing the current channel
     *                           status see the CHST_ definitions for symbolic
     *                           bit names.
     */
    std::uint32_t
    VX2750Pha::getChannelStatus(unsigned chan) const
    {
        return GetChanInteger(chan, "ChStatus");
    }
    /**
     * getDCOffset
     *    @param chan - the channel to fetch.
     *    @return double percentage DC offset.,
     */
    double
    VX2750Pha::getDCOffset(unsigned chan) const
    {
        return GetChanReal(chan, "DCOffset");
    }
    /**
     * setDCOffset
     *   @param chan - channel to set.
     *   @param pct  - Percentage DC Offset to set.
     */
    void
    VX2750Pha::setDCOffset(unsigned chan, double pct) const
    {
        SetChanValue(chan, "DCOffset", pct);
    }
    /**
     * getGainFactor
     *    @param chan - channel number.
     *    @return double - the channel gain value stored in internal flash.
     */
    double
    VX2750Pha::getGainFactor(unsigned chan) const
    {
        return GetChanReal(chan, "GainFactor");
    
    }
    /**
     * getTriggerThreshold
     *   @param chan - channel to fetch.
     *   @return std::uint32_t - trigger threshold of that channel.
     */
    std::uint32_t
    VX2750Pha::getTriggerThreshold(unsigned chan) const
    {
      return GetChanInteger(chan, "TriggerThr");
    }
    /**
     * setTriggerThreshold
     *   @param chan - channel to set
     *   @param threshold - the new threshold value.
     */
    void
    VX2750Pha::setTriggerThreshold(unsigned chan, std::uint32_t threshold) const
    {
      SetChanValue(chan, "TriggerThr", static_cast<int>(threshold));
    }
    /**
     * getPulsePolarity
     *   @param chan - channel to query.
     *   @return VX2750Pha::Polarity - the channel's polarity setting.
     */
    VX2750Pha::Polarity
    VX2750Pha::getPulsePolarity(unsigned chan) const
    {
        return stringToEnum(stringToPolarity, GetChanValue(chan, "PulsePolarity"));
    }
    /**
     * setPulsePolarity
     *   @param chan - channel to set.
     *   @param pol  - New channel polarity.
     */
    void
    VX2750Pha::setPulsePolarity(unsigned chan, Polarity pol) const
    {
        std::string value = enumToString(polarityToString, pol);
        SetChanValue(chan, "PulsePolarity", value.c_str());
    }
    /**
     * getEnergySkimLowDiscriminator
     *   @param chan - channel to fetch.
     *   @return std::uint16_t
     */
    std::uint16_t
    VX2750Pha::getEnergySkimLowDiscriminator(unsigned chan) const
    {
        return GetChanInteger(chan, "EnergySkimLowDiscriminator");
    }
    /**
     * setEnergySkimLowDiscriminator
     *   @parm chan    - channel to set
     *   @param value  - New low threshold for energy selection.
     */
    void
    VX2750Pha::setEnergySkimLowDiscriminator(unsigned chan, std::uint16_t value) const
    {
        SetChanValue(chan, "EnergySkimLowDiscriminator", value);
    }
    /**
     * getEnerygSkimHighDiscriminator
     *    @param chan -channel to get
     *    @return std::uint16_t - Energy high cutoff value.
     */
    std::uint16_t
    VX2750Pha::getEnergySkimHighDiscriminator(unsigned chan) const
    {
        return GetChanInteger(chan, "EnergySkimHighDiscriminator");
    }
    /**
     * setEnergySkimHighDiscriminator
     *    @param chan -  channel to set.
     *    @param value - new high cutoff.
     */
    void
    VX2750Pha::setEnergySkimHighDiscriminator(unsigned chan, std::uint16_t value) const
    {
        SetChanValue(chan, "EnergySkimHighDiscriminator", value);
    }
    /**
     * getEventSelector
     *    @param chan - channel number.
     *    @return EventSelection - describes which events are actually saved by the channel.
     */
    VX2750Pha::EventSelection
    VX2750Pha::getEventSelector(unsigned chan) const
    {
        return stringToEnum(stringToEventSelection, GetChanValue(chan, "EventSelector"));
    }
    /**
     * setEventSelector
     *    @param chan - channel number
     *    @param sel  - Selection of events to save.
     */
    void
    VX2750Pha::setEventSelector(unsigned chan, EventSelection sel) const
    {
        std::string value = enumToString(eventSelectionToString, sel);
        SetChanValue(chan, "EventSelector", value.c_str());
    }
    /**
     * getWaveSelector
     *    @paran chan
     *    @return EventSelection - filtr on which waveforms are saved.
     */
    VX2750Pha::EventSelection
    VX2750Pha::getWaveformSelector(unsigned chan) const
    {
        return stringToEnum(stringToEventSelection, GetChanValue(chan, "WaveSelector"));
    }
    /**
     * setWaveSelector
     *    Select which waveform are recorded.
     *  @param chan - channel number
     *  @param sel  - Event selector.
     */
    void
    VX2750Pha::setWaveformSelector(unsigned chan, EventSelection sel) const
    {
        std::string value = enumToString(eventSelectionToString, sel);
        SetChanValue(chan, "WaveSelector", value.c_str());
    }
    /**
     * getCoincidenceMask
     *   @param chan - channel whose selection we'll fetch.
     */
    VX2750Pha::CoincidenceMask
    VX2750Pha::getCoincidenceMask(unsigned chan) const
    {
        return stringToEnum(stringToCoincidenceMask, GetChanValue(chan, "CoincidenceMask"));
    }
    /**
     * setCoincidenceMask
     *    @param chan -  channel to set.
     *    @param sel  - Desired mask.
     */
    void
    VX2750Pha::setCoincidenceMask(unsigned chan, CoincidenceMask sel) const
    {
        std::string value = enumToString(coincidenceMaskToString, sel);
        SetChanValue(chan, "CoincidenceMask", value.c_str());
    }
    /**
     * getAntiCoincidenceMask
     *    @param chan - channel to fetch from.
     *    @return CoincidenceMask - the anticoincidence triggr selection.
     */
    VX2750Pha::CoincidenceMask
    VX2750Pha::getAntiCoincidenceMask(unsigned chan) const
    {
        return stringToEnum(stringToCoincidenceMask, GetChanValue(chan, "AntiCoincidenceMask"));
    }
    /**
     * setAntiCoincidenceMask
     *   @param chan - channel to fetch from
     *   @param sel  - Anticoincidence trigger selection.
     */
    void
    VX2750Pha::setAntiCoincidenceMask(unsigned chan, CoincidenceMask sel) const
    {
        std::string value = enumToString(coincidenceMaskToString, sel);
        SetChanValue(chan, "AntiCoincidenceMask", value.c_str());
    }
    /**
     * getCoincidenceSamples
     *    @param chan - channel to fetch from.
     *    @return std::uint32_t - number of samples in the coincidence trigger window.
     */
    std::uint32_t
    VX2750Pha::getCoincidenceSamples(unsigned chan) const
    {
        return GetChanInteger(chan, "CoincidenceLengthS");
    }
    /**
     * setCoincidenceSamples
     *    @param chan - channel to set.
     *    @param samples -# samples to be used for that channel's coincidence window.
     */
    void
    VX2750Pha::setCoincidenceSamples(unsigned chan, std::uint32_t samples) const
    {
      SetChanValue(chan, "CoincidenceLengthS", static_cast<int>(samples));
    }
    /**
     * setCoincidenceNs
     *    @param chan - channel to get.
     *    @return std::uint32_t - same as above but the result is in ns units./
     */
    std::uint32_t
    VX2750Pha::getCoincidenceNs(unsigned chan) const
    {
        return GetChanInteger(chan, "CoincidenceLengthT");
    }
    /**
     * setCoincidenceNs
     *   @param chan - channel number.
     *   @para ns    - COincidence window in nanoseconds.
     */
    void
    VX2750Pha::setCoincidenceNs(unsigned chan, std::uint32_t ns) const
    {
      SetChanValue(chan, "CoincidenceLengthT", static_cast<int>(ns));
    }
    /**
     * getTimeFilterRiseTime
     *   @param chan - channel to get
     *   @return std::unit32_t ns of rise time set in the time filter.
     */
    std::uint32_t
    VX2750Pha::getTimeFilterRiseTime(unsigned chan) const
    {
        return GetChanInteger(chan, "TimeFilterRiseTimeT");
    }
    /**
     * getTimeFilterRiseSamples
     *    @param chan - channel to get.
     *    @return std::uint32_t - samples in the time filte rise time.
     */
    std::uint32_t
    VX2750Pha::getTimeFilterRiseSamples(unsigned chan) const
    {
        return GetChanInteger(chan, "TimeFilterRiseTimeS");
    }
    /**
     * setTimeFilterRiseTime
     *    Set the time filter rise-time in ns
     *  @param chan - channel number to set.
     *  @param ns   - rise time requested.
     */
    void
    VX2750Pha::setTimeFilterRiseTime(unsigned chan, std::uint32_t ns) const
    {
      SetChanValue(chan, "TimeFilterRiseTimeT", static_cast<int>(ns));
    }
    /**
     * setTimeFilterRiseSamples
     *   @param chan - channel number to set.
     *   @param samples - samples in the time filter rise-time to set.
     */
    void
    VX2750Pha::setTimeFilterRiseSamples(unsigned chan, std::uint32_t samples) const
    {
      SetChanValue(chan, "TimeFilterRiseTimeS", static_cast<int>(samples));
    }
    /**
     * getTimeFilterRetriggerGaurdTime
     *   @param chan
     *   @return std::uint32_t - ns in the time filter retrigger guard.
     */
    std::uint32_t
    VX2750Pha::getTimeFilterRetriggerGuardTime(unsigned chan) const
    {
        return GetChanInteger(chan, "TimeFilterRetriggerGuardT");
    }
    /**
     * getTimeFilterRetriggerGuardSamples
     *   @param chan
     *   @return std::uint32_t -same as above but the units are digitizer samples.
     */
    std::uint32_t
    VX2750Pha::getTimeFilterRetriggerGuardSamples(unsigned chan) const
    {
        return GetChanInteger(chan, "TimeFilterRetriggerGuardS");
    }
    /**
     * setTimeFilterRetriggerGuardTime
     *    @param chan
     *    @param ns  - ns to set the retrigger guard time to for chan.
     */
    void
    VX2750Pha::setTimeFilterRetriggerGuardTime(unsigned chan, std::uint32_t ns) const
    {
      SetChanValue(chan, "TimeFilterRetriggerGuardT", static_cast<int>(ns));
    }
    /**
     * setTimeFilterRetriggerGuardSamples
     *    Same as above except the guard window is expressed in samples
     */
    void
    VX2750Pha::setTimeFilterRetriggerGuardSamples(unsigned chan, std::uint32_t samples) const
    {
      SetChanValue(chan, "TimeFilterRetriggerGuardS", static_cast<int>(samples));
    }
    /**
     * getEnergyFilterRiseTime
     *    @param chan - channel to query.
     *    @return std::uint32_t - energy filter rise time expressed in ns.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterRiseTime(unsigned chan) const
    {
        return GetChanInteger(chan, "EnergyFilterRiseTimeT");
    }
    /**
     * getEnergyFilterRiseSamples
     *    Same as above but the result is expressed in digitizer samples.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterRiseSamples(unsigned chan) const
    {
        return GetChanInteger(chan, "EnergyFilterRiseTimeS");
    }
    /**
     * setEnergyFilterRisetime
     *    @param chan - channel to set.
     *    @param ns   - Nanoseconds to set the energy filter rise time.
     */
    void
    VX2750Pha::setEnergyFilterRiseTime(unsigned chan, std::uint32_t ns) const
    {
      SetChanValue(chan, "EnergyFilterRiseTimeT", static_cast<int>(ns));
    }
    /**
     * setEnergyFilterRiseSamples
     *    Same as above but units are samples.
     */
    void
    VX2750Pha::setEnergyFilterRiseSamples(unsigned chan, std::uint32_t samples) const
    {
      SetChanValue(chan, "EnergyFilterRiseTimeS", static_cast<int>(samples));
    }
    /**
     * getEnergyFilterFlatTopTime
     *    @param ch - channel
     *    @return uint32_t   - ns of flattop time.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterFlatTopTime(unsigned ch) const
    {
        return GetChanInteger(ch, "EnergyFilterFlatTopT");
    }
    /**
     * getEnergyFilterFlatTopSamples
     *    @param ch -channel
     *    @return uint32_t - number of samples in the flattop region.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterFlatTopSamples(unsigned ch) const
    {
        return GetChanInteger(ch, "EnergyFilterFlatTopS");
    }
    /**
     * setEnergyFilterFlatTopTime
     *    @param ch - channel
     *    @param ns  - nanoseconds of flattop time.
     */
    void
    VX2750Pha::setEnergyFilterFlatTopTime(unsigned ch, std::uint32_t ns) const
    {
      SetChanValue(ch, "EnergyFilterFlatTopT", static_cast<int>(ns));
    }
    /**
     *  setEnergyFilterFlatTopSamples
     *     @param ch - channel
     *     @param samples - samples in the flattop region.
     */
    void
    VX2750Pha::setEnergyFilterFlatTopSamples(unsigned ch, std::uint32_t samples) const
    {
      SetChanValue(ch, "EnergyFilterFlatTopS", static_cast<int>(samples));
    }
    /**
     * getEnergyFilterPeakingPosition
     *    @param ch - channel number.
     *    @return std::uint32_t - The trapezoid Peaking Position in percentage (%) of the flat top.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterPeakingPosition(unsigned chan) const
    {
        return GetChanInteger(chan, "EnergyFilterPeakingPosition");
    }
    /**
     * setEnergyFilterPeakingPosition.
     *   @param ch - channel number.
     *   @param pct - The trapezoid Peaking Position in percentage (%) of the flat top.
     */
    void
    VX2750Pha::setEnergyFilterPeakingPosition(unsigned ch, std::uint32_t pct) const
    {
      SetChanValue(ch, "EnergyFilterPeakingPosition", static_cast<int>(pct));
    }
    /**
     * getEnergyFilterPeakingAverage
     *    @param ch - channel
     *    @return VX2750Pha::EnergyPeakingAverage - Enum that describes the number
     *                      of samples averaged in peak evaluation.
     */
    VX2750Pha::EnergyPeakingAverage
    VX2750Pha::getEnergyFilterPeakingAverage(unsigned ch) const
    {
        return stringToEnum(stringToPeakingAverage, GetChanValue(ch, "EnergyFilterPeakingAvg"));
    }
    /**
     * setEnerygFilterPeakingAverage
     *    @param ch - channel
     *    @param selection - VX2750Pha::EnergyPeakingAverage that selects the
     *                        samples over which to average.
     */
    void
    VX2750Pha::setEnergyFilterPeakingAverage(unsigned ch, EnergyPeakingAverage selection) const
    {
        std::string value = enumToString(peakingAverageToString, selection);
        SetChanValue(ch, "EnergyFilterPeakingAvg", value.c_str());
    }
    /**
     * getEnergyFilterPoleZeroTime
     *    @param ch - channel number.
     *    @return std::uint32_t pole zero compensation time in ns.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterPoleZeroTime(unsigned ch) const
    {
        return GetChanInteger(ch, "EnergyFilterPoleZeroT");
    }
    /**
     * getEnergyFilterPoleZeroSamples
     *   @param ch
     *   @return std::uint32_t - same as above but in units of ADC samples
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterPoleZeroSamples(unsigned ch) const
    {
        return GetChanInteger(ch, "EnergyFilterPoleZeroS");
    }
    /**
     * setEnergyFilterPoleZeroTime
     *    @param ch - channel number.
     *    @param ns - nanoseconds for pole zero compenation time
     */
    void
    VX2750Pha::setEnergyFilterPoleZeroTime(unsigned chan, std::uint32_t ns) const
    {
      SetChanValue(chan, "EnergyFilterPoleZeroT", static_cast<int>(ns));
    }
    /**
     * setEnergyFilterPoleZeroSamples
     *   Same as above but units of measure are samples:
     */
    void
    VX2750Pha::setEnergyFilterPoleZeroSamples(unsigned ch, std::uint32_t samples) const
    {
      SetChanValue(ch, "EnergyFilterPoleZeroS", static_cast<int>(samples));
    }
    /**
     * getEnergyFilterFineGain
     *    @param chan - channel number.
     *    @return double -Digital fine gain in the energy filter.
     */
    double
    VX2750Pha::getEnergyFilterFineGain(unsigned ch) const
    {
        return GetChanReal(ch, "EnergyFilterFineGain");
    }
    /**
     * setEnergyFilterFineGain
     *    @param chan - channel number.
     *    @param gain  - fine gain value.
     */
    void
    VX2750Pha::setEnergyFilterFineGain(unsigned ch, double value) const
    {
      SetChanValue(ch, "EnergyFilterFineGain", static_cast<int>(value));
    }
    /**
     * isEnergyFilterFLimitationEnabled
     *    @param ch  - channel number.
     *    @return bool - true if the low frequence filter is enabled for the chan.
     */
    bool
    VX2750Pha::isEnergyFilterFLimitationEnabled(unsigned chan) const
    {
        std::string value = GetChanValue(chan, "EnergyFilterLFLimitation");
        value = toUpper(value);
        return value == "ON" ? true : false;    
    }
    /**
     * enableEnergyFilterFLimitation
     *    @param ch -channel.
     *    @param enable - true to enable.
     */
    void
    VX2750Pha::enableEnergyFilterFLimitation(unsigned chan, bool enable) const
    {
        SetChanValue(chan, "EnergyFilterLFLimitation", enable ? "On" : "Off");
    }
    /** getEnergyFilterBaselineAverage
     *      Returns an enum value that indicates the number of samples that are averaged
     *      to compute the running baseline.
     *   @param ch - channel
     *   @return VX2750Pha::EnergyFilterBaselineAverage
     */
    VX2750Pha::EnergyFilterBaselineAverage
    VX2750Pha::getEnergyFilterBaselineAverage(unsigned ch) const
    {
        return stringToEnum(
            stringToBLAverage, GetChanValue(ch, "EnergyFilterBaselineAvg")
        );
    }
    /**
     * setEnergyFilterBaselineAverage
     *    @param ch - channel
     *    @param sel - selection for baseline averaging.
     */
    void
    VX2750Pha::setEnergyFilterBaselineAverage(
        unsigned chan, EnergyFilterBaselineAverage sel
    ) const
    {
        std::string value = enumToString(BLAverageToString, sel);
        SetChanValue(chan, "EnergyFilterBaselineAvg", value.c_str());
    }
    /**
     * getEnergyFilterBaselineGuardTime
     *    @param ch - channel number.
     *    @return std::uint32_t ns baseline is frozen after the trapezoid  peak.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterBaselineGuardTime(unsigned ch) const
    {
        return GetChanInteger(ch, "EnergyFilterBaselineGuardT");
    }
    /**
     * getEnergyFilterBaselineGuardSamples
     *    Same as above but in units of samples not ns.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterBaselineGuardSamples(unsigned ch) const
    {
        return GetChanInteger(ch, "EnergyFilterBaselineGuardS");
    }
    /**
     * setEnergyFilterBaselineGuardTime
     *    @param ch - channel
     *    @param ns - ns of baseline freeze after peak.
    */
    void
    VX2750Pha::setEnergyFilterBaselineGuardTime(unsigned ch, std::uint32_t ns) const
    {
      SetChanValue(ch, "EnergyFilterBaselineGuardT", static_cast<int>(ns));
    }
    /**
     * setEnergyFilterBaselineGuardSamples
     *   Same as above but in units of samples
     *   @param ch - channel
     *   @param samples - samples in the guard time.
     */
    void
    VX2750Pha::setEnergyFilterBaselineGuardSamples(unsigned ch, std::uint32_t samples) const
    {
      SetChanValue(ch, "EnergyFilterBaselineGuardS", static_cast<int>(samples));
    }
    /**
     * getEnergyFilterPileupGuardTime
     *    @param ch - channel
     *    @return std::uint32_t ns in the pileup guard time.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterPileupGuardTime(unsigned ch) const
    {
        return GetChanInteger(ch, "EnergyFilterPileUpGuardT");
    }
    /**
     * getEnergyFilterPileupGuardSamples
     *    Same as above but in units of samples.
     */
    std::uint32_t
    VX2750Pha::getEnergyFilterPileupGuardSamples(unsigned ch) const
    {
        return GetChanInteger(ch, "EnergyFilterPileupGuardS");
    }
    /**
     * setEnergyFilterPileupGaurdTime
     *   @param chan
     *   @param ns - nanoseconds of desired pileup guard time.
     */
    void
    VX2750Pha::setEnergyFilterPileupGuardTime(unsigned chan, std::uint32_t ns) const
    {
      SetChanValue(chan, "EnergyFilterPileupGuardT", static_cast<int>(ns));
    }
    /**
     * setEnergyFilterPileupGuardSamples
     *   @param chan
     *   @param samples - digitizer samples int he guard time.
     */
    void
    VX2750Pha::setEnergyFilterPileupGuardSamples(unsigned chan, std::uint32_t samples) const
    {
      SetChanValue(chan, "EnergyFilterPileupGuardS", static_cast<int>(samples));
    }
    /**
     * getEnergyBits
     *    @param  ch channel
     *    @return std::uint8_t number of bits of resolution in the energy value.
     */
    std::uint8_t
    VX2750Pha::getEnergyBits(unsigned ch) const
    {
        return GetChanInteger(ch, "Energy_Nbit");
    }
    /**
     * getRealTime
     *   Get the real time according to the FPGA in ns.
     *  @param ch - channel
     *  @return std::uint64_t - ns of realtime
     *  @note The docs say the raw value is in 8ns units.
     *        We'll turn that into ns.  The docs also say these units are FPGA
     *        clocks so this may be vulnerable to error in case boards come out
     *        with this API but different FPGA clock periods.
     */
    std::uint64_t
    VX2750Pha::getRealtime(unsigned ch) const
    {
        std::uint64_t raw = GetChanInteger(ch, "ChRealtimeMonitor");
        return raw * FPGA_NS_PER_CLOCK;
    }
    /**
     * getDeadtime
     *    Same as above but get the deadtime
     *    @param ch
     *    @return std::uint64_t - again converted to ns.  ALl the same caveats.
     */
    std::uint64_t
    VX2750Pha::getDeadtime(unsigned ch) const
    {
        std::uint64_t raw = GetChanInteger(ch, "ChDeadtimeMonitor");
        return raw * FPGA_NS_PER_CLOCK;
    }
    // Command jackets:
    /**
     * Reset
     *   Resets the board. This command sets all registers to the default value
     *   and clear data from memories. It does not act on
     *   the communication interfaces, the PLLs and the clocks
     */
    void
    VX2750Pha::Reset()
    {
        Command("Reset");
    }
    /**
     * ClearData
     *    Clear data from memories. Register’s content is not affected.
     *    This command is typically used before starting an
     *    acquisition to guarantee that no data belonging to a previous run is
     *    still present in the internal memory of the digitizer.
     */
    void
    VX2750Pha::Clear()
    {
        Command("ClearData");
    }
    /**
     * Arm
     *    Arms the digitizer to start an acquisition. When the start of run is
     *    software controlled, the arming is not implicit in the
     *    start command and it is necessary to use the ArmAcquisition command.
     */
    void
    VX2750Pha::Arm()
    {
        Command("ArmAcquisition");
    }
    /*
     * Disarm:
     *   Disarms the acquisition and prevents the digitizer to start a new run
     *   (for instance controlled by an external signal feeding SIN) without
     *   the grant of the software. It is possible to set an automatic disarm
     *   after the stop of the acquisition by means of the EnAutoDisarmAcq
     *   parameter. However, in order to make the higher level software
     *   aware of this occurrence and allow it to properly manage the Disarm
     *   condition, the use of the DisarmAcquistion command is mandatory even
     *   when the EnAutoDisarmAcq parameter is used.
     */
    void
    VX2750Pha::Disarm()
    {
        Command("DisarmAcquisition");
    }
    /**
     * Start
     *    Starts the acquisition, provided that the option SwStart is enabled
     *    in the parameter StartSource. This start command does require a
     *    previous arming command
     */
    void
    VX2750Pha::Start()
    {
        Command("SwStartAcquisition");    
    }
    /**
     * Stop
     *    Forces the acquisition to stop, whatever is the start source.
     */
    void
    VX2750Pha::Stop()
    {
        Command("SwStopAcquisition");
    }
    /**
     * Trigger
     *    Send a software trigger to the digitizer, provided that the option
     *    SwTrg is enabled in the GlobalTriggerSource parameter
     */
    void
    VX2750Pha::Trigger()
    {
        Command("SendSWTrigger");
    }
    /**
     * ReloadCalibration
     *    Delete calibration file and reload it from flash or, if invalid, use
     *    default calibration
     */
    void
    VX2750Pha::ReloadCalibration()
    {
        Command("ReloadCalibration");
    }
    /**
     * getEndpoint
     *   @return VX2750Pha::Endpoint - the active endpoint.
     */
    VX2750Pha::Endpoint
    VX2750Pha::getEndpoint()
    {
        return stringToEnum(stringToEndpoint, GetActiveEndpoint());
    }
    /**
     * selectEndpoint
     *   @param selection - desired active endpoint.
     */
    void
    VX2750Pha::selectEndpoint(Endpoint selection)
    {
        std::string value = enumToString(endpointToString, selection);
        SetActiveEndpoint(value.c_str());
    }
    /**
     * initializeRawEndPoint
     *   Create the JSON required to initialize the format of the raw endpoint
     *   to default.
     */
    void
    VX2750Pha::initializeRawEndpoint()
    {
        Json::Value d = createScalar("SIZE", "SIZE_T");
        Json::Value wf = createArray("DATA", "U8", 1);
        Json::Value description;
        description[0] = d;
        description[1] = wf;
        std::stringstream strJson;
        strJson << description;
        std::string json = strJson.str();
        SetReadDataFormat(json.c_str());
        
    }
    /**
     * readRawEndpoint
     *   Performa  read on the default endpoint.. arguments are in order:
     *   - size
     *   - data.
     * @param pBuffer - pointer to a buffer which must be sized using
     *       getMaxRawDataSize.
     * @return size_t - size of data read (bytes?).
     * @note initializeRawEndPoint shoulid have been called after
     *       selecting the raw endpoint.
     */
    size_t
    VX2750Pha::readRawEndpoint(void* pBuffer)
    {
        size_t s;
        void* argv[2];
        argv[0] = &s;
        argv[1] = pBuffer;
        bool status = ReadData(1000000, 2, argv);
        if (status) {
            return s;
        } else {
            return 0;                  // TImeout.
        }
    }
    /**
     * setDefaultFormat
     *     Applies to DPP-PHA data - resets the internal data structures
     *     so that if initializeDPPPHAReadout the default readout format
     *     will be used.
     */
    void
    VX2750Pha::setDefaultFormat()
    {
        m_dppPhaOptions.resetOptions();
    }
    /**
     * enableRawTimestamp
     *    Determines if the raw timestamp will be read from the DPP-PHA
     *    end point.
     *   @param enable - if true, raw timestamps will be read.
     *   @note After all of the desired enablexxx methods are called to
     *         specify the desired event contents, the method
     *         initializeDPPPHAReadout must be called to generate the approprate
     *         JSON and send it to the device.
     */
    void
    VX2750Pha::enableRawTimestamp(bool enable)
    {
        m_dppPhaOptions.s_enableRawTimestamps = enable;
    }
    /**
     * enableFineTimestamp
     *    As enablRawTimestamp but the CFD fine timestamp is enabled to be read
     *    out.
     * @param enable - true to enable, false to disable.
     * @note After all of the desired enablexxx methods are called to
     *         specify the desired event contents, the method
     *         initializeDPPPHAReadout must be called to generate the approprate
     *         JSON and send it to the device.
     */
    void
    VX2750Pha::enableFineTimestamp(bool enable)
    {
        m_dppPhaOptions.s_enableFineTimestamps = enable;
    }
    /**
     * enableFlags
     *    Same as for enableRawTimestamp but enables/disables the status flags.
     *    Note that there are two flags and they both get enabled or disabled
     *    together.  These are the low priority and high priority flag sets.
     * @param enable - true to enable, false to disable.
     * @note After all of the desired enablexxx methods are called to
     *         specify the desired event contents, the method
     *         initializeDPPPHAReadout must be called to generate the approprate
     *         JSON and send it to the device.
     */
    void
    VX2750Pha::enableFlags(bool enable)
    {
        m_dppPhaOptions.s_enableFlags = enable;
    }
    /**
     * enable Downsampling
     *    Turn on/off adding the clock downsampling information to the data
     *    read from the DPP_PHA endpoint:
     * @param enable - true to enable, false to disable.
     * @note After all of the desired enablexxx methods are called to
     *         specify the desired event contents, the method
     *         initializeDPPPHAReadout must be called to generate the approprate
     *         JSON and send it to the device.
     */
    void
    VX2750Pha::enableDownsampling(bool enable)
    {
        m_dppPhaOptions.s_enableDownsampledTime = enable;
    }
    /**
     * enableAnalogProbes
     *    Enable the readout of analog probes.  Note that this is also how to
     *    enable traces.. one assigns the raw adc inputs to a probe and then
     *    enables the readout of that probe.
     *    There are two analog probes that can be enabled, probe 1 and probe 2.
     *    These can be indpendently enabled:
     * @param probe1 - if true the readout of probe 1 is enabled
     * @param probe2 - if true, the readout of probe 2 is enabled.
     * @note After all of the desired enablexxx methods are called to
     *         specify the desired event contents, the method
     *         initializeDPPPHAReadout must be called to generate the approprate
     *         JSON and send it to the device.
     *  @note that enabling an analog probe also enables the type of probe to be
     *        read.
     */
    void
    VX2750Pha::enableAnalogProbes(bool probe1, bool probe2)
    {
        m_dppPhaOptions.s_enableAnalogProbe1 = probe1;
        m_dppPhaOptions.s_enableAnalogProbe2 = probe2;
    }
    /**
     * enbaleDigitalProbes
     *   Up to four digital probes can be assigned and independently enabled for readout.
     *   Note that enabling a probe also enables the corresponding probe
     *   description.
     *
     *   @oaram probe1  - enable probe 1 for readout.
     *   @oaram probe2  - enable probe 2 for readout.
     *   @oaram probe3  - enable probe 3 for readout.
     *   @oaram probe4  - enable probe 4 for readout.
     */
    void
    VX2750Pha::enableDigitalProbes(
        bool probe1, bool probe2, bool probe3, bool probe4
    )
    {
        m_dppPhaOptions.s_enableDigitalProbe1 = probe1;
        m_dppPhaOptions.s_enableDigitalProbe2 = probe2;
        m_dppPhaOptions.s_enableDigitalProbe3 = probe3;
        m_dppPhaOptions.s_enableDigitalProbe4 = probe4;
    }
    /**
     * enableSampleSize
     *    Turn on/off the readout of the number of samples in the analog probes.
     *  @param enable - true to turn on, false to turn  off.
     */
    void
    VX2750Pha::enableSampleSize(bool enable)
    {
        m_dppPhaOptions.s_enableSampleCount = enable;
    }
    /**
     * enableRawEventSize
     *    Enable the recording of the raw event size.  My understanding is that
     *    this is the same as the size field from the raw endpoint..that is it
     *    describes the total number of bytes in the raw event.
     *  @param enable - true to enable. false to disable.
     */
    void
    VX2750Pha::enableRawEventSize(bool enable)
    {
        m_dppPhaOptions.s_enableEventSize = enable;
    }
    /**
     * initializeDPPPHAReadout
     *    Initializes the DPP-PHA readout format in accordance with the default
     *    set of items read and the values of the enables for the optional
     *    items in m_dppPhaOptions.
     */
    void
    VX2750Pha::initializeDPPPHAReadout()
    {
        unsigned    index(0);
        Json::Value description;
        
        // These elements are always read out:
    
        description[index++] = createScalar("CHANNEL", "U8");
        description[index++] = createScalar("TIMESTAMP_NS", "U64");
        description[index++] = createScalar("ENERGY", "U16");
        
        //description[index++] = createScalar("BOARD_FAIL", "BOOL");
            
        // Now add in any selected optional elements to the readout format:
        
        if (m_dppPhaOptions.s_enableRawTimestamps) {
            
            description[index++] = createScalar("TIMESTAMP", "U64");
        }
        if (m_dppPhaOptions.s_enableFineTimestamps) {
            
            description[index++] = createScalar("FINE_TIMSTAMP", "U16");
        }
        if(m_dppPhaOptions.s_enableFlags) {
            
            
            description[index++] = createScalar("FLAGS_LOW_PRIORITY", "U16");
            description[index++] = createScalar("FLAGS_HIGH_PRIORITY","U8");
        }
        if (m_dppPhaOptions.s_enableDownsampledTime) {
            description[index++] = createScalar("TIME_RESOLUTION", "U8");
        }
        if (m_dppPhaOptions.s_enableAnalogProbe1) {
            description[index++] = createArray("ANALOG_PROBE_1", "I32", 1);
            description[index++] = createScalar("ANALOG_PROBE_1_TYPE", "U8");
        }
        if (m_dppPhaOptions.s_enableAnalogProbe2) {
            description[index++] = createArray("ANALOG_PROBE_2", "I32", 1);
            description[index++] = createScalar("ANALOG_PROBE_2_TYPE", "U8");
        }
        if (m_dppPhaOptions.s_enableDigitalProbe1) {
            description[index++] = createArray("DIGITAL_PROBE_1", "U8", 1);
            description[index++] = createScalar("DIGITAL_PROBE_1_TYPE", "U8");
        }
        if (m_dppPhaOptions.s_enableDigitalProbe2) {
            description[index++] = createArray("DIGITAL_PROBE_2", "U8", 1);
            description[index++] = createScalar("DIGITAL_PROBE_2_TYPE", "U8");
        }
        if (m_dppPhaOptions.s_enableDigitalProbe3) {
            description[index++] = createArray("DIGITAL_PROBE_3", "U8", 1);
            description[index++] = createScalar("DIGITAL_PROBE_3_TYPE", "U8");
        }
        if (m_dppPhaOptions.s_enableDigitalProbe4) {
            description[index++] = createArray("DIGITAL_PROBE_4", "U8", 1);
            description[index++] = createScalar("DIGITAL_PROBE_4_TYPE", "U8");
        }
        if (m_dppPhaOptions.s_enableSampleCount) {
            description[index++] = createScalar("WAVEFORM_SIZE SIZE_T", "SIZE_T");
        }
        if (m_dppPhaOptions.s_enableEventSize) {
            description[index++] = createScalar("EVENT_SIZE", "SIZE_T");
        }
        // Turn this JSON array into a string and send it off as the data format:
        
        std::stringstream strJson;
        strJson << description;
        std::string json = strJson.str();
        SetReadDataFormat(json.c_str());
        
    }
    /**
     * readDPPPHAEndPoint
     *    Read an event from the decoded event point.  Prerequisites:
     *    -  The end point must have been selected.
     *    -  The various optional bits and pieces must have been enabled.
     *    -  initializeDPPPHAReadout must have been called to set the data format.
     *  @param[out] event - The struct of values and pointers that will be used to
     *       get the data for the event.  All pointer elements for data that will
     *       be returned must have been initialized by the caller to point to
     *       appropriately sized blocks of data to receive data for that item.
     *       Appropriate queries can be used to determine the maximum 
     *       analog/digital probe samples required.  The actual number of samples
     *       received can be gotten by enabling the wave form size.
     *       Incorrectly initializing these pointers will, most likely, result
     *       in segfaults or other very bad things.
     *   @note - blocks until reads are satisified.  This means that you'd better know
     *       that data is available.
     *   @note - or more a todo:  It' possible, if the user re-uses event read after
     *        read to provide a method that will hand back the argv/argc values
     *        to them which would streamline the actual read process.
     *        The code in this method is clunky but supports run-time decision
     *        of the desired data.
     */
    void
    VX2750Pha::readDPPPHAEndpoint(DecodedEvent& event)
    {
        void* argv[DPP_MAX_PARAMS];                  // Some extra slots...but beware if DecodedEvents expands.
        int argc = 0;
        
        // Add in arguments that are always there  - this is the use case
        // I gave the developers for needing support for an argc/argv version
        // of their reads -- yes I'm rubbing that in.
        
        argv[argc++] = &(event.s_channel);
        argv[argc++] = &(event.s_nsTimestamp);
        argv[argc++] = &(event.s_energy);
        //argv[argc++] = &(event.s_fail);
        
        // The remaining arguments depend on the values of the dpp PHA options
        // struct flags:
        
        if (m_dppPhaOptions.s_enableRawTimestamps) {
            argv[argc++] = &(event.s_rawTimestamp);
        }
        if (m_dppPhaOptions.s_enableFineTimestamps) {
            argv[argc++] = &(event.s_fineTimestamp);
        }
        if (m_dppPhaOptions.s_enableFlags) {
            argv[argc++] = &(event.s_lowPriorityFlags);
            argv[argc++] = &(event.s_highPriorityFlags);
        }
        if (m_dppPhaOptions.s_enableDownsampledTime) {
            argv[argc++] = &(event.s_timeDownSampling);
        }
        if (m_dppPhaOptions.s_enableAnalogProbe1) {
            argv[argc++] = event.s_pAnalogProbe1;
            argv[argc++] = &(event.s_analogProbe1Type);
        }
        if (m_dppPhaOptions.s_enableAnalogProbe2) {
            argv[argc++] = event.s_pAnalogProbe2;
            argv[argc++] = &(event.s_analogProbe2Type);
        }
        if (m_dppPhaOptions.s_enableDigitalProbe1)  {
            argv[argc++] = event.s_pDigitalProbe1;
            argv[argc++] = &(event.s_digitalProbe1Type);
        }
        if (m_dppPhaOptions.s_enableDigitalProbe2)  {
            argv[argc++] = event.s_pDigitalProbe2;
            argv[argc++] = &(event.s_digitalProbe2Type);
        }
        if (m_dppPhaOptions.s_enableDigitalProbe3)  {
            argv[argc++] = event.s_pDigitalProbe3;
            argv[argc++] = &(event.s_digitalProbe3Type);
        }
        if (m_dppPhaOptions.s_enableDigitalProbe4)  {
            argv[argc++] = event.s_pDigitalProbe4;
            argv[argc++] = &(event.s_digitalProbe4Type);
        }
        if (m_dppPhaOptions.s_enableSampleCount) {
            argv[argc++] = &(event.s_samples);
        }
        if (m_dppPhaOptions.s_enableEventSize) {
            argv[argc++] = &(event.s_eventSize);
        }
        while(!ReadData(1000000, argc, argv))
            ;                                          // Block until read.
        
    }
    //////////////////////////////////////////////////////////////////////////
    // Decoded buffer management (DPPPHAEvent):
    
    /**
     * initDecodedBuffer
     *    Given a block of storage that is a decoded event,
     *    initializes it for the first time.  Subsequently, setup and free
     *    should be used on this block of storage.
     *  @param[out] event  - The block of storage for a decoded event.
     */
    void
    VX2750Pha::initDecodedBuffer(DecodedEvent& event)
    {
        // This is the simplest/minimalist way to ensure the
        // buffer pointers are nulled out.  Really that's all we really care
        //about:
        
        std::memset(&event, 0, sizeof(DecodedEvent));
    }
    /**
     * setupDecodedBuffer
     *    Given the objects current read options, allocates an appropriate
     *    set of buffers for analog and digital probes.  Note that:
     *    -  Be sure that the first thing you do to a DecodedEvent is invoke
     *        initDecodedBuffer.
     *    -  If this has already been done, first invoke freeDecodedBuffer
     *       free any previous values.
     *    -  If after doing this, the trace lengths and options are modified,
     *       you really should invoke freeDecodedBuffer and thenn do another
     *       setupDecoded buffer as there's no assurance the buffers in this
     *       object will be large enough to accommodate a read.
     * @param[out] event -the event to setup.
     */
    void
    VX2750Pha::setupDecodedBuffer(DecodedEvent& event)
    {
        // Regardless we figure out the length of the longest trace:
        
        int nChans = channelCount();
        std::uint32_t samples = 0;
        for (int i=0; i < nChans; i++) {
            auto n = getRecordSamples(i);
            if (n > samples) samples = n;
        }
        // samples is the number of analog samples and there's one byte per
        // digital probe sample so:
        
        std::uint32_t dProbeLen = samples;
        if (samples % sizeof(std::uint8_t) > 0) dProbeLen++;     // Partial byte.
        
        // Allocate the appropriate analog probe buffers:
        
        if (m_dppPhaOptions.s_enableAnalogProbe1) {
            event.s_pAnalogProbe1 = new std::int32_t[samples];
            if (!event.s_pAnalogProbe1) {
                throw std::runtime_error("New failed for analog probe1 buffer");
            }
        }
        if (m_dppPhaOptions.s_enableAnalogProbe2) {
            event.s_pAnalogProbe2 = new std::int32_t[samples];
            if (!event.s_pAnalogProbe2) {
                throw std::runtime_error("New failed for analog probe 2 buffer");
            }
        }
        // Now the digital probes- in this case the buffers are
        // dProbelen bytes long.
        
        if (m_dppPhaOptions.s_enableDigitalProbe1) {
            event.s_pDigitalProbe1 = new std::uint8_t[dProbeLen];
            if (!event.s_pDigitalProbe1) {
                throw std::runtime_error("New failed for digital probe 1");
            }
        }
        
        if (m_dppPhaOptions.s_enableDigitalProbe2) {
            event.s_pDigitalProbe2 = new std::uint8_t[dProbeLen];
            if (!event.s_pDigitalProbe2) {
                throw std::runtime_error("New failed for digital probe 2");
            }
        }
        
        if (m_dppPhaOptions.s_enableDigitalProbe3) {
            event.s_pDigitalProbe3 = new std::uint8_t[dProbeLen];
            if (!event.s_pDigitalProbe3) {
                throw std::runtime_error("New failed for digital probe 3");
            }
        }
        
        if (m_dppPhaOptions.s_enableDigitalProbe4) {
            event.s_pDigitalProbe4 = new std::uint8_t[dProbeLen];
            if (!event.s_pDigitalProbe4) {
                throw std::runtime_error("New failed for digital probe 4");
            }
        }
        
        
    }
    /**
     * freeDecodedBuffer
     *     Frees any dynamically allocated bits of a DecodedEvent.
     *     If the storage has been properly managed with init and setup,
     *     we can just delete the dynamic bits
     *     Note that the entire event storage is zeroed before return
     * @param event - the buffer to free:
     *
     */
    void
    VX2750Pha::freeDecodedBuffer(DecodedEvent& event)
    {
        delete []event.s_pAnalogProbe1;
        delete []event.s_pAnalogProbe2;
        delete []event.s_pDigitalProbe1;
        delete []event.s_pDigitalProbe2;
        delete []event.s_pDigitalProbe3;
        delete []event.s_pDigitalProbe4;
        
        initDecodedBuffer(event);
    }
    ////////////////////////////////////////////////////////////////////////////
    // Implementation of private (utility) functions.
    
    /**
     * dottedToInt
     *    Convert a dotted ip address to an integer.
     *  @param dotted - string containing the dotted IP address.
     *  @retrun int - integer equivalent IP address.
     *  @note  we assume the input string is a valid dotted address.
     */
    uint32_t
    VX2750Pha::dottedToInt(const std::string& dotted) const
    {
        int result = 0;
        // there are three dots.
        
        size_t startAt(0);
        std::string octets[4];    // whichi gives four fields
        for (int i =0; i < 3; i++) {
            size_t dotpos = dotted.find(".", startAt);
            octets[i] = dotted.substr(startAt, dotpos - startAt);
            startAt = dotpos+1;    // Next octet startpoint.
        }
        // startAt is pointing to the remainder of the string (octets[3])
        
        octets[3] = dotted.substr(startAt);
        
        // Now figure out the final value
        //
        for (int i = 0; i < 4; i++) {
            result  = result << 8;
            result |= strtoul(octets[i].c_str(), nullptr, 0);
        }
        return result;
        
    }
    /**
     * textToBool
     *    Covert a string to a boolean value:
     *    -  "True" is true
     *    -  "False is false.
     *    Just in case, the comparison is done case-blind
     *  @param str - string to compare.
     *  @return bool - boolean value of string.
     *  @throws std::invalid_argument if str is neither.
     */
    bool
    VX2750Pha::textToBool(const std::string& str) const
    {
        // Convert to lower case:
        
        std::string lstr;
        for (int i = 0; i < str.size(); i++) {
            lstr.push_back(tolower(str[i]));
        }
        if (lstr == "true") return true;
        if (lstr == "false") return false;
        throw std::invalid_argument("VX2750Pha - invalid boolean string");
    }
    /**
     * checkInclusiveRange
     *    Check that an integer is in an inclusive range.  If not throws
     *    std::range_error
     * @param low - low limit of range.
     * @param high - High limit of range.
     * @param value  - Value to check.
     */
    void
    VX2750Pha::checkInclusiveRange(int low, int high, int value) const
    {
        if ((value < low) || (value > high)) {
            std::stringstream strMessage;
            strMessage << "The value " << value << " is not in the range ["
            << low << ',' << high << ']';
            std::string msg = strMessage.str();
            throw std::range_error(msg);
        }
    }
    /**
     * appendNumber
     *   Given a string, append a number to it. E.g. "abcd", 10 gives
     *   "abcd10"
     * @param base - base value
     * @param numbver - number to append.
     * @return std::string
     */
    std::string
    VX2750Pha::appendNumber(const char* base, unsigned value) const
    {
        // stupid slow way:
        
        std::stringstream strResult;
        strResult << base << value;
        std::string result = strResult.str();
        return result;
    }
    /**
     * createScaler
     *    Creates the JSON specification for a scalar value.
     *    This will be a keyed array with name value pairs
     *    -  name - the name of the item./
     *    -  type - The data type of the object.
     * @param  name - value for name key.
     * @param type - value for the type key.
     * @return Json::Value - JSON value encapsulating the string.
     * 
     */
    Json::Value
    VX2750Pha::createScalar(const char* name, const char* type) const
    {
        Json::Value result;
        
        result["name"] = name;
        result["type"] = type;
	
        
        return result;
    }
    /**
     * createArray
     *    Creates a format specification for an array parameter.
     *    In addition to the name, type keys for a scalar, these
     *    have a dim key which represents the dimensionality of the object.
     * @param name
     * @param type
     * @param dimension - value to put in the "dim" key.
     * @return Json::Value
     */
    Json::Value
    VX2750Pha::createArray(const char* name, const char* type, unsigned dimension) const
    {
        Json::Value result;
        
        result["name"] = name;
        result["type"] = type;
        result["dim"]  = dimension;
        
        return result;
    }
    /**
     *    toUpper
     *      @param s - input string.
     *      @return std::string - string converted to upper case.
     *      @note this is used in stringToEnum to do case blind searches.
     */
    std::string
    VX2750Pha::toUpper(const std::string& s)
    {
        std::string result;
        result.reserve(s.size());
        for (auto c : s) {
            result.push_back(std::toupper(c));
        }
        return result;
    }
    /**
     * stringListToOrList
     *    Needed to support that some triggers can have multiple Or-d sources.
     *    Takes a  vector of strings and produces a single | separated string.
     *    E.g. {"a", "b", "c"} becomes  "a | b | c"
     *  @param strings - vector of strings to convert.
     *  @return std::string - the | separated result string.
     *  @note there must be at least one string.
     */
    std::string
    VX2750Pha::stringListToOrList(const std::vector<std::string>& strings) 
    {   
        // Need to use a counted loop since otherwise we don't know the
        // first entry as clearly.
        
        std::string result;
        for (int i= 0; i < strings.size(); i++) {
            if (i != 0) {
                result += " | ";      // Or separator.
            }
            result += strings[i];            
        }
        
        return result;
    }
    /**
     * orListToSTringList
     *     Takes a string of pipe separated items and
     *     turns it into an array of those items.  It is assumed that whitespace
     *     is not significant.
     *  @param orlist - the input string
     *  @return std::vector<std::string> the elements.  Note that
     *      we make no assumption that this is not empty, however, in practice
     *      it should be non-empty.
     */
    std::vector<std::string>
    VX2750Pha::orListToStringList(std::string orlist)
    {
        // Strip the whitespace with thanks to
        // https://www.techiedelight.com/remove-whitespaces-string-cpp/

        orlist.erase(
            std::remove_if(orlist.begin(), orlist.end(), ::isspace),
            orlist.end()
        );
        // Now we can split on the "|" characters:
        // For that we steal:
        // https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
        std::vector<std::string> result;
        std::string delim = "|";

        auto start = 0U;
        auto end = orlist.find(delim);
        while (end != std::string::npos)
        {
            result.push_back(orlist.substr(start, end-start));
            start = end + 1;
            end = orlist.find(delim, start);
        }
    
        result.push_back(orlist.substr(start));
        
        return result;
        
    }
    
    
}                 // namespace
