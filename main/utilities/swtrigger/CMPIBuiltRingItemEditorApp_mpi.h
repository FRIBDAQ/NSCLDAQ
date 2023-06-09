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

/** @file:  CMPIBuILTRingItemEditorApp_mpi.h
 *  @brief: Defines  the application class for the MPI based ring item editor.
 */
#ifndef CMPIBUILTRINGITEMEDITORAPP_MPI_H
#define CMPIBUILTRINGITEMEDITORAPP_MPI_H

#include "CBuiltRingItemEditorApp.h"

class CMPIAppStrategy;

/**
 * @class CMPIBuiltRingItemEditor
 *   This just has to select the correct processing element
 *   to use based on the rank.
 */
class CMPIBuiltRingItemEditorApp : public CBuiltRingItemEditorApp
{
private:
    CMPIAppStrategy* m_strategy;
public:
    CMPIBuiltRingItemEditorApp(int argc, char** argv, gengetopt_args_info& args);
    virtual ~CMPIBuiltRingItemEditorApp();
    
    virtual void operator()();

};


#endif