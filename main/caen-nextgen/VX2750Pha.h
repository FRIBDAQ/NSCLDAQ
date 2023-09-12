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
* @file    VX2750Pha.h
* @brief    Class definition for PHA firmware in the CAEN VX2750
* @author   Ron Fox
*
*/
#ifndef VX2750PHA_H
#define VX2750PHA_H
#include "Dig2Device.h"
#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <json/json.h>

namespace caen_nscldaq {
/**
 * VX2750PHA - support for parameters in the VX2750.
 *             Since this derives from the Dig2Device class low level
 *             parameter access is also provided.
 */
class VX2750Pha : public Dig2Device
{
public:
    typedef enum _FwType {
        DPP_PHA, DPP_ZLE, DPP_PSD, DPP_DAW, DPP_OPEN, Scope
    } FwType;
    typedef enum _FormFactor {
        VME, VME64X, DT
    } FormFactor;
    
    typedef enum _ClockSource {
        Internal, FrontPanel, Clock_P0, Link, DIPSelected
    } ClockSource;
    
    typedef enum _StartSource {
        Start_EncodedClockIn, SINLevel, SINEdge, SWCommand, Start_LVDS, Start_P0
    } StartSource;
    
    typedef enum _GlobalTriggerSource {
        GlobalTrigger_TriggerIn, GlobalTrigger_P0,
        GlobalTrigger_Software, GlobalTrigger_LVDS, GlobalTrigger_InternalA,
        GlobalTrigger_InternalB, GlobalTrigger_InternalAandB,
        GlobalTrigger_InternalAorB, GlobalTrigger_EncodedClockIn, GlobalTrigger_GPIO,
        GlobalTrigger_TestPulse
    } GlobalTriggerSource;
    
    typedef enum _WaveTriggerSource {
        WaveTrigger_InternalA, WaveTrigger_InternalB,
        WaveTrigger_GlobalTriggerSource, WaveTrigger_TRGIN, ExternalInhibit,
        ADCUnderSaturation, ADCOverSaturation,
        WaveTrigger_Software, WaveTrigger_ChannelSelfTrigger,
        WaveTrigger_AnyChannelSelfTrigger, WaveTrigger_Disabled
    } WaveTriggerSource;
    
    typedef enum _EventTriggerSource {
        EventTrigger_InternalA, EventTrigger_InternalB,
        EventTrigger_GlobalTriggerSource, EventTrigger_TRGIN,
        EventTrigger_Software, EventTrigger_ChannelSelfTrigger,
        EventTrigger_AnyChannelSelfTrigger, EventTrigger_Disabled
    } EventTriggerSource;
    
    
    
    typedef enum _TraceRecordMode {
        Always, OnRequest
    } TraceRecordMode;
    
    typedef enum _TRGOUTMode {
        TriggerOut_TRGIN, TriggerOut_P0,
        TriggerOut_Software, TriggerOut_LVDS, TriggerOut_InternalA,
        TriggerOut_InternalB, TriggerOut_InternalAandInternalB,
        TriggerOut_InternalAorInternalB, TriggerOut_EncodedClockIn,
        TriggerOut_Run, TriggerOut_ReferenceClock, TriggerOut_TestPulse,
        TriggerOut_Busy, TriggerOut_Zero,
        TriggerOut_One, TriggerOut_SynchIn, TriggerOut_SIN, TriggerOut_GPIO,
        AcceptedTrigger, TriggerClock
    } TRGOUTMode;
    
    typedef enum _GPIOMode {
        GPIOMode_Disabled, GPIOMode_TriggerIn, GPIOMode_P0, GPIOMode_SIN, GPIOMode_LVDS, GPIOMode_InternalA,
        GPIOMode_InternalB, GPIOMode_InternalAandInternalB,
        GPIOMode_InternalAorInternalB, GPIOMode_EncodedClockIn,
        GPIOMode_SoftwareTrigger, GPIOMode_Run, GPIOMode_ReferenceClock,
        GPIOMode_TestPulse, GPIOMode_Busy, GPIOMode_Zero, GPIOMode_One
    } GPIOMode;
    
    typedef enum _BusyInSource {
        BusyIn_SIN, BusyIn_GPIO, BusyIn_LVDS, BusyIn_Disabled 
    } BusyInSource;
    
    typedef enum _SyncOutMode {
        SyncOut_Disabled, SyncOut_SynchIn, SyncOut_TestPulse, InternalClock, SyncOut_Run
    } SyncOutMode;
    
    typedef enum _VetoSource {
        Veto_SIN, Veto_LVDS, Veto_GPIO, Veto_P0, Veto_EncodedClock, Veto_Disabled
    } VetoSource;
    
    
    typedef enum _VetoPolarity {
        ActiveHigh, ActiveLow
    } VetoPolarity;
    
    typedef enum _ChannelVetoSource {
        BoardVeto, OverSaturation, UnderSaturation, ChanVeto_Disabled
    } ChannelVetoSource;
    
    typedef enum _PauseTimestampHandling {
        Hold, PauseTstamp_Run
    } PauseTimestampHandling;
    
    // Led bit masks:
    
    static const std::uint32_t LEDFP_JESD_Y_PASS = 1;
    static const std::uint32_t LEDFP_JESD_H_PASS = 2;
    static const std::uint32_t LEDFP_DDR4_0_PASS = 4;
    static const std::uint32_t LEDFP_DDR4_1_PASS = 8;
    static const std::uint32_t LEDFP_DDR4_2_PASS = 0x10;
    static const std::uint32_t LEDFP_FAIL      = 0x20;
    static const std::uint32_t LEDFP_NIM       = 0x40;
    static const std::uint32_t LEDFP_TTL       = 0x80;
    static const std::uint32_t LEDFP_DTLOSS    = 0x100;
    static const std::uint32_t LEDFP_DTRDY     = 0x200;
    static const std::uint32_t LEDFP_TRG       = 0x400;
    static const std::uint32_t LEDFP_RUN       = 0x800;
    static const std::uint32_t LEDFP_PLL_LOCK  = 0x1000;
    static const std::uint32_t LEDFP_CLKOUT    = 0x2000;
    static const std::uint32_t LEDFP_CLKIN     = 0x4000;
    static const std::uint32_t LEDFP_USB       = 0x8000;
    static const std::uint32_t LEDFP_SFP_SD    = 0x10000;
    static const std::uint32_t LEDFP_SFP_ACT   = 0x20000;
    static const std::uint32_t LEDFP_ACT       = 0x40000;
    
    // Acquisition status bit mask:
    
    static const std::uint32_t ACQ_ARMED       = 1;
    static const std::uint32_t ACQ_RUN         = 2;
    static const std::uint32_t ACQ_RUN_MW      = 4;
    static const std::uint32_t ACQ_JESD_CLK_VALID = 8;
    static const std::uint32_t ACQ_BUSY        = 0x10;
    static const std::uint32_t ACQ_PRETRIGGER_READY = 0x20;
    static const std::uint32_t ACQ_LICENSE_FAIL = 0x40;
    
    // Waveform sources:
    
    typedef enum _WaveDataSource {
        ADC_DATA, ADC_TEST_TOGGLE, ADC_TEST_RAMP, ADC_SIN, WaveSource_IPE,
        WaveSource_Ramp, SquareWave,
        ADC_TEST_PRBS
    } WaveDataSource;
    
    typedef enum _WaveResolution {
        Res8, Res16, Res32, Res64
    } WaveResolution;
    
    typedef enum _AnalogProbe {
        ADCInput, TimeFilter, EnergyFilter, EnergyFilterBaseline,
        EnergyFilterMinusBaseline
    } AnalogProbe;
    
    typedef enum _DigitalProbe {
        DProbe_Trigger, TimeFilterArmed, ReTriggerGuard, EneryFilterBaselineFreeze,
        EnergyFilterPeaking, EnergyFilterPeakReady, EnergyFilterPileupGuard,
        EventPileup, ADCSaturation, ADCSaturationProtection, PostSaturationEvent,
        EnergyFilterSaturation, AcquisitionInhibit
    } DigitalProbe;
    
    typedef enum _IOLevel {
        TTL, NIM
    } IOLevel;
    
    // Error bits in the ErrorFlag and ErrorFlagData masks:
    
    static const std::uint32_t ERR_power_fail              = 1;
    static const std::uint32_t ERR_board_init_fault        = 2;
    static const std::uint32_t ERR_si5341_unlock           = 4;
    static const std::uint32_t ERR_si5395_unlock           = 8;
    static const std::uint32_t ERR_LMK04832_unlock         = 0x10;
    static const std::uint32_t ERR_jesd_unlock             = 0x20;
    static const std::uint32_t ERR_ddr_pl_bank0_calib_fail = 0x40;
    static const std::uint32_t ERR_ddr_pl_bank1_calib_fail = 0x80;
    static const std::uint32_t ERR_ddr_ps_calib_fail       = 0x100;
    static const std::uint32_t ERR_fpga_config_fail        = 0x200;
    static const std::uint32_t ERR_bic_error               = 0x400;
    static const std::uint32_t ERR_adc_overtemp            = 0x800;
    static const std::uint32_t ERR_air_overtemp            = 0x1000;
    static const std::uint32_t ERR_fpga_overtemp           = 0x2000;
    static const std::uint32_t ERR_dcdc_overtemp           = 0x4000;
    static const std::uint32_t ERR_clkin_miss              = 0x8000;
    static const std::uint32_t ERR_adc_shutdown            = 0x10000;
    
    // individual Trigger generation values:
    
    typedef enum _IndividualTriggerLogic {
        ITL_OR, ITL_AND, Majority
    } IndividualTriggerLogic;
    
    typedef enum _PairTriggerLogic {
        PTL_AND, PTL_OR, NONE
    } PairTriggerLogic;
    
    typedef enum _ITLConnect {
        ITL_Disabled, ITL_ITLA, ITL_ITLB
    } ITLConnect;
    
    typedef enum _LVDSMode {
        SelfTriggers, Sync, IORegister
    } LVDSMode;
    typedef enum _LVDSDirection {
        Input, Output
    } LVDSDirection;
    
    typedef enum _DACOutputMode {
        Static, DACOut_IPE, DACOut_ChInput, MemOccupancy, ChSum, OverThrSum,
        DACOut_Ramp, Sine5MHz, Square
    } DACOutputMode;
    
    static const std::uint32_t CHST_DelayInitdone = 1;
    static const std::uint32_t CHST_TFInitDone    = 2;
    static const std::uint32_t CHST_EFInitDone    = 4;
    static const std::uint32_t CHST_FullInitDone  = 8;
    static const std::uint32_t CHST_Unused1       = 0x10;
    static const std::uint32_t CHST_AcqEnabled    = 0x20;
    static const std::uint32_t CHST_InnerActive   = 0x40;
    static const std::uint32_t CHST_CanWriteTE    = 0x80;
    static const std::uint32_t CHST_CanWriteWF    = 0x100;
    
    typedef enum _Polarity {
        Positive, Negative
    } Polarity;
    
    typedef enum _EventSelection {
        All, Pileup, EnergySkim
    } EventSelection;
    
    typedef enum _CoincidenceMask {
        Coincidence_Disabled, Ch64Trigger, Coincidence_TRGIN, Coincidence_GlobalTriggerSource,
        Coincidence_ITLA, Coincidence_ITLB
    } CoincidenceMask;
    
    typedef enum _EnergyPeakingAverage {
        Average1, Average4, EPeakAvg_Average16, EPeakAvg_Average64
    } EnergyPeakingAverage;
    
    typedef enum _EnergyFilterBaselineAverage {
        Fixed, EFilterBlineAvg_Average16, EFilterBlineAvg_Average64,
        Average256, Average1024, Average4K, Average16K
    } EnergyFilterBaselineAverage;
    
    typedef enum _Endpoint {
        Raw, PHA
    } Endpoint;

    
    // Struct to hold decoded data. The user can select which fields are filled in:
    
    typedef struct _DecodedEvent {
        std::uint8_t   s_channel;               // Always present.
        std::uint64_t  s_nsTimestamp;           // Always present.
        std::uint64_t  s_rawTimestamp;          // s_enableRawTimestamps 
        std::uint16_t  s_fineTimestamp;         // s_enableFineTimestamps
        std::uint16_t  s_energy;                // Always present.
        std::uint16_t  s_lowPriorityFlags;      // s_enableFlags
        std::uint16_t  s_highPriorityFlags;     // s_enableFlags
        std::uint8_t   s_timeDownSampling;      // s_enableDownSampledTime.
        std::int32_t*  s_pAnalogProbe1;         // s_enableAnalogProbe1
        std::uint8_t   s_analogProbe1Type;      // s_enableAnalogProbe1
        std::int32_t*  s_pAnalogProbe2;         // s_enableAnalogProbe2
        std::uint8_t   s_analogProbe2Type;      // s_enableAnalogProbe2
        std::uint8_t*  s_pDigitalProbe1;        // s_enableDigitalProbe1
        std::uint8_t   s_digitalProbe1Type;     // s_enableDigitalProbe1
        std::uint8_t*  s_pDigitalProbe2;        // s_enableDigitalProbe2
        std::uint8_t   s_digitalProbe2Type;     // s_enableDigitalProbe2
        std::uint8_t*  s_pDigitalProbe3;        // s_enableDigitalProbe3
        std::uint8_t   s_digitalProbe3Type;     // s_enableDigitalProbe3
        std::uint8_t*  s_pDigitalProbe4;        // s_enableDigitalProbe4
        std::uint8_t   s_digitalProbe4Type;      // s_enableDigitalProbe4
        size_t         s_samples;               // s_enableSampleCount
        bool           s_fail;                  // Always present.
        uint32_t       s_pad;                   // Maybe bool sizes differ?
        size_t         s_eventSize;             // s_enableEventSize
        
    } DecodedEvent, *pDecodedEvent;
    
    // this struct is used to store the initial enables for optional
    // fields of the data from the DPP-PHA endpoint.
    // It's used to generate the JSON required to configure the
    // dpp-pha endpoint.
    struct EnabledItems {
        //                      channel is always on.
        bool s_enableRawTimestamps;
        //                      ns timestamp always enabled.
        bool s_enableFineTimestamps;
        //                      energy always enabled.
        bool s_enableFlags;     // Enables all flags.
        bool s_enableDownsampledTime;
        bool s_enableAnalogProbe1;
        bool s_enableAnalogProbe2;
        bool s_enableDigitalProbe1;
        bool s_enableDigitalProbe2;
        bool s_enableDigitalProbe3;
        bool s_enableDigitalProbe4;
        bool s_enableSampleCount;
        //                       fail always enabled.
        bool s_enableEventSize;
        // constructor:
        EnabledItems() {
            resetOptions();
        }
        void resetOptions() {
            s_enableRawTimestamps = false;
            s_enableFineTimestamps = false;
            s_enableFlags    = false;
            s_enableDownsampledTime = false;;
            s_enableAnalogProbe1 = false;;
            s_enableAnalogProbe2 = false;
            s_enableDigitalProbe1 = false;
            s_enableDigitalProbe2 = false;
            s_enableDigitalProbe3 = false;
            s_enableDigitalProbe4 = false;
            s_enableSampleCount   = false;
            s_enableEventSize      = false;
        }
    };
    // These are useful for string based configuration modules.
    
    static const  std::map<std::string, VX2750Pha::ClockSource> stringToClockSource;
    static const  std::map<std::string, VX2750Pha::StartSource> stringToStartSource;
    static const  std::map<std::string, VX2750Pha::GlobalTriggerSource> stringToGlobalTriggerSource;
    static const  std::map<std::string, VX2750Pha::WaveTriggerSource> stringToWaveTrigger;
    static const  std::map<std::string, VX2750Pha::EventTriggerSource> stringToEventTrigger;
    static const  std::map<std::string, VX2750Pha::TRGOUTMode> stringToTRGOUT;
    static const  std::map<std::string, VX2750Pha::GPIOMode>  stringToGPIO;
    static const  std::map<std::string, VX2750Pha::BusyInSource> stringToBusyIn;
    static const  std::map<std::string, VX2750Pha::SyncOutMode> stringToSyncOut;
    static const  std::map<std::string , VX2750Pha::VetoSource>  stringToVeto;
    static const  std::map<std::string, VX2750Pha::VetoPolarity> stringToVetoPolarity;
    static const  std::map<std::string, VX2750Pha::ChannelVetoSource> stringToChannelVeto;
    static const  std::map<std::string, VX2750Pha::WaveDataSource> stringToWaveDataSource;
    static const  std::map<std::string, VX2750Pha::WaveResolution> stringToWaveResolution;
    static const  std::map<std::string, VX2750Pha::AnalogProbe> stringToAnalogProbe;
    static const  std::map<std::string, VX2750Pha::DigitalProbe> stringToDigitalProbe;
    static const  std::map<std::string, VX2750Pha::IndividualTriggerLogic> stringToIndividualTriggerLogic;
    static const  std::map<std::string, VX2750Pha::PairTriggerLogic> stringToPairLogic;
    static const  std::map<std::string, VX2750Pha::ITLConnect> stringToITLConnect;
    static const  std::map<std::string, VX2750Pha::LVDSMode> stringToLVDSMode;
    static const  std::map<std::string, VX2750Pha::DACOutputMode> stringToDACOutMode;
    static const  std::map<std::string, VX2750Pha::EventSelection> stringToEventSelection;
    static const std::map<std::string, VX2750Pha::CoincidenceMask> stringToCoincidenceMask;
    
    // InternalData.
private:
    EnabledItems  m_dppPhaOptions;
public:
    VX2750Pha(const char* hostOrPid, bool isUsb = false);
    virtual ~VX2750Pha();
    
private:
    VX2750Pha(const VX2750Pha&);
    VX2750Pha& operator=(const VX2750Pha& );
    int operator==(const VX2750Pha&);
    int operator!=(const VX2750Pha&);
public:
    
    std::string  getCupVersion() const;     // Get the CUP version
    std::string  getFpgaFWVersion() const;  // Get the firmware Version
    FwType       getFirmwareType() const;   // Get the type of the firmware.
    std::string  getModelCode() const;
    std::string  getPiggyBackCode() const;
    std::string  getModelName() const;
    FormFactor   getFormFactor() const;
    int          getFamilyCode() const;
    std::string  getSerialNumber() const;
    int          getMotherboardRev() const;
    int          getPiggybackRev() const;
    std::string  getLicense() const;
    bool         isLicensed() const;
    int          remainingUnlicensedTime() const;
    int          channelCount() const;
    int          bitsOfResolution() const;
    int          sampleRate() const;
    int          inputRange() const;
    bool         isDifferentialInput() const;
    int          inputImpedance() const;
    int          ipAddress() const;
    int          netMask() const;
    int          gateway() const;
    ClockSource  getClockSource() const;
    void         setClockSource(ClockSource) const;
    bool         isClockOutOnP0() const;
    void         setClockOutOnP0(bool state) const;
    bool         isClockOutOnFP() const;
    void         setClockOutOnFP(bool state) const;
    std::vector<StartSource>  getStartSource() const;
    void         setStartSource(const std::vector<StartSource>& src) const;
    std::vector<GlobalTriggerSource> getGlobalTriggerSource() const;
    void         setGlobalTriggerSource(const std::vector<GlobalTriggerSource>& sources) const;
    std::vector<WaveTriggerSource> getWaveTriggerSource(unsigned ch) const;
    void         setWaveTriggerSource(unsigned ch, const std::vector<WaveTriggerSource>& src) const;
    std::vector<EventTriggerSource> getEventTriggerSource(unsigned ch) const;
    void         setEventTriggerSource(unsigned ch, const std::vector<EventTriggerSource>& src) const;
    
    std::uint64_t getChannelTriggerMask(unsigned ch) const;
    void          setChannelTriggerMask(unsigned ch, std::uint64_t mask) const;
    TraceRecordMode getTraceRecordMode(unsigned ch) const;
    void           setTraceRecordMode(unsigned ch, TraceRecordMode mode) const;
    TRGOUTMode    getTRGOUTMode() const;
    void          setTRGOUTMode(TRGOUTMode mode) const;
    GPIOMode      getGPIOMode() const;
    void          setGPIOMode(GPIOMode mode) const;
    BusyInSource   getBusyInputSource() const;
    void          setBusyInputSource(BusyInSource src) const;
    SyncOutMode   getSyncOutMode() const;
    void          setSyncOutMode(SyncOutMode mode) const;
    VetoSource    getBoardVetoSource() const;
    void          setBoardVetoSource(VetoSource src) const;
    std::uint64_t   getBoardVetoWidth() const;
    void          setBoardVetoWidth(std::uint64_t ns) const;
    VetoPolarity  getBoardVetoPolarity() const;
    void          setBoardVetoPolarity(VetoPolarity pol) const;
    ChannelVetoSource getChannelVetoSource(unsigned chan) const;
    void          setChannelVetoSource(unsigned chan, ChannelVetoSource src) const;
    std::uint32_t getChannelVetoWidth(unsigned chan) const;
    void          setChannelVetoWidth(unsigned chan, std::uint32_t ns) const;
    std::uint32_t getRunDelay() const;
    void          setRunDelay(std::uint32_t ns) const;
    bool          isAutoDisarmEnabled() const;
    void          setAutoDisarmEnabled(bool state) const;
private:   // deprecated.
    bool          isMultiWindowRunEnabled() const;
    void          setMultiWindowRunEnable(bool state) const;
    bool          isPauseTimestampHoldEnabled() const;
    void          setPauseTimestampHold(bool enable) const;
public:
    std::uint32_t      getLEDStatus() const;
    std::uint32_t      getAcquisitionStatus() const;
    std::uint32_t      getMaxRawDataSize() const;
    double        getVolatileClockDelay() const;
    void          setVolatileClockDelay(double value) const;
    double        getPermanentClockDelay() const;
    void          setPermanentClockDelay(double value) const;
    WaveDataSource getWaveDataSource(unsigned chan) const;
    void          setWaveDataSource(unsigned chan, WaveDataSource source) const;
    std::uint32_t getRecordSamples(unsigned chan) const;
    void          setRecordSamples(unsigned chan, std::uint32_t samples) const;
    std::uint32_t getRecordNs(unsigned chan) const;
    void          setRecordNs(unsigned chan, std::uint32_t ns) const;
    WaveResolution getWaveResolution(unsigned chan) const;
    void          setWaveResolution(unsigned chan, WaveResolution resolution) const;
    AnalogProbe   getAnalogProbe(unsigned chan, unsigned probeNum) const;
    void          setAnalogProbe(unsigned chan, unsigned probeNum, AnalogProbe probe) const;
    DigitalProbe  getDigitalProbe(unsigned chan, unsigned probeNum) const;
    void          setDigitalProbe(
        unsigned chan, unsigned probeNum, DigitalProbe probe
    ) const;
    std::uint32_t getPreTriggerSamples(unsigned chan) const;
    void          setPreTriggerSamples(unsigned chan, std::uint32_t nsamples) const;
    std::uint32_t getPreTriggerNs(unsigned chan) const;
    void          setPreTriggerNs(unsigned chan, std::uint32_t ns) const;
    std::uint64_t getTestPulsePeriod() const;
    void          setTestPulsePeriod(std::uint64_t ns) const;
    std::uint64_t getTestPulseWidth() const;
    void          setTestPulseWidth(std::uint64_t ns) const;
    std::uint32_t getTestPulseLowLevel() const;
    void          setTestPulseLowLevel(std::uint32_t counts) const;
    std::uint32_t getTestPulseHighLevel() const;
    void          setTestPulseHighLevel(std::uint32_t counts) const;
    IOLevel       getIOLevel() const;
    void          setIOLevel(IOLevel level) const;
    double        getAirInTemp() const;
    double        getAirOutTemp() const;
    double        getCoreTemp() const;
    double        getFirstADCTemp() const;         // All in degrees C.
    double        getLastADCTemp() const;
    double        getHottestADCTemp() const;
    double        getADCTemp(unsigned chip) const;
    double        getDCDCConverterTemp() const;
    
    double        getDCDCConverterInputVoltage() const;
    double        getDCDCConverterOutputVoltage() const;
    double        getDCDCConverterAmps() const;
    double        getDCDCConverterHz() const;
    double        getDCDCConverterDutyCycle() const;
    
    std::uint32_t getFanSpeed(unsigned fan) const;
    
    // Bit encoded errors per ERR_* bit definitions.
    
    std::uint32_t getErrorFlagMask() const;
    void          setErrorFlagMask(std::uint32_t mask) const;
    std::uint32_t getErrorFlagDataMask() const;
    void          setErrorFlagDataMask(std::uint32_t mask) const;
    std::uint32_t getErrorFlags() const;
    
    bool          isBoardReady() const;
    
    IndividualTriggerLogic getITLAMainLogic() const;
    IndividualTriggerLogic getITLBMainLogic() const;
    void          setITLAMainLogic(IndividualTriggerLogic sel) const;
    void          setITLBMainLogic(IndividualTriggerLogic sel) const;
    unsigned      getITLAMajorityLevel() const;
    unsigned      getITLBMajorityLevel() const;
    void          setITLAMajorityLevel(unsigned level) const;
    void          setITLBMajorityLevel(unsigned level) const;
    
    PairTriggerLogic getITLAPairLogic() const;
    PairTriggerLogic getITLBPairLogic() const;
    void         setITLAPairLogic(PairTriggerLogic sel) const;
    void         setITLBPairLogic(PairTriggerLogic sel) const;
    bool          isITLAInverted() const;
    bool          isITLBInverted() const;
    void          setITLAInverted(bool invert) const;
    void          setITLBInverted(bool invert) const;
    ITLConnect    getITLConnect(unsigned ch) const;
    void          setITLConnect(unsigned ch, ITLConnect selection) const;
    
    std::uint64_t getITLAMask() const;
    std::uint64_t getITLBMask() const;
    void          setITLAMask(std::uint64_t mask) const;
    void          setITLBMask(std::uint64_t mask) const;
    
    std::uint32_t getITLAGateWidth() const;
    std::uint32_t getITLBGateWidth() const;
    void          setITLAGateWidth(std::uint32_t ns) const;
    void          setITLBGateWidth(std::uint32_t ns) const;
    
    LVDSMode      getLVDSMode(unsigned quartet) const;
    void          setLVDSMode(unsigned quartet, LVDSMode mode) const;
    LVDSDirection getLVDSDirection(unsigned quartet) const;
    void          setLVDSDirection(unsigned quartet, LVDSDirection direction) const;
    std::uint16_t getLVDSIOReg() const;
    void          setLVDSIOReg(std::uint16_t mask) const;
    std::uint64_t getLVDSTriggerMask(unsigned inputNum) const;
    void          setLVDSTriggerMask(unsigned inputNum, std::uint64_t mask) const;
    
    void setDACOutMode(DACOutputMode mode) const;
    DACOutputMode getDACOutMode() const;
    std::uint16_t getDACOutValue() const;
    void          setDACOutValue(std::uint16_t value) const;
    unsigned      getDACChannel() const;
    void          setDACChannel(unsigned chan) const;
    
    
    
    double        getVGAGain(unsigned group) const;
    void          setVGAGain(unsigned group, double value) const;
    
    bool          isOffsetCalibrationEnabled() const;
    void          enableOffsetCalibration(bool enable) const;
    bool          isChannelEnabled(unsigned chan) const;
    void          enableChannel(unsigned chan, bool enable) const;
    int           getSelfTriggerRate(unsigned chan) const;
    std::uint32_t getChannelStatus(unsigned chan) const;
    double        getDCOffset(unsigned chan) const;
    void          setDCOffset(unsigned chan, double pct) const;
    double        getGainFactor(unsigned chan) const;
    std::uint32_t getTriggerThreshold(unsigned chan) const;
    void          setTriggerThreshold(unsigned chan, std::uint32_t threshold) const;
    Polarity      getPulsePolarity(unsigned chan) const;
    void          setPulsePolarity(unsigned chan, Polarity pol) const;
    
    std::uint16_t getEnergySkimLowDiscriminator(unsigned chan) const;
    void          setEnergySkimLowDiscriminator(unsigned chan, std::uint16_t value) const;
    std::uint16_t getEnergySkimHighDiscriminator(unsigned chan) const;
    void          setEnergySkimHighDiscriminator(unsigned chan, std::uint16_t value) const;
    
    EventSelection getEventSelector(unsigned chan) const;
    void           setEventSelector(unsigned chan, EventSelection sel) const;
    EventSelection getWaveformSelector(unsigned chan) const;
    void           setWaveformSelector(unsigned chan, EventSelection sel) const;
    CoincidenceMask getCoincidenceMask(unsigned chan) const;
    void           setCoincidenceMask(unsigned chan, CoincidenceMask sel) const;
    CoincidenceMask getAntiCoincidenceMask(unsigned chan) const;
    void           setAntiCoincidenceMask(unsigned chan, CoincidenceMask sel) const;
    
    std::uint32_t  getCoincidenceSamples(unsigned chan) const;
    void           setCoincidenceSamples(unsigned chan, std::uint32_t samples) const;
    std::uint32_t  getCoincidenceNs(unsigned chan) const;
    void           setCoincidenceNs(unsigned chan, std::uint32_t ns) const;
    
    std::uint32_t  getTimeFilterRiseTime(unsigned chan) const;
    std::uint32_t  getTimeFilterRiseSamples(unsigned chan) const;
    void           setTimeFilterRiseTime(unsigned chan, std::uint32_t ns) const;
    void           setTimeFilterRiseSamples(unsigned chan, std::uint32_t samples) const;
    
    std::uint32_t  getTimeFilterRetriggerGuardTime(unsigned chan) const;
    std::uint32_t  getTimeFilterRetriggerGuardSamples(unsigned chan) const;
    void           setTimeFilterRetriggerGuardTime(unsigned chan, std::uint32_t ns) const;
    void           setTimeFilterRetriggerGuardSamples(unsigned chan, std::uint32_t samples) const;
    
    std::uint32_t  getEnergyFilterRiseTime(unsigned chan) const;
    std::uint32_t  getEnergyFilterRiseSamples(unsigned chan) const;
    void           setEnergyFilterRiseTime(unsigned chan, std::uint32_t ns) const;
    void           setEnergyFilterRiseSamples(unsigned chan, std::uint32_t samples) const;
    
    std::uint32_t  getEnergyFilterFlatTopTime(unsigned chan) const;
    std::uint32_t  getEnergyFilterFlatTopSamples(unsigned chan) const;
    void           setEnergyFilterFlatTopTime(unsigned chan, std::uint32_t ns) const;
    void           setEnergyFilterFlatTopSamples(unsigned chan, std::uint32_t samples) const;
    
    std::uint32_t  getEnergyFilterPeakingPosition(unsigned chan) const;
    void           setEnergyFilterPeakingPosition(unsigned chan, std::uint32_t pct) const;
    
    EnergyPeakingAverage getEnergyFilterPeakingAverage(unsigned chan) const;
    void          setEnergyFilterPeakingAverage(unsigned chan, EnergyPeakingAverage sel) const;
    
    std::uint32_t getEnergyFilterPoleZeroTime(unsigned chan) const;
    std::uint32_t getEnergyFilterPoleZeroSamples(unsigned chan) const;
    void          setEnergyFilterPoleZeroTime(unsigned chan, std::uint32_t ns) const;
    void          setEnergyFilterPoleZeroSamples(unsigned chan, std::uint32_t samples) const;
    
    double        getEnergyFilterFineGain(unsigned chan) const;
    void          setEnergyFilterFineGain(unsigned chan, double gain) const;
    
    bool          isEnergyFilterFLimitationEnabled(unsigned chan) const;
    void          enableEnergyFilterFLimitation(unsigned chan, bool enable) const;
    
    EnergyFilterBaselineAverage getEnergyFilterBaselineAverage(unsigned chan) const;
    void          setEnergyFilterBaselineAverage(unsigned chan, EnergyFilterBaselineAverage sel) const;
    
    std::uint32_t getEnergyFilterBaselineGuardTime(unsigned chan) const;
    std::uint32_t getEnergyFilterBaselineGuardSamples(unsigned chan) const;
    void          setEnergyFilterBaselineGuardTime(unsigned chan, std::uint32_t ns) const;
    void          setEnergyFilterBaselineGuardSamples(unsigned chan, std::uint32_t samples) const;
    
    std::uint32_t getEnergyFilterPileupGuardTime(unsigned chan) const;
    std::uint32_t getEnergyFilterPileupGuardSamples(unsigned chan) const;
    void          setEnergyFilterPileupGuardTime(unsigned chan, std::uint32_t ns) const;
    void          setEnergyFilterPileupGuardSamples(unsigned chan, std::uint32_t samples) const;
    
    std::uint8_t  getEnergyBits(unsigned chan) const;
    std::uint64_t getRealtime(unsigned chan) const;
    std::uint64_t getDeadtime(unsigned chan) const;
    
    // Commands:
    
    void          Reset();
    void          Clear();
    void          Arm();
    void          Disarm();
    void          Start();
    void          Stop();
    void          Trigger();
    void          ReloadCalibration();
    
    // Endpoint management:
    
    Endpoint      getEndpoint();
    void          selectEndpoint(Endpoint selection);
    
    // Raw endpoint - note that when reading raw waveforms we always read
    // both the size and data fields.
    
    void   initializeRawEndpoint();          // In case someone changes the json.
    size_t readRawEndpoint(void* pBuffer);
    
    // We're going to try to hide that awful JSON crap inside our class
    // Some items will be mandatory, others not and are off by default
    // but can be turned on/off programatically
    // Mandatory:  channel, nsTimestamp, energy, fail
    //
    void setDefaultFormat();
    void enableRawTimestamp(bool enable);
    void enableFineTimestamp(bool enable);
    void enableFlags(bool enable);
    void enableDownsampling(bool enable);
    void enableAnalogProbes(bool probe1, bool probe2);
    void enableDigitalProbes(bool probe1, bool probe2, bool probe3, bool probe4);
    void enableSampleSize(bool enable);
    void enableRawEventSize(bool enable);
    
    // This sends the JSON:
    
    void initializeDPPPHAReadout();
    void readDPPPHAEndpoint(DecodedEvent& event);
    
    // Buffer management for the formatted (DPPPHAEndpoint):
    
    void initDecodedBuffer(DecodedEvent& event);
    void setupDecodedBuffer(DecodedEvent& event);
    void freeDecodedBuffer(DecodedEvent& event);
    
    
private:
    uint32_t    dottedToInt(const std::string& dotted) const; 
    template<class T> std::string enumToString(const std::map<T, std::string>& map, T value) const;
    template<class T> T stringToEnum(const std::map<std::string, T>& map, const std::string& value) const;
    bool textToBool(const std::string& str) const;
    void checkInclusiveRange(int low, int high, int value) const;
    std::string appendNumber(const char* base, unsigned number) const;
    Json::Value createScalar(const char* name, const char* type) const;
    Json::Value createArray(const char* name, const char* type, unsigned dimension) const;
    static std::string toUpper(const std::string& s);
    static std::string stringListToOrList(const std::vector<std::string>& strings);
    static std::vector<std::string> orListToStringList(std::string orlist);
  
};

// Implementation of template methods:
//  We use enums where possible so we can get compile time checking of
//  values, therefore the main thing we need to do that's templated is
//  conversions from enumerated values -> strings and back:

/**
 * enumToString
 *    Given an enumerated value return the associated string.
 *    This is templated on the key of the conversion mapping.
 * @param map  - Map of enumerated value -> strings.
 * @param value - Value in the key type of the map
 * @return std::string  - mapped string value.
 * @throw std::invalid_argument - lookup failure
 * @note it is possible for a lookup failure to occur given C/C++'s ability to
 *       treat enumerated values like integers including being able to perform
 *       arithmetic on them DON'T DO THIS, but we protect anyway.
 */
template<class T>
std::string
VX2750Pha::enumToString(const std::map<T, std::string>&map, T value) const
{
    auto p = map.find(value);
    if (p == map.end()) {
        std::string msg = "Invalid lookup key passed to enumToString: ";
        msg += std::to_string(value);
        throw std::invalid_argument("Invalid lookup key passed to enumToString");
    }
    return p->second;
}
/**
 * stringToEnum
 *    Given a  string looks up an enumeraged value.
 *    Yes I could really implement both with two templated arguments
 *    but I'm too stupid to do that, and this provides
 *    a meaningful pair of method names which that would not
 *
 * @param map - mapping from string->enum.
 * @param value - String value to look up.
 * @return T.
 * @throw std::invalid_argument if there's no mapping for the string
 * @note If the map has been constructed properly, there's less chance of throw
 *       than in the previous method because the 'hardware's returning the
 *       strings that are getting tossed into this.
 */
template<class T>
T
VX2750Pha::stringToEnum(const std::map<std::string, T>& map, const std::string& value) const
{
    // Case blind fine of value in the string keys of the map:
    
    auto p = map.begin();
    std::string needle = toUpper(value);
    while (p != map.end()) {
        if (toUpper(p->first) == needle) {
            break;
        }
        ++p;
    }
    
    if (p == map.end()) {
        // We can also be a bit more detailed in our message:
        
        std::string msg("Invalid lookup key passed to stringtoenum : ");
        msg += value;
        throw std::invalid_argument(msg);
    }
    return p->second;
}

}                                 // namespace.

#endif
