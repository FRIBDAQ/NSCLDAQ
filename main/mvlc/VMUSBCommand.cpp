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

@file VMUSBComand.cpp
@author Ron Fox <fox at frib dot msu dot edu>
@brief implementation  for  Tcl encapsulation of CVMUSB
*/

#include "VMUSBCommand.h"
#include "CVMUSB.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <Exception.h>
#include <stdexcept>
#include <string>
#include <sstream>

/**
 *  constructor
 *     @param interp -the intepreter on wihch this command ('mvlc') is registered.
 */
VMUSBCommand::VMUSBCommand(CTCLInterpreter& interp) :
    CTCLObjectProcessor(interp, "mvlc", TCLPLUS::kfFALSE),
    m_pController(0)
{}

/**
 *  destructor
 *     Since our client retain ownership of the CVMUSB we don't really need to do
 * anythning.
 */
VMUSBCommand::~VMUSBCommand() {}

/**
 * setController
 *     Provide the controller on which the subcommands will operate.  If this is not
 * done prior to the command being used, the mvlc command will generate an error
 * indicating this must be done.  See operator() and throwIfNoController.
 * 
 * @param controller - references the controller we will operate on.
 * @note when the controller is no longer available (e.g. goes out of scope) clearController must be
 * called or else probably segfaults will be the result of any 'mvlc' command
 */
void
VMUSBCommand::setController(CVMSBU& controller) {
    m_pController = &controller;
}
/**
 * clearController
 *    Sets the controller to null.  A setController must be done prior to allowing any
 * 'mvlc' commands.  This must be called when the controller object wrapped by us is 
 * no longer in scope or deleted.
 */
void
VMUSBCommand::clearController() {
    m_pController = nullptr;
}
/**
 *  getController
 *     Returns the controller object that the command is using.
 * 
 * @return CVMUSB* - possibly null if either no setController was done or clearController was the
 * last controller management operation.
 */
CVMUSB*
VMUSBCommand::getController() {
    return m_pController;
}

/**
 * operator()
 *     Perform the 'mvlc' command.  This requires a valid subcommand, and the existence of
 * a wrapped controller.  The setup is done for exception based processing.
 * 
 * @param interp - interpreter that's running the command.
 * @param objv   - THe encapsulated command words.  We require at least two of them.
 * @return int   - Command status, TCL_OK on success, TCL_ERROR on failure.
 */
int
VMUSBCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    bindAll(interp, objv);

    try {
        throwIfNoController();                  // Don't even bother if no controller.
        requireAtLeast(objv, 2, "Subcommand is required");
        
        // Dispatch to the subcommand handler or error:

        std::string subcommand = objv[1];
        if (subcommand == "vmewrite32") {
            vmewrite32(interp, objv);
        } else if (subcommand == "vmewrite16") {
            vmewrite16(interp, objv);
        } else if (subcommand == "delay") {
            delay(interp, obvj);
        } else if (subcommand == "loopuntil32") {
            loopuntil32(interp, objv);
        } else if (subcommand == "loopuntil16") {
            loopuntil16(interp, objv);
        } else {
            std::stringstream smsg;
            smsg << subcommand << " Is not a valid subcommand for " << std::string(objv[0]);
            std::string msg(smsg.str());
            throw std::logic_error(msg);
        }
        


    }
    catch (std::exception& e) {
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (std::string msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    

    return TCL_OK;
}

//////////////////////////////////// Private utilities (including subcommand handlers)

/**
 * vmewrite32
 *    Add a 32 bit write to the controlle (already verified).
 * 
 * @param interp - the interpreter running the command.
 * @param objv   - The command words
 * 
 * All errors are thrown
 */
void
VMUSBCommand::vmewrite32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    auto params = getWriteParameters(objv);              // Throws on conversion and bad number.
    
    //                              addr                amod               data
    m_pController->vmeWrite32(std::get<0>(params), std::get<1>(params), std::get<2>(params));
}
/**
 *  vmewrite16
 *    Same as above but the write is 16 bits not 32.
 */
void
VMUSBCommand::vmewrite16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    auto params = getWriteParameters(objv);

    m_pController->vmeWrite16(
        std::get<0>(params), std::get<1>(params), 
        static_cast<uint16_t>(std::get<3>(params)
    );
}
/**
 * delay
 *    Perform (or rather add) a delay:
 * 
 * 
 * @param interp - interpreter running the command.
 * @param objv   - the command words.
 */
void
VMUSBCommand::delay(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    requireExactly(3, objv, "Delay requires a duration and only a duration");
    int ms = objv[2];

    m_pController->delay(ms);
}
/**
 * loopuntil32
 *     Waits until a condition is satisfied.  This  does a 32 bit read from
 * VME, and loops on that read until the value provided is equal to the field
 * defined by the mask.  So, for example.  See the CVMUSB comments for more
 * information.
 * 
 * @param interp - interpreter executing the command.
 * @param objv   - the command words.
 */
void
VMUSBCommand::loopuntil32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
    auto params = getLoopParameters(objv);

    m_pcontroller->loopUntil32(
        std::get<0>(params), std::get<1>(params), 
        std::get<2>(params), std::get<3>(params)
    );
}
/**
 *  loopuntil16
 *    Same as above but the read is a 16 bit read.
 */
void
VMUSBCommand::loopuntil16(CTLInterpreter& interp, std::vector<CTCLObject>& objv) {
    auto params = getLoopParameters(objv);

    m_pcontroller->loopUntil16(
        std::get<0>(params), std::get<1>(params), 
        std::get<2>(params), std::get<3>(params)
    );
}

/**
 * throwIfNocontroller
 *     If there's not a controller, throw std::logic_error
 */
void
VMUSBCommand::throwIfNoController() {
    if (!m_pController) {
        throw std::logic_error(
            "BUG BUG - the mvlc command was executed before a controller was established"
        );
    }
}


/**
 * getWriteParameters
 *    Both writes are of the form 'mvlc vmewritexx adress amod data'
 * 
 * This method
 * 1.  Ensures there are exactly the correct number of parameters.
 * 2.  extracts the paramters into a tuple.
 * 
 * @param objv - Command words (already bound to an interpreter).
 * @return std::tuple<uint32_t, uint8_t, uint32_t> - in order the address,
 *  address modifier, and data.
 * @throw std::string - there are not the correct number of command parameters.
 * @throw std::invalid_argument amod is not an 8 bit clean word.
 * @throw CTCLException (CException) - objv conversions fail.
 */
std::tuple<uint32_t, uint8_t, uint32_t>
VMUSBCommand::getWriteParameters(std::vector<CTCLObject>& objv) {
    requireExactly(objv, 5, "mvlc writs require address, modifier. and data");

    uint32_t addr = int(objv[2]);
    uint8_t amod = decodeAmod(objv[3]):
    uint32_t data = int(objv[4]);


    if(int(amod) != amodbig) {
        std::stringstream smsg;
        smsg << "Address modifier " << std::hex << amodbig << "  is not an 8 bit value";
        std::string msg(smsg.str());

        throw std::invalid_argument(msg);
    }

    return std::make_tuple(addr, amod, data);
}

/**
 * getLoopParameters
 * 
 *    Both loops are of the form 'mvlc loopuntilxx addr amod mask value'
 * 
 * This method 
 * 1. Ensures there are exactly the right number of parameters.
 * 2. decodes the parameters into a tuple.
 * 
 * @param objv - command line parameters.
 * @return std::tuple<uin32_t, uint8_t, uint32_t, uint32_t> containing
 *   in order, the address, address modifier, mask and value.
 * 
 * Throws the same exception set that getWriteParamters does.
 */
std::tuple<uint32_t, uint8_t, uint32_t, uint32_t>
VMUSBCommand::getLoopParameters(std::vector<CTCLObject>& objv) {
    requireExactly(objv, 6, "mvlc loops require address, modifier, mask and value");

    uint32_t addr = int(objv[2]);
    uint8_t  amod = decodeAmod(objv[3]);
    uint32_t mask = int(objv[4]);
    uint32_t value= int(objv[5])


    return std::make_tuple(addr, amod, mask, value);
}

/**
 *  decodeAmod
 *    Decode an address modifier from an encapsulated word.
 * 
 * @param val - the intepreter bound CTCLOobject& the amod should be in.
 * @return uint8_t
 * @throw CTCLException - if the value does not convert to an int.
 * @throw std::invalid_argumennt - if the value doesn't fit into 8 bits.
 */
uint8_t
VMUSBCommand::decodeAmod(CTCLObject& val) {
    uint32_t wideAmod = int(val);
    uint32_t amod =   wideAmod & 0xff;              // 8 bits only:

    if (amod != wideAmod) {
        std::stringstream smsg;
        smsg << std::hex << "0x" << wideAmod <<
             " is not a valid address modifier, must be only 8 bits wide";
        std::string msg(smsg.str());
        throw std::invalid_argument(msg);
    }
    return uint8_t(amod);
}