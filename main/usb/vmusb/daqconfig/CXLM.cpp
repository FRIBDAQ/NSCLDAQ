/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <config.h>
#include "CXLM.h"

#include <CReadoutModule.h>

#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <os.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <sstream>
#ifdef MVLC_GENERATOR
#include <XXUSBConfigurableObject.h>
using  XXUSB::CConfigurableObject;
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////
/////////////////////  File level constants /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// The XLM memory layout relative to the base address:

// Address modifiers we are going to use:

static const uint8_t   registerAmod(CVMUSBReadoutList::a32UserData);
static const uint8_t   sramaAmod(CVMUSBReadoutList::a32UserData);
static const uint8_t   blockTransferAmod(CVMUSBReadoutList::a32UserBlock); 


// The IRQ/serial number register is some mess of bits; both in/out
// we're just going to define a function for extracting the serial number from a read:

static inline uint32_t serial(uint32_t rawRegister)
{
  return (rawRegister && 0x1ff) >> 16; // Bits 16-25 of the 32 bit data word 4.3.1.6 Xlm manual
}

/////////////////////////////////////////////////////
// Begin namespace XLM implementation
//
namespace XLM 
{

//////////////////////////////////////////////////////////////////////////////////
/////////////////////  Class level constants  ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//  These constants define bits in the XLM bus request/ownership register:

const uint32_t CXLM::REQ_A(0x00000001);
const uint32_t CXLM::REQ_B(0x00000002);
const uint32_t CXLM::REQ_X(0x00010000);

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////// Canonical methods ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

/*!
  Default construction just ensures that references to the configuration will
  SEGFault until the configuration has been attached:
*/
CXLM::CXLM() :
  m_pConfiguration(0)
{}
#ifndef MVLC_GENERATOR
/*!
  The copy constructor will clone the configuration if it's define in the
  object being copied:
  @param rhs  The object that we will be making a copy of.
*/
CXLM::CXLM(const CXLM& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*rhs.m_pConfiguration);
  }
}
#endif
/*!
  Destruction:
  Well if we were copy constructed, we will leak our configuration.
  Otherwise the configuration is managed by the guy that owned us.
  If ambitious we could deal with this by having a flag to determine if
  we were copy constructed... however for now just let this defect be.

*/
CXLM::~CXLM()
{
}
#ifndef MVLC_GENERATOR
/*!
   Assignment.. we're just going return *this .. really assignment should
   not have been implemented. There's evidently an infinite recursion loop if we
   actually try to implement operator=.
   @param rhs What we are `assigning' to *this.
   @return CXLM&
   @retval *this
*/
CXLM&
CXLM::operator=(const CXLM& rhs)
{
  return *this;
}
#endif
////////////////////////////////////////////////////////////////////////////////////
///////////////// CReadoutHardware interface methods   /////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/*
   Note that CXLM is an abstract base class.  This means that we won't be 
   implementing all of the CReadoutHardwareFunctions... only a minimal set that
   we need for all XLM applications.
*/

/*!
  Handle the attachment of the configuration object to our object.
  We're going to define two parameters, the base address and the path to the
  firmware file.  The firmware file will be validated with our custom validator.
  @param configuration Reference to the configuration object.

*/
void
#ifdef MVLC_GENERATOR
CXLM::onAttach(CConfigurableObject& configuration)
#else
CXLM::onAttach(CReadoutModule& configuration)
#endif
{
  m_pConfiguration = &configuration; // Save for later use

  // Define our base configuration parameters.

  configuration.addParameter("-base", XXUSB::CConfigurableObject::isInteger, NULL, "0");

  configuration.addParameter("-firmware",
			     Utils::validFirmwareFile, NULL, "");

  // whether to load firmware or not
  configuration.addBooleanParameter("-loadfirmware", true);
}

/////////////////////////////////////////////////////////////////////////////////
///////////////// Utility functions for derived classes /////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*!
  Load a firmware file into the FPGA.  Normally this will be used by:
  loadFirmware(vmusb, cget("-firmware")); .. that is loading the firmware that
  has been configured via the -firmware configuration parameter.

  @param controller  - Reference to a CVMUSB controller object.
  @param path Path to the firmware file.  This has to have been validated as existing
              and readable by the caller.  Note that if this value came from
	      -firmware, this has already been done by the configuration subsystem.
  @exception std::string If there is an error.
*/
void 
CXLM::loadFirmware(CVMUSB& controller, string path) 
{
  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
#ifdef MVLC_GENERATOR
  std::cout << "Will generate codee to load firmware from  " << path << " for module at "
    << std::hex << base << std::dec <<std::endl;
#else    
  std::cout << "XLM::loadFirmware " << path << " for module at "
    << std::hex << base << std::dec <<std::endl;
#endif    
  CFirmwareLoader loader(controller, base);
  loader(path);
}

/*!
  Access some combination of the busses.  The assumption is that since this
  is a one shot, and since I'm ensured that I'll get the bus within 200ns, the host
  will not be able to turn the read around faster than that and therefore I don't need
  to check that I actually have the bus.

  Note this can be used to release the busses by not setting the appropriate 
  bus request bits.

  @param controller - VMUSB controller object.
  @param accessPattern - bitwise oro of one of the following:
  - CXLM::REQ_X   - Request the X (FPGA) bus.
  - CXLM::REQ_A   - Request the A (SRAM A) bus.
  - CXLM::REQ_B   - Request the B (SRAM B) bus.

*/
void
CXLM::accessBus(CVMUSB& controller, uint32_t accessPattern)
{
  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");
  XLM::accessBus(controller, base, accessPattern);
}

/*!
  Adds a bus access request to a readout list.
  @param list - Reference to a VMUSBReadout list to which the appropriate instructions will be added.
  @param accessPattern - The requested access pattern, See accessBus above for a description of this.
  @param delay - Delay to insert after the bus request.  This should be the worst case time between
                 arbitration request and grant..determined by the FPGA firmware you are using.
		 delay is in 12.5ns units.
*/
void
CXLM::addBusAccess(CVMUSBReadoutList& list, uint32_t accessPattern,
		   uint8_t delay)
{
  uint32_t base = m_pConfiguration->getUnsignedParameter("-base");

  XLM::addBusAccess(list, base, accessPattern, delay);

}

/*! 
  Convenience function to return the base address of SRAM A
  @return uint32_t
  @retval -base + SRAMA

*/
uint32_t
CXLM::sramA()
{
  return m_pConfiguration->getUnsignedParameter("-base") + SRAMA;
}
/*!
  Convenience function to return the base address of SRAM B
  @return uint32_t
  @retval -base + SRAM
*/
uint32_t 
CXLM::sramB()
{
  return m_pConfiguration->getUnsignedParameter("-base") + SRAMB;
}
/*!
  Convenience function to return the base address of the FPGA register set.  These are the 
  registers that are maintained by the FPGA firmware, not the interface registers.
  @return uint32_t
  @retval -base + FPGA
*/
uint32_t
CXLM::FPGA()
{
  uint32_t b =    m_pConfiguration->getUnsignedParameter("-base");
  return b + FPGABase;
}

/*!
  Convenience function to return the interface register set base... though really low level access functions
  to those registers ought to be added to this base class with application level functions only in the 
  derived class
  @return uint32_t
  @return -base + Interface

*/
uint32_t
CXLM::Interface()
{
  return m_pConfiguration->getUnsignedParameter("-base")  + InterfaceBase;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////// Namespace functions /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

CFirmwareLoader::CFirmwareLoader(CVMUSB& ctlr, uint32_t baseAddr)
  : m_ctlr(ctlr), m_baseAddr(baseAddr) 
{}

void CFirmwareLoader::operator()(const string& pathToFirmware)
{
  loadFirmware(pathToFirmware);
}

void CFirmwareLoader::loadFirmware(const string& pathToFirmware)
{
  cerr << hex << "Loading firmware for XLM at " << m_baseAddr << endl << dec;

  // Prep the FPGA for loading.  Specifically:
  // 1. Set the load source to SRAMA
  // 2. Force a falling edge on the FGPA Reset.. while not modifying the state of the other
  //    components....and hold the FPGA reset.
  // 3. Force the FPGA off the bus.
  // 4. Request the bus to SRAM A.
  // With the exception of determining the state of the Interrupt/reset register so it can be manipulated,
  // these operations are done in an immediate list:

  // Get the state of the interrupt register.. well it's another crappy write only reigster so
  // let's assume everything is held reset and that we only want to start the fpga.

  uint32_t interruptRegister = 0; /* InterruptResetFPGA | InterruptResetDSP |
                                     InterruptInterruptFPGA | InterruptInterruptDSP */

  // Build the list of operations:
  // Done in a block so the list is destroyed after it's executed:

  // Open and read the entire fpga file into memory (can't be too large)

  uint32_t  bytesInFile = fileSize(pathToFirmware);
  uint8_t*  contents    = new uint8_t[bytesInFile];
  uint32_t* sramAImage  = new uint32_t[bytesInFile]; // Each byte becomes anSRAM Longword.
  memset(sramAImage, 0, bytesInFile * sizeof(uint32_t));

  // The remainder is in a try block so we can delete the file contents:

  try {

    acquireBusses();

    // Read the file, convert it to an sram a image and load it into SRAM A:
    loadFile(pathToFirmware, contents, bytesInFile);	// Read the file into memory.

    // Skip the header:
    uint8_t* pc = skipHeader(contents);
    bytesInFile -= (pc-contents);

    // create sram image to load
    remapBits(sramAImage, pc, bytesInFile);

    // load the sram image to the device
    loadSRAM0(m_baseAddr+XLM::SRAMA, sramAImage, bytesInFile*sizeof(uint32_t));

    // wait a little bit for things to settle
#ifdef MVLC_GENERATOR
    m_ctlr.delay(1000);
#else
    this_thread::sleep_for( chrono::milliseconds(100) );
#endif
    // Release the SRAMA Bus, 
    // release the 'force'.
    releaseBusses();
    setBootSource();
    bootFPGA();

    // rest a bit while it loads
#ifdef MVLC_GENERATOR
    m_ctlr.delay(100000);
#else    
    this_thread::sleep_for( chrono::milliseconds(1000) );
#endif
    delete []contents;
    delete []sramAImage;

  } catch (...) {
    delete []contents;
    delete []sramAImage;
    throw;			// Let some higher creature deal with this.
  }
}


/// Read the contents of a file into memory
void CFirmwareLoader::loadFile(const string& pathToFirmware, 
                              uint8_t* contents, uint32_t nBytes)
{
  int fd = open(pathToFirmware.c_str(), O_RDONLY);
  if (fd < 0) {
    string error = strerror(errno);
    string msg   = "CXLM::loadFile - Failed to open the file: ";
    msg         += error;
    throw msg;
  }

  // read can be partial... this can happen on signals or  just due to buffering;
  // therefore the loop below is the safe way to use read(2) to load a file.
  // TODO:  use os::readdata instead.
  // 
  uint8_t* p = contents; // qty read on each read(2) call are bytes.
  try {
    while (nBytes > 0) {
      ssize_t bytesRead = read(fd, p, nBytes);
      if (bytesRead < 0) {
        // only throw if errno is not EAGAIN or EINTR.

        int reason = errno;
        if ((reason != EAGAIN) && (reason != EINTR)) {
          string error = strerror(reason);
          string msg   = "CXLM::loadFile - read(2) failed on firmware file: ";
          msg         += error;
          throw msg;
        }
      }
      else {
        nBytes -= bytesRead;
        p      += bytesRead;
      }
    }
  }
  catch (...) {
    // Close the file... and rethrow.

    close(fd);
    throw;
  }
  // close the file

  close(fd); 			// should _NEVER_ fail.

}

void CFirmwareLoader::remapBits(uint32_t* sramImage, uint8_t* fileImage, uint32_t nBytes)
{
  static const uint32_t bitMap[]  = {
    0x4, 0x8, 0x10, 0x400, 0x40000, 0x80000, 0x100000, 0x4000000
  };

  uint8_t*   src  = fileImage;
  uint32_t*  dest = sramImage;

  for(uint32_t i =0; i < nBytes; i++) {
    uint32_t  lword = 0;		// build destination here.
    uint8_t   byte  = *src++;	// Source byte
    uint32_t  bit   = 1;
    for (int b = 0; b < 8; b++) { // remap the bits for a byte -> longword
      if (byte & bit) {
        lword |= bitMap[b];
      }
      bit = bit << 1;
    }
    *dest++ = lword;		// set a destination longword.
  }
}

void CFirmwareLoader::loadSRAM0(uint32_t destAddr, uint32_t* image, uint32_t nBytes)
{
  static const size_t   blockSize = 64;
  uint32_t              nRemainingBytes    = nBytes;

  // for now load it one byte at a time... in 256 tansfer chunks:

  if (nRemainingBytes == 0) return;	// Stupid edge case but we'll handle it correctly.

#ifdef DUMPDEBUG  

  std::ofstream dump("fwloader.txt");
  dump << hex << setfill('0');
#endif  
  uint32_t* p  = image;
  
  while (nRemainingBytes > blockSize*sizeof(uint32_t)) {
    CVMUSBReadoutList  loadList;
    for (int i =0; i < blockSize; i++) {
#ifdef DUMPDEBUG      
      dump << "\n" << setw(8) << destAddr 
           << " " << setw(2)  << static_cast<int>(sramaAmod)
           << " " << setw(8) << *p;
#endif      
      loadList.addWrite32(destAddr, sramaAmod,  *p++);
//      loadList.addRead32(destAddr, sramaAmod);
      destAddr += sizeof(uint32_t);
    }
    nRemainingBytes -= blockSize*sizeof(uint32_t);
    // Write the block:
#ifdef MVLC_GENERATOR
    m_ctlr.executeList(loadList, blockSize*sizeof(uint32_t));
#else
    std::vector<uint8_t> retData = m_ctlr.executeList(loadList, sizeof(uint16_t));

    if (retData.size() == 0) {
      string error = strerror(errno);
      string msg   = "XLM::CFirmwareLoader::loadSRAMA - list execution failed to load the SRAM: ";
      msg         += error;
      throw msg;
    }
#endif
  }

  // Handle any odd partial block:
  if (nRemainingBytes > 0) {
    CVMUSBReadoutList loadList;
    while (nRemainingBytes > 0) {
#ifdef DUMPDEBUG     
      dump << "\n" << setw(8) << destAddr 
           << " " << setw(2)  << static_cast<int>(sramaAmod)
           << " " << setw(8) << *p;
#endif      
      loadList.addWrite32(destAddr, sramaAmod, *p++);
//      loadList.addRead32(destAddr, sramaAmod);
      destAddr += sizeof(uint32_t);
      nRemainingBytes -= sizeof(uint32_t);
    }
    // Write the block:
#ifdef MVLC_GENERATOR
    m_ctlr.executeList(loadList, blockSize*sizeof(uint32_t));
#else
    std::vector<uint8_t> retData = m_ctlr.executeList(loadList, sizeof(uint16_t));
    if (retData.size() == 0) {
      string error = strerror(errno);
      string msg   = "CXM::loadSRAMA - list execution failed to load the SRAM: ";
      msg         += error;
      throw msg;
    }
  #endif
  }
#ifdef DUMPDEBUG  
  dump << endl;
#endif  
}

void CFirmwareLoader::loadSRAM1(uint32_t destAddr, uint32_t* image, uint32_t nBytes)
{
  // for now load it one byte at a time... in 256 tansfer chunks:

  if (nBytes == 0) return;	// Stupid edge case but we'll handle it correctly.

#ifdef DUMPDEBUG
  std::ofstream dump("fwloader.txt");
  dump << hex << setfill('0');
#endif
  CVMUSBReadoutList loadList;
  loadList.addBlockWrite32(destAddr, blockTransferAmod, image, nBytes/sizeof(uint32_t));
#ifdef DUMPDEZBUG
  loadList.dump(dump);
#endif 
  std::vector<uint8_t> retData = m_ctlr.executeList(loadList, sizeof(uint16_t));
#ifndef MVLC_GENERATOR
  if (retData.size() == 0) {
    string error = strerror(errno);
    string msg   = "XLM::CFirmwareLoader::loadSRAMA - list execution failed to load the SRAM: ";
    msg         += error;
    throw msg;
  }
#endif
}
void CFirmwareLoader::setBootSource()
{
  CVMUSBReadoutList list;
  list.addWrite32(m_baseAddr + FPGABootSrc, registerAmod, BootSrcSRAMA); // Set boot source
  list.addRead32(m_baseAddr + FPGABootSrc, registerAmod);

  auto retData = m_ctlr.executeList(list, sizeof(uint32_t));
#ifndef MVLC_GENERATOR
  if (retData.size()==0) {
      string error = strerror(errno);
      string msg   = "CXM::setBootSource - list execution failed to set boot source to SRAMA: ";
      msg         += error;
      throw msg;

  }
#endif
}
uint32_t CFirmwareLoader::readFwSignature(uint32_t signatureAddr)
{
#ifdef MVLC_GENERATOR
// WE can't read the firmware signature in this .
  // acquire bus
  std::cerr << "The generator does not support checking the firmware signature\n";
  return 0;
#else
  // acquire bus
  accessBus(m_ctlr, m_baseAddr, CXLM::REQ_X | CXLM::REQ_A | CXLM::REQ_B);
  m_ctlr.vmeWrite32(m_baseAddr + ForceOffBus, registerAmod, ForceOffBusForce); // Inhibit FPGA Bus access.

  // perform read
  uint32_t value;
  m_ctlr.vmeRead32(signatureAddr, registerAmod, &value);

  // release bus
  m_ctlr.vmeWrite32(m_baseAddr + ForceOffBus, registerAmod, 0); // Inhibit FPGA Bus access.
  accessBus(m_ctlr, m_baseAddr, 0);

  return value;
#endif
}

bool CFirmwareLoader::validate(uint32_t signatureAddr, uint32_t expectedSignature)
{
#ifndef MVLC_GENERATOR             // Could not have read a signature.
  return true;
#else
  uint32_t actualSignature = readFwSignature(signatureAddr);
  return  (expectedSignature == actualSignature);
#endif
}


///**
// * Returns the size of a file.  The file must already exist and  be a valid target for 
// * stat(2). As this is used in the firmware load process, this has typically been assured by
// * a call to validFirmwareFile.
// * @param path - Absolute or relative path to the firmware file.
// * @return size_t
// * @retval Number of bytes in the file.  This includes 'holes' if the file is spares.
// * @throw std::string - if stat fails.
// */
uint32_t CFirmwareLoader::fileSize(const string& path)
{
  struct stat fileInfo;
  int status = stat(path.c_str(), &fileInfo);
  if(status) {
    string msg = strerror(errno);
    string error = "CXLM::fileSize - Unable to stat firmware(?) file: ";
    error       += msg;
    throw error;
  }
  return static_cast<uint32_t>(fileInfo.st_size); // Limited to 4Gbyte firmware files should be ok ;-]
}


// search for the end of the header identified by 0xff
uint8_t* CFirmwareLoader::skipHeader(uint8_t* contents)
{
  uint8_t* pc = contents;
  while (*pc != 0xff) {
    pc++;
  }
  return pc;
}

void CFirmwareLoader::acquireBusses()
{
  CVMUSBReadoutList initList;
  initList.addWrite32(m_baseAddr + ForceOffBus, 
                      registerAmod, ForceOffBusForce); // Inhibit FPGA Bus access.
  addBusAccess(initList, m_baseAddr, CXLM::REQ_A, 0);       //  Request bus A.


  // run the list:
  


  unsigned  retries = 10;
  vector<uint8_t> retData;
#ifdef MVLC_GENERATOR
  retData = m_ctlr.executeList(initList, sizeof(uint16_t));
#else
  do {
    retries--;
    retData = m_ctlr.executeList(initList, sizeof(uint16_t));
  } while ((retries > 0) && (retData.size() == 0) && (errno == EAGAIN));

  // Success or retries exhausted.
  
  if (retData.size() == 0) {
    string reason = strerror(errno);
    string msg = "XLM::CFirmwareLoader::initialize - failed to execute initialization list: ";
    msg       += reason;
    if (retries == 0) {
      msg += " (retries exhausted)";
    } 
    throw msg;
  }

  // I should have bus A:
  uint32_t busAOwner =0;
  int status = m_ctlr.vmeRead32(m_baseAddr + BUSAOwner, registerAmod, &busAOwner);
  
  if ((busAOwner & 3) != 1)  {
    std::stringstream msg;
    msg << "CXLM::CFirmwareLoader::initialize - failed to acquire bus A ";
    msg << std::hex << " Expected 1 got " << busAOwner << std::endl;
    msg << " Read from " << m_baseAddr + BUSAOwner << std::dec << std::endl;
    msg << " Claim bus gave " << (int)retData[0] << " " << (int)retData[1];
    if (status < 0) {
      string reason = strerror(errno);
      msg << " VME Read 32 failed: " << reason;
    }
    

    throw msg.str();
  }
#endif
}

// Booting the fpga amounts to setting the FPGA reset register and releasing it.
void CFirmwareLoader::bootFPGA()
{
  CVMUSBReadoutList  bootList;
  bootList.addWrite32(m_baseAddr + Interrupt, registerAmod, InterruptResetFPGA); // Hold FPGA reset.
  bootList.addWrite32(m_baseAddr + Interrupt, registerAmod, uint32_t(0) );	// Release FPGA reset 

  // send the commands to the VM-USB
  auto retData = m_ctlr.executeList(bootList, sizeof(uint16_t));
#ifndef MVLC_GENERATOR
  if (retData.size() == 0) {
    string reason = strerror(errno);
    string message = "XLM::CFirmwareLoader::bootFPGA failed to execute reset list ";
    message       += reason;
    throw message;
  }
#endif
}

void CFirmwareLoader::releaseBusses()
{
  CVMUSBReadoutList list;
  list.addWrite32(m_baseAddr + BusRequest, registerAmod, uint32_t(0));	// Release bus request.
  list.addWrite32(m_baseAddr + ForceOffBus, registerAmod, uint32_t(0)); // Remove force

  auto retData = m_ctlr.executeList(list, sizeof(uint16_t));
  #ifndef MVLC_GENERATOR
  if (retData.size() == 0) {
    string reason = strerror(errno);
    string message = "XLM::CFirmwareLoader::releaseBusses failed to execute reset list ";
    message       += reason;
    throw message;
  }
#endif

}

/*!
  Access some combination of the busses.  The assumption is that since this
  is a one shot, and since I'm ensured that I'll get the bus within 200ns, the host
  will not be able to turn the read around faster than that and therefore I don't need
  to check that I actually have the bus.

  Note this can be used to release the busses by not setting the appropriate 
  bus request bits.

  @param controller - VMUSB controller object.
  @param accessPattern - bitwise oro of one of the following:
  - CXLM::REQ_X   - Request the X (FPGA) bus.
  - CXLM::REQ_A   - Request the A (SRAM A) bus.
  - CXLM::REQ_B   - Request the B (SRAM B) bus.

*/
void accessBus(CVMUSB& controller, uint32_t base, uint32_t accessPattern)
{
  controller.vmeWrite32(base + BusRequest, registerAmod, accessPattern);
}

/*!
  Adds a bus access request to a readout list.
  @param list - Reference to a VMUSBReadout list to which the appropriate instructions will be added.
  @param accessPattern - The requested access pattern, See accessBus above for a description of this.
  @param delay - Delay to insert after the bus request.  This should be the worst case time between
                 arbitration request and grant..determined by the FPGA firmware you are using.
		 delay is in 12.5ns units.
*/
void addBusAccess(CVMUSBReadoutList& list, uint32_t base, uint32_t accessPattern,
		   uint8_t delay)
{
  list.addWrite32(base + BusRequest, registerAmod, accessPattern);
  if ( delay > 0) {		// Don't delay if none requested.
    list.addDelay(delay);
  }

}

// ...oooOOOooo......oooOOOooo......oooOOOooo......oooOOOooo......oooOOOooo......oooOOOooo......oooOOOooo...
// /////////////////////////////////////////////////////////////////////////////////////////////////////////   
//
/**! A namespace to store utility functions
    \namespace Utils

  These functions do not need to be as visible as loadFirmware becuase they are primarily used 
  to implement loadFirmware. For this reason, they will be grouped into the XLM::Utils namespace.
  
*/
namespace Utils
{
/**
 * Determines if a file is a valid firmware file.  At this point we just use access(2) to see if
 * the file exists and is readable by us.  This is a configuration validator and hence must
 * fit in with that call signature:
 *
 * @param name - Name of the configuration parameter that holds the file.
 * @param value - Path to the file
 * @param arg - Unused optional parameter 
 * @return bool
 * @retval true - Firmware file is valid.
 * @retval false - Firmware file is not valid.
 */
bool validFirmwareFile(string name, string value, void* arg)
{
  int status = access(value.c_str(), R_OK);

  // TODO: Some day we should figure out a better way for parameter checkers to get error information
  //       back to the configuration subsystem.. e.g. errno would exactly describe what was wrong here.

  return (status == 0);
}

} // end of Utils namespace

} // end of XLM namespace



