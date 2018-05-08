/* Null trigger */

#include <config.h>
#include <iostream>
#include "CMyTrigger.h"
#include <stdlib.h>

#include "pixie16app_export.h"
#include "pixie16sys_export.h"

//#ifdef HAVE_STD_NAMESPACE
using namespace std;
//#endif

CMyTrigger::CMyTrigger(): m_retrigger(false), m_fifoThreshold(EXTFIFO_READ_THRESH*10)
{
  //  If FIFO_THRESHOLD is defined and is a positive integer, it replaces
  //  the default value of m_fifoThreshold - the number of words that must be
  //  in the fifo for the trigger to fire.
  //
  const char* pThreshold = getenv("FIFO_THRESHOLD");
  if (pThreshold) {
    char* pEnd;
    unsigned long newThreshold = strtoul(pThreshold, &pEnd, 0);
    if (newThreshold && (pEnd != pThreshold)) {
      m_fifoThreshold = newThreshold;
    }
  }
}

CMyTrigger::~CMyTrigger()
{
}

void CMyTrigger::setup()
{
    // nothing to be done at start of data taking regarding trigger
}

void CMyTrigger::teardown()
{
    // called as data taking ends.  
    // DDAS does not need any further signal as data taking ends
    // since this function is also called on a pause of data taking
    // don't even think about desyncing modules here
}


// control for determing if trigger should poll modules or pass control back
// to CEventSegment for processing the previous block of data
void CMyTrigger::Reset()
{
    m_retrigger = false;
}

/* Receive the number of modules in the pixie16 setup from the event 
   segment class */
void CMyTrigger::Initialize(int nummod)
{
    NumberOfModules = nummod;
    m_retrigger = false;
}

bool CMyTrigger::operator()() 
{
    try{

        /* If pixie16 is in the middle of processing a data buffer in the event 
           segment, continue processing the data buffer.  Return a true trigger 
           to pass control back the event segment. */
        if(m_retrigger){
            return true;
        } else {
            /* If there are no buffers currently being processed in the event segment 
               look at the pixie16 hardware to see if data currently needs to be 
               read out. */

            // Read number of 32-bit words inside the FIFO on Pixie16
            int retval = -1;

            for (int i=0; i<NumberOfModules; i++) {
                // Check how many words are stored in Pixie16's readout FIFO
                ModNum = i;
                nFIFOWords = 0;
                retval = Pixie_Read_ExtFIFOStatus(&nFIFOWords,ModNum);

                /* Trigger a read if the number of words in the external FIFO of 
                   any pixie16 module in a crate exceeds a threshold defined in 
                   pixie16app_defs.h */

                if(nFIFOWords > m_fifoThreshold){
#ifdef PRINTQUEINFO
                    std::cout << "CMyTrigger:: trig satisfied...mod=" << i << " nwords=" << nFIFOWords << std::endl;
#endif
                    m_retrigger = true;
                    return true;
                }

            } // end module loop

        }

        return false;	// Currently not enough data to trigger

    }
    catch(...){
        cout << "exception in trigger " << endl;
    }

    return false;
}


