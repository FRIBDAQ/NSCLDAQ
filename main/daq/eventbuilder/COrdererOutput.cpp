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
#include "COrdererOutput.h"
#include <CBufferedOutput.h>
#include <TCLInterpreter.h>
#include <iostream>
#include <iostream>

#include <string>
#include <fragment.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <io.h>

using namespace ufmt;

static const int BUFFERSIZE=1024*1024;  // Hard coded for now.
/*------------------------------------------------------------------------
**  Canonical methods
*/

/**
 * Constructor:
 *    Translates the channel name into a Tcl_Channel.  If that fails throws
 *    an exception string.
 *    Gets an instance of the Fragment handler and registers ourselves as an 
 *    observer so that we'll get fragments.
 *    we also ensure the -encoding is binary and the -translation binary
 *
 * @param fd  - File descriptor on which to write dta.
 */
COrdererOutput::COrdererOutput(int fd) :
  m_OutputChannel(fd),
  m_pVectors(nullptr),
  m_nVectors(0)
{

  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->addObserver(this);

  long maxWrite = fcntl(m_OutputChannel, F_GETPIPE_SZ, nullptr);
  if (maxWrite == -1) {
    std::cerr << "* Warning * fcntl would not return a value for F_GETPIPE_SZ\n";
    std::cerr << "* Using 1Mbyte by default\n";
    std::cerr.flush();
    maxWrite = 1024*1024;
  } else {
    //std::cerr << " Maximum output write size: " << maxWrite << std::endl;
    std::cerr.flush();
  }
  m_nMaxWrite = maxWrite;
}
/**
 * Destructor:
 *    Since this will be invalid, we need to remove ourselves from the listener list:
 */
COrdererOutput::~COrdererOutput()
{
  CFragmentHandler* pHandler = CFragmentHandler::getInstance();
  pHandler->removeObserver(this);
  free(m_pVectors);
}
/*------------------------------------------------------------------------
** Public methods.
*/

/**
 * operator()
 *    Receives a vector of events from the fragment handler.
 *    These events are time ordered, unless there are time order
 *    errors.  The events are output the file descriptor m_OutputChannel
 *    via the buffered I/O package.
 *
 * @param event - vector of fragments to output.
 */
void
COrdererOutput::operator()(const EvbFragments& event)
{
  // Optimal writes are the size of the PIPE buffer.
  // This allows us to overlap the production of the next batch of iovs
  // while the consumer end of the pipe is reading the block of data
  // we'v put in.  This gets the maximum write size we'll allow:
  
  
  int maxWrite = m_nMaxWrite;
  
  // Minimize dynamic memory management:
  
  int nIovs = event.size()*2;
  if (nIovs > m_nVectors) {
    free(m_pVectors);
    m_pVectors = static_cast<iovec*>(malloc(nIovs * sizeof(iovec)));
    m_nVectors = nIovs;
  }
  
  iovec*  iovs = m_pVectors;
  int n(0);
  size_t totalWrite(0);
  for (auto i = event.begin(); i != event.end(); i++) {
    auto p = i->second;
    
    // If this will overflow maxwrite - output what we have so far:
    // and reset stuff.  This allows us to produce the next set
    // of iovs overlapped with the consumer getting this set:
    // note we require at least one write so that we will progress.
    
    size_t nBytes = sizeof(EVB::FragmentHeader) + p->s_header.s_size;
    if (((totalWrite + nBytes) > maxWrite) && (n > 0)) {
      dumpOutput(iovs, n);
      n = 0;
      totalWrite = 0;
    }
    
    iovs[n].iov_base = &(p->s_header);
    iovs[n].iov_len  = sizeof(EVB::FragmentHeader);
    
    iovs[n+1].iov_base = p->s_pBody;
    iovs[n+1].iov_len  = p->s_header.s_size;
    
    n += 2;
    totalWrite += nBytes;
    if (totalWrite > maxWrite) {
      dumpOutput(iovs, n);
      n = 0;
      totalWrite = 0;
    }
    
  }
  // If we have a partial buffer, output it:

  if (n) {
    dumpOutput(iovs, n);
    n = 0;  // Not strictly needed.
    totalWrite = 0;
  }
  return;


}

/*-----------------------------------------------------------------------------
** Utiltity methods
*/

/**
 * Throw an error string given an error with errno:
 *.
 * @param prefixMessage - prefixes the error message.
 */
void
COrdererOutput::ThrowErrnoString(const char* prefixMessage) const
{
  int error = Tcl_GetErrno();
  std::string msg(prefixMessage);
  msg += Tcl_ErrnoMsg(error);
  
  throw msg;
}
/**
 * dump output via writeDataVUnlimited
 *
 * @param iovs - array of write vectors.
 * @para n    - Number of write vecstors.
 * @note      - Caller must assure that n>0.
 */
void
COrdererOutput::dumpOutput(iovec* iovs, int n)
{
  try {
    io::writeDataVUnlimited(m_OutputChannel, iovs, n);
  
  }
  catch (...) {
    std::cerr << "Output Thread caught an exception writing data\n";
    std::cerr.flush();
    throw;
  }  
}
