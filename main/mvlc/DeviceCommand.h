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
@brief Header for the base class of commands that control devices.

*/
#ifndef  MVLC_DEVICE_COMMAND_H
#define  MVLC_DEVICE_COMMAND_H

#include <TCLObjectProcessor.h> 
#include <string>

class TCLConfigParser;
class CReadoutModule;

/**
 *  @class DeviceCommand
 * 
 * This class provides a base class for the commands that will 
 * manage devices.  Each created device is a readout hardware
 * object and is attached to a configuration with various
 * options.
 * 
 * Each command is an ensemble within which, we can use the
 * strategy pattern to hoist quite a bit of code into this
 * command processor, kind of wish I'd done that for the
 * VMUSBReadout DAQConfig classes.
 * 
 * The ensemble includes the subcommands:
 * 
 * - create = we are able to see if this is a duplicate device name and,
 *   after callout out to the concreate device's creation, enter the new
 *   device inthe parser's device map and configure the 
 *   new device with any  additional configuration parameters.
 * - config - we can do all of that work; locating the device, 
 *        and processing its configuration.
 * - cget - again we can do all of the work.alignas
 * 
 * So in the end the only strategy callout i9s a factory method
 * to produce a new CReadoutModule.
 * 
 */
class DeviceCommand : public CTCLObjectProcessor {
    // object data:
private:
    TCLConfigParser& m_parser;    // We need parser services.

    // canonicals:

public:
    DeviceCommand(CTCLInterpreter& interp, const char* cmd, TCLConfigParser& parser);
    virtual ~DeviceCommand();
    
    // Forbidden canonicals:

private:
    DeviceCommand(const DeviceCommand& rhs);
    DeviceCommand& operator=(const DeviceCommand& rhs);
    int operator==(const DeviceCommand& rhs);
    int operator!=(const DeviceCommand& rhs);

    
    // Visible methods:

public:
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

    // Ensemble commands can be overidden, just in case.

protected:
    virtual void create(std::string name, std::vector<CTCLObject>& config);
    virtual void config(std::string name, std::vector<CTCLObject>& config);
    virtual void cget(CTCLInterpreter& interp, std::string name, const char* option = nullptr);
    
    // Strategy method(s)  These must be implemented in a concrete class.

protected:
    //
    virtual CReadoutModule* createDevice(std::string name) = 0; 

    // Selectors for derived classes:

    TCLConfigParser& getParser() {return m_parser; }
    
    // Private utilities:

private:
    std::vector<CTCLObject> getConfigArray(std::vector<CTCLObject>& objv) const;
    void throwDeviceError(std::string name, const char* reason) const;

};

#endif