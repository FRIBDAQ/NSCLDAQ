
#ifndef CXLMTIMESTAMP_H
#define CXLMTIMESTAMP_H

#include <CXLM.h>
#ifdef MVLC_GENERATOR
#include <DeviceCommand.h>
#endif
class CXLMTimestamp : public XLM::CXLM
{

public:
  CXLMTimestamp();
  CXLMTimestamp(const CXLMTimestamp& rhs);
  virtual ~CXLMTimestamp();

private:
  CXLMTimestamp& operator=(const CXLMTimestamp& rhs); // assignment not allowed.
  int operator==(const CXLMTimestamp& rhs) const;	  // Comparison for == and != not suported.
  int operator!=(const CXLMTimestamp& rhs) const;


public:
#ifdef MVLC_GENERATOR
  virtual void onAttach(XXUSB::CConfigurableObject & configuration);
#else
  virtual void onAttach(CReadoutModule& configuration);
#endif
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
#ifndef MVLC_GENERATOR
  virtual CReadoutHardware* clone() const; 
#endif

};

#ifdef MVLC_GENERATOR

// Creator command:

class XLMTSCommand : public DeviceCommand {
public:
  XLMTSCommand(CTCLInterpreter& interp, TCLConfigParser& parser);
  virtual ~XLMTSCommand();

protected:
  virtual CReadoutModule* createDevice(std::string name);
};
#endif



#endif
