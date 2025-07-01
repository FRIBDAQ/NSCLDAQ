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
#include <MonitorException.h>
#include <stdio.h>
#include <limits.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Construction/destruction


/*!
  Construct the exception. The context text will be 
  "Performing operations on a Synchronizable object"
  The reason will be:
  Mutex should be owned by correctOwner but was owned by actualOwner at __FILE__:__LINE__ : WasDoing

  \param correctOwner - The Pid of the thread that should be owning the mutex.
  \param actualOwner  - The Pid of the thread that actually owns the mutex.
  \param file         - Name of the file in which the exception was constructed
                        Typically use __FILE__
  \param line         - Line number in file where exception was constructed,
                        Typically use __LINE__

*/
CMonitorException::CMonitorException(int correctOwner, int actualOwner,
				     const char* file, const char* line) :
  CException("Performing operations on a synchrnonizeable object")
{
  char s[1000];
  snprintf(s, sizeof(s), 
	   "Mutex should be owned by %d but was owned by %d at %s:%s",
	   correctOwner, actualOwner, file, line);
  m_MessageText = s;
}

/*!
  Copy construction:
*/
CMonitorException::CMonitorException (const CMonitorException& rhs) :
  CException(rhs)
{
  m_MessageText = rhs.m_MessageText;
}

/////////////////////////////////////////////////////////////////////////////////
// Canonical functions (or things the compiler could do to you when you're not looking.

/*!
   Assignement
*/
CMonitorException&
CMonitorException::operator=(const CMonitorException& rhs)
{
  if (this != &rhs) {
    CException::operator=(rhs);
    m_MessageText = rhs.m_MessageText;
  }
  return *this;
}
/*!
   Comparison for equality.
*/
int
CMonitorException::operator==(const CMonitorException& rhs) const
{
  return (CException::operator==(rhs)                  &&
	  (m_MessageText == rhs.m_MessageText));
}
/*!
   Inequality is when equality is not true:
*/
int
CMonitorException::operator!=(const CMonitorException& rhs) const
{
  return !(*this == rhs);
}

////////////////////////////////////////////////////////////////////////////
// Implementing the CException interface:

/*!
  Return human readable exception information. See the class description to
  see what that means.

*/
const char*
CMonitorException::ReasonText() const
{
  return m_MessageText.c_str();
}
/*!
  Return the computer usable reason code. This is always -1 in this 
  class.
*/
int
CMonitorException::ReasonCode() const
{
  return -1;
}
