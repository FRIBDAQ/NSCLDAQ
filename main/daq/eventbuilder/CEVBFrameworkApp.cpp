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
#include "EVBFramework.h"
#include "CEVBFrameworkApp.h"
#include "CEVBClientApp.h"
#include "CEventOrderClient.h"
#include "GetOpt.h"
#include "fragment.h"



#include <Exception.h>
#include <ErrnoException.h>

#include <string>
#include <iostream>
#include <functional>
#include <algorithm>

#include <stdlib.h>
#include <netdb.h>

#include <thread>
#include <chrono>

/**
 * CEVBFrameworkApp.cpp - implementation of the CEVBFrameworkApp class.
 * this class is the application level class for the event builder client framework.
 */


/*---------------------------------------------------------------------------
** Static member data:
*/

CEVBFrameworkApp* CEVBFrameworkApp::m_pInstance(0); // Singleton instance.


/**
 *  construction:
 *  - nulls out m_pArgs
 *  - nulls out m_pBuilder:
 *
 *  This cannot be explicitly called by an external client as this class is a
 *  singleton.
 */
CEVBFrameworkApp::CEVBFrameworkApp() :
  m_pArgs(0),
  m_pBuilder(0)
{
}
/**
 * getInstance - If necessary constructs the singleton instance and saves a
 *               pointer to it in m_pInstance.  Regardless returns a pointer
 *               to the single allowed instance.  This method, in conjunction
 *               with the private constructor realizes the singleton pattern.
 * @return CEVBFrameworkApp*
 * @retval A pointer to the singleton instance which, by then, is fully constructed.
 */
CEVBFrameworkApp*
CEVBFrameworkApp::getInstance()
{
  if (!m_pInstance) {
    m_pInstance = new CEVBFrameworkApp;
  }
  return m_pInstance;
}

/**
 * addSource - Adds a new event source to the set of data sources.
 *             Normally an event source constructor invokes this to 
 *             automatically register its needt to be called with us.
 *             The pointer must point to an object with a lifetime
 *             that is at least the life of the client of the main
 *             method.
 *
 * @param pSource - Pointer to the event source to register.
 */
void
CEVBFrameworkApp::addSource(CEVBClientApp* pSource)
{
  m_sources.push_back(pSource);
}
/**
 * Remove an event source from the list of sources being processed.
 * This is not a normal thing to do but must be supported just in case.
 *
 * @param pSource - Pointerr to the source.
 */
void
CEVBFrameworkApp::removeSource(CEVBClientApp* pSource)
{
  std::list<CEVBClientApp*>::iterator p = std::find(m_sources.begin(), m_sources.end(), pSource);
  if (p != m_sources.end()) {
    m_sources.erase(p);
  }
}

/**
 * Send data to the server.  This has typically been a trampoline target via the
 * interface described in EVBFramework.h
 *
 * @param fragmentList - list of fragments to send.
 * 
 */
void
CEVBFrameworkApp::send(CEVBFragmentList& fragmentList)
{
  /* Marshall the data into a fragment pointer list and ship it off.
     this means:
     - building an array of fragments.
     - marshalling them into a list.
     - calling submitFragments on the connection to send the list.
     - freeing storage.
  */

  // Build the array of fragments: 

  size_t nFragments      = fragmentList.size();
  EVB::pFragment   pFragments = reinterpret_cast<EVB::pFragment>(malloc(nFragments * sizeof(EVB::Fragment)));
  if (!pFragments) {
    throw std::string("Failed to allocated fragment list buffer (send)");
  }

  // Marshall them into a list:



  EVB::pFragment pDest = pFragments;
  EVB::FragmentPointerList evbList;

  for (CEVBFragmentList::iterator psrc = fragmentList.begin(); 
       psrc != fragmentList.end(); psrc++) {
    pDest->s_header.s_timestamp = psrc->s_timestamp;
    pDest->s_header.s_sourceId  = psrc->s_sourceId;
    pDest->s_header.s_size      = psrc->s_size;
    pDest->s_header.s_barrier   = psrc->s_barrierType;
    pDest->s_pBody                = psrc->s_payload;

    evbList.push_back(pDest);
    pDest++;
  }

  // Send them off to the builder:

  m_pBuilder->submitFragments(evbList);

  // Free the storage.

  free(pFragments);

}
/**
 * Return the parsed arguments struct.  This allows people to extend
 * the args and fetch their stuff.
 *
 * @param const gengetopt_args_info* - pointer to the args parsed by gengetopt.
 */
const gengetopt_args_info*
CEVBFrameworkApp::getParsedArgs() const
{
  return m_pArgs;
}

/**
 * main - simply ensure there is an object and invoke its operator()
 *        which does all the real work.
 *
 * @param argc - Count of command line words.
 * @param argv - array of pointers to the command line words.
 *
 * @return int
 * @return EXIT_SUCCESS or EXIT_FAILURE depending on the status of
 *         object execution.
 */
int
CEVBFrameworkApp::main(int argc, char** argv)
{
  CEVBFrameworkApp* pApp = getInstance();
  return (*pApp)(argc, argv);
}
/*---------------------------------------------------------------------------
** Private methods.
*/

/**
 * operator() - This is really the main entry point of the application after
 *              all the bouncing around in static methods.
 *             - Process the command parameters.
 *             - Make an event builder connection (if possible).
 *             - initialize all of the fragment sources.
 *             - run the main loop.
 *             - shutdown the fragment sources.
 *             - Sutdown the event bulder connection.
 *             - return normal.
 */
int
CEVBFrameworkApp::operator()(int argc, char** argv)
{
  /* Process arguments:
   */

  GetOpt options(argc, argv);
  std::string evbhost = options.getHost();
  std::string evbport = options.getPort();
  std::string description = options.getDescription();
  std::list<int> sourceIds = options.getSourceIds();

  const char* pName = options.evbName();


  /**
   * Connect to the event builder:
   */

  const char* pContext;

  try {

    pContext = "Connecting to the event builder";
    EVBConnect(evbhost.c_str(), evbport.c_str(), description.c_str(), sourceIds, pName);

    /**
     * Initialize all of the fragment sources in m_sources
     */
    
    pContext = "Initializing data sources";
    std::for_each(m_sources.begin(), m_sources.end(), 
		  std::mem_fun(&CEVBClientApp::initialize) );
    
    
    
    // main loop let the sources contribute data.
    

      SourcePoll poller(100);
      pContext = "Main loop";
      try {
	while (1) {
	  for_each(m_sources.begin(), m_sources.end(),
		   poller);
	  // std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
      }
      catch (...) {
	// If the main loop fails, shut down the sources and
	// re-throw the error.
	const char* pOldContext = pContext;

	pContext = "Shutting down sources";
	
	std::for_each(m_sources.begin(), m_sources.end(),
		      std::mem_fun(&CEVBClientApp::shutdown) );

	pContext = pOldContext;
	throw;
      }
      

      
  }
  catch(std::string msg) {
    ReportError(pContext, "std::string", msg.c_str());
    return EXIT_FAILURE;
  }
  catch(const char* pMesg) {
    ReportError(pContext, "const char*", pMesg);
    return EXIT_FAILURE;
  }
  catch(CException& e) {
    ReportError(pContext, "CException", e.ReasonText());
    return EXIT_FAILURE;
  }
  catch(std::exception& e) {
    ReportError(pContext, "std::exception", e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


/**
 * Connect to the event builder.
 *
 * @param[in] host - Host on which the event buidler is running.
 * @param[in]  port - If this is 'managed', we use the port manager to locate the server.
 *               otherwise this must be either an integer (in which case it is the port number)
 *               or a service name that can be translated by getservbyname_r
 * @param[in] description - Describes the connection.
 * @param[in] sources - List of source ids that are being provided by this client.
 * @param[in] pName - If not null the name of the event builder to connect to. 
 *
 * @note If successful, a pointer to the resulting connection is stored in m_pBuilder.
 */
void
CEVBFrameworkApp::EVBConnect(const char* host, const char* port, const char* description,
			     std::list<int> sources, const char* pName)
{
  uint16_t portNum;
  if (std::string("managed") == port) {
    portNum = CEventOrderClient::Lookup(std::string(host), pName);
  } 
  else if (atoi(port) != 0) {	// Port number 0 is illegal.
    portNum = atoi(port);
  } else {
    portNum = LookupService(port);
  }

  m_pBuilder = new CEventOrderClient(std::string(host), portNum);
  m_pBuilder->Connect(std::string(description), sources);
}

/**
 * ReportError - reports an error message to stderr:
 *
 * @param pContext - part of the message that provides context.
 * @param pType    - type of exception.
 * @param pMessage - The part of the message that provides error information.
 *
 */
void 
CEVBFrameworkApp::ReportError(const char* pContext, const char* pType,  const char* pMessage) 
{
  std::string exceptionType = " (";
  exceptionType += pType;
  exceptionType += ") ";
  std::cerr << pContext << exceptionType << pMessage << std::endl;
}

/**
 * Determinet an internet service port given a service name.
 * This uses getservbyname_r so as to remain threadsafe.
 * 
 * @param serviceName - Name of the service to find.
 * 
 * @return int - Port number.
 */
int
CEVBFrameworkApp::LookupService(const char* serviceName)
{
  struct servent serviceInfo;	// Result buffer.
  struct servent *pEntry(0);	// Hopefully will be &serviceInfo when done
  char           buffer[1024];	// Workspace for getservb yname_r


  int status = getservbyname_r(serviceName, "tcp",
			       &serviceInfo, buffer, sizeof(buffer),
			       &pEntry);
  if (status) {
    throw CErrnoException("Looking up service in e.g. /etc/services");
  }
  if (!pEntry) {
    throw std::string("No such service");
  }
  return serviceInfo.s_port;
}

/*-------------------------------------------------------------------
*  Inner class: SourcePoll
*
* SourcePoll is used in the main loop for_each loop to 
* check for data in each data source and, if present, invoke the
* the getEvents method.
*/


/**
 * Construction saves the number of ms to pass to the
 * objects dataReady member.
 */

CEVBFrameworkApp::SourcePoll::SourcePoll(int ms) :
  m_ms(ms)
{}
/**
 * operator() - invoke the dataReady method of the parameter
 *              if it returns true, invoke getEvents.
 *
 * @param pSource -Pointer to the source object.
 */
void
CEVBFrameworkApp::SourcePoll::operator()(CEVBClientApp* pSource)
{
  if (pSource->dataReady(m_ms)) {
    pSource->getEvents();
  }
}

