/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors
             Ron Fox
             Aaron Chester
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

/**
 * @file DDASReadoutMain.h
 * @brief Defines the production readout software for DDAS systems using
 * NSCLDAQ 10.0 and later.
 */

#ifndef DDASREADOUTMAIN_H
#define DDASREADOUTMAIN_H

#include <CReadoutMain.h>

class CTCLInterpreter;
class CExperiment;

/**
 * @class DDASReadoutMain
 * @brief Production readout class for DDAS systems.
 * @details
 * DDASReadoutMain is the 'application' class for the production readout
 * software for DDAS systems i.e. systems utilizing XIA digitizer modules.
 * The application class has overridden and implemented several member
 * functions from the CReadoutMain base class for use in this application.
 *
 * These are:
 * - AddCommands         : Extend the Tcl interpreter with additional commands.
 * - SetupRunVariables   : Creates an initial set of run variables.
 * - SetupStateVariables : Creates an initial set of state variables.
 * - SetupReadout        : Sets up the software's trigger and its response to
 *                         that trigger.
 * - SetupScalers        : Sets up the response to the scaler trigger and, if
 *                         desired, modifies the scaler trigger from a periodic
 *                         trigger controlled by the 'frequency' Tcl variable
 *                         to something else.
 *
 * For more information about how to tailor this code, see the SBS readout
 * CReadoutMain and Skeleton classes.
 */

class DDASReadoutMain : public CReadoutMain {
public:
  /**
   * @brief Setup the Readout.
   * @param pExperiment Pointer to the experiment object.
   */
  virtual void SetupReadout(CExperiment *pExperiment);
  /**
   * @brief Setup the scaler Readout.
   * @param pExperiment Pointer to the experiment object.
   */
  virtual void SetupScalers(CExperiment *pExperiment);
  /**
   * @brief Used to add Tcl commands. See the CTCLObjectProcessor class.
   * @param pInterp Pointer to CTCLInterpreter object that encapsulates the
   *   Tcl_Interp* of our main interpreter.
   */
  virtual void addCommands(CTCLInterpreter *pInterp);
  /**
   * @brief Setup run variables.
   * @param pInterp Pointer to CTCLInterpreter object that encapsulates the
   *   Tcl_Interp* of our main interpreter.
   */
  virtual void SetupRunVariables(CTCLInterpreter *pInterp);
  /**
   * @brief Setup state variables.
   * @param pInterp Pointer to CTCLInterpreter object that encapsulates the
   *   Tcl_Interp* of our main interpreter.
   */
  virtual void SetupStateVariables(CTCLInterpreter *pInterp);
};

#endif
