/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef SKELETON_H
#define SKELETON_H

#include <CReadoutMain.h>

class CTCLInterpreter;
class CExperiment;

/*
** This file is a skeleton for the production readout software for
** NSCLDAQ 10.0 and later.  The programmatic interface
** to NSCLDAQ 10.0 at the application level is a 'close match' to that
** of earlier versions.  The software itself is a complete re-write so
** some incompatibilities may exist.  If you find an incompatibility,
** please post it at daqbugs.nscl.msu.edu so that it can be documented,
** and addressed.  Note that this does not necessarily mean that
** the incompatibility will be 'fixed'.
**
**   ------------------------------------------------------------
**
** How to use this skeleton:
**
**  This skeleton is the 'application' class for the production readout software.
**  The application class has several member functions you can override
**  and implement to perform user specific initialization.
**  These are:
**    AddCommands         - Extend the Tcl interpreter with additional commands.
**    SetupRunVariables   - Creates an initial set of run variables.
**    SetupStateVariables - Creates an initial set of state variables.
**    SetupReadout        - Sets up the software's trigger and its response to 
**                          that trigger.
**    SetupScalers        - Sets up the response to the scaler trigger and, if desired,
**                          modifies the scaler trigger from a periodic trigger controlled
**                          by the 'frequency' Tcl variable to something else.
**
** For more information about how to tailor this skeleton, see
** the comments in Skeleton.cpp
*/
class Skeleton : public CReadoutMain
{
private:
  // if you need per instance data add it here:

public:
  // Overrides for the base class members described above.

  virtual void addCommands(CTCLInterpreter* pInterp);
  virtual void SetupRunVariables(CTCLInterpreter* pInterp);
  virtual void SetupStateVariables(CTCLInterpreter* pInterp);
  virtual void SetupReadout(CExperiment* pExperiment);
  virtual void SetupScalers(CExperiment* pExperiment);

private:
  // If you want to define some private member utilities, define them here.

};

#endif
