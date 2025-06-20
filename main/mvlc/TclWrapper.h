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

@file TclWrapper.h
@author Ron Fox <fox at frib dot msu dot edu>
@brief Header for command to wrap a Tcl driver.
*/
#ifndef MVLC_TCLWRAPPER_H
#define MVLC_TCLWRAPPER_H
#include "CReadoutHardware.h"
#include "DeviceCommand.h"

namespace XXUSB {
    class CConfigurableObject;
}

/**
 *  TclWrapper
 *     This is a device hardware module that wraps a command ensemble which 
 * can generate code for MVLC readout configuration files.
 * The drivers supported have a high compatibility with VMUBSReadout modules.
 * It is, however, always important to keep in mind they run disconnected from hardware
 * and, therefore, cannot read from the devices they manage.
 * 
 * Very briefly, the way to use a Tcl driver is to:
 * - Create a Tcl class using e.g. namespace ensemble, snit::type, itcl::class or TclOO.
 * the class should support configuring instances snit::type and itcl::class instances
 * support option databases directly.  These are the recommended (though not only) ways
 * to create drivers.
 *  - In the daqqconfig file, create and configure an instancde of the Tcl class.
 *  - Use the addtcldriver command (defined hin this header as well) to wrap
 *  the instance you created in a TclWrapper. The wrapping is expressed by configuring
 *  the wrapper's -ensemble option.
 * 
 * Suppose I have a snit type named "MyDriver" all of this  is shown  below:
 * 
 * ```tcl
 * package require mydriver   ; # Pulls in snit::type MyDriver presumably.
 * set driver [MyDriver %AUTO%];   # Make the driver instance.
 * $driver config -base 0x12340000; # COnfigure hardware base address e.g.
 * tclwrapper create awrapper
 * tclwrapper config awrapper -ensemble #driver;  # Wrap the driver in awrapper.
 * ```
 * 
 * The -ensemble option is the only option tclwrapper drivers have.
 * 
 * The driver must implement the follwoing subcommands:
 * 
 * - Initialize - recieves a VMUSB command which can record VME operations as its
 * only parameter - this should initialize the hardware and make it ready to take data.
 * - addReadoutList - Called witha VMSUBReadoutList command that can record VME operations.
 * The driver should add operations to this list which are needed to read the device.
 * - onEndRun - Called to generate code with a VMUBS command.  The driver should provide
 * VME operations that are needed to shut down the device at the end of the run.
 *  
 * You can imagine that each of this driver's code generating methods just delegate to
 * the appropriate subcommand of the Tcl driver instance.
 */
class TclWrapper : public CReadoutHardware {
private:
    XXUSB::CConfigurableObject* m_pConfig;
    CTCLInterpreter*            m_pInterp;
    // canonicals:
public:
    TclWrapper(CTCLInterpreter& interp);
    virtual ~TclWrapper();

    // forbidden canonicals:
private:
    TclWrapper(const TclWrapper&);
    TclWrapper& operator=(const TclWrapper&);
    int operator==(const TclWrapper&) const;
    int operator!=(const TclWrapper&) const;

    // Driver methods:
public:
    virtual void onAttach(XXUSB::CConfigurableObject& configuration);
    virtual void Initialize(CVMUSB& controller);
    virtual void addReadoutList(CVMUSBReadoutList& list);
    virtual void onEndRun(CVMUSB& controller);
private:
    void executeCommand(const char* command);    
    std::string requireConfigured();                        

};


/**
 * @class AddTclDriver 
 *    Class that implements the 'addtcldriver' command to create a wrapping.
 */
class AddTclDriver : public DeviceCommand {
public:
    AddTclDriver(CTCLInterpreter& interp, TCLConfigParser& parser);
    virtual ~AddTclDriver();

protected:
    CReadoutModule* createDevice(std::string name);
};



#endif