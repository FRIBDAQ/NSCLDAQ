/**
 * Header defining minimal support for the SIS3316 module.
 * Minimal means that we support a sort of single event mode and
 * trace acquisition only.  Additional support can be added as needed
 * presumably.
 * 
 * Configuration Parameters rare:
 * 
 *\verbatim
 *  -base - module base addresss on VME bus.
 *  -clock - Clock source one of "fp", "250MHz","125Mhz, 50MHz 25Mhz or 12.5MHz"
 *  -samples - Number of trace samples to acquire 0-65536 - 4 values 0-65535
 *     These apply to banks of four digitizers
 *  -id - top bits of the ID put in header header of the data.
 *  -pretrigger - value of pre-trigger delay registers list of 4 integers 0-65535
 *      These apply to banks of four digitizers.
 *  -enable (16 bools true means a channel is enabled).
 * \endverbatim
 * 
 * Data format:
 * 
 * Each channel will be preceded by a 32 bit marker containing the channel
 * number.  this is then followed by a 3 word header after which are the
 * samples.  I _believe_ each sample is bottom justified in a 32 bit word.
 * See section 4.6 the manual with format bits all zero and the MAW test data 
 * are not present so only the  waveform is present.
 */

/**
 * @brief Header for the CSIS3316 class that provides support for the SIS3316
 * @author Ron Fox <fox@frib.msu.edu>
 * 
 * Facility for Rare Isotope Beams
 * 640 S. Shaw Lane
 * East Lansing, MI 48824-1321
 * (c) Copyright 2025 Board of Trustees of Michigan State University 
 * 
 *  You may use this software under the terms of the GNU public license
 *   (GPL).  The terms of this license are described at:
 *
 *   http://www.gnu.org/licenses/gpl.txt
 * 
 * 
 */
#ifndef CSIS3316_H
#define CSIS3316_H

#include "CReadoutHardware.h"
#include <vector>
#include <stdint.h>

// Forward class definitions.
class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;
class sis3316_adc;
class sis_vmusb_interface;

/**
 *  @class CSIS3316 - Class definition, whose implementation provides support for
 * the sis3316 module.
 */
class CSIS3316 : public CReadoutHardware {

// Internal data:

private:
    CReadoutModule* m_pConfiguration;
    sis3316_adc* m_pModule;
    sis_vmusb_interface* m_pVmeBus;

// canonicals:
public:
    CSIS3316();
    virtual ~CSIS3316();
    CSIS3316(const CSIS3316& rhs);
    CSIS3316& operator=(const CSIS3316& rhs);

    // Comparison makes no real sense.
    private:
    int operator==(const CSIS3316& rhs) const;
    int operator!=(const CSIS3316& rhs) const;
    // Readout hardware API
public:
    virtual void onAttach(CReadoutModule& configuration);
    virtual void Initialize(CVMUSB& controller);
    virtual void addReadoutList(CVMUSBReadoutList& list);
    virtual CReadoutHardware* clone() const;
    
    // Utilities:
private:
    int sizeGroup(int groupNum, std::vector<bool>& enables);
    void dumpSetup();
    uint32_t readRegister(unsigned offset);
    void setClock();
    void setClockParameters(int samplerate_enum);
};

#endif