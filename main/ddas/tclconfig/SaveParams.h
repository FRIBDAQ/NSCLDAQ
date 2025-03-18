/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  SaveParams.h
 *  @brief: Wrapper for Pixie16SaveDSPParametersToFile
 */
#ifndef SAVEPARAMS_H
#define SAVEPARAMS_H
#include "CTclCommand.h"
#include <string>

/**
 * @class CSaveParams
 *    Save the DSP parameters for all modules to file.
 *    Only one extra command line parameer is accepted, the name
 *    of the file to which the parameters will be written.
 */
class CSaveParams : public CTclCommand
{
public:
    CSaveParams(Tcl_Interp* pInterp);
    virtual ~CSaveParams();
    
    virtual int operator()(std::vector<Tcl_Obj*>& objv);
    
private:
    std::string apiMessage(const char* filename, int status);
};

#endif
