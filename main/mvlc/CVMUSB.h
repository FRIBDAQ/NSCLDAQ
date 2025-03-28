/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for VMUSB Stand-in class.
*/
#ifndef MVLC_CVMUSB_H
#define MVLC_CVMUSB_H
#include "CVMUSBReadoutList.h"
/**
 *  @class CVMUSB
 *    This file is a stand-in for CVMUSB for device support objects. It's handed to their
 * iniialization methods to collect the VME operations done. We 'chean' by encapsulating
 * our CVMUSBReadoutList and just turing our operations into add's for that, exposing
 * that object's dumpForMVLC method.
 */
class CVMUSB {
private:
    CVMUSBReadoutList m_operationList;
public:
};
#endif