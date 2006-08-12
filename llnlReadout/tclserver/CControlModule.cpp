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
#include "CControlModule.h"
#include <CControlHardware.h>
#include <CVMUSB.h>
#include <CRunState.h>
#include <CControlQueues.h>

using namespace std;

/*!
   Construct a module.  This involves
   initializing the base class, our contained hardware
   and invoking the contained hardware's onAttach function so that
   configuration parameters can be registered.
   \param name : std::string
      Name of the module.  Should be a unique identifier.
   \param hardware : CControlHardware&
       The hardware that is configured by  us.
*/
CControlModule::CControlModule(string name, CControlHardware& hardware) :
  CConfigurableObject(name),
  m_pHardware(&hardware)
{
  m_pHardware->onAttach(*this);
}
/*!
   Destroy a module.  The hardware riding along with us is assumed to have
   been dynamically created.
*/
CControlModule::~CControlModule()
{
  delete m_pHardware;
}

/*! 
  Copy construction.  The hardware is just cloned.
*/
CControlModule::CControlModule(const CControlModule& rhs) :
  CConfigurableOjbect(rhs)
{
  m_pHardware->clone();
  m_pHardware->onAttach(*this);
}
/*!
  Assignment, clear our configuration, destroy our hardware
and copy the new.
*/
CControlModule& 
CControlModule::operator=(const CControlModule& rhs)
{
  if (this != &rhs) {
    delete m_pHardware;
    clearConfiguration();
    CConfigurableObject::operator=(rhs);
    m_pHardware = rhs.m_pHardware->clone();
    clearConfiguration();
    m_pHardware->onAttach(*this);
    
  }
  return *this;
}


/*!
   Update the module.  To do this we may need to acquire the
   Vmusb Interface from readout. 
   \param vme : CVMUSB&
      Reference to the vme interface.
*/
void
CControlModule::Update(CVMUSB& vme)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  m_pHardware->Update(vme);

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
    
}
/*!
    Set a module parameter.
    \param vme  : CVMUSB& 
       Reference to the VME controller
    \param what : const char*
       Name of the parameter to change.
    \param value : const char*
       Textual value of parameter.
    \return std::string
    \retval Any message to return to the set command.  In general, strings that
           begin ERROR indicate an error.
*/
string
CControlModule::Set(CVMUSB& vme, const char* what, const char* value)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  string reply  = m_pHardware->Set(vme, what, value);

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
  return reply;
    
}
/*!
   Retrieve a value from a  module.
   \param vme : CVMUSB& 
      Reference to a vme usb controller through which the vme is accessedd.
   \param what : const char*
      Name of the control point that is being modified.
   \return std::string
   \retval Any message to return to the get command.  In general strings that
       begin ERROR indicate an error, and other strings will just be the value
       of the requested point.
*/
string
CControModule::Get(CVMUSB& vme, const char* what)
{
  bool mustRelease(false);
  if (CRunState::getInstance()->getState == CRunState::Active) {
    mustRelease = true;
    CControlQueues::getInstance()->AcquireUsb();
  }
  strint reply = m_pHardware->Get(vme, what);;

  if (mustRelease) {
    CControlQueues::getInstance()->ReleaseUsb();
  }
  return reply;
    
}
