/*
@file CXLMFERA.cpp
@brief Implement FERA Readout over XLM.
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "CXLMFERA.h"

#include "CReadoutModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"
#include <XXUSBConfigurableObject.h>
#include <iostream>


#include <cstdlib>
#include <string>
#include <sstream>
#include <stdint.h>
#include <unistd.h>
#include <string.h>


// VME Interface addresses
static const uint32_t Interrupt   (0x000004); // Interrupt/reset register.
static const uint32_t BootSrc     (0x000008);
static const uint32_t ForceOffBus (0x00000c);

static const uint32_t BusAOwner   (0x010000);
static const uint32_t BusBOwner   (0x010004);
static const uint32_t BusXOwner   (0x010008);

// Boot source values
static const uint32_t BootSRAMA   (0x010000);


static const uint32_t FPGADAQEnable (0x000008);
static const uint32_t FPGAWaitLoops (0x00000c);

static const uint8_t  registerAmod     (CVMUSBReadoutList::a32UserData);
static const uint8_t  blockTransferAmod(CVMUSBReadoutList::a32UserBlock);

static const float    vmusbClockTick(12.5);
static const float    busDelay(200/vmusbClockTick); // delay to allow for arbitration


#define BLOCK_SIZE 256
//////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and other 'canonical' methods

/**
 * Construct an instance of the device.  Note that in this framework this will
 * typically only be used to make a 'template' instance which will be cloned to
 * create instances that are bound to configurations and actual hardware.
 */
CXLMFERA::CXLMFERA() : XLM::CXLM()
{
    // This will point to an instance's config base class protected).
    //  m_pConfiguration = 0;	

}


/**
 * Destruction.  If your object creatd any dynamic data it must be freed here:
 */
CXLMFERA::~CXLMFERA() 
{
    // will leak memory if we copy constructed

}
///////////////////////////////////////////////////////////////////////////////////////
// Interfaces the driver provides to the framework.

/**
 * This function is called when an instance of the driver has been associated with
 * its configuration database.  The template code stores that in m_pConfiguration
 * The configuration is a CReadoutModule which in turn is derived from
 * XXUSB::CConfigurableObject which encapsulates the configuration database.
 *
 *  You need to invoke methods from XXUSB::CConfigurableObject to create configuration parameters.
 *  by convention a configuration parameter starts with a -.  To illustrate this,
 *  template code will create a -base parameter that captures the base address of the module.
 *  In addition we'll create an -id parameter which will be the value of a marker that will
 *  be put in the event.  The marker value will be constrainted to be 16 bits wide.
 *
 * @parm configuration - Reference to the configuration object for this instance of the driver.
 */
void
CXLMFERA::onAttach(XXUSB::CConfigurableObject& configuration)
{
  
  CXLM::onAttach(configuration);      // superclass adds the -base and -firmware flags

  configuration.addParameter("-configurationID",
			     XXUSB::CConfigurableObject::isInteger,
			     NULL, "0");
  m_pConfiguration->addBooleanParameter("-forceFirmwareLoad",false);

}
/**
 * This method is called when a driver instance is being asked to initialize the hardware
 * associated with it. Usually this involves querying the configuration of the device
 * and using VMUSB controller functions and possibily building and executing
 * CVMUSBReadoutList objects to initialize the device to the configuration requested.
 * 
 * @param controller - Refers to a CCUSB controller object connected to the CAMAC crate
 *                     being managed by this framework.
 *
 */
void
CXLMFERA::Initialize(CVMUSB& controller)
{
    using std::hex;
    using std::dec;

    // ensure that red turns on BERR
    // We'll almost always need the module base address.  This line gets it from the
    // configuration database for you:
    uint32_t base = m_pConfiguration->getUnsignedParameter("-base");

    bool success = true;
    XLM::CFirmwareLoader loader(controller, base);
    try {

        std::string firmwareFname = m_pConfiguration->cget("-firmware"); 

        bool forceFirmwareLoad = m_pConfiguration->getBoolParameter("-forceFirmwareLoad");

        uint16_t qx=0;
        int status = 0;
    
        // Always load.      
        
        loader(firmwareFname);
        controller.vmeWrite32(base + XLM::Interrupt, registerAmod, XLM::InterruptResetDSP);
        // Was previously the AXLM72V_CES::Configure

        // Load firmware file and also boot the XLM
        // loadFirmware(controller,firmwareFname);
        // myloadFirmware(controller,firmwareFname);
        
        //usleep(2000000);
        controller.delay(2000);
        
        
       
        initializeFPGA(controller);
        Clear(controller);
        loader.releaseBusses();           // Be damned sure we're off the buses
    
    }
    
    catch (std::string& what) {
        std::cerr << "!!! " << what << std::endl;
        success = false;
    }

    std::cout << (success ?  " SUCCESS" : "FAILED!!!!");
    std::cout << std::endl;

  /* END MODIFICATIONS */

}
void CXLMFERA::initializeFPGA(CVMUSB& controller)
{
    // Begin XFERA::Initialize
    const uint32_t busRequestAddr = Interface();
    const uint32_t busses = REQ_A | REQ_X;


    // use a scoped bus locking scheme for exception safety
    CXLMBusController busLock(controller,*this,busses,1,busDelay); 
    
    // at this point the VMEbus owns the A and X buses... 
    // start sending commands
    const uint32_t daqEnableAddr  = FPGA() + FPGADAQEnable;
    const uint32_t nWaitLoopsAddr = FPGA() + FPGAWaitLoops;

    uint32_t enable = 1;
    uint32_t nLoopsToWait = 0;
    uint32_t data;

    int status = controller.vmeWrite32(daqEnableAddr,registerAmod,enable);
    if (status<0) {
        std::cout << "after write daqenable...error " << status << std::endl;
    }

    status = controller.vmeWrite32(nWaitLoopsAddr,registerAmod,nLoopsToWait);
    if (status<0) {
        std::cout << "after write nloops...error " << status << std::endl;
    }

    

    std::cout << " COMPLETED" << std::endl;
}

void CXLMFERA::Clear(CVMUSB& controller)
{
    const uint32_t busRequestAddr = Interface();
    const uint32_t busses = REQ_A | REQ_B;
    CXLMBusController(controller,*this,busses,1,busDelay);

    // clear first word of each sram block. 
    controller.vmeWrite32(sramA(),registerAmod,uint32_t(0x0));
    controller.vmeWrite32(sramB(),registerAmod,uint32_t(0x0));

    // bus releases automatically
}

void CXLMFERA::addClear(CVMUSBReadoutList& list)
{
    const uint32_t busRequestAddr = Interface();
    const uint32_t busses = REQ_A | REQ_B;

    addBusAccess(list,busses,0);

    // clear first word of each sram block. 
    list.addWrite32(sramA(),registerAmod,uint32_t(0x0));
    list.addWrite32(sramB(),registerAmod,uint32_t(0x0));

    addBusAccess(list,uint32_t(0),0);
}


/**
 * This method is called to ask a driver instance to contribute to the readout list (stack)
 * in which the module has been placed.  Normally you'll need to get some of the configuration
 * parameters and use them to add elements to the readout list using CCUSBReadoutList methods.
 *
 * @param list - A CCUSBReadoutList reference to the list that will be loaded into the
 *               CCUSB.
 */
void CXLMFERA::addReadoutList(CVMUSBReadoutList& list)
{
    addSramAReadout(list);

    addClear(list);
}


void CXLMFERA::addSramAReadout(CVMUSBReadoutList& list)
{
    //  this wil be a bit different than before because we wont use 
    // the XLMBusLock for bus access. This needs to be a self contained
    // set of commands.
    //
    // inhibit other FPGA and DSP from obtaining bus control
    // This arbitrates the bus master to be VMEbus unconditionally
    uint32_t inhibitAddr= Interface() + ForceOffBus;
    uint32_t enableInhibit = 1;
    list.addWrite32(inhibitAddr, registerAmod, enableInhibit);

    // Request VMEbus control of bus 
    const uint32_t busses = REQ_A;
    addBusAccess(list,busses,0);


    // >>> DEBUGGING <<<< - read the bus ownership register into the data
    // for SRAM A - should be 1 indicating ownership by VME by now:
#ifdef ENABLE_DEBUGGING    
    list.addRead32(Interface() + BusAOwner,  registerAmod);    // SRAMA bus owner.
#endif
    // Dynamically read out data stored in sramA given the number
    // stored at sramA  
    uint32_t sramAAddr = sramA();
    uint32_t numberMask = 0xFFFF; // specifies the bits to interpret as
                                  // the number of words to read.
    list.addBlockCountRead32(sramAAddr,numberMask,registerAmod);

    // Now add the block transfer whose length is dtermined by the
    // previous command. 
    list.addMaskedCountBlockRead32(sramAAddr + 1*sizeof(uint32_t),
                                   blockTransferAmod);

    // Release the bus
    addBusAccess(list,0,0/*no delay*/);
    list.addWrite32(inhibitAddr, registerAmod,uint32_t(0));

    // the prevous driver never cleared the fpga so I wont as well.

}

////////////////// Bus controller implementation

CXLMFERA::CXLMBusController::CXLMBusController(CVMUSB& controller, 
                            CXLMFERA& xlm, 
                            uint32_t request, 
                            uint32_t busInhibit, 
                            uint8_t nDelayCycles)// throw (std::string)
: m_controller(controller),
    m_interfaceAddr(xlm.Interface()),
    m_request(request)
{
    CVMUSBReadoutList list;

    if (busInhibit!=0) { // inhibit FPGA and DSP as well...
        list.addWrite32(m_interfaceAddr+ForceOffBus,registerAmod,busInhibit);

#ifdef PRINTBUSLOCK
        std::cout << "### XLM FPGA and DSP inhibit requested"; 
        std::cout << std::endl;
#endif
    }

    list.addWrite32(m_interfaceAddr, registerAmod, m_request);


    if (nDelayCycles>0) {
        list.addDelay(nDelayCycles);
    } 

    // execute the commands
    size_t nbytes = 0;
    uint32_t data=0;
    int status = controller.executeList(list,(void*)&data,sizeof(data),&nbytes);

    if (status<0) {
        std::cerr << "CXLMBusController::CXLMBusController(CVMUSB&,uint32_t,uint32_t,uint32_t,uint8_t) ";
        std::cerr << "readout list failed to execute with error " << status;
        std::cerr << std::endl;
    }

#ifdef PRINTBUSLOCK
    std::cout << "### XLM VMEBus successfully accquired : ";
    if ((m_request&0x000001)!=0) std::cout << "BUS_A ";
    if ((m_request&0x000002)!=0) std::cout << "BUS_B ";
    if ((m_request&0x010000)!=0) std::cout << "BUS_X ";
    std::cout << std::endl;
#endif
    //at this point VMEbus owns the desired bus(ses)

}

/**! Unconditionally release the bus(ses)
 */
CXLMFERA::CXLMBusController::~CXLMBusController() 
{
    releaseBusses();

}

void CXLMFERA::CXLMBusController::releaseBusses()
{

    uint32_t release = 0;
    m_controller.vmeWrite32(m_interfaceAddr,registerAmod,release);

    // unconditionally remove inhibit
    m_controller.vmeWrite32(m_interfaceAddr+ForceOffBus,registerAmod,release);

#ifdef PRINTBUSLOCK
    std::cout << "### XLM VMEBus released               : ";
    if ((m_request&0x000001)!=0) std::cout << "BUS_A ";
    if ((m_request&0x000002)!=0) std::cout << "BUS_B ";
    if ((m_request&0x010000)!=0) std::cout << "BUS_X ";
    std::cout << std::endl;
    std::cout << "### Inhibit removed from XLM FPGA and DSP" << std::endl; 
#endif

}

