/**
 * @brief Header for the CSIS3316 class that provides support for the SIS3316
 * @author Ron Fox <fox@frib.msu.edu>
 * 
 * Facility for Rare Isotope Beams
 * 640 S. Shaw Lane
 * East Lansing, MI 48824-1321
 * (c) Copyright 2025 Board of Trustees of Michigan State University 
 * 
 *  You may use this software under the terms of the GNU public license
 *   (GPL).  The terms of this license are described at:
 *
 *   http://www.gnu.org/licenses/gpl.txt
 * 
 * 
 */

#include "CSIS3316.h"
#include "CReadoutModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include "sis3316_adc.h"
#include <sis_vmusb_interface.h>
#include "sis3316.h"                   // Register definitions.

#include <sstream>
#include <unistd.h>
#include <assert.h>
#include <vector>

// Parameter constraints:

// CLock sources:

static const char* ClockSources[] = {
    "fp", "250MHz","125Mhz", "50MHz", "25Mhz", "12.5MHz", NULL
};
static XXUSB::ConfigurableObject::isEnumParameter
    ClockSourceValues(XXUSB::CConfigurableObject::makeEnumSet(ClockSources));

static const uint64_t Zero(0);    // Shared low limit for many things.
static const uint64_t MaxSamples(65535);
static const uint64_t MaxId(128);
static const uint64_t MaxPretrigger(0x3fff);   // 14 bits of pre-trigger.


/**
 *  constructor:
 *      JUst needs to initialize the pointers to null; and create the bus object.
 */
CSIS3316::CSIS3316() :
    m_pConfiguration(nullptr), m_pModule(nullptr), m_pVmeBus(nullptr)
{
    m_pVmeBus = new sis_vmusb_interface;         // By now there's a VMUSB object.
}
/**
 *  destructor:
 */
CSIS3316::~CSIS3316() {
    // Just need to delete the dynamic stuff:

    delete m_pModule;
    delete m_pVmeBus;
    delete m_pConfiguration;
}
/**
 *  Copy constructor = might actually be illegal we'll see.
 * 
 * @param rhs - the object being copied.
 */
CSIS3316::CSIS3316(const CSIS3316& rhs) :
    m_pConfiguration(nullptr), m_pModule(nullptr), m_pVmeBus(nullptr)
{
    if (rhs.m_pConfiguration) {
        m_pConfiguration = new CReadoutModule(*rhs.m_pConfiguration);
    }
    if (rhs.m_pVmeBus) {
        m_pVmeBus = new sis_vmusb_interface(*rhs.m_pVmeBus);
    }
    if (m_pModule) {
        m_pModule = new  sis3316_adc(*rhs.m_pModule);
    }
}

/**
 * operator= 
 *   @param rhs - the object being assigned to this.
 *   @return CSIS3316& - referenced to this.
 */
CSIS3316&
CSIS3316::operator=(const CSIS3316& rhs) {
    if (this != &rhs) {            // Else noop.,
        m_pConfiguration = rhs.m_pConfiguration ?
            new CReadoutModule(*rhs.m_pConfiguration) :
            nullptr;
        m_pVmeBus = rhs.m_pVmeBusj ?
            new sis_vmusb_inhterface(*rhs.m_pVmeBus) :
            nullptr;
        m_pModule = rhs.m_pModule ?
            new sis3316_adc(*rhs.m_pModule) :
            nullptr;
    }

    return *this;
}

/**
 * onAttach:
 *    Provides an empty configuration to the object.  We save it for later
 * and define out configuration options into  it so we can be appropriately configured.
 * 
 * @param[inout] configuration - referencdes out configuration object.
 * 
 * 
 */
void
CSIS3316::onAttach(CReadoutModule& configuration) {
    // Save the configuration as a pointer.

    m_pConfiguration = &configuration;

    // Describe the configurable parameters:

    m_pConfiguration->addIntegerParameter("-base", 0);
    m_pConfiguration->addEnumParameter("-clock", ClockSourceValues, "250MHz" );
    m_pConfiguration->addIntListParameter("-samples", Zero, MaxSamples, 4,4,4);
    m_pConfiguration->addIntegerParameter("-id", 0,  MaxSamples, MaxSamples/4);
    m_pConfiguration->addIntListParameter("-pretrigger", Zero, MaxPretrigger, Zero);
    m_pConfiguration->addBoolListParameter("-enable", 16);

}
/**
 * Initialize
 *    Called as data taking is about to start.  We must initialize the
 * module for readout in accordance with the configuration. Note that
 * some bits of the configuration (-enables, -samples e.g.) are actually
 * used to setup the readout list for the module.
 * We'll setup for front panel trigger.
 * 
 * @note
 * We set up the module to take a single event into bank1.   The readout
 * list will, in addition to reading the data taken, Disarm the bank, reset the
 * start address(es) and rearm bank 1.  In this way we defeat the bankswitch
 * and also ensure the next readout list will know where to transfer data from
 * the buffer memory to the FIFO.
 * 
 * @note
 * This is when/where we also make the VME interface object and the
 * module object we'll use to do the bits of the setup it can do.
 * 
 * 
 * @param[inout] controller - references the CVMUSB module that runs the
 * VME crate the module is in.
 */
void
CSIS3316::Initialize(CVMUSB& controller) {
    // Make the bus interface and module objects:

    delete m_pModule;
    m_pModule  = nullptr;
    delete m_pVmeBus;
    delete m_pVmeBus = nullptr;

    m_pVmeBus = new sis_vme_interface;  // Gets the controller from Globals.
    m_pModule = new sis3316_adc(m_pVmeBus, m_pConfiguration->getIntegerParameter("-base"));

    // Let's make sure we really have an SIS3316:

    UINT value;              // Thing read:

    m_pModule->register_read(SIS3316_MODID, &value);
    if ((value >> 16) != 0x3316) {
        std::stringstream strMsg;
        strMsg << m_pConfiguration->getIntegerParameter("-base") 
            << " Is not an SIS3316 module!\n";
        strMsg << "You can use $DAQROOT/sis3316/inventory to list the SIS3316 modules in the crate\n";
        std::string msg = strMsg.str();
        throw msg;
    }

    // We're going to assum all our operatons actually work - because I'm lazy
    // and very likely it's true.

    m_pModule->register_write(SIS3316_KEY_RESET, 0);  // Module reset.
    usleep(5*1000);                                   // wait. for it
    m_pModule->register_write(SIS3316_KEY_ADC_FPGA_RESET, 0);  // Reset the FPGAs.
    usleep(5*1000);                                   // Wait for it.
    m_pModule->register_write(SIS3316_KEY_DISARM, 0); // Keep disarmed.
    m_pModule->register_write(SIS3316_KEY_TIMESTAMP_CLEAR); 

    // Set up the clock source:

    int whichClock = m_pConfiguration->getEnumParameter("-clock", ClockSources);
    if (std::string("fp") == ClockSources[whichClock]) {
        // Set up for NIM clock input:

        m_pModule->register_write(SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL, 3);
    } else {
        // internal clock:

        m_pModule->register_write(SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL, 0);
        
        // Now set the sample freq:

        std::string freq = ClockSources[whichClock]
        int hs_div, n1div, fft_freq;
        if (freq == "250MHz") {
            sis3316_adc::get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERAGE_250MSPS, 
                &hs_div, &n1div, &fft_freq
            );
        } else if (freq == "125MHz") {
            sis3316_adc::get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERATE_125MSPS, 
                &hs_div, &n1div, &fft_freq
            );

        } else if (freq == "50MHz") {
            sis3316_adc::get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERATE_50MSPS, 
                &hs_div, &n1div, &fft_freq
            );

        } else if (freq == "25MHz") {
            sis3316_adc::get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERATE_25MSPS, 
                &hs_div, &n1div, &fft_freq
            );
            
        } else if (freq == "12.5MHz") {
            sis3316_adc::get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERATE_12P5MSPS, 
                &hs_div, &n1div, &fft_freq
            );

        } else {
            std::stringstream strmsg;
            strmsg << freq << " Is not a supported clock frequency\n";
            thow strmsg;
        }
        // Set up the header ids for the ADC groups:

        auto ids = m_pConfiguration->getUnsignedList("-id");
        std::vector<int> idregs = {
            SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG,
            SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG,
            SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG,
            SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG
        };
        assert(ids.size() == 4);
        for (int i =0; i < 4; i++) {
            // We get to write bits 4-11 if the id and
            // bits 2,3 are the group.  The bottom 2 bits are the
            // adc within the group.  Note that
            // all of this is shifted 20 bits up in to the register. 
            // See the manual:  6.47
            m_pModule->register_write(idregs[i], ids[i] << 24 | (i << 22);
        }
        // Set the trace lengths for each group.
        // Note this also sets th raw buffer start indices -> 0.

        auto samples = m_pConfiguration->getUnsignedList("-samples");
        std::vector<int> bufregs = {
            SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG,
            SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG,
            SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG,
            SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG
        };
        assert(samples.size() == 4);
        for (int i =0; i < 4; i++) {
            m_pModule->register_write(bufregs[i], samples[i] << 16);
        }

        // Set the pre-triger for each group.

        auto pretriggers = m_pConfiguration->getUnsignedList("-pretrigger");
        std::vector<int> pretrigregs = {
            SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG,
            SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG,
            SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG,
            SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG
        };
        assert(pretriggers.size() =- 4);
        for (int i=0; i < 4; i++) {
            m_pModule->register_write(pretrigregs[i], pretriggers[i])
        }
        // Set each ADC group in 'single-event' mode.
        // starting address 0.
        // We set the end threshold registesr to 1 and disable storing hits
        // after full:

        std::vector<int> endAddressRegisters = {
            SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG,
            SIS3316_ADC_CH5_8_ADDRESS_THRESHOLD_REG,
            SIS3316_ADC_CH9_12_ADDRESS_THRESHOLD_REG,
            SIS3316_ADC_CH13_16_ADDRESS_THRESHOLD_REG
        };
        // Even a single write will make the threshold.
        for (int i = 0; i < endAddressRegisters.size(); i++) {
            m_pModule->register_write(endAddressRegisters[i], 0x80000001);
        }


        // Set external trigger (I think) 0x80 set trigger function for TI,
        // 0x10 is NIM input TI as trigger enable.

        m_pModule->register_write(SIS3316_NIM_INPUT_CONTROL_REG, 0x90);
        

        // Note 0x100 is FP trigger enable (I think)
        // The write to the T0 select register allows a stretched trigger-> ADCFPGA
        // to be monitored on TO
        // The write to U0 select makes it monitor sample logic  ready.

        m_pModule->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, 0x100);
        m_pModule->register_write(SIS3316_LEMO_OUT_TO_SELECT_REG, 0x02000000);
        m_pModule->register_write(SIS3316_LEMO_OUT_UO_SELECT_REG, 0x100);
        
        // Set the enables for each ADC.

        auto enables = m_pConfiguration->getBoolList("-enable");
        std::vector<int> enableRegs = {
            SIS3316_ADC_CH1_4_EVENT_CONFIG_REG,  
            SIS3316_ADC_CH5_8_EVENT_CONFIG_REG,
            SIS3316_ADC_CH9_12_EVENT_CONFIG_REG,
            SIS3316_ADC_CH13_16_EVENT_CONFIG_REG
        };
        assert(enables.size() == 16);
        for (int i = 0; i < enableRegs.size(); i++) {
            uint32_t mask(0);          // Defaults are non enabled.
            int firstchan = i*4;       // offset to ch 0 in the register.
            for(int ch = 0; ch < 4; ch++) {
                if (enableRegs[ch]) {
                    mask |= 2 << (ch*8);    // Enable trigger response
                }
            }
            // mask has the full register value:

            m_pModule->register_write(enableRegs[i], mask);
        }
        // Don't save anything but the waveforms:

        std::vector<int> evformatRegs = {
            SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG,
            SIS3316_ADC_CH5_8_DATAFORMAT_CONFIG_REG,
            SIS3316_ADC_CH9_12_DATAFORMAT_CONFIG_REG,
            SIS3316_ADC_CH13_16_DATAFORMAT_CONFIG_REG
        };
        for (int i = 0; i < evformatRegs.size(); i++) {
            m_pModule->register_write(evformatRegs[i], 0);
        }
    }

    // I think I can arm bank 1 and go:

    m_pModule->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK1, 0);
}
/**
 * addReadoutList:
 *    Called to set up our part of the readout list:
 *    For each group of 4 adcs ;
 *     - Add to the list a disarm just in case something we do rearms prematurely
 *       (I'm thinkinga bout the rewrites of the raw data config regs)
 *     - Figure out the total number of longs expected from that ADC.
 *     - If that's > 0, add to the list:
 *         * Data transfer initiation to FIFO for each enabled ADC
 *           with a 3usec delay after the initiation (per manual + chicken factor).
 *         *  Read from the FIFO of the appropriate # of longs.
 *     - add to the list a rewrite of the raw data config registers, mostly
 *       to ensure that we are going to write into location of the buffers.
 *     -  add to the list a re-arm of bank1.
 * 
 * @param list  - reference to the CVMUSBReadoutList we are populating.
 * 
 * 
 */
void
CSIS3316::addReadoutList(CVMUSBReadoutList& list) {
    auto base = m_pModule->getUnsignedParameter("-base");
    auto amod = CVMUSBReadoutList::a32UserData;

    // Disarm:

    list.addWrite32(base+SIS3316_KEY_DISARM, amod, 0);

    // figure out what to do for each bank:

    std::vector<int> xferRegisters = {
        SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG,
        SIS3316_DATA_TRANSFER_CH5_8_CTRL_REG,
        SIS3316_DATA_TRANSFER_CH9_12_CTRL_REG,
        SIS3316_DATA_TRANSFER_CH13_16_CTRL_REG
    };
    std::vector<int> fifoBases = {
        SIS3316_FPGA_ADC1_MEM_BASE,
        SIS3316_FPGA_ADC2_MEM_BASE,
        SIS3316_FPGA_ADC3_MEM_BASE,
        SIS3316_FPGA_ADC4_MEM_BASE
    };
    auto enables = m_pConfiguration->getBoolList("-enables");
    auto samples = m_pConfiguration->gettUnsignedList("-samples");
    for (int i =0; i < xferRegisters.size(); i++) {
        int size = groupSize(i, enables);
        if (size) {      // There are enabled ADCs:
            auto xferReg = base+xferRegisters[i];
            auto fifo = base + fifoBases[i];   // Fifo for this group.
            // See 4.6 of SIS manual.
            // the three extra words are:
            //   High bits of timestampid and format bits.
            //   Low bits of the timestamp
            //   Sample count etc.
            auto xfers = samples[i] +  3; 
            int ch = i*4;             // First chan of group.
            for (int off = 0; off < 4; off++) {   // over an adc in the group
                if (enables[ch]) {
                    // Have to tranfer and read out:
                    unsigned value(0x80000000);   // Start read xfer.
                    if (off < 2) {      // Space select 0.
                        value |= off * 0x03000000;
                    } else {       
                        value |= 0x10000000;   // Space select 1:
                        value |= (off-2) * 0x03000000;   // address in space1.

                    }
                    list.addWrite32(xferReg, amod, value);
                    list.addDelay(0xff);   //3.18usec
                    // transfer resets the FIFO so we need to read now:
                
                    list.addFifoRead32(fifo, amod, xfers);
                }
                ch++;
            }
            
        }
        // Ok I think that will read the data properly.
    }

    // Reset the starts and sizes and rearm.

    auto samples = m_pConfiguration->getUnsignedList("-samples");
    std::vector<int> bufregs = {
        SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG,
        SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG,
        SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG,
        SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG
    };
    assert(samples.size() == 4);
    for (int i =0; i < 4; i++) {
        list.addWrite32(base+bufregs[i], samples[i] << 16);
    }

    list.addWrite32(base+SIS3316_KEY_DISARM_AND_ARM_BANK1, 0);
}

/////////////////////////////// private utilities ///////////////////

/**
 * sizeGroup
 * 
 * @param groupNum   - number of the group.
 * @param enables    - vector of enables
 * @return int -  Number of 32 bit words that will be read from this group.
 * 
 * @note - we will access the -samples configuration option.
 */
int
CSIS3316::sizeGroup(int groupNum, std::vector<bool>&enables)  {
    int firstChan = gropuNum*4;
    int result(0);
    auto samples = m_pConfiguration->getUnsignedList("-samples");
    for (int i =0; i < 4; i++) {
        if (enables[firstChan+i]) {
            result += samples[groupNum] + 3;   /// 3 long header.
        }
    }
    return result;
}