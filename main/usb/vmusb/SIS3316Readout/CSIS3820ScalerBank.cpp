/**
 * @file CSIS3820ScalerBank.cpp
 * @brief implementation of a scaler bank bank that holds CSIS3820Scalers.
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
#include "CSIS3820ScalerBank.h"
#include "CSIS3820Scaler.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <memory>
#include <TCLException.h>

typedef CSIS3820ScalerBank::CSIS3820Command CMDCLASS; 

//////////////////////////////////// CSIS3820ScalerBank implementation ///////////////////////////
/**
 * constructor
 *    @param pConfigFilename - name of the configuration file that is interpreted at initialization time.
 */
CSIS3820ScalerBank::CSIS3820ScalerBank(const char* pConfigFilename) :
    m_configFile(pConfigFilename) {}

/**
 *  destructor:
 *     I don't thikn this will ever get called but maybe we should
 * delete any modules we now have.
 */
CSIS3820ScalerBank::~CSIS3820ScalerBank() {}


/**
 * initialize
 *    - Delete all of the modules we have now.
 *    - Make a captive interpreter that has the CSIS3820Command installed.
 *    - Interpret the configuration file with it.
 *    - Do the base class initialization.
 *    - Tear down the interpreter.
 */
void
CSIS3820ScalerBank::initialize() {
    // Clear the ban of modules:

    while (begin() != end()) {
        CScaler* p = reinterpret_cast<CScaler*>(*begin());   // Should be a pointer.
        DeleteScaler(p);
        delete p;                // our scalers are made with new.
    }
    // Interpret the configuration file:

    CTCLInterpreter interp;
    std::unique_ptr<CMDCLASS> pCommand(new CMDCLASS(interp, this));
    interp.EvalFile(m_configFile);           

    // Now we can initialize:

    CScalerBank::initialize();

    // Destructors wil tear down both the command and the intepreter.
}

 ///////////////////////////////// CSIS3820SCommand nested class implementation //////////////////////

/** 
 * constructor
 *    @param interp - the intepreter we will be registered on.
 *    @param container - pointer to the scaler bank we will configure.
 */
CMDCLASS::CSIS3820Command(CTCLInterpreter& interp, CSIS3820ScalerBank* container) :
    CTCLObjectProcessor(interp, "sis3820"), m_pBank(container) 
{}
/**
 * destructor.
 */
CMDCLASS::~CSIS3820Command() {}

/**
 *  operator()
 *    Called in response to the sis3820 command. The form of that command is:
 * \verbatim
 *    sis3820 name base-address.
 * \endverbatim
 * 
 * This will create an CSIS3820Scaler object and add it to the scaler bank.
 * Duplicate module names are not allowed.
 * 
 *  @param interp -interpreter running the command.
 *  @param objv  - the command words encapsulated in CTCLObjects
 */
int
CMDCLASS::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    bindAll(interp, objv);      // Bind thewords to the interpreter.

    try {
        requireExactly(objv, 3, "Incorrect number of command parameters");
        std::string name = objv[1];
        int         base = objv[2];


        if (exists(name)) {
            throw std::string("Scaler already exists");
        }
        m_pBank->AddScalerModule(new CSIS3820Scaler(name.c_str(), base));
    }
    catch(std::string msg) {
        throw CTCLException(interp, TCL_ERROR, msg.c_str());   // Our caller handles that.
    }
    return TCL_OK;
}

/**
 *  exists
 *    @param name - name of a possibly existing module.k
 *    @return bool - true if there's already a scaler module named that.
 */
bool
CMDCLASS::exists(const std::string& name) {
    for (auto p : *m_pBank) {         // I think this works since we scaler banks have the iterator protocol.
        CSIS3820Scaler* pBank = dynamic_cast<CSIS3820Scaler*>(p);
        if(pBank && pBank->name() == name) return true;
    
    }
    return false;
}