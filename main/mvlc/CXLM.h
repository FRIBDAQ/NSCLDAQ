/*
@file CXLM.h 
@brief Header file for base class for XLM FPA board.
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

#ifndef MVLC_CXLM_H
#define MVLC_CXLM_H

#include "CReadoutModule.h"
#include <stdint.h>
#include <string>
#include <vector>

// Forward class definitions:

class CVMUSB;
class CVMUSBReadoutList;
namespace XXUSB {
  class CConfigurableObject;
}


/*!
   Provides base class support for XLM based readout targets.
   We do register (by chaining to our onAttach) two configuration parameters:
\verbatim
   parametr       Type       Default     Meaning
   -base          uint32_t   0           Base address of the module.
   -firmware      string     ""          Path to XLM firmware file.
   -loadfirmware  boolean    ""          Whether to load firmware or not on initialization
\verbatim


*/
namespace XLM
{

class CXLM : public CReadoutModule
{
protected:			// Data available to derived classes:

  XXUSB::CConfigurableObject* m_pConfiguration;

public:				// 'constants'.
  // Bus access bits:

  static const uint32_t  REQ_X;		// FPGA bus.
  static const uint32_t  REQ_A;		// SRAM A bus
  static const uint32_t  REQ_B;		// SRAM B bus

public:				// Canonicals:
  CXLM();
  
  virtual ~CXLM();
  

 private:			// Unimplemented canonicals:
  CXLM(const CXLM& rhs);
  CXLM& operator=(const CXLM& rhs);
  int operator==(const CXLM& rhs) const;
  int operator!=(const CXLM& rhs) const;

  // Element of the standard readout hardware interface we implement:

public:
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);  

  // XLM support functions derived classes can use these:
protected:
  void loadFirmware(CVMUSB& controller,  std::string path) ;
  void accessBus(CVMUSB& controller,  uint32_t accessPattern);

  void addBusAccess(CVMUSBReadoutList& list, uint32_t accessPattern, 
		    uint8_t delay);

  uint32_t sramA();		// Return base address of SRAM A
  uint32_t sramB();		// Return base address of SRAM B
  uint32_t FPGA();		// Return base address of FPGA 'registers'.
  uint32_t Interface();		/* Return the base address of the interface registers. */
};


///////////////////////////////////////////////////////////////////////////////
//////////////////////// NAMESPACE MEMBERS ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//  Base addresses of 'unstructured' areas.

const uint32_t  SRAMA(0x000000);	//  Base address of static ram A
const uint32_t  SRAMB(0x200000);	//  Base address of static RAM B
const uint32_t  FPGABase (0x400000); //  Base address of FPGA `register' set.
const uint32_t  DSP  (0x600000); //  Base address of DSP interface.
const uint32_t  InterfaceBase(0x800000);

// Interface layout:

const uint32_t  BusRequest (0x800000); // Register for submitting bus requests.
const uint32_t  Interrupt  (0x800004); // Interrupt/reset register.
const uint32_t  FPGABootSrc(0x800008); // Select boot source for FPGA.
const uint32_t  ForceOffBus(0x80000c); // Register to force FPGA/DSP off bus.
const uint32_t  BUSAOwner  (0x810000); // Shows who has bus A (SRAM A)
const uint32_t  BUSBOwner  (0x810004); // Shows who has bus B (SRAM B)
const uint32_t  BUSXOwner  (0x810008); // Shows who has bus X (FPGA).
const uint32_t  IRQSerial  (0x820048); // Write for IRQ id reads serial number.
const uint32_t  POLLISR    (0x820824); // 'mailbox' betweenFPGA and DSP.

// Note that the REQ_A/B/X definitions above define the bits in the bus request register
// therefore we don't define these bits here.

// Bits in Interrupt register; Note these are 'negative logic bits' in that to assert
// the reset, the bit shown below should be set to zero.

const uint32_t InterruptResetFPGA    (0x00000001);
const uint32_t InterruptResetDSP     (0x00000002);
const uint32_t InterruptInterruptFPGA(0x00010000);
const uint32_t InterruptInterruptDSP (0x00020000);

// Boot source register contents.  This defines where the FPGA loads its
// microcode from.  There are 4 ROM locations as well as the possibility
// to load the microcode in to SRAM A.  This is coded rather than bits.

const uint32_t BootSrcRom0 (0x00000000);	// Load from sector 0 of PROM.
const uint32_t BootSrcRom1 (0x00000001);	// Load from sector 1 of PROM.
const uint32_t BootSrcRom2 (0x00000002); // Load from sector 2 of PROM.
const uint32_t BootSrcRom3 (0x00000003); // Load from sector 3 of PROM.
const uint32_t BootSrcSRAMA(0x00010000);	// Load from SRAM A image.

// The ForceOffBus register only has a single bit.... the LSB.  When set, the FPGA
// and DSP are forced off the bus.  When clear they are allowed to arbitrate for the bus.
// XLM Manual 4.3.1.4 states that even when that bit is set the VME bus host must arbitrate
// for the bus via the BusRequest register.

const uint32_t ForceOffBusForce(0x00000001); // Force all but VME off bus.

// BUS*Owner values.  These describe who is actually the owner of the specific bus.

const uint32_t BusOwnerNone(0);	// Bus not owned.
const uint32_t BusOwnerVME (1);	// VME host owns bus.
const uint32_t BusOwnerFPGA(2);	// Bus owned by FPGA.
const uint32_t BusOwnerDSP (3);	// Bus owned by DSP.

/*! \brief A class that handles the loading of firmware for a generic XLM
 */
class CFirmwareLoader 
{
  private:
    CVMUSB&   m_ctlr;       //!< non-owning reference to a VMUSB
    uint32_t  m_baseAddr;   //!< module base address

  public:

    /*! \bried Constructor
     *
     * \param ctlr     - a VMUSB
     * \param baseAddr - base address
     */
    CFirmwareLoader(CVMUSB& ctlr, uint32_t baseAddr);

    /*! \brief Entry point for the load operation
     *
     *  \param pathToFirmware  location of firmware file 
     */
    void operator()(const std::string& pathToFirmware);

    /*! \brief Checks whther register contains expected value
     *
     *  \param signatureAddr      address of register (e.g. base+offset)
     *  \param expectedSignature  value that should be at signaturedAddr
     *
     *  \retval true - value at address matches expectedSignature
     *  \retval false - otherwise
     */
    bool validate(uint32_t signatureAddr, uint32_t expectedSignature);

    uint32_t getBaseAddress() const { return m_baseAddr; }
    /**! \brief Acquire busses and hold FPGA interrupt 
     *
     *  - Acquires bus A for SramA communication.
     *  - Forces the FPGA off the bus.
     *  - Verifies that we own bus A 
     */
    void acquireBusses();

    /**! \brief Release busses and FPGA interrupt */
    void releaseBusses();
  private:
    /**! 
     * Implements the firmware loading algorithm for the  
     *
     *   \param filename   path to the firmware file
     */ 
    void loadFirmware(const std::string& pathToFirmware);

    /**! \brief Read a 32-bit register from specified address
     *
     * \param controller      the VM-USB device 
     * \param base            the base address of xlm
     *
     */
    uint32_t readFwSignature(uint32_t signatureAddr);

    /**! \bried Loads the contents of the file into memory
     *
     * \param path     - path to the firmware file
     * \param contents - allocated buffer to store the file content
     * \param nBytes   - bytes allocated to contents buffer
     */
    void loadFile(const std::string& path, uint8_t* contents, uint32_t nBytes);

    /**! \brief Transforms bytes in firmware file to sram loadable image
     *
     *  \param sramImage   - pre-allocated buffer to fill
     *  \param fileImage   - file contents (presumably filled by loadFile)
     *  \param nBytes      - # bytes in fileImage
     */
    void remapBits(uint32_t* sramImage, uint8_t* fileImage, uint32_t nBytes);

    /**! \brief Algorothim to actually write to devices
     *
     * Uses the precomputed sramImage and writes it to the device's sramA.
     *
     * \param dest_addr   address to begin writing to
     * \param image       sram image
     * \param nBytes      # bytes in sram image to write
     */
    void loadSRAM0(uint32_t dest_addr, uint32_t* image, uint32_t nBytes);
    void loadSRAM1(uint32_t dest_addr, uint32_t* image, uint32_t nBytes);

    /**! \brief Compute size of file
     * 
     * The file must already exist and  be a valid target for 
     * stat(2). As this is used in the firmware load process, this has typically been assured by
     * a call to validFirmwareFile.
     *
     * @param path - Absolute or relative path to the firmware file.
     * @return size_t
     * @retval Number of bytes in the file.  This includes 'holes' if the file is spares.
     * @throw std::string - if stat fails.
     */
    uint32_t fileSize(const std::string& path);

    /**! \brief Find the end of the header 
     * 
     * Searches for the end of the header marked by 0xff.
     *
     * \param contents  pointer to start of buffer holding firmware file contents
     *
     * \return location of 0xff
     * */
    uint8_t* skipHeader(uint8_t* contents);



    /**! \brief Changes the boot source of the FPGA */
    void setBootSource();

    /**! \brief Actually boots the device 
     * 
     * Sets and then releases the FPGA Reset register bits.
     * DSP is not booted.
     */ 
    void bootFPGA();
}; // end of FirmwareLoader class

///////////////////////////////////////////////////////////////////////////////
//////////////////////// NAMESPACE FUNCTIONS //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**! \fn accessBus(CVMUSB& ctlr, uint32_t baseAddr, uint32_t accessPattern)
*
* Obtain ownership of select internal busses in the XLM (A, B, and X)  
*
*   \param controller     the VM-USB device to communicate through
*   \param baseAddr       the base address of the target XLM
*   \param accessPattern  the bit-wise OR of CXLM::REQ_A, CXLM::REQ_B, 
*                         and CXLM::REQ_X
*/ 
extern 
void accessBus(CVMUSB& controller, uint32_t base, uint32_t accessPattern);

/**! \fn addBusAccess(CVMUSBReadoutList& ctlr, uint32_t baseAddr, 
                      uint32_t accessPattern, uint8_t delay)
*
* Add commands to a readout list that will acquire ownership of select internal
* busses in the XLM (A, B, and X)  
*
*   \param controller     the VM-USB device to communicate through
*   \param baseAddr       the base address of the target XLM
*   \param accessPattern  the bit-wise OR of CXLM::REQ_A, CXLM::REQ_B, 
*                         and CXLM::REQ_X
*   \param delay          delay length in units of 12.5 ns to delay proceeding 
*                         execution of subsequent VME commands in the stack 
*/ 
extern 
void addBusAccess(CVMUSBReadoutList& list, uint32_t base, uint32_t accessPattern, 
                  uint8_t delay);

///////////////////////////////////////////////////////////////////////////////
//////////////////////// NAMESPACE UTILITY FUNCTIONS //////////////////////////
///////////////////////////////////////////////////////////////////////////////
namespace Utils 
{
  extern bool validFirmwareFile(std::string name, std::string value, void* arg);
}




} // end of namespace

#endif
