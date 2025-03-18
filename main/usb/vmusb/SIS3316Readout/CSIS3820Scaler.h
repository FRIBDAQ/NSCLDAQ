/**
 * @file CSIS3820Scaler.h
 * @brief header for the SIS3820 scaler used as a scaler in a scaler bank.
 * 
  * @author Ron Fox <fox at frib dot msu dot edu>
 *  This software is Copyright by the Board of Trustees of Michigan
 *  State University (c) Copyright 2025
*
*  You may use this software under the terms of the GNU public license
*   (GPL).  The terms of this license are described at:
*
*    http://www.gnu.org/licenses/gpl.txt
*
*    Author:
*            Ron Fox
*            Facility for Rare Isotop Beams
*            Michigan State University
*            East Lansing, MI 48824-1321
*
 */

#ifndef CSIS3820SCALER_H
#define CSIS3820SCALER_H
#include <CScaler.h>
#include <string>
#include <stdint.h>


/**
 * @class CSIS3820Scaler
 * 
 *    This class provides for the SIS3820 scaler to be used as a scaler (not timestamp) module
 * it is intended to be placed in a scaler bank.  The CSIS3820ScalerBank  class supports
 * a scaler bank that can be configured via a file to contain a set of CSIS3820 scalers.
 * Normally one would instantiate and add one of those and then you'd have dynamically configured
 * scaler modules.  Nothing to stop you from just statically creating and inserting these into a scaler bank.
 * Note this is not an incremental scaler - that is it will only clear at the start of a run.
 */
class CSIS3820Scaler : public CScaler {
private:
    std::string m_name;                         // Name of the scaler.
    uint32_t    m_base;                         // base address of the scaler.

    // Public canonicals:
public:
    CSIS3820Scaler(const char* pName, uint32_t base);
    virtual ~CSIS3820Scaler();

    // Disallowed canonicals:
private:
    CSIS3820Scaler(const CSIS3820Scaler& rhs);
    CSIS3820Scaler& operator=(const CSIS3820Scaler& rhs);
    int operator==(const CSIS3820Scaler& rhs) const;
    int operator!=(const CSIS3820Scaler& rhs) const;

    // Selectors:
public:
    uint32_t base() const;
    std::string name() const;

    // Virtual methods implemented:

    virtual void initialize();
    virtual std::vector<uint32_t> read();
};

 #endif