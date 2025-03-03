/**
 * @file CSIS3316EventSegment.cpp
 * @brief implements the event segment that will readout a single SIS3316.
 * @author Ron Fox <fox at frib dot msu dot edu>
 * 
 * 
 *  This software is Copyright by the Board of Trustees of Michigan
 *  State University (c) Copyright 2025
*
*  You may use this software under the terms of the GNU public license
*   (GPL).  The terms of this license are described at:
*
*    http://www.gnu.org/licenses/gpl.txt
*
*    Author:
*            Ron Fox
*            Facility for Rare Isotop Beams
*            Michigan State University
*            East Lansing, MI 48824-1321
*
 * 
 */
#include "CSIS3316EventSegment.h"
#include <XXUSBConfigurableObject.h>
#include <sis_vmusb_interface.h>
#include "sis3316.h"
#include "sis3316_class.h"
#include "CVMUSB.h"
#include <CVMUSBReadoutList.h>
#include <Globals.h>                  // THE VMUSB controller will be there:
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <sstream>
#include <unistd.h>

// Static data:
//   The parameter constraints:

// CLock sources:

static const char* ClockSources[] = {
    "fp", "250MHz","125MHz", "50MHz", "25MHz", "12.5MHz", NULL
};

static const uint64_t Zero(0);    // Shared low limit for many things.
static const uint64_t MaxSamples(65535);
static const uint64_t MaxId(127);
static const uint64_t MaxPretrigger(0x3fff);   // 14 bits of pre-trigger.


///////////////////////// Canonical implementations ////////////////////////////

/**
 * Constructor:
 *    @param name - name given to both the configuration and the module.
 * 
 *  Null out pointers to the VME and module objects but save the name and
 * make a new configuration to save.
 */
CSIS3316EventSegment::CSIS3316EventSegment(const char* name) :
    m_name(name), m_pConfiguration(nullptr), m_pVME(nullptr), m_pModule(nullptr)
{
    m_pConfiguration = new XXUSB::CConfigurableObject(m_name);
    setupConfiguration();                         // Set up our configuration parameters.
}
/**
 *  destructor
 *     
 * 
 */
CSIS3316EventSegment::~CSIS3316EventSegment() {
    delete m_pConfiguration;
    delete m_pModule;                   // Module before VME.
    delete m_pVME;
}

///////////////////////// Selector implementation ////////////////////////

/**
 *  getConfiguration
 * 
 * @return XXUSB::CConfigurableObject* - pointer to our configuration.
 * 
 * This is going to allow the Tcl interpreter to configure us.
 */
XXUSB::CConfigurableObject*
CSIS3316EventSegment::getConfiguration() {
    return m_pConfiguration;
}
/**
 *  getName
 *     @return std::string - copy of our name
 */
std::string
CSIS3316EventSegment::getName() const {
    return m_name;
}
/////////////////////////////////// Implementation methods ///////////////////////////////////////

/**
 * Initialize 
 *     Initialize the module.  The configuration has been set up by the 
 * compound event segment prior to being called so everything in m_pConfiguration 
 * matches the the desired module configuration.
 * At this point the VMUSB controller will have been set up as well.
 */
void
CSIS3316EventSegment::initialize() {

    // WE can build the module and controller:

    delete m_pModule;
    delete m_pVME;
    m_pVME = new sis_vmusb_interface;          // Fishes the VMUSB from Globals.
    m_pModule = new sis3316_adc(
        m_pVME, m_pConfiguration->getUnsignedParameter("-base")  // Our base was configured.
    );


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
    usleep(10*1000);                                   // wait. for it
    m_pModule->register_write(SIS3316_KEY_ADC_FPGA_RESET, 0);  // Reset the FPGAs.
    usleep(10*1000);                                   // Wait for it.
    m_pModule->register_write(SIS3316_KEY_DISARM, 0); // Keep disarmed.
    m_pModule->register_write(SIS3316_KEY_TIMESTAMP_CLEAR, 0); 

    // Set up the clock source:

    int whichClock = m_pConfiguration->getEnumParameter("-clock", ClockSources);
    if (std::string("fp") == ClockSources[whichClock]) {
        // Set up for NIM clock input:

        m_pModule->register_write(SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL, 3);
    } else {
        // internal clock:

        m_pModule->register_write(SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL, 0);
        
        // Now set the sample freq:

        std::string freq = ClockSources[whichClock];
        unsigned int hs_div, n1div;
        double fft_freq;
        if (freq == "250MHz") {
            m_pModule->get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERATE_250MSPS, 
                &hs_div, &n1div, &fft_freq
            );
        } else if (freq == "125MHz") {
            m_pModule->get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERATE_125MSPS, 
                &hs_div, &n1div, &fft_freq
            );

        } else if (freq == "50MHz") {
            m_pModule->get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERATE_50MSPS, 
                &hs_div, &n1div, &fft_freq
            );

        } else if (freq == "25MHz") {
            m_pModule->get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERATE_25MSPS, 
                &hs_div, &n1div, &fft_freq
            );
            
        } else if (freq == "12.5MHz") {
            m_pModule->get_SI570_oscillator_hs_div_and_n1_div_values(
                SIS::ADC::SIS3316::SAMPLERATE_12P5MSPS, 
                &hs_div, &n1div, &fft_freq
            );

        } else {
            std::stringstream strmsg;
            strmsg << freq << " Is not a supported clock frequency\n";
            throw strmsg;
        }
	// Set the clock frequency:
        // Disabled for now.
	// m_pModule->change_frequency_HSdiv_N1div(0, hs_div, n1div);
	
        // Set up the header ids for the ADC groups:

        auto id = m_pConfiguration->getUnsignedParameter("-id");
        std::vector<int> idregs = {
            SIS3316_ADC_CH1_4_CHANNEL_HEADER_REG,
            SIS3316_ADC_CH5_8_CHANNEL_HEADER_REG,
            SIS3316_ADC_CH9_12_CHANNEL_HEADER_REG,
            SIS3316_ADC_CH13_16_CHANNEL_HEADER_REG
        };

        for (int i =0; i < 4; i++) {
            // We get to write bits 4-11 if the id and
            // bits 2,3 are the group.  The bottom 2 bits are the
            // adc within the group.  Note that
            // all of this is shifted 20 bits up in to the register. 
            // See the manual:  6.47
            m_pModule->register_write(idregs[i], id << 24 | (i << 22));
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
        assert(pretriggers.size() == 4);
        for (int i=0; i < 4; i++) {
            m_pModule->register_write(pretrigregs[i], pretriggers[i]); 
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


        // Set external trigger  bit 4 is NIM INPUT as trigger enable (actuall
        // Trigger function).

        m_pModule->register_write(SIS3316_NIM_INPUT_CONTROL_REG, 0x10);
        

        // Note 0x100 is FP trigger enable according to Tino.
        // The write to the T0 select register allows a stretched trigger-> ADCFPGA
        // to be monitored on TO
        // The write to U0 select makes it monitor sample logic  ready.
        // TO - trigger signal (unstretched).
        // C0 - Sampling clock.
        // U0 - sample logic ready.

        m_pModule->register_write(SIS3316_ACQUISITION_CONTROL_STATUS, 0x100);
        m_pModule->register_write(SIS3316_LEMO_OUT_TO_SELECT_REG, 0x04000000);
        m_pModule->register_write(SIS3316_LEMO_OUT_UO_SELECT_REG, 0x100);
        m_pModule->register_write(SIS3316_LEMO_OUT_CO_SELECT_REG, 1);
        
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
            int firstchan = i*4;       // offset to ch 0 in the enables array
            for(int ch = 0; ch < 4; ch++) {
                if (enables[firstchan+ch]) {
                    mask |= 8 << (ch*8);    // Enable trigger response
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
    // Reset the transfer FSMs.
    
    std::vector<int> xferRegisters = {
        SIS3316_DATA_TRANSFER_CH1_4_CTRL_REG,
        SIS3316_DATA_TRANSFER_CH5_8_CTRL_REG,
        SIS3316_DATA_TRANSFER_CH9_12_CTRL_REG,
        SIS3316_DATA_TRANSFER_CH13_16_CTRL_REG
    };
    // Reset the transfer state machines:

    for (auto r: xferRegisters) {
	m_pModule->register_write(r, 0);
    }
    // I think I can arm bank 1 and go:

    m_pModule->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK1, 0);
}
/**
 * disable
 *    Turn off the module.... Done by disarming.  The 
 * configuration is assumed to be loaded as is the VMUSB controller.... in fact
 * we assume that initialize() has also been called alread.ACCUMULATOR_MAX_LENGTH
 * 
 * 
 */
void
CSIS3316EventSegment::disable() {
    m_pModule->register_write(SIS3316_KEY_DISARM, 0);
}
/**
 * read
 *    Read out the module into the buffer we're pointed at.  Only the enabled
 * channels will be read.  The ony identifying information will be what the
 * module provides in data headers.
 * 
 * @param pBuffer -  pointer to where data should be stored.
 * @param maxwords - Maximum number of 16 bit words that will fit in the buffer.
 * @return size_t - Number of 16 bit words that were read (or is it 8 bit bytes?).
 */
size_t
CSIS3316EventSegment::read(void* pBuffer, size_t maxwords) {
    size_t totalWords = computeTotalWords();
    uint32_t*   pLongBuf = reinterpret_cast<uint32_t*>(pBuffer);   
    size_t      nRead(0);
    CVMUSB*    pController = Globals::pUSBController;
    if (totalWords*2 > maxwords) {
        std::stringstream strMsg;
        strMsg << m_name << " wants to read out "  << totalWords
            << " that's  more than the remaining buffer words which are: " << maxwords;
        std::string msg(strMsg.str());
        throw std::length_error(msg);
    }
    // Some of the stuff we do must be done without the help of the adc class:

    auto base = m_pConfiguration->getUnsignedParameter("-base");
    auto amod = CVMUSBReadoutList::a32UserData;
    auto blockAmod = CVMUSBReadoutList::a32UserBlock;

    // Module should now be disarmed but...

    m_pModule->register_write(SIS3316_KEY_DISARM, 0);

    // We need to figure out what to do for each enabled channel, notiing that the channels
    // are arranged in groups of 4 (per ADC FPGA) and therefore share registers.

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
    std::vector<int> statusReg = {
	SIS3316_DATA_TRANSFER_ADC1_4_STATUS_REG,
	SIS3316_DATA_TRANSFER_ADC5_8_STATUS_REG,
	SIS3316_DATA_TRANSFER_ADC9_12_STATUS_REG,
	SIS3316_DATA_TRANSFER_ADC13_16_STATUS_REG
    };
    auto enables = m_pConfiguration->getBoolList("-enable");          // 16 of these.
    auto samples = m_pConfiguration->getUnsignedList("-samples");    // 4 of these

    // Loop over the groups:

    for (int group = 0; group < 4; group++) {
        int firstchan = group*4;                                  // Init to first chan in group.
        int groupLongs = sizeGroup(group);                        // Total longs to read in group:
        if (groupLongs > 0) {                                         // Group has enabled channels.
            auto xferReg = xferRegisters[group];              // group xfer control register.
            auto status = statusReg[group];
            auto fifo    = base + fifoBases[group];                  // FIFO address for the group.  
            for(int i = 0; i < 4; i++) {                          // Loop over channels:
                int chan = firstchan+i;                           // Absolute channel #.

                // set up and start the transfer:
                unsigned value (0x80000000);                     // Code to start a transfer.
                if (enables[chan]) {                              // need to read the channel.
                    if (i < 2) {
                        value |= i * 0x03000000;                 // Offset in address space 0.
                    } else  {
                        value |= 0x10000000;                     // Space select 1.
                        value |= (i -2) * 0x03000000;            // offset.
                    }
                    // Start the transfer:

                    m_pModule->register_write(xferReg, value);  // Start the transfer -> fifo.
                    // Spin for it to finish:

                    while (true) {
                        UINT datum;

                        m_pModule->register_read(status, &datum);
                        if ((datum & 0x80000000) == 0)   break;  // done.
                    }
                    // Read the FIF into the buffer.

                    size_t numToRead = samples[group] + 3;       // Total words to read.
                    size_t nTransferred;
                    pController->vmeFifoRead(fifo, blockAmod, pLongBuf, numToRead, &nTransferred);
                    pLongBuf+= nTransferred;
                    nRead += nTransferred;

                }

            }
        }
    }
    // Reset the offsets to zero reset the sample counts nad re-arm:

    std::vector<int> bufregs = {
        SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG,
        SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG,
        SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG,
        SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG
    };
    assert(samples.size() == 4);
    for (int i =0; i < 4; i++) {
        m_pModule->register_write(bufregs[i],  samples[i] << 16);
    }

    m_pModule->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK1, 0);


    return nRead * 2;                      // 16 bit words.
}
/**
 *  readable
 *    @return true - if the module is readable.
 */
bool
CSIS3316EventSegment::readable() {
    if (!m_pModule) return false;        // no module so not readable.
    uint32_t acqreg;
    m_pModule->register_read(SIS3316_ACQUISITION_CONTROL_STATUS, &acqreg);

    uint32_t thresh = acqreg & 0x00080000;    // Threshold made.
    uint32_t sampling = acqreg & 0x00040000;
    return (thresh != 0) && (sampling == 0);
}
///////////////////////////////// Private Utilities //////////////////////////////////////////////

/**
 * setupConfiguration
 *    Interact with m_pConfiguration to setup the configuration parameters and their constraints.
 */
void
CSIS3316EventSegment::setupConfiguration() {
    m_pConfiguration->addIntegerParameter("-base", 0);
    m_pConfiguration->addEnumParameter("-clock", ClockSources, "250MHz" );
    m_pConfiguration->addIntListParameter(
        "-samples",  Zero, MaxSamples, 4,4,4, MaxSamples);
    m_pConfiguration->addIntegerParameter("-id", 0,  127, 0);
    m_pConfiguration->addIntListParameter(
        "-pretrigger",  0, MaxPretrigger ,4,4,4, MaxPretrigger/4 );
    m_pConfiguration->addBoolListParameter("-enable", 16, true);
    m_pConfiguration->addIntListParameter(
        "-dcoffset", 0, 0xffff, 16,16,16, 0    // Each chan has a DC offset. 
    );
}

/**
 *  computeTotalWords
 *     Compute the total number of 32 bit words we'll readout
 * 
 * @return size_t
 */
size_t
CSIS3316EventSegment::computeTotalWords() {
    size_t result(0);
    for (int i = 0; i < 4; i++) {
        result += sizeGroup(i);
    }

    return result;
}
/**
 *  sizeGroup
 *    For each channel enabled in the group, the number of longs read is the sample size + 3 header
 * longs.
 * @param group - 4 channel group number (0-3).  Up to the caller to get this right.
 * @return size_t - number of 32 bit words that will be read from a 4 channel group.
 */
size_t
CSIS3316EventSegment::sizeGroup(int group) {
    unsigned samples = m_pConfiguration->getUnsignedList("-samples")[group];  // Samples are a per group thing.
    auto enables = m_pConfiguration->getBoolList("-enable");

    int ch = group*4;      // First channel in group.
    size_t result(0);
    for (int i = 0 ; i < 4; i++) {                  // loop over chans in group.
        if (enables[ch]) {
            result += samples+3;                    // Contribution of an enabled channel.
        }
        ch++;
    }
    return result;
}
