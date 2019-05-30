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

/**
* @file   CZMQThreadedClassifierApp.cpp
* @brief Implements the CZMQThreadedClassifierAPp class.
*/
#include "CZMQThreadedClassifierApp.h"
#include "CRingItemZMQSourceElement.h"
#include "CThreadedProcessingElement.h"
#include "CZMQCommunicatorFactory.h"
#include "CTransport.h"
#include "CRingItemTransport.h"
#include "CSender.h"
#include "CReceiver.h"
#include "CRingItemSorter.h"
#include "CDataSinkElement.h"
#include "CRingItemTransportFactory.h"
#include "CZMQDealerTransport.h"


#include <dlfcn.h>
#include <stdlib.h>
#include <stdexcept>
#include <errno.h>

static const int DISTRIBUTION_SERVICE (1);
static const int SORT_SERVICE(2);
static const int SORTEDDATA_SERVICE(3);

/**
 * constructor
 *   @param args -the parsed arguments.
 */
CZMQThreadedClassifierApp::CZMQThreadedClassifierApp(gengetopt_args_info& args) :
    CClassifierApp(args),
    m_pSourceElement(nullptr), m_pSourceThread(nullptr),
    
    m_pSortServer(nullptr), m_pSortReceiver(nullptr), m_pSortSource(nullptr),
    m_pSortSender(nullptr), m_pSortElement(nullptr), m_pSortThread(nullptr),
    
    m_pSortClient(nullptr), m_pSortData(nullptr),
    m_pRingSink(nullptr), m_pRingSender(nullptr),
    m_pSinkElement(nullptr), m_pSinkThread(nullptr)
    
{}
    
/**
 * destructor
 */
CZMQThreadedClassifierApp::~CZMQThreadedClassifierApp()
{
    delete m_pSourceThread;
    delete m_pSourceElement;
    
    
    delete m_pSortThread;
    delete m_pSortElement;          // Deletes the CSender & CReceiver.
    delete m_pSortSource;
    delete m_pSortServer;
    
    delete m_pSinkThread;
    delete m_pSinkElement;
    delete m_pSortClient;
    delete m_pRingSink;
    
    for(int i =0; i < m_workers.size(); i++) {
        delete m_workers[i];
    }
    
    
}

/**
 * operator()
 *    Runs the application.
 * @return - the status to return on application exit.
 */
int
CZMQThreadedClassifierApp::operator()()
{
    // Create the data source object and encapsulate it in a thread:
    // Note that since the router is a req/rep style deal it's not
    // going to start sending data until there's at least one worker.
    
    CZMQCommunicatorFactory commFactory;          // URL translation.
    std::string routerUri = commFactory.getUri(DISTRIBUTION_SERVICE);
    m_pSourceElement =
        new CRingItemZMQSourceElement(m_params.source_arg, routerUri.c_str());
    m_pSourceThread = new CThreadedProcessingElement(m_pSourceElement);
    m_pSourceThread->start();                    // Can start the thread.
    
    // The next server we need to establish is the sorter.
    // The sorter is a pull server for fanin and a push server for
    // one to one.
    
    m_pSortServer = commFactory.createFanInSink(SORT_SERVICE);
    m_pSortReceiver = new CReceiver(*m_pSortServer);
    m_pSortSource   = commFactory.createOneToOneSource(SORTEDDATA_SERVICE);
    m_pSortSender   = new CSender(*m_pSortSource);
    m_pSortElement  = new CRingItemSorter(
        *m_pSortReceiver, *m_pSortSender, m_params.sort_window_arg,
        m_params.workers_arg
    );
    m_pSortThread = new CThreadedProcessingElement(m_pSortElement);
    m_pSortThread->start();
    
    //  Create the ultimate data sink.  Gets data from the
    // SORTEDDATA_SERVICE and disposes it as determined by the --sink
    // command parameter:
    
    
    m_pSortClient   = commFactory.createOneToOneSink(SORTEDDATA_SERVICE);
    m_pSortData     = new CReceiver(*m_pSortClient);
    m_pRingSink     =
        CRingItemTransportFactory::createTransport(
            m_params.sink_arg, CRingBuffer::producer
        );
    m_pRingSender = new CSender(*m_pRingSink);
    m_pSinkElement = new CDataSinkElement(*m_pSortData, *m_pRingSender);
    m_pSinkThread  = new CThreadedProcessingElement(m_pSinkElement);
    m_pSinkThread->start();
    startWorkers();
    
    sleep(1);                    // Let the threads get established.
    
    // Wait for them all to end:
    
    m_pSourceThread->join();
    m_pSortThread->join();
    m_pSinkThread->join();
    for(int i =0; i < m_workers.size(); i++) {
        m_workers[i]->join();
    }
    return EXIT_SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////
// PRivate methods.

/**
 * startWorkers
 *    - Get the classifier factory from the sll.
 *    - Create a worker threads that encapsulate a CRingMarkingWorker
 *      that uses the user classifier class.
 *    - Thread objects pointers are stored in m_workers and started.
 */
void
CZMQThreadedClassifierApp::startWorkers()
{
    CZMQCommunicatorFactory commFactory;
    ClassifierFactory fact = getClassifierFactory();
    for (int i =0; i < m_params.workers_arg; i++) {
        CRingMarkingWorker::Classifier* pClassifier = (*fact)();
        std::string dealerUri = commFactory.getUri(DISTRIBUTION_SERVICE);
        CFanoutClientTransport *pFanoutClient =
            new CZMQDealerTransport(dealerUri.c_str(), i);
        CTransport* pFaninXport =
            commFactory.createFanInSource(SORT_SERVICE);
        CSender*    pFaninSender = new CSender(*pFaninXport);
        CRingMarkingWorker* pWorker =
            new CRingMarkingWorker(*pFanoutClient, *pFaninSender, i, pClassifier);
        CThreadedProcessingElement* pThread =
            new CThreadedProcessingElement(pWorker);
        pThread->start();
        m_workers.push_back(pThread);
    }
}
/**
 * getClassifierFactory
 *    Returns a pointer to the classfier factory in the users's --classifier
 *    library.
 *
 *    This is an extern "C" function called createClassifier
 *    it must have the signatrure:
 *
 * \verbatim
 *       CRingMarkingWorker::Classifier* createClassifier();
 * \endverbatim
 *
 *   @return pointer to the factory function.
 *   @throw std::runtime_error - if we can't get that function for whatever
 *                               reason.
 */
CZMQThreadedClassifierApp::ClassifierFactory
CZMQThreadedClassifierApp::getClassifierFactory()
{
    void* soHandle = dlopen(m_params.classifier_arg, RTLD_NOW |RTLD_GLOBAL);
    if (!soHandle) {
        std::string error = dlerror();
        std::string msg   = "Failed to open shared library: ";
        msg += m_params.classifier_arg;
        msg += " ";
        msg += error;
        throw std::runtime_error(msg);
    }
    dlerror();                         // Clear errors (from manpage example).
    void* rawFactory = dlsym(soHandle, "createClassifier");
    char* error = dlerror();
    if (error != nullptr) { 
        std::string msg = "Unable to locate 'createClassifier' in  ";
        msg += m_params.classifier_arg;
        msg += " ";
        msg += error;
        msg += " be sure it's delcared extern \"C\"";
        throw std::runtime_error(msg);
    }
    
    ClassifierFactory result = reinterpret_cast<ClassifierFactory>(rawFactory);
    return result;
}