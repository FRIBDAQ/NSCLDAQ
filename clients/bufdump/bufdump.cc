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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";
// Assumptions:
//   Buffers tagged type 2 are event buffers.
//   Buffers tagged type 3 are control buffers.
//   We want them all.. unsampled for now.
//
#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <Iostream.h>
#include <Iomanip.h>
#include <buftypes.h>
#include <daqdatatypes.h>
#include <buffer.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


#ifndef SPECTRODAQ_H
#include <spectrodaq.h>
#endif

#include <CopyrightNotice.h>

// How big a buffer do we want?

#define BUFLEN 0


//=====================================================================
//
// CopyOut - Copy a bunch of data out of a word buffer.
//
void CopyOut(void* pDest, DAQWordBufferPtr pSrc, unsigned int nWds)
{
  short* pWDest = (short*)pDest;
  for(int i = 0; i < nWds; i++) {
    *pWDest++ = *pSrc;
    ++pSrc;			// With objects ++pre is faster than post++
  }
}


/*===================================================================*/
class DAQBuff : public DAQROCNode {
  int operator()(int argc,char **argv) {
    int j;
    DAQWordBuffer bbuf(BUFLEN);
    DAQURL sinkurl("TCP://localhost:2602/");
    long sinkid;

    // Output copyright and author credit on stderr:

    CopyrightNotice::Notice(cerr, 
			    argv[0], "2.0", "2002");
    CopyrightNotice::AuthorCredit(cerr,
				  argv[0], 
				  "Ron Fox", "Eric Kasten", NULL);


    // In case we're running spectrodaq on something other than
    // the default URL.
    if (argc > 1) {
      cerr << "Using URL: " << argv[1] << endl;
      sinkurl = argv[1];
    } 

    // Tag this buffer so we know what type of buffer to receive.


    // Add a sink for this tag
    sinkid = daq_link_mgr.AddSink(sinkurl,2,2);

    // If the sinkid == 0, then the AddSink failed.
    cerr << "Added Sink Id " << sinkid << endl;
    if (sinkid <= 0) {
      cerr << "Failed to add a sink." << endl;
      exit(-1);
    }

    // receive buffers and put them out to stdout (normally a pipe to
    // SpecTcl.
    // 
    while(1) {

      // Accept a buffer (with wait).
      SetProcessTitle("dumper - Accepting");
      do {
	bbuf.SetTag(2);
	bbuf.SetMask(2);
	bbuf.Accept();	// 
      } while(bbuf.GetLen() == 0);
      SetProcessTitle("dumper - Copying");

      DAQWordBufferPtr pBuf = &bbuf;
      
      int nLength = bbuf.GetLen();
      short* pLocalBuffer = new short[nLength];

      CopyOut(pLocalBuffer, pBuf, bbuf.GetLen());

      bbuf.Release();		// Implicitly sets the buffer size -> 0
				// Avoiding deadlocks.

      // From now on we can operate on the local copy of the buffer.
      
      {
	char titlestring[1000];
	sprintf(titlestring, "dumper - piping fd=%d", fileno(stdout));
	DAQString process(titlestring);


	SetProcessTitle(process);
      }
      bool jumbo = false;
      struct bheader* pHeader = (struct bheader*)pLocalBuffer;
      if(pHeader->type == DATABF) { // Data buffer
	if (pHeader->buffmt >= JUMBO_BUFFER_REVISION) jumbo = true;

        if(pHeader->nevt != 0) {
	   cout << "-------------------------- Event (first Event) -------------------\n";

	   // For now assume little endian ..

	   unsigned int size    = pLocalBuffer[16];

	   if (jumbo) {
	     size  |= ((unsigned int)(pLocalBuffer[17]) << 16);
	   }
	   cout << dec <<   " Header: \n";
	   for(int i =0; i < 16; i ++) {
	     cout << pLocalBuffer[i] << " "; 
	   }
	   // For the VM-USB it's possible for the size to look larger than the buffer
	   // size if stacks other than zero are used.. in that case, assume it's a VM_usb
	   // and mask the size by 0xfff and add one since size is not self-inclusive.
	   // 
	   if (size > bbuf.GetLen()) {
	     size = (size & 0xfff) + 1;
	   }
	   cout << dec << endl  << "Event: \n";
	   short *pEvent          = &(pLocalBuffer[16]);
	   cout << size << "(10) words of data" << hex;
	   for(int i = 0; i < size; i++) {
	     if((i % 8) == 0) cout << endl;
	     cout << *pEvent << " "; 
	     pEvent++;
	   }
	   cout << dec << endl;
	 }
	 else {
	    cout << "--------------------------- Empty Event Buffer ------------------\n";
	 }
      }
      else {
	cout << "---------------------- Non event --------------\n";
	cout << "Buffer size is: " << pLocalBuffer[0];
	for(int i = 0; i < 16; i++) {
	  if((i % 8) == 0) cout << endl;
	  cout << pLocalBuffer[i] << " ";
	}
	cout << endl;
      }
      delete []pLocalBuffer;
    }

  //sleep(10);
  // Delete the sink.
    daq_link_mgr.DeleteSink(sinkid);
  } 
};

// DAQBuff mydaq;

int main(int argc, char** argv, char** envp) {
  DAQBuff mydaq;
  spectrodaq_main(argc, argv, envp);
}




