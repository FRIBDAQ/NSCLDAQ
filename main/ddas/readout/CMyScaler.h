/*********************************************************
 Declaration of Scaler class for DDAS
 Access statistics directly from the Pixie16 modules
 H.L. Crawford 6/13/2010
*********************************************************/

#ifndef __MYSCALER_H
#define __MYSCALER_H

#include <config.h>
#include <CScaler.h>
#include <vector>
//#include <stdint.h>
//#include "pixie16app_globals.h"

//#ifdef HAVE_STD_NAMESPACE
using namespace std;
//#endif

class CMyScaler : public CScaler
{
 private:
  unsigned short numModules;
  unsigned short crateID;
  unsigned short moduleNumber;
  double PreviousCounts[16];
  double PreviousCountsLive[16];

  vector<uint32_t> scalers;

 public:
  CMyScaler(unsigned short moduleNr, unsigned short crateid); // Constructor
  ~CMyScaler();
  virtual void initialize();
  virtual vector<uint32_t> read();
  virtual void clear();
  virtual void disable();
  virtual unsigned int size() {return 32;};
};

#endif
