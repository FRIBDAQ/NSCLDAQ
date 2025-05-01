/**
#******************************************************************************
#
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#***************************************************************************//**
# 

##
# @file CMesytecBase.h
# @brief Base class for mesytec digitizers to support heterogenous chains.
# @author Ron Fox (rfoxkendo@gmail.com)

*/

#ifndef MVLC_CMESYTECBASE_H
#define MVLC_CMESYTECBASE_H

#include "CReadoutHardware.h"
#include <stdint.h>

class CVMUSB;
class CVMUSBReadoutList;

/**
 * @class CMesytecBase
 *
 * This file is a base class for mesytec modules  it provides the interface
 * required for modules to participate in a chain of heterogenous modules.
 */
class CMesytecBase : public CReadoutHardware {
public:
  typedef enum _ChainPosition {                // Where module is in a readout chain.
    first,
    middle,
    last
  } ChainPosition;  
public:
  // The following functions are used by the madcchain module.
  //
  virtual void setChainAddresses(CVMUSB& controller,
			 ChainPosition position,
			 uint32_t      cbltBase,
			 uint32_t      mcastBase) = 0;

  virtual void initCBLTReadout(CVMUSB& controller,
                               uint32_t cbltAddress,
                               int wordsPermodule) = 0;
};

#endif
