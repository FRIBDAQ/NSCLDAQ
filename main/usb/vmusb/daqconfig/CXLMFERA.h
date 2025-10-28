
#ifndef CXLMFERA_H
#define CXLMFERA_H
#include <stdint.h>
#include <CXLM.h>
#ifdef MVLC_GENERATOR
#include <DeviceCommand.h>
#endif


#ifndef CVMUSB_H
class CVMUSB;
#endif

#ifndef CREADOUTMODULE_H
class CReadoutModule;
#endif

#ifndef CVMUSBREADOUTLIST_H
class CVMUSBReadoutList;
#endif

/**! An XLM driver for FERA ECLLine readout
*/
class CXLMFERA : public XLM::CXLM 
{

public:
    CXLMFERA();
    virtual ~CXLMFERA();
#ifdef MVLC_GNERATOR
private:
#endif
    CXLMFERA(const CXLMFERA& rhs);
private:
    CXLMFERA& operator=(const CXLMFERA& rhs); // assignment not allowed.
    int operator==(const CXLMFERA& rhs) const;	  // Comparison for == and != not suported.
    int operator!=(const CXLMFERA& rhs) const;


public:
#ifdef MVLC_GENERATOR
    virtual void onAttach(XXUSB::CConfigurableObject& configuration);
#else
    virtual void onAttach(CReadoutModule& configuration);
#endif
    virtual void Initialize(CVMUSB& controller);
    virtual void addReadoutList(CVMUSBReadoutList& list);
#ifndef MVLC_GENERATOR
    virtual CReadoutHardware* clone() const; 
#endif
private:

    // initialization routines
    bool isConfigured(CVMUSB& controller);
    void bootFPGA(CVMUSB& controller);
    void initializeFPGA(CVMUSB& controller);
    void myloadFirmware(CVMUSB& controller, std::string fname);

    // readout routines
    void addSramAReadout(CVMUSBReadoutList& list);

    // for completeness we define a clear, though it does not get used
    void Clear(CVMUSB& controller);
    void addClear(CVMUSBReadoutList& list);

private:
    /**! Scoped bus access for XLM module
     *   An RAII inspired bus access to ensure that bus ownership is always 
     *   released.
     * 
     *   This is important considering exceptional exits to the 
     *   various XLM routines could leave the XLM in a bad state (i.e.
     *   FPGA and DSP are inhibited from becoming bus masters, VMEbus
     *   never relinquishes control of its owned busses).
     */

    class CXLMBusController
    {
        private:
            CVMUSB&  m_controller;      ///< The VMUSB controller
            uint32_t m_interfaceAddr;   ///< The address of interface + base
            uint32_t m_request;         ///< The bit pattern of the requested busses

        private:
            /**! Default constructor is not sensible */
            CXLMBusController();
            /**! Copy constructor is not sensible */
            CXLMBusController(const CXLMBusController&);
            /**! Assignment is not sensible */
            CXLMBusController& operator=(const CXLMBusController&);

        public:
            /**! \brief Construct ... establish "lock"
             *
             * Request XLM interface to arbitrate control of specified
             * busses to the VMEbus. If desired the user can also inhibit 
             * FPGA and DSP from competing for bus ownership. Following
             * a request, a delay is written to the VMUSB
             * 
             *   \param controller the VMUSB module to communicate through
             *   \param baseAddr the XLM base address
             *   \param request the a data word specifying the busses to "lock"
             *   \param busInibit flag to inhibit FPGA and DSP from competing to own bus
             *           [default=0 (do not inhibit)]
             */
            CXLMBusController(CVMUSB& controller, 
                CXLMFERA& xlm, 
                uint32_t request, 
                uint32_t busInhibit=0x0,
                uint8_t nDelayCycles=0); // throw(std::string);

            /**! Unconditionally release the bus(ses)
             * 
             * Calls releaseBusses(). See that documentation.
             */
            ~CXLMBusController(); 

        private:

            /**! \brief Unconditionally release busses 
             *   Release ownership of all busses and clear FPGA and DSP inhibit.
             */
            void releaseBusses();
    };

};

#ifdef MVLC_GENERATOR

//// Creational class:

class XLMFERACmd : public DeviceCommand {
public:
    XLMFERACmd(CTCLInterpreter& interp, TCLConfigParser& parser);
    ~XLMFERACmd();

protected:
  virtual CReadoutModule* createDevice(std::string name);    
};
#endif

#endif