#ifndef __CCAENMODULE_H
#define __CCAENMODULE_H
//
//  Header files:
#ifndef __CDIGITIZERMODULE_H
#include "CDigitizerModule.h"
#endif

#ifndef __SPECTRODAQ_H
#include <spectrodaq.h>
#endif

#ifndef __STL_STRING
#include <string>
#endif

// forward class definitions:

class CAENcard;
class CTCLInterpreter;

/*!
   This class encapsulates the common features of the
   CAEN 32 channel adc,tdc,qdc modules.
   For the most part these modules are all identical,
   they are peak sensing ADC's with differnt x to peak
   converters.  All modules support the following configuration
   parameters:

  - crate     n  - n is an integer VME crate number
                   for the crate holding the module.
  - slot      n  - n is the VME slot number holding the 
                   VME card.
  - threshold t[32]  - t are the 32 thresholds in mv. Value 
                    will be computed taking into account 
                    the full scale selection.
  - keepunder b  - b is "on" or "off" depending on whether or not to keep
                  data that does not make threshold.
  - keepoverflow b - b is "on" or "off" depending on whether 
                  or not to keep overflow data.
  - keepinvalid b - b is "on" or "off" depending on whether 
                  or not to keep data that is strobed in 
                  when the module is busy resetting.
  - card  b   - "on" to enable the card "off" to disable it.
  - enable i[32] - 32 channel enables.  Nonzero is enabled, 
                  zero disabled.
  - fastclearwindow n - N is the width of the fast clear window in
                    ns.  Note that the current module has a width
		    window width granularity of 31.25ns.  The software
		    will choose the closest value, and enforce a 
		    minimum of 0 and a maximum of 2ms (2,000,000 ns).
  - multievent b - True to run the module in multievent mode.
  in multievent mode, the module is note cleared after each readout 
  in Prepare()

*/
class CCAENModule : public CDigitizerModule
{
private:
  CAENcard* m_pCAENcard;	//!< Pointer to the underlying driver
  bool      m_fMultiEvent;	//!< If true Prepare does not clear.
  int       m_nLoopTime;         //!< # times to wait for data ready.
public:
  // Constructors:
  CCAENModule(const string& rCommand,
	      CTCLInterpreter& rInterp);
  virtual ~CCAENModule();
private:
  CCAENModule(const CCAENModule& rhs);
  CCAENModule& operator= (const CCAENModule& rhs);
  int          operator==(const CCAENModule& rhs);
  int          operator!=(const CCAENModule& rhs);

  // Selectors.   The derived classes need to ensure that thes
  // are not returning consts...

public:
  CAENcard* getCard() {
    return m_pCAENcard;
  }
  bool isMultiEvent() const {
    return m_fMultiEvent;
  }

  // Mutators..
protected:
  void setCard(CAENcard* pCard) {
    m_pCAENcard = pCard;
  }
  void setMultiEvent(bool on) {
    m_fMultiEvent = on;
  }
  // Class operations:

public:

  virtual   void Initialize ();  
  virtual   void Prepare ();  
  virtual   void Read (DAQWordBufferPtr& rBuffer); 
  virtual   int  Read(void*  pBuffer);
  virtual   void Clear ();


};

#endif
