
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include "CConfiguration.h"
#include "CamacSlotLimits.h"

template<class Controller, class RdoList>
CLeCroy4300B<Controller,RdoList>::CLeCroy4300B() 
   : m_pConfig(0)
{
}


template<class Controller, class RdoList>
CLeCroy4300B<Controller,RdoList>::CLeCroy4300B(const CLeCroy4300B& rhs)
   : CReadoutHardwareT<Controller,RdoList>(rhs)
{
    //    if (rhs.m_pConfig) {
    //        m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
    //    }

}

template<class Controller, class RdoList>
CLeCroy4300B<Controller,RdoList>& 
CLeCroy4300B<Controller,RdoList>::operator=(const CLeCroy4300B& rhs)
{
    return *this;
}

template<class Controller, class RdoList>
CLeCroy4300B<Controller,RdoList>::~CLeCroy4300B()
{
//    if(m_pConfig) delete m_pConfig;
}


template<class Controller, class RdoList>
void  CLeCroy4300B<Controller,RdoList>::onAttach(CReadoutModule& config)
{
//    std::cout << "Attaching CLeCroy4300B" << std::endl;
    m_pConfig = &config;
    m_pConfig->addParameter("-slot",
                            XXUSB::CConfigurableObject::isInteger,
                            &SlotLimits,
                            "1");

    m_pConfig->addIntListParameter("-pedestals",16);

    // Provides direct access to the command register
    //    m_pConfig->addIntegerParameter("-cmdregister",0);

    // The following provide flags to set the command register
    m_pConfig->addIntegerParameter("-vsn",0,255,0);

    m_pConfig->addBooleanParameter("-eclpedsub",false);
    m_pConfig->addBooleanParameter("-eclcompression",false);
    m_pConfig->addBooleanParameter("-eclenable",false);

    m_pConfig->addBooleanParameter("-camacpedsub",false);
    m_pConfig->addBooleanParameter("-camaccompression",false);
    m_pConfig->addBooleanParameter("-camacseqrdo",false);
    m_pConfig->addBooleanParameter("-camaclam",false);

    m_pConfig->addBooleanParameter("-overflowsuppress",false);

    // Determines whether the module should be cleared via 
    // the camac backplane or not
    m_pConfig->addBooleanParameter("-camacclear",false);
}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::Initialize(Controller& controller)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    std::vector<long int> peds = m_pConfig->getIntegerList("-pedestals");
    uint16_t q=0;

    // clear
    execClear(controller);

    // COnvert the pedestals:

    std::vector<int> ipeds;
    for(auto p : peds) {
        ipeds.push_back(static_cast<int>(p));
    }

    writePedestals(controller, ipeds);

//    setCommandRegister(controller);
    configureCommandRegister(controller);

}


template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::addReadoutList(RdoList& list)
{
    if (m_pConfig->getBoolParameter("-camacclear")) {
      addClear(list);
    }
}

template<class Controller, class RdoList> 
void 
CLeCroy4300B<Controller,RdoList>::writePedestals(Controller& controller, 
                                                 const std::vector<int>& peds)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    unsigned int nPeds = peds.size();
    if (nPeds>16) {
        std::cerr << "!!! CLeCroy4300B configured with more than 16 specifiers"
                  << std::endl;
        std::cerr << "    Only the first sixteen values will be used. " 
                  << std::endl;
    }

    int status = 0;
    for (int i=0; i<nPeds; ++i) {
        uint16_t qx = 0;
        uint16_t ped_i = static_cast<uint16_t>(peds.at(i));

        do {
            // clear to enable for input
            execClear(controller);

            // write the pedestal to chan i
            status = controller.simpleWrite16(slot,i,17,ped_i,qx);
            handleStatus(status);
            handleQ(qx&0x1);
        } while (qx==0);
    }
}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::execClear(Controller& controller)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    // clear to enable for input
    uint16_t qx;
    int status = controller.simpleControl(slot,0,9, qx);
    handleStatus(status);
    handleQ(qx&0x1);

}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::addClear(RdoList& list)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    // clear to enable for input
    list.addControl(slot,0,9);
}


template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::setCommandRegister(Controller& controller, 
                                                          uint16_t cmdreg)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
//    int cmdreg = m_pConfig->getIntegerParameter("-cmdregister"); 

    // Check that user has provided a value been specified.
    // If not, complain and return.
    if (cmdreg==0) {
        std::cerr << "!!! CLeCroy4300B slot " << slot;
        std::cerr << " has no command register value specified!" << std::endl;
        return;
    }

    uint16_t qx;
    uint16_t reg=0;

    // Write command register
    int status = controller.simpleWrite16(slot,0,16,cmdreg,qx);
    handleStatus(status);
    handleQ(qx&0x1);

    // wait a little
    usleep(10);

    // Read back the command register
    status = controller.simpleRead16(slot,0,0,reg,qx);
    handleStatus(status);
    handleQ(qx&0x1);

    // Is it the same
    if ( cmdreg != reg) {
        std::cerr << "!!! CLeCroy4300B::setCommandRegister(Controller&, uint16_t)";
        std::cerr << " Failed to read back same value that was previously written." << std::endl;
    }

}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::configureCommandRegister(Controller& controller)
{
    uint16_t vsn = m_pConfig->getUnsignedParameter("-vsn"); 

    uint16_t reg = vsn;
    
    if (m_pConfig->getBoolParameter("-eclpedsub")) {
      reg |= (1<<8);
    } 

    if (m_pConfig->getBoolParameter("-eclcompression")) {
      reg |= (1<<9);
    } 

    if (m_pConfig->getBoolParameter("-eclenable")) {
      reg |= (1<<10);
    } 

    if (m_pConfig->getBoolParameter("-camacpedsub")) {
      reg |= (1<<11);
    } 

    if (m_pConfig->getBoolParameter("-camaccompression")) {
      reg |= (1<<12);
    } 

    if (m_pConfig->getBoolParameter("-camacseqrdo")) {
      reg |= (1<<13);
    } 

    if (m_pConfig->getBoolParameter("-camaclam")) {
      reg |= (1<<14);
    } 

    if (m_pConfig->getBoolParameter("-overflowsuppress")) {
      reg |= (1<<15);
    } 

    setCommandRegister(controller, reg);
  
}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::handleStatus(int status)
{
    if (status<0) {
        std::cout << "Controller operation returned with error " << status << std::endl;
    }
}

template<class Controller, class RdoList>
void CLeCroy4300B<Controller,RdoList>::handleQ(uint16_t q)
{
    if (q!=1) {
        std::cout << "Controller operation returned with q!=1" << std::endl;
    }
}


