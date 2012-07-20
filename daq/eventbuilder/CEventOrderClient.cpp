/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "CEventOrderClient.h"
#include <ErrnoException.h>
#include <CSocket.h>
#include <CTCPConnectionFailed.h>

#include <CPortManager.h>
#include <os.h>
#include <errno.h>
#include <stdio.h>

#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <fragment.h>

static const std::string EventBuilderService("ORDERER"); // Advertised service name.

/**
 * Construct the object.
 * @param host - The host on which the event builder is listening for client
 *               connections.
 * @param port - The port on which the event builder is listening for client connections.
 */
CEventOrderClient::CEventOrderClient(std::string host, uint16_t port) :
  m_host(host),
  m_port(port),
  m_pConnection(0),
  m_fConnected(false)
{}

/**
 * Destroy the object. Any existing connection to the event builder is dropped.
 * properly. See the event builder protocol documentations for a description of 
 * what that means.
 */
CEventOrderClient::~CEventOrderClient()
{
  delete m_pConnection;
}

/**
 * Locate the event builder on the specified host and return
 * the port on which its server is listening for connections.
 * 
 * @param host - the host in which to perform the inquiry.
 * 
 * @return uint16_t
 * @retval - the port on which the event builder is listening for our username.
 *
 */
uint16_t
CEventOrderClient::Lookup(std::string host)
{
  CPortManager manager(host);
  std::vector<CPortManager::portInfo> services = manager.getPortUsage();
  std::string me  = Os::whoami();

  // Look for the first match for my username and the correct service.

  for (int i =0; i < services.size(); i++) {
    if (services[i].s_Application == EventBuilderService &&
	services[i].s_User        == me) {
      return services[i].s_Port;
    }
  }
  // Not running.
  // Use errno = ENOENT

  errno = ENOENT;
  throw CErrnoException("Looking up event builder service");

}
/**
 * Connect to a server.
 *
 * See eventorderer(5daq) for protocol information.
 *
 * @param description - the description used in the CONNECT message
 *                      to describe the client to the server.
 *
 */
void
CEventOrderClient::Connect(std::string description)
{
  char portNumber[32];
  sprintf(portNumber, "%u", m_port);
  m_pConnection = new CSocket();
  void* pConnectMessage(0);
  try {
    m_pConnection->Connect(m_host, std::string(portNumber));


    size_t length= message(&pConnectMessage, "CONNECT", strlen("CONNECT"), 
				    description.c_str(), description.size());

    m_pConnection->Write(pConnectMessage, length);


    std::string reply = getReplyString();
    if (reply != "OK") {
      errno = ECONNREFUSED;
      throw CErrnoException("ERROR reply from server");
    }
			     
  }
  catch (CTCPConnectionFailed& e) {
    free(pConnectMessage);


    // Convert to ECONNREFUSED errno
    
    errno = ECONNREFUSED;
    throw CErrnoException("Failed connection to server");
  }
  free(pConnectMessage);
  m_fConnected = true;

}
/**
 * Disconnect from the server.
 * If we are not connected this should throw a CErrnoException with
 * ENOTCONN as the ReasonCode.
 */
void
CEventOrderClient::disconnect()
{
  if (!m_fConnected) {
    errno = ENOTCONN;
    throw CErrnoException("Disconnect from server");
  }
  void* pDisconnectMessage(0);
  size_t msgLength = message(&pDisconnectMessage, "DISCONNECT", strlen("DISCONNECT"), NULL, 0);
  try {
    m_pConnection->Write(pDisconnectMessage, msgLength);
    free(pDisconnectMessage);

  }
  catch (...) {
    free(pDisconnectMessage);
    throw;
  }
  std::string  reply = getReplyString();
  if (reply != "OK") {
    errno = EOPNOTSUPP;
    throw CErrnoException("ERROR - Reply from server");
  }
  
}
/**
 * Submits a chain of fragments.  (FragmentChain).  The chain is marshalled into 
 * a body buffer, and submitted to the event builder.
 * via 'message'.
 *
 * @param pChain - Pointer to the first element of the chain.
 *
 *  @exception CErrnoException if we are not connected or some other error occurs.
 *
 */
void
CEventOrderClient::submitFragments(EVB::pFragmentChain pChain)
{
  if (m_fConnected) {

    // Size the body buffer, allocate it and fill it in from the chain.

    size_t nBytes = fragmentChainLength(pChain);
    char* pBodyBuffer = reinterpret_cast<char*>(
            malloc(nBytes + sizeof(uint32_t))); // Need space for the size.
    if (!pBodyBuffer) {
      throw CErrnoException("Allocating body buffer memory");
    }
    char* pDest       = pBodyBuffer;
    *pDest++          = nBytes;
    while (pChain) {
      memcpy(pDest, &(pChain->s_pFragment->s_header), sizeof(EVB::FragmentHeader));
      pDest += sizeof(EVB::FragmentHeader);
      uint32_t bodySize =  pChain->s_pFragment->s_header.s_size;
      memcpy(pDest, pChain->s_pFragment->s_pBody, bodySize);
      pDest += bodySize;
      pChain = pChain->s_pNext;
    }
    // Output the buffer in a try/catch block so that we can be sure it gets freed:

    try {
      void* msg;
      size_t msgLen = message(&msg, "FRAGMENTS", sizeof("FRAGMENTS"), pBodyBuffer, nBytes);
      m_pConnection->Write(msg, nBytes);
      free(pBodyBuffer);
    
    }
    catch (...) {
      free(pBodyBuffer);
      throw;
    }
    // get the reply and hope it's what we expect.

    std:: string reply = getReplyString();
    if (reply != "OK") {
      errno = ENOTSUP;
      throw CErrnoException("Reply from 'FRAGMENTS' message");

    }

  } else {
    errno = ENOTCONN;		// Not connected.
    throw CErrnoException ("submitting fragment chain");
  }
}

/**
 * Given a pointer to an array of fragments, and the number of fragments,
 * submits them to the event builder.  This is done by
 * - marshalling the pointers into a EVB::FragmentPointerList
 * - Invoking  void submitFragments(EVB::FragmentPointerList fragments);
 *
 * @param nFragments - Number of fragments in the array.
 * @param ppFragments - Pointer to the first fragment in the array.
 */
void
CEventOrderClient::submitFragments(size_t nFragments, EVB::pFragment ppFragments)
{
  EVB::FragmentPointerList fragments;

  for (int i = 0; i < nFragments; i++) {
    fragments.push_back(ppFragments);
    ppFragments++;		// Next fragment (scaled arith).
  }
  submitFragments(fragments);
  
}
/**
 * Given an STL list of pointers to events:
 * - Marshalls these into an event fragment chain
 * - submits those to the event builder.
 *
 * @param fragments - the list of fragments to send.
 *
 * @note Only the fragment nodes are created dynamically.
 *       this minimizes data movement.
 */
void
CEventOrderClient::submitFragments(EVB::FragmentPointerList& fragments)
{
  if (fragments.size() == 0) return; // degenerate edge case...empty list...don't send.
  
  EVB::pFragmentChain pChain = new EVB::FragmentChain;
  try {

    // Do the first one:
    
    EVB::FragmentPointerList::iterator src = fragments.begin();
    EVB::pFragmentChain pPrior = pChain;
    pPrior->s_pNext = 0;
    pPrior->s_pFragment = *src++;
    
    // Do the remainder of the chain.. if there are any:'
    
    while (src != fragments.end()) {
      // Allocate the new element and fill it in.
      EVB::pFragmentChain pNext = new EVB::FragmentChain;
      pNext->s_pNext = 0;
      pNext->s_pFragment = *src++;
      
      // Link this to the previous element.
      
      pPrior->s_pNext = pNext;
      pPrior = pNext;
    }
    submitFragments(pChain);
  } catch(...) {
    // Clean the chain and then rethrow.

    freeChain(pChain);
    throw;
  }
  freeChain(pChain);

}

/*-------------------------------------------------------------------------------------*/
// Private methods 

/**
 * Return a message consisting of a request header and a body.
 * The message is dynamically allocated and must be freed by the caller as
 * delete []message.
 *
 * @param msg     - Pointer to a pointer that will hold the message.
 * @param request - Pointer to the request part of the message.
 * @param requestSize - number of bytes in the request part of the message.
 * @param body    - Pointer to the bytes of data in the body.
 * @param bodySize - number of bytes in the body.
 *
 * @return size_t
 * @retval  size of the message.
 */
size_t
CEventOrderClient::message(void** msg,
			   const void*  request, size_t requestSize, const void* body , size_t bodySize)
{
  // figure out the size of the message:

  uint32_t rsize = requestSize;
  uint32_t bsize = bodySize;
  size_t totalSize = rsize + bsize + 2*sizeof(uint32_t);

  void* pMessage = malloc(totalSize);
  if (!pMessage) {
    throw CErrnoException("Allocating buffer");
  }
  // There must always be a request:
  char* p = reinterpret_cast<char*>(pMessage);
  memcpy(p, &rsize, sizeof(uint32_t));
  p += sizeof(uint32_t);
  memcpy(p, request, rsize);
  p += rsize;

  // If bsize == 0 or body == NULL, don't put them in the message (request only msg).

  if (body && bsize) {
    memcpy(p, &bsize, sizeof(uint32_t));
    p += sizeof(uint32_t);
    memcpy(p, body, bsize);
  }
  *msg = pMessage;
  return totalSize;


	
  
}
/**
 * Get a reply string from the server.
 * Reply strings are fully textual lines.  This just means
 * reading a character at a time until the newline.
 *
 * @return std::string.
 */
std::string
CEventOrderClient::getReplyString()
{
  std::string reply;
  while(1) {
    char c;
    m_pConnection->Read(&c, sizeof(c));
    if (c == '\n') return reply;
    reply += c;
  }
}
/**
 * Free a fragment chain (the fragments themselves are not freed by this).
 *
 * @param pChain - Pointer to the first chain element.
 */
void
CEventOrderClient::freeChain(EVB::pFragmentChain pChain)
{
  while (pChain) {
    EVB::pFragmentChain pNext = pChain->s_pNext;
    delete pChain;
    pChain = pNext;
  }
}
