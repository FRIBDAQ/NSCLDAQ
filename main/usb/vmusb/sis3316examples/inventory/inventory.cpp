/**
 * @file inventory.cpp
 * @brief Inventory the SIS3316/SIS3316-2 modules in a VME crate
 * @author Ron Fox.
 */

#include <iostream>
#include <CVUSBFactory.h>
#include <CVMUSB.h>
#include <sis_vmusb_interface.h>
#include <sis3316_class.h>
#include <sis3316.h>
#include <string>
#include <stdlib.h>
#include <stdint.h>


static const uint32_t SIS3316_MODULE_TYPE = 0x3316;

/// Needed for the sis3316_vmusb_interface:

namespace Globals {
    CVMUSB* pUSBController(0);
}


// isSIS3316
//   If a given base address is an SIS 3316
//  Construct an sis3316_adc and pass its address back.
//  ownership transfers to the caller who must delete it
//  to prevent memory leaks:
//
// @param[in] interface&  - reference to the interface object.
// @param[in] base        - possible module base address.
// @return sis3316_adc*  
// @retval nullptr - if the base is not an SIS 3316 module.
static sis3316_adc*
isSIS3316(vme_interface_class* interface, uint32_t base) {
    // The module ID register should have the top 16 bits
    // 0x3316:
    uint32_t idreg(0);
    if (interface->vme_A32D32_read(base + SIS3316_MODID, &idreg)) {
        // read failed e.g. bus error:

        return nullptr;
    }
    uint32_t id = idreg >> 16;      // Right justify the module type:
    if (id == SIS3316_MODULE_TYPE) {
        return 
    } else {
        return nullptr;
    }
}

//
//  dumpModuleInfo:
//  Outputs information about a module that has been found:
//   - base address
//   - Serial number
//   - Module type 
//       SIS3316 or SIS3316-2
//       Memory
//       Hardware variant
//   - MOdule revision.   
//   - FPGA boot CSR
//
// @param[in] base -base address (just for documentation).
// @param[in] pModule -  Pointer to module object (sis3316_adc class instance).  
//
static void
dumpModuleInfo(uint32_t base, sis3316_adc* pModule) {
    // Read the registers we need to read:

    uint32_t modid;
    if (pModule->register_read(SIS3316_MODID, &modid)) {
        std::cerr << "Failed to read module id registser\n";
        exit(EXIT_FAILURE);
    }

    uint32_t hwversion;
    if(pModule->register_read(SIS3316_HARDWARE_VERSION, &hwversion)) {
        std::cerr << "Failed to read the hardware version  register\n";
        exit(EXIT_FAILURE);
    }

    uint32_t serialno;
    if (pModule->register_read(SIS3316_SIS3316_SERIAL_NUMBER_REG, &serialno)) {
        std::cerr << "Failed to read the serial no. register\n";
        exit(EXIT_FAILURE);
    }

    uint32_t adcfpgacsr;
    if (pModule->register_read(SIS3316_ADC_FPGA_BOOTCSR, &adcfpgacsr)) {
        std::cerr << "Failed to read the ADC FPA boot csr\n";
        exit(EXIT_FAILURE);
    }


    // Pull what we intend to output:

    // Hardware rev - meaning depends on the module type
    uint32_t major_rev = (modid >> 8) & 0xff;
    uint32_t minor_rev = (modid & 0xff);
    
    // MOdule type.

    std::string modType = pModule->device_variant ? "SIS3316-2" : "SIS3316";
    uint32_t pcbversion = hwversion & 0xf;

    uint32_t mbytes = (serial_no & 00800000) ? 512 : 256;

    // here we go:

    std::cout << "\n------------------------------------------------------------------\n";
    std::cout << "Module at    : 0x" << std::hex << base << std::dec << std::endl;
    std::cout << "Serial number: " << pModule->serial_number << std::endl;
    std::cout << "Module type  : " << modType << std::endl;
    std::cout << "Hardware rev : " << major_rev << "." << minor_rev << std::endl;
    std::cout << "PCB version  : " << pcbversion << std::endl;
    std::cout << "Memory MB    : " << mbytes << std::endl;
    std::cout << "ADC FPGA CSR : 0x" << std::hex << adcfpgacsr << std::endl;


}

// Entry point Connect to the VMUSB and try all potential
// base addresses dumping the ones that work.


int main(int argc, char** argv) {
    // Open the interface:

    
    try {
        Globals::pUSBController = 
            CVMUSBFactory::createUSBController(CVMUSBFactory::local);
    }
    catch(std::string msg) {
        std::cerr << "Failed to attach a VMUSB controller: " << msg << std::endl;
        exit(EXIT_FAILURE);
    }
    sis_vmusb_interface interface();
    interface.vmeopen();
    // There can be at most 256 controllers with addresses of the form
    // 0xnn000000

    for (uint32_t i = 0; i < 256; i++)  {
        uint32_t base = i << 0x24;     // Candidate base address:

        sis3316_adc* pModule = isSIS3316(interface, base);
        if (pModule) {
            dumpModuleInfo(base, pModule);
        }
        delete pModule;                // delete 0 is a no-op.

    }
    eixt(EXIT_SUCCESS);
}



