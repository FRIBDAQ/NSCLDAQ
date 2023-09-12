/*
*-------------------------------------------------------------
 
 CAEN SpA 
 Via Vetraia, 11 - 55049 - Viareggio ITALY
 +390594388398 - www.caen.it

------------------------------------------------------------

**************************************************************************
* @note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
*
* @file     VX2750TclConfig.h
* @brief    Define a command that can configure CAENVX27x0 modules via Tcl.
* @author   Ron Fox
*
*/
#ifndef VX2750TCLCONFIG_H
#define VX2750TCLCONFIG_H
#include <TCLObjectProcessor.h>
#include <vector>
#include <map>

class CTCLInterpreter;
class CTCLobject;

namespace caen_nscldaq {

class VX2750PHAModuleConfiguration;



/**
 * @class VX2750TclConfig
 *    TCL Object command which can configure and fetch the configuration
 *    of  modules via Tcl. Note this does not actually configure the modules.
 *  vx27x0 create name  - Creates a new module.
 *  vx27x0 config  name param value ...
 *  vx25x0 cget name ?param?
 *  vx25x0 delete name
 *  vx25x0 list
 */
class VX2750TclConfig : public ::CTCLObjectProcessor
{
private:
    std::map<std::string, VX2750PHAModuleConfiguration*>  m_modules;
public:
    VX2750TclConfig(CTCLInterpreter& interp, const char* pName);
    virtual ~VX2750TclConfig();
    
    int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    VX2750PHAModuleConfiguration* getModule(const char* pName);
    std::vector<std::string> listModules();
    
private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void list(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    VX2750PHAModuleConfiguration* getConfigOrThrow(std::string name);
  
};

 
 }                         // namespasce.


#endif
