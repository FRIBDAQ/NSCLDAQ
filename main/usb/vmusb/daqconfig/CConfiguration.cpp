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

#include <config.h>
#include <TCLVariable.h>
#include "CConfiguration.h"
#include "CScalerCommand.h"
#include "CADCCommand.h"
#include "CCAENChainCommand.h"
#include "CCAENV830Command.h"
#include "CMarkerCommand.h"
#include "CStackCommand.h"
#include <CNADC2530.h>
#include "CMADCScalerCommand.h"
#include "CMADCChainCommand.h"
#include "CPSDCommand.h"
#include "CHINPCommand.h"
#include "CV1729Command.h"
#include "CV1495scCommand.h"
#include "CAddTclDriver.h"
#include "CVMUSBCommand.h"
#include "CDelay.h"
#include "CCBDCamacBranch.h"
#include "CCamacCrate.h"
#include "CULMTrigger.h"
#include "CLeCroy4300B.h"
#include "CLeCroy4434.h"
#include "CLeCroy2551.h"
#include "CXLMFERA.h"
#include "CCBD8210CrateController.h"
#include "CCBD8210ReadoutList.h"
#include "CSIS3316.h"
#include "CUserCommand.h"

#include <CMASE.h>
#include <CMADC32.h>
#include "CXLMTimestamp.h"
#include "CMQDC32RdoHdwr.h"
#include <CMTDC32.h>
#include <C3804.h>
#include <CV977.h>
#include <CV1x90.h>

#include "CMDPP16QDC.h"
#include "CMDPP32QDC.h"
#include "CMDPP32SCP.h"

#include <CReadoutModule.h>
#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>
#include <Exception.h>
#include <Globals.h>

#include <tcl.h>
#include <iostream>
#include <algorithm>

using namespace std;

//////////////////////////////////////////////////////////////////////////
//////////////////////////// Canonicals //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*!
  Construct the object.  This consists of 
  creating a new Tcl interpreter, wrapping into an object and
  the adding the commands to the interpreter needed to manage the
  configuration as well as saving those command processor objects for destruction
  lateron.
*/
CConfiguration::CConfiguration() :
  m_pInterp(0)
{
  Tcl_Interp* pInterp = Tcl_CreateInterp();
  Tcl_Init(pInterp);		// Initialize the pkg search paths.
  m_pInterp = new CTCLInterpreter(pInterp);


  // Register and keep the commands.

  m_Commands.push_back(new CADCCommand(*m_pInterp, *this));
  m_Commands.push_back(new CADCCommand(*m_pInterp, *this, "caenv965"));
  m_Commands.push_back(new CADCCommand(*m_pInterp, *this, "caenv792"));
  
  m_Commands.push_back(new CCAENChainCommand(*m_pInterp, *this));
  m_Commands.push_back(new CScalerCommand(*m_pInterp, *this));
  m_Commands.push_back(new CCAENV830Command(*m_pInterp, *this));
  m_Commands.push_back(new CMarkerCommand(*m_pInterp, *this));
  m_Commands.push_back(new CStackCommand(*m_pInterp, *this));
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "sis3804", new C3804));
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "hytec", new CNADC2530));
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "madc", new CMADC32));
  m_Commands.push_back(new CMADCScalerCommand(*m_pInterp, *this));
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "tdc1x90", new CV1x90));
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "v977", new CV977));
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "mase", new CMASE));
  m_Commands.push_back(new CMADCChainCommand(*m_pInterp, *this));
  m_Commands.push_back(new CPSDCommand(*m_pInterp, *this));
  m_Commands.push_back(new CHINPCommand(*m_pInterp, *this));
  m_Commands.push_back(new CV1729Command(*m_pInterp, *this));
  m_Commands.push_back(new CV1495scCommand(*m_pInterp, *this));
  m_Commands.push_back(new CAddTclDriver(*m_pInterp, *this));
  m_Commands.push_back(new CVMUSBCommand(*m_pInterp, *this));

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "mtdc", new CMTDC32));

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "delay", new CDelay));

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "XLMFERA", new CXLMFERA));

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "CBDCamacBranch", 
                                        new CCBDCamacBranch) );
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "XLMTimestamp", 
                                        new CXLMTimestamp) );
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "mqdc", new CMQDC32RdoHdwr) );
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "mdpp16qdc", new CMDPP16QDC));
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "mdpp32qdc", new CMDPP32QDC));
  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "mdpp32scp", new CMDPP32SCP));
  m_Commands.push_back(new CUserCommand(
       *m_pInterp, *this, "sis3316", new CSIS3316)
  );
  

  // Add hybrid drivers
  typedef CCBD8210CrateController Ctlr;
  typedef CCBD8210ReadoutList RdoList;

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "CBDCamacCrate", 
                            compat_clone(CCamacCrate<Ctlr,RdoList>())) );

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "CBDULMTrigger", 
                            compat_clone(CULMTrigger<Ctlr,RdoList>())) );

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "CBDLeCroy4300B", 
                            compat_clone(CLeCroy4300B<Ctlr,RdoList>())) );

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "CBDLeCroy4434", 
                            compat_clone(CLeCroy4434<Ctlr,RdoList>())) );

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "CBDLeCroy2551", 
                            compat_clone(CLeCroy2551<Ctlr,RdoList>())) );

    
//  m_Commands.push_back(new C3820TstampCommand(*m_pInterp, *this));
}
/*!
   Destruction must:
   - Destroy all the CReadoutModule object pointers that were added to m_Adcs
   - Destroy all the CReadoutModule object pointers that were added to m_Scalers.
   - Destroy all readout stacks.
   - Destroy all the CTCLObjectProcessors that were added to m_Commands.
   - Destroy m_pInterp which will implicitly invoke Tcl_DeleteInterp() on
     the interpreter it wraps.
*/
CConfiguration::~CConfiguration()
{
  for (int i = 0; i < m_Adcs.size(); i++) {
    delete m_Adcs[i];
  }
  for (int i = 0; i < m_Scalers.size(); i++) {
    delete m_Scalers[i];
  }
  for (int i = 0; i < m_Stacks.size(); i++) {
    delete m_Stacks[i];
  }
  for (int i = 0; i < m_Commands.size(); i++) {
    delete m_Commands[i];
  }
  delete m_pInterp;
}

////////////////////////////////////////////////////////////////////////////
//////////////////// Support for configuration /////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
   Process a configuration file.  Since configuration files are just
   Tcl scripts for our enhanced interpreter, we just EvalFile.
   errors are allowed to propagate upwareds.
   \param configFile : std::string
       Name of the configuration file to process.
*/
void
CConfiguration::processConfiguration(string configFile)
{
  try {
    m_pInterp->EvalFile(configFile);
    
  }
  catch (string msg) {
    cerr << "CConfiguration::processConfiguration caught string exception: "
	 << msg << endl;
    throw;
  }
  catch (char* msg) {
    cerr << "CConfiguration::processConfiguration caught char* exception: "
	 << msg << endl;
    throw;
  }
  catch (CException& error) {
    cerr << "CConfiguration::processConfiguration caught CException : "
	 << error.ReasonText() << " while " << error.WasDoing() << endl;
    cerr << Tcl_GetStringResult(m_pInterp->getInterpreter()) << endl;
    throw;
  }
  catch (...) {
    cerr << "CConfiguration::processConfiguration caught an unknown exception type\n";
    throw;
  }
  cout << "Configuration file successfully processed. \n";
}
/*!
   Locate an adc module by name.  This is used e.g. by configuration commands
   to prevent duplicat module creation and to locate a module to be configured.
   \param name : std::string
      Name of the module to locate.
   \return CReadoutModule*
   \retval NULL - Readout module not found.
   \retval other - A pointer to the CReadoutModule that was being looked for.

   \note - Since the configuration is likely only a few objects there's no
           need for anything fancier than a linear search.
*/
CReadoutModule*
CConfiguration::findAdc(string name)
{
  return find(m_Adcs, name);
}
/*!
   See findAdc above, but this version finds a scaler.
*/
CReadoutModule*
CConfiguration::findScaler(string name)
{
  return find(m_Scalers, name);
}
/*!
   See findAdc above, but this version finds a readout stack.
*/
CReadoutModule*
CConfiguration::findStack(string name)
{
  return find(m_Stacks, name);
}


/*!
   Add a new scaler module to the configuration.
   \param module : CReadoutModule
      Pointer to the new module.. Restrictions:
      - The module must have been dynamically created.
      - The module's storage ownership is passsed to this object and will be
        deleted by us when *this is destroyed.
*/
void
CConfiguration::addScaler(CReadoutModule* module)
{
  m_Scalers.push_back(module);
}

/*!
    Add a new adc module to the configuration.  See addScaler for
    description.
*/
void
CConfiguration::addAdc(CReadoutModule* module)
{
  m_Adcs.push_back(module);
}

/*!
   Adds a new stack to the list of stacks.  Note that there a ton
   of errors we could but don't check for:
   - There should not be duplicate stack numbers.
   - There should not be colliding vector/IPLS for triggered stacks etc.
*/
void
CConfiguration::addStack(CReadoutModule* module)
{
  m_Stacks.push_back(module);
}

/*!
  set the interpreter result.
  \param msg : std::string
*/
void
CConfiguration::setResult(string msg)
{
  Tcl_Obj* result = Tcl_NewStringObj(msg.c_str(), -1);
  Tcl_SetObjResult(m_pInterp->getInterpreter(), result);
}

////////////////////////////////////////////////////////////////////////////
/////////////////////// Support for config retrieval ///////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
  Return the adc configuration vector (a copy of it).

*/
vector<CReadoutModule*>
CConfiguration::getAdcs()
{
  return m_Adcs;
}
/*!
  Return the Scaler configuration vector
*/
vector<CReadoutModule*>
CConfiguration::getScalers()
{
  return m_Scalers;
}

/*!
  Return the stack configuration vector
*/
vector<CReadoutModule*>
CConfiguration::getStacks()
{
  return m_Stacks;
}

////////////////////////////////////////////////////////////////////////////
////////////////////// Private utilities ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CReadoutModule* 
CConfiguration::find(std::vector<CReadoutModule*>& modules,
		     std::string name)
{
  vector<CReadoutModule*>::iterator i = find_if(modules.begin(), modules.end(),
						MatchName(name));
  if(i != modules.end()) {
    return *i;
  } 
  else {
    return static_cast<CReadoutModule*>(NULL);
  }
}


bool
CConfiguration::MatchName::operator()(CReadoutModule* module)
{
  return (module->getName() == m_name);
}
