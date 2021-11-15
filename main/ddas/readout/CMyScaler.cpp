/*********************************************************
 Scaler class for DDAS
 Access statistics directly from the Pixie 16 modules
 H.L. Crawford 6/13/2010
*********************************************************/

#include "CMyScaler.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

#include <config.h>


extern unsigned int ChanEventsB_Address[];
extern unsigned int ChanEventsA_Address[];
extern unsigned int FastPeaksA_Address[];
extern unsigned int FastPeaksB_Address[];


/* Constructor */
CMyScaler::CMyScaler(unsigned short moduleNr, unsigned short crateid) 
{
  moduleNumber = moduleNr;
  for (int i=0; i<16; i++) {
    PreviousCounts[i] = 0;
    PreviousCountsLive[i] = 0;
  }

  crateID = crateid;

  //  const char config[20]="cfgPixie16.txt";

  /* Scalers need to know crateID */
  //  ifstream input;
  //  char *temp = new char[80];
  //  input.open(config, ios::in);
  
  //  if(input.fail()){
  //   cout << "Scalers can't open the config input file ! " << config << endl 
  //	 << flush;
  //  } else {
  //  cout << "\nScalers reading config input file: " << config << endl << flush;
  // }

  //  input >> crateID;
  cout << "Scalers know crate ID = " << crateID << endl;
  //  input.getline(temp,80);
  //  input >> numModules;
  //  input.getline(temp,80);

  //  input.close();

}

CMyScaler::~CMyScaler()
{

  // Destructing here

}

void CMyScaler::initialize() 
{

  for (int a=0; a<16; a++) {
    PreviousCounts[a] = 0;
    PreviousCountsLive[a] = 0;
  }

}

void CMyScaler::disable(){
  // modules do not need scalers disabled at end of run
}

vector<uint32_t> CMyScaler::read()
{
  try{

  int retval;
  unsigned int statistics[448] = {0};
  
  retval = Pixie16ReadStatisticsFromModule(statistics, moduleNumber);
  if (retval < 0) {
    cout << "Error accessing scaler statistics from module " 
	 << moduleNumber << endl;
  } else {
    // cout << "Accessing statistics for module " 
    //      << moduleNumber << endl;
  }



  /* Now we need to calculate the # of events from the last
     read of the scalers -- NSCL scaler buffers just expect
     the # events since the last read.  However, Pixie16 statistics
     cannot be cleared, so we need to do some math */

  double ICR[16] = {0};
  double OCR[16] = {0};
  double LiveTime[16] = {0};
  double RealTime = 0;
  double Counts[16] = {0};
  double CountsLive[16] = {0};
  double ChanEvents[16] = {0}; 
  double FastPeaks[16] = {0};
  unsigned long OutputScalerData[33] = {0};
  
  /* Compute module RealTime */
  RealTime = Pixie16ComputeRealTime(statistics, moduleNumber);
  OutputScalerData[0] = (unsigned long)crateID;
  
  for (int i=0; i<16; i++) {
   
    /* Compute input count rate in counts/second for each channel */  
    ICR[i] = Pixie16ComputeInputCountRate (statistics, moduleNumber, i);

    /* Compute output count rate in counts/second for each channel */
    OCR[i] = Pixie16ComputeOutputCountRate (statistics, moduleNumber, i);

    /* Compute LiveTime for each channel */    
    LiveTime[i] = Pixie16ComputeLiveTime (statistics, moduleNumber, i);

    
    ChanEvents[i] = (double)statistics[ChanEventsA_Address[moduleNumber] + i 
				       - DATA_MEMORY_ADDRESS - 
				       DSP_IO_BORDER] * pow(2.0, 32.0);
    ChanEvents[i] += (double)statistics[ChanEventsB_Address[moduleNumber] + i 
					- DATA_MEMORY_ADDRESS - DSP_IO_BORDER];
    
    FastPeaks[i] = (double)statistics[FastPeaksA_Address[moduleNumber] + i - 
				      DATA_MEMORY_ADDRESS 
				      - DSP_IO_BORDER] * pow(2.0, 32.0);
    FastPeaks[i] += (double)statistics[FastPeaksB_Address[moduleNumber] + i 
				       - DATA_MEMORY_ADDRESS - DSP_IO_BORDER];


    /* Now compute total events for each channel, and total "live"
       events for each channel. */
    //    Counts[i] = (unsigned long)ICR[i]*LiveTime[i];
    //    CountsLive[i] = (unsigned long)OCR[i]*RealTime;
    Counts[i] = FastPeaks[i];
    CountsLive[i] = ChanEvents[i];
    
    /* Finally compute the events since the last scaler read! */
    OutputScalerData[(2*i + 1)] = (unsigned long) (Counts[i] - 
						   PreviousCounts[i]);
    OutputScalerData[(2*i + 2)] = (unsigned long) (CountsLive[i] - 
						   PreviousCountsLive[i]);

    //    if (i == 15 && crateID == 0 && moduleNumber == 0) {
    //      cout << ICR[i] << " " << OCR[i] << " " << LiveTime[i] << " " 
    //	   << RealTime << " " << Counts[i] << " " << PreviousCounts[i] 
    //	   << " " << CountsLive[i] << " " << PreviousCountsLive[i] << " " 
    //	   << ChanEvents << " " << FastPeaks << endl; 
    //    }

    PreviousCounts[i] = Counts[i];
    PreviousCountsLive[i] = CountsLive[i];

  }

  /* Copy scaler information into the output vector */
  scalers.clear(); //SNL added for new readout
  scalers.insert(scalers.end(), OutputScalerData, OutputScalerData+33);
  
  return scalers;

  }
  catch(...){
    cout << "exception in scaler " << endl;
  }
}

void CMyScaler::clear() 
{

  // Clear? Don't think we need this with Pixie16 -- can't clear!

}
