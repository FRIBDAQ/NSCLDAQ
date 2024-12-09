/******************************************************************************
*
* CAEN SpA - Software Division
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
***************************************************************************//**
* 
* Name      CAENComm.h
* Project   CAENComm - Communication Layer - foreign devices.
*/

/**
 * @brief Implementation of CAENComm for VM-USB
 * @author Ron Fox (rfoxkendo@gmail.com)
 *
 * @note - only the functions needed to drive the firmare loader are 
 *         implemented.
 */

/**
 * Includes
 */

#include <CAENComm.h>
#include <vector>
#include <CVMUSBusb.h>
#include <CVMUSBReadoutList.h>
#include <usb.h>		// Probably don't need this(?)
#include <stdio.h>

//
// Handles are indices into this vector of pointers to structs
// of the following type:

typedef struct _unit {
  CVMUSB*  s_pDevice;		// VMUSB device
  uint32_t s_base;		// Base address added to all references.
} Unit, *pUnit;

static std::vector<Unit> handles;

static uint8_t Amod = CVMUSBReadoutList::a32UserData;   // Address modifier we'll use

/**
 * Local utility methods:
 */

/**
 * @fn validHandle
 *
 * Determines if a handle number is valid.   A handle is valid if
 * it is in the range of valid indices for handles above and
 * the contents of handles[handle] is not null.
 *
 * @param handle - integer candidate handle to check
 *
 * @return bool - true if the handle is valid else false.
 */
static bool
validHandle(int handle)
{
    if ((handle < 0) || (handle >= handles.size())) return false;
    
    return handles[handle].s_pDevice != 0;
}

/**
 * @fn VMUSBStatusToCaenStatus(status)
 *
 *  Map status returns from single shot VME operations to
 *  the ones the CAENComm library needs to return.
 *
 *  @param usbStatus - Status from VMUSB
 *
 *  @return CAENComm_ErrorCode
 *  @retval usbStatus == -3 CAENComm_VMEBusError
 *  @retval usbStatus == 0  CAENComm_Success
 *  @retval other values; CAENComm_CommError
 */
static CAENComm_ErrorCode
VMUSBStatusToCaenStatus(int status)
{
    switch (status) {
    case -3:
        return CAENComm_VMEBusError;
    case 0:
        return CAENComm_Success;
    default:
        return CAENComm_CommError;
            
    }
}

/**
 * C interface is required..but the support software is in C++ so:
 */

extern "C" {
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_OpenDevice(CAENComm_DeviceDescr dd, int *handle);
* \brief   Open the device
*
* \param	[IN] LinkType: the link used by the device(CAENComm_USB|CAENComm_PCI_OpticalLink|CAENComm_PCIE_OpticalLink|CAENComm_PCIE)
* \param	[IN] LinkNum: The link number
* \param	[IN] ConetNode: The board number in the link.
* \param	[IN] VMEBaseAddress: The VME base address of the board
* \param	[OUT] handle: device handler
* \return  0 = Success; negative numbers are error codes
*
* @note for the VM-USB, here's how the parameters are interpreted:
*    - LinkType is ignored.
*    - LinkNum If 0 the first VM-USB that can be located is opened otherwise, the VM-USB
*              with serial 'number' 'VMxxx' where xxx=LinkNum is opened.
*    - ConetNode - ignored.
*    - VMEBaseAddress - ignored.
*
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_OpenDevice(CAENComm_ConnectionType LinkType, int LinkNum, int ConetNode, uint32_t VMEBaseAddress, int *handle)
{
  // Enumerate the modules:

  std::vector<struct usb_device*> controllers;
  controllers = CVMUSB::enumerate();

  // No devices at all means CAENComm_DeviceNotFound.

  if (controllers.size() == 0) {
    return CAENComm_DeviceNotFound; 
  }
  // Figure out which one we want to open:

  struct usb_device* pDevice = NULL;
  if (LinkNum != 0) {
    char czSerialDesired[100];
    sprintf(czSerialDesired, "VM%04d", LinkNum);
    std::string SerialDesired = czSerialDesired;

    for (int i = 0; i < controllers.size(); i++) {
      if (CVMUSB::serialNo(controllers[i]) == SerialDesired) {
	pDevice = controllers[i];
	break;
      }
    } 
  } else {
    pDevice = controllers[0]; // Default to first.
  }
  if (!pDevice) {
    return CAENComm_DeviceNotFound;
  }
  CVMUSB* pVMUSB = new CVMUSBusb(pDevice);
  Unit unitData;
  unitData.s_pDevice = pVMUSB;
  unitData.s_base    = VMEBaseAddress;

  handles.push_back(unitData);
  *handle = handles.size()-1;

  return CAENComm_Success;


}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_CloseDevice(int handle);
* \brief   Close the device
*
* \param   [IN]  handle: device handler
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_CloseDevice(int handle)
{
  if(!validHandle(handle)) {
    return CAENComm_InvalidHandler;
  }
  delete handles[handle].s_pDevice;
  handles[handle].s_pDevice=0;	// Make referencing it a disaster.

  return CAENComm_Success;

}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Write32(int handle, uint32_t Address, uint32_t *Data)
* \brief   Write a 32 bit register of the device
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offset
* \param   [IN]  Data: new register content to write into the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Write32(int handle, uint32_t Address, uint32_t Data)
{
    // Retrieve the handle:
    
    if (!validHandle(handle)) {
        return CAENComm_InvalidHandler;
    }
    CVMUSB* pController = handles[handle].s_pDevice;
    
    // Attempt the operation and analyze the result.
    
    int status = pController->vmeWrite32(Address + handles[handle].s_base, 
					 Amod, Data);
    return VMUSBStatusToCaenStatus(status);
    
}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Write16(int handle, uint32_t Address, uint16_t *Data)
* \brief   Write a 16 bit register of the device
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offset
* \param   [IN]  Data: new register content to write into the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Write16(int handle, uint32_t Address, uint16_t Data)
{
    if (!validHandle(handle)) {
        return CAENComm_InvalidHandler;
    }
    
    CVMUSB* pController = handles[handle].s_pDevice;
    
    int status = pController->vmeWrite16(Address + handles[handle].s_base, 
					 Amod, Data);
    return VMUSBStatusToCaenStatus(status);
}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Read32(int handle, uint32_t Address, uint32_t *Data)
* \brief   Read a 32 bit data from the specified register.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offset
* \param   [OUT] Data: The data read from the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Read32(
    int handle, uint32_t Address, uint32_t *Data)
{
    if(!validHandle(handle)) {
        return CAENComm_InvalidHandler;
    }
    CVMUSB* pController = handles[handle].s_pDevice;
    
    int status = pController->vmeRead32(Address + handles[handle].s_base, 
					Amod, Data);
    return VMUSBStatusToCaenStatus(status);

}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Read16(int handle, uint32_t Address, uint16_t *Data)
* \brief   Read a 16 bit data from the specified register.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offset
* \param   [OUT]  Data: The data read from the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Read16(
    int handle, uint32_t Address, uint16_t *Data)
{
    if (!validHandle(handle)) {
        return CAENComm_InvalidHandler;
    }
    
    CVMUSB* pController = handles[handle].s_pDevice;
    int status = pController->vmeRead16(Address + handles[handle].s_base,
					Amod, Data);
    return VMUSBStatusToCaenStatus(status);
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MultiRead32(
*              int handle, int32_t Address, int nCycles, uint32_t *data,
*              CAENComm_ErrorCode *ErrorCode)
* \brief   The function performs a block of single 32bit ReadRegister.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offsets
* \param   [IN]  nCycles: the number of read to perform
* \param   [OUT] data: The datas read from the device
* \param   [OUT] ErrorCode: The error codes relaive to each cycle.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MultiRead32(
    int handle, uint32_t *Address, int nCycles, uint32_t *data,
    CAENComm_ErrorCode *ErrorCode)
{
  // Our final status will be CAENComm_Success on success or the first
  // bad status on failure.

  CAENComm_ErrorCode finalStatus = CAENComm_Success;
  for (int i = 0; i < nCycles; i++) {
    // Do one of the reads and its associated book-keeping:

    CAENComm_ErrorCode status = CAENComm_Read32(handle, *Address, data);
    *ErrorCode++ = status;
    Address++;
    data++;
   
    // If this was the first failure save it for the return value:
    if ((status != CAENComm_Success ) && (finalStatus == CAENComm_Success)) {
      finalStatus = status;
    }
  }
  return finalStatus;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MultiRead16(int handle, uint32_t *Address, int nCycles, uint32_t *data, CAENComm_ErrorCode *ErrorCode)
* \brief   The function performs a block of single 16bit ReadRegister.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offsets
* \param   [IN]  nCycles: the number of read to perform
* \param   [OUT] data: The datas read from the device
* \param   [OUT] ErrorCode: The error codes relaive to each cycle.
* \return  0 = Success; negative numbers are error codes
*
* @note TODO: This and the previous function should be combined into a common
*             templated function since the logic is identical..only data sizes
*             and the underlying API function differ
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MultiRead16(
    int handle, uint32_t *Address, int nCycles, uint16_t *data,
    CAENComm_ErrorCode *ErrorCode)
{
  // Our final status will be CAENComm_Success on success or the first
  // bad status on failure.

  CAENComm_ErrorCode finalStatus = CAENComm_Success;
  for (int i = 0; i < nCycles; i++) {
    // Do one of the reads and its associated book-keeping:

    CAENComm_ErrorCode status = CAENComm_Read16(handle, *Address, data);
    *ErrorCode++ = status;
    Address++;
    data++;
   
    // If this was the first failure save it for the return value:
    if ((status != CAENComm_Success ) && (finalStatus == CAENComm_Success)) {
      finalStatus = status;
    }
  }
  return finalStatus;  
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MultiWrite16(int handle, uint32_t *Address, int nCycles, uint32_t *data, CAENComm_ErrorCode *ErrorCode)
* \brief   The function performs a block of single 16bit WriteRegister.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offsets
* \param   [IN]  nCycles: the number of read to perform
* \param   [IN]  data: The datas to write to the device
* \param   [OUT] ErrorCode: The error codes relaive to each cycle.
* \return  0 = Success; negative numbers are error codes
*
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MultiWrite16(
    int handle, uint32_t *Address,int nCycles, uint16_t *data,
    CAENComm_ErrorCode *ErrorCode)
{
  CAENComm_ErrorCode finalStatus = CAENComm_Success;

  for (int i = 0; i < nCycles; i++) {
    CAENComm_ErrorCode status = CAENComm_Write16(handle, *Address++, *data++);
    *ErrorCode++ = status;

    if ((status != CAENComm_Success) && (finalStatus != CAENComm_Success)) {
      finalStatus = status;
    }
  }
  return finalStatus;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MultiWrite32(int handle, uint32_t *Address, int nCycles, uint32_t *data, CAENComm_ErrorCode *ErrorCode)
* \brief   The function performs a block of single 32bit WriteRegister.
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: register address offsets
* \param   [IN]  nCycles: the number of read to perform
* \param   [IN]  data: The datas to write to the device
* \param   [OUT] ErrorCode: The error codes relaive to each cycle.
* \return  0 = Success; negative numbers are error codes
*
* @note TODO: This should be combined with the prior function via 
*             templates and function arguments.  The logic is the same
*             only the data size and actual underlying API function differ.
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MultiWrite32(
    int handle, uint32_t *Address, int nCycles, uint32_t *data,
    CAENComm_ErrorCode *ErrorCode)
{
  CAENComm_ErrorCode finalStatus = CAENComm_Success;

  for (int i = 0; i < nCycles; i++) {
    CAENComm_ErrorCode status = CAENComm_Write32(handle, *Address++, *data++);
    *ErrorCode++ = status;

    if ((status != CAENComm_Success) && (finalStatus != CAENComm_Success)) {
      finalStatus = status;
    }
  }
  return finalStatus;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_BLTRead(int handle, uint32_t Address, uint32_t *Buff, int BltSize, int *nw)
* \brief   Read a block of data from the device using a BLT (32 bit) cycle
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: data space starting address
* \param   [IN]  BltSize: size of the Block Read Cycle (in bytes)
* \param   [OUT] buff: pointer to the data buffer
* \param   [OUT] nw: number of longwords (32 bit) actually read from the device
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_BLTRead(
    int handle, uint32_t Address, uint32_t *Buff, int BltSize, int *nw)
{
    if (!validHandle(handle)) {
        return CAENComm_InvalidHandler; 
    }
    CVMUSB* pController = handles[handle].s_pDevice;
    size_t actualTransferCount;
    int status = pController->vmeBlockRead(Address + handles[handle].s_base, 
					   Amod, Buff, 
					   static_cast<size_t>(BltSize),
					   &actualTransferCount);
    *nw = actualTransferCount;
    return VMUSBStatusToCaenStatus(status);
    
}
/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_MBLTRead(int handle, uint32_t Address, uint32_t *Buff, int BltSize, int *nw)
* \brief   Read a block of data from the device using an MBLT (64 bit) cycle
*
* \param   [IN]  handle: device handler
* \param   [IN]  Address: data space starting address
* \param   [IN]  BltSize: size of the Block Read Cycle (in bytes)
* \param   [OUT] buff: pointer to the data buffer
* \param   [OUT] nw: number of longwords (32 bit) actually read from the device
* \return  0 = Success; negative numbers are error codes
*
* @note TODO: I believe the VM-USB supports MBLT/64 transfers but this is not in the
*             current API.
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_MBLTRead(
    int handle, uint32_t Address, uint32_t *Buff, int BltSize, int *nw)
{
    return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_VMEIRQCheck(int handle, uint8_t *Mask)
* \brief   The function returns a bit mask indicating the active VME IRQ lines.

* \param   [IN]  handle: device handler
* \param   [OUT]  Mask: A bit-mask indicating the VME IRQ lines.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_VMEIRQCheck(int VMEhandle, uint8_t *Mask)
{
    return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_IRQDisable(int handle)
* \brief   The function disables the IRQ lines.

* \param   [IN]  handle: device handler
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_IRQDisable(int handle)
{
    return CAENComm_NotSupported;
}

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_IRQEnable(int handle)
* \brief   The function enaables the IRQ lines.

* \param   [IN]  handle: device handler
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_IRQEnable(int handle)
{
    return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_VMEIACKCycle16(int VMEhandle, IRQLevels Level, int *BoardID)
* \brief   The function performs a 16 bit VME interrupt acknowledge cycle .

* \param   [IN]  handle: device handler
* \param   [IN]  Level: The VME IRQ level to acknowledge (see IRQLevels enum).
* \param   [OUT] BoardID: the Id of the Board that reased the interrupt.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_VMEIACKCycle16(
    int VMEhandle, IRQLevels Level, int *BoardID)
{
    return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_VMEIACKCycle32(int VMEhandle, IRQLevels Level, int *BoardID)
* \brief   The function performs a 32 bit VME interrupt acknowledge cycle.

* \param   [IN]  handle: device handler
* \param   [IN]  Level: The VME IRQ level to acknowledge (see IRQLevels enum).
* \param   [OUT] BoardID: the Id of the Board that reased the interrupt.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_VMEIACKCycle32(
    int VMEhandle, IRQLevels Level, int *BoardID)
{
    return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_IACKCycle(int handle, IRQLevels Level, int *BoardID)
* \brief   The function performs a 16 bit VME interrupt acknowledge cycle .

* \param   [IN]  handle: device handler
* \param   [IN]  Level: The VME IRQ level to acknowledge (see IRQLevels enum).
* \param   [OUT] BoardID: the Id of the Board that reased the interrupt.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_IACKCycle(
    int handle, IRQLevels Level, int *BoardID)
{
    return CAENComm_NotSupported;
}

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_IRQWait(int handle, uint32_t Timeout)
* \brief   The function wait the IRQ until one of them raise or timeout expires.
		   This function can be used ONLY on board NOT controlled by CAEN VME Bridges 

* \param   [IN]  handle: device handler
* \param   [IN]  Timeout: Timeout in milliseconds.
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_IRQWait(int handle, uint32_t Timeout)
{
    return CAENComm_NotSupported;
}

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_VMEIRQWait(int LinkType, int LinkNum, int ConetNode, uint8_t IRQMask, uint32_t Timeout, int *VMEHandle)
* \brief   The function wait the IRQ until one of them raise or timeout expires.

* \param   [IN] LinkType: the link used to connect to the CAEN VME Bridge(CAENComm_USB|CAENComm_PCI_OpticalLink|CAENComm_PCIE_OpticalLink|CAENComm_PCIE)
* \param   [IN] LinkNum: The link number
* \param   [IN] ConetNode: The CAEN VME Bridge number in the link.
* \param   [IN] Timeout: Timeout in milliseconds.
* \param   [IN] IRQMask: A bit-mask indicating the IRQ lines
* \param   [OUT] VMEHandle: The CAEN Bridhe handle to use in VMEIRQCheck and VMEIACKCycle
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_VMEIRQWait(
    CAENComm_ConnectionType LinkType, int LinkNum, int ConetNode,
    uint8_t IRQMask, uint32_t Timeout,int *VMEHandle)
{
    return CAENComm_NotSupported;
}

/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_Info(int handle, CAENCOMM_INFO info, char *data)
* \brief   The function returns information about serial number or firmware release of the device.

* \param   [IN]  handle: device handler
* \param   [IN]  info: The interested info (see CAENCOMM_INFO enum).
* \param   [OUT]  data: an array (user defined to 30 byte) with the requested info
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_Info(
    int handle, CAENCOMM_INFO info, char *data)
{
    return CAENComm_NotSupported;
}


/**************************************************************************//**
* \fn      CAENComm_ErrorCode STDCALL CAENComm_SWRelease(char *SwRel)
* \brief   Returns the Software Release of the library
*
* \param   [OUT] SwRel: the Software Release of the library
* \return  0 = Success; negative numbers are error codes
******************************************************************************/
CAENComm_ErrorCode STDCALL CAENComm_SWRelease(char *SwRel)
{
    return CAENComm_NotSupported;
}

}
void* gpTCLApplication(0);
namespace Globals {
  CVMUSB* pUSBController(0);
}