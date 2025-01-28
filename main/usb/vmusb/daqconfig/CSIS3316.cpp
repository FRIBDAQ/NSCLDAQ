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
static const uint64_t MaxId(4095);
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
            m_pModule->register_write(idregs[i], ids[i] << 20);
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
            m_pModule->register_write(bufregs[i], samples[i] << 16)l
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
        
        // Set the enables for each ADC.



    }


    

}
