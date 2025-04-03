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
@brief Header for the base class for device driver classes.
*/
#ifndef MVLC_CREADOUTHARDWARE_H
#define MVLC_CREADOUTHARDWARE_H

namespace XXUSB {
    class CConfigurableObject;
}
class CVMUSB;
class CVMUSBReadoutList;

/**
 * @class CReadoutHardware
 *    This is the base class for the device support modules that will be used
 *    to generate MVLC stacks for the fribdaq-readout program in the MVLC driver tree.
 * 
 * This class, unlike the VMUSB version is not pure virtual but has do-nothing functions.
 * for the default implementations of most operations.
 */

class CReadoutHardware {
private:
    XXUSB::CConfigurableObject* m_pConfiguration;

    // The canonicals that are allowed:
public:
    CReadoutHardware();
    virtual ~CReadoutHardware();

    // Canonicals that are not allowed.
private:
    CReadoutHardware(const CReadoutHardware& rhs);
    CReadoutHardware& operator=(const CReadoutHardware& rhs);
    int operator==(const CReadoutHardware& rhs);
    int operator!=(const CReadoutHardware& rhs);

    // Mutators and selectors:
public:
    void setConfiguration(XXUSB::CConfigurableObject* pConfig);
    XXUSB::CConfigurableObject* getConfiguration() { return m_pConfiguration; }
    
    // The interface
public:
  virtual void onAttach(XXUSB::CConfigurableObject& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual void onEndRun(CVMUSB& interface);
    
 };
#endif
