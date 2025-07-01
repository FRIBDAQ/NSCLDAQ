#ifndef __MONITOREXCEPTION_H
#define __MONITOREXCEPTION_H
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


#ifndef __EXCEPTION_H
#include <Exception.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

/*!
   Objects of this class are exceptions that describe errors in a monitor states.
   A monitor is a synchronization primitive that is supported by the threads library in the
   NSCL DAQ base software libraries.

   There are several operations on monitors that require the monitor to be held by
   the invoking thread.  This exception is thrown if that is not the case.
*/
class CMonitorException : public CException
{
  // Object data:
private:
  std::string m_MessageText;

  // Constructors
public:
  CMonitorException(int correctOwner, int actualOwner, const char* file, const char* line);
  CMonitorException(const CMonitorException& rhs);
  virtual ~CMonitorException() {}

  // Canonicals:

public:
  CMonitorException& operator=(const CMonitorException& rhs);
  int operator==(const CMonitorException& rhs) const;
  int operator!=(const CMonitorException& rhs) const;

  // The Exception interface:

public:
  virtual const char* ReasonText() const;
  virtual Int_t       ReasonCode() const;



};

#endif
