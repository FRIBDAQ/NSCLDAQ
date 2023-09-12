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
* @file     VX2750PHAConfiguration.h
* @brief    Header for the configuration associated with a 
* @author   Ron Fox
*
*/

#ifndef CAENVX2750_PHAMODULECONFIGURATION_H
#define CAENVX2750_PHAMODULECONFIGURATION_H


#include "XXUSBConfigurableObject.h"      // nice base class from NSCLDAQ.


namespace caen_nscldaq {
class VX2750Pha;

/**
 * @class V2750PHAModuleConfiguration
 *   Provides a configuration for the readout of a VX2750 (e.g.) module
 *   with PHA firmware.
 *   constructing this class (unlike how the XXUSB readouts work) constructs
 *   the configuration of the module with a default configuration.
 *  Configurable parameters; these divide into sections:
 *
 *  ### Controlling readout:
 *     -  readrawtimestamps - bool enable readout of raw timestamps
 *     -  readfinetimestamps - bool enable readout of the CFD fine timestamp.
 *     -  readflags          - bool enable readout of the high and low priority flags.
 *     -  readtimedownsampling - bool read the time downsampling factor.
 *     -  readanalogprobes    - list of two bools:  {probe1enable probe2enable}
 *     -  readdigitalprobes   - list of four bools {probe1 probe2 probe3 probe4}
 *     -  readsamplecount     - bool -enable read of number of samples in fragment.
 *     -  readeventsize        - bool enable read of event size.
 *  ### General Parameters:
 *     -  clocksource - enumerated "Internal", "FPClkIn", "P0ClkIn", "Link", "DIPswitchSel"
 *     -  outputp0clock - bool  Output clock on backplane.
 *     -  outputfpclock - bool  Output clock on front panel.
 *
 *  ### Acquisition, Trigger and Veto Parameters:
 *     -  startsource - enum EncodedClkIn, SINLevel, SINedge, SWcmd, LVDS, P0
 *     -  gbltriggersrc - enum TrgIn, P0, SwTrg, LVDS, ITLA, ITLB,
 *                             ITLA_AND_ITLB, ITLA_OR_ITLB, EncodedClkIn,
 *                             GPIO, TestPulse
 *     - wavetriggersrc - enum "ITLB", "ITLA", "GlobalTriggerSource", "TRGIN",
 *                              "ExternalInhibit", "ADCUnderSaturation",
 *                              "ADCOverSaturation", :SWTrigger", "ChSelfTrigger",
 *                              "Ch64Trigger", "Disabled"
 *     - eventtriggersrc - enum - same as wavetriggersrc.
 *     - tstampresetsrc  - enum - "Start", "SIN", "GPIO", "EncodedClkIn"
 *     - channeltriggermasks- list of up to 64 uint64 masks of channel triggers.
 *                           that can trigger the associated channel.
 *     - savetraces       - list of up to 64 bool values true to save traces from
 *                          the corresponding channel.
 *     - triggeroutmode   -  enum TRGIN, P0, SwTrg, LVDS, ITLA, ATLB, ITLA_AND_ITLB,
 *                                ITLA_OR_ITLB, EncodedClkIn, Run, RefClk, TestPulse,
 *                                Busy, Fixed0, Fixed1, SyncIn, SIN, GPIO,
 *                                AcceptTrg, TrgClk
 *     - gpiomode   - Enum: Disabled, TrgIn, P0, SIN, LVDS, ITLA, ITLB,
 *                          ITLA_AND_ITLB, EncodedClkIn, SwTrg, Run, RefClk,
 *                          TestPulse, Busy, Fixed0, Fixed1
 *     - busyinsrc - enum - SIN, GPIO, LVDS, Disabled
 *     - syncoutmode - enum - Disabled, SyncIn, TestPulse, Run
 *     - boardvetosrc - enum SIN, LVDS, GPIO, P0, EncodedClkIn, Disabled
 *     - boardvetowidth - integer ns 0 - 34359738360 inclusive.
 *     - boardvetopolarity - enum ActiveHigh, ActiveLow
 *     - chanvetosrc  - list of up to 64 enums: BoardVeto, ADCOverSaturation,
 *                      ADCUnderSaturation, Disabled
 *     - chanvetowidth - List of up to 64 integers 0-524280
 *     - rundelay     - integer 0 - 54280
 *     - autodisarm   - bool
 *     - multiwindow - bool
 *     - pausestamp  - enum hold, run
 *     - volclkoutdelay - float -18888.888 - +18888.888
 *     - permclkoutdelay - float -18888.888 - +18888.888
 *     
 *  ### Waveform inspection parameters:
 *      -  wavesource - list of enums: "ADC_DATA", "ADC_TEST_TOGGLE",
 *                  "ADC_TEST_RAMP", "ADC_TEST_SIN", "IPE", "Ramp", "SquareWave",
 *                  "ADC_TEST_PRBS"
 *      -  recordsamples - list of integer record lengths in sampls: 4-8100
 *      -  waveresolutions - list of 64 waveform resolution enums:
 *                  "Res8", "Res16", "Res32", "Res64"
 *      -  analogprobe0 = list of enums "ADCInput", "TimeFilter", "EnergyFilter",
 *                          "EnergyFilterBaseline", "EnergyFilterMinusBaseline"
 *      -  analogprobe1 = list of enums as above but for analog probe 1.
 *      -  digitalprobe{1-4} list of enums describing the digital probes.
 *                  values: "Trigger", "TimeFilterArmed", "RetriggerGuard",
 *                  "EnerygFilterBaslineFreeze","EnergyFilterPeaking",
 *                  "EnergyFilterPileUpGuard", "EventPileup", "ADCSaturation",
 *                  "ADCSaturationProtection", "PostSaturationEvent",
 *                  "EnergyFilterSaturation", "AcqusitionInhibit"
 *      - pretriggersamples - samples prior to trigger list of integers 4-4000
 *
 *   ### Service Parameters
 *
 *      - testpulseperiod Integer 0 - 34359738360  Default 100000 (100usec).
 *      - testpulsewidth  Integer 0 - 34359738360 Default 50000 (50usec).
 *      - testpulselowlevel Integer 0 - 66535 - Default 0
 *      - testpulsehighlevel Integer 0- 65535 - Default 65535
 *      - iolevel  Enum NIM, TTL
 *      - errorflagmask - integer - 0 - 65535 - default 0
 *      - errorflagdatamask - integer - 65535 - default 0
 *
 *   ### Individual trigger parameters:
 *
 *      -  itlamainlogic, itlbmainlogic - enum "OR"tr, "AND", "Majority
 *      -  itlamajoritylevel, itlbmajoritylevel - integer 1-63
 *      -  itlapairlogic, itlbpairlogic - enum - "AND", "OR", "NONE"
 *      -  itlapolarity, itlbpolarity - Enum "Direct", "Inverted"
 *      -  itlconnect - Enum list "Disabled", "ITLA", "ITLB"
 *      -  itlagatewidth, itlbgatewidth - integer 0 - 524280 (default 100).
 *      
 *   ### LVDS Parameters
 *
 *      - lvdsmode - list of four enums: "SelfTriggers", "Sync", "IORegister"
 *      - lvdsdirection - list of four enums: "Input", "Output"
 *      - lvdsioreg  - LVDS I/O register values... 4 16 bit integers (?).
 *      - lvdstrgmask - LVDS Trigger masks for each of 16 outputs (16 element list).
 *
 *   ### DAC Parameters:
 *
 *      - dacoutmode enum: "Static", "IPE", "ChInput", "MemOccupancy",
 *          "ChSum", "OverThrSum", "Ramp", "Sin5MHz", "Square"
 *      - dacoutputlevel integer 0 - 64383 defaults to 0.
 *      - dacoutchannel integer 0 - 63. defaults to 0.
 *      
 *   ###  Input signalling parameters:
 *
 *      -  vgagain  - integer 0-40
 *      -  offsetcalibrationenable - list of 64 bools.
 *      -  channelenables - list of 64 bools.
 *      -  dcoffsets - list of 64 floats 0-100 (percent).
 *      -  triggerthresholds - list of 64 integers 0-8191.
 *      -  inputpolarities - list of 64 enums (Positive, Negative)
 *
 *  ### Event Selection and Coincidence parameters:
 *
 *       - energyskimlow - 64 integers 0-65534
 *       - energyskimhigh -  64 integers 0-65534
 *       - eventselector - 64 enums All, Pileup, EnergSkim
 *       - waveselector  - 64 enums as above.
 *       - coincidencemask - 64 enums Disabled, Ch64Trigger, TRGIN,
 *                        GlobalTriggerSource, ITLA, ITLB
 *       - anticoincidencemask - same as above.
 *       - coincidencelength - 64 integers nanosecond units.
 *
 *   ### DPP-PHA parameters
 *      tfrisetime - List of 64 channel time filter rise times in ns 80-2000
 *      tfretriggerguard - list of 64 channel TF retrigger guards in ns 0-8000
 *      efristetime  - list of 64 per channel energy filter rise times in ns 80-13000
 *      efflattop    - list of 64 per channel energy filter rise times in ns 80-3000
 *      efpeakingpos - list of 64 per channel energy filter trap filter peaking positions 0-100 (percent).
 *      efpeakingavg - samples averaged to locate the pieak enum: 1, 4, 16, 64.
 *      efpolezero    - list of 64 per channel pole zero adjust ns 80 - 524000
 *      effinegain   - 64 fine gains floats 1-10.
 *      eflflimitation - 64 bools enable/disable low frequency filter.
 *      efbaselineavg   -64 enums baseline averages: 0, 16, 64, 256, 1024, 4096, 16384.
 *      efbaselineguardt - 64 ints baseline guard time ns 0-8000
 *      efpileupguardt - 64 ints baseilne filter pileup guard ns 0-80000
 *      
 *       
 */
class VX2750PHAModuleConfiguration : public ::XXUSB::CConfigurableObject
{
public:
    VX2750PHAModuleConfiguration(const char* name);
    VX2750PHAModuleConfiguration(const VX2750PHAModuleConfiguration& rhs);
    virtual ~VX2750PHAModuleConfiguration();
    
    VX2750PHAModuleConfiguration& operator=(const VX2750PHAModuleConfiguration& rhs);
    int operator==(const VX2750PHAModuleConfiguration& rhs);
    int operator!=(const VX2750PHAModuleConfiguration& rhs);
    
    void configureModule(VX2750Pha& module);
    
    // Everything else public is done by the base class.
private:
    void defineReadoutOptions();
    void configureReadoutOptions(VX2750Pha& module);
    void defineGeneralOptions();
    void configureGeneralOptions(VX2750Pha& module);
    void defineAcqTriggerOptions();
    void configureAcquisitionTriggerOptions(VX2750Pha& module);
    void defineWfInspectionOptions();
    void configureWfInspectionOptions(VX2750Pha& module);
    void defineServiceOptions();
    void configureServiceOptions(VX2750Pha& module);
    void defineITLOptions();
    void configureITLOptions(VX2750Pha& module);
    void defineLVDSOptions();
    void configureLVDSOptions(VX2750Pha& module);
    void defineDACOptions();
    void configureDACOptions(VX2750Pha& module);
    void defineInputConditioningOptions();
    void configureInputConditioning(VX2750Pha& module);
    void defineEventSelectionOptions();
    void configureEventSelection(VX2750Pha& module);
    void defineFilterOptions();
    void configureFilter(VX2750Pha& module);
};
    
}                                             // caen_nscldaq namespace



#endif
