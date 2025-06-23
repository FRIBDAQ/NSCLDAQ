
/*
@file CXLMTimstamp.h 
@brief Header for the XLM timestamp reader.
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef MVLC_CXLMTIMESTAMP_H
#define MVLC_CXLMTIMESTAMP_H

#include <CXLM.h>
#include "DeviceCommand.h"

class CXLMTimestamp : public XLM::CXLM
{

public:
  CXLMTimestamp();
  virtual ~CXLMTimestamp();

private:
  CXLMTimestamp(const CXLMTimestamp& rhs);
  CXLMTimestamp& operator=(const CXLMTimestamp& rhs); // assignment not allowed.
  int operator==(const CXLMTimestamp& rhs) const;	  // Comparison for == and != not suported.
  int operator!=(const CXLMTimestamp& rhs) const;


public:
  virtual void onAttach(XXUSB::CConfigurableObject & configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
   

};

// Creator command:

class XLMTSCommand : public DeviceCommand {
public:
  XLMTSCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~XLMTSCommand();

protected:
  virtual CReadoutModule* createDevice(std::string name);
};
#endif
