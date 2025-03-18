/**
 * @file CSIS3820ScalerBank.h
 * @brief header for a scaler bank that holds CSIS3820Scalers.
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
#ifndef CSIS3820SCALERBANK_H
#define CSIS3820SCALERBANK_H

#include <CScalerBank.h>
#include <vector>
#include <TCLObjectProcessor.h>
class CTCLInterpreter;
class CTCLObject;

/**
 * @class CSIS3820ScalerBank
 *    This is a class that holds CSIS3820Scaler modules and can be
 * added to the experiment's scaler bank to readout those modules.
 * This is modeled loosely after the VMUSBReadout configuration in that
 * associated with this object is a configuration file.  The configuration
 * file is processed on initialize time to determne the scalers that will
 * be used for this run.
 * 
 * The nexted class CSIS3820Command implements a Tcl extension that 
 * knows how to create the scalers in the bank.
 */
class CSIS3820ScalerBank : public CScalerBank {
public:
    class CSIS3820Command : public CTCLObjectProcessor {
    private:
        CSIS3820ScalerBank* m_pBank;
    public:
        CSIS3820Command (CTCLInterpreter& interp, CSIS3820ScalerBank* container);
        virtual ~CSIS3820Command ();

        int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);


    private:
        bool exists(const std::string& name);
    };
private:
    std::string m_configFile;
public:
    CSIS3820ScalerBank(const char* pConfigFilename);
    virtual ~CSIS3820ScalerBank();

private:
    CSIS3820ScalerBank(const CSIS3820ScalerBank& rhs);
    CSIS3820ScalerBank& operator=(const CSIS3820ScalerBank& rhs);
    int operator==(const CSIS3820ScalerBank& rhs) const;
    int operator!=(const CSIS3820ScalerBank& rhs) const;

public:
    virtual void initialize();                // This is the only squirrely thing.


};



#endif