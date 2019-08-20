/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CZMQBuiltRingITemEditorApp.h
 *  @brief: ZMQ Threaded application class for the event editor.
 */

#ifndef CZMQBUILTRINGITEMEDITORAPP_H
#define CZMQBUILTRINGITEMEDITORAPP_H
#include "CBuiltRingItemEditorApp.h"

#include <vector>

class CRingItemZMQSourceElement;
class CThreadedProcessingElement;

class CTransport;
class CSender;
class CReceiver;
class CRingItemSorter;

class CDataSinkElement;


/**
 *
 *  To run we need a .zmqservices file with the following
 *  services:
 *     - 1 distro -- Fanout that distributes events to workers.
 *     - 2 sort   -- Accepts data from the workers and sorts by timestamp.
 *     - 3 sorted -- Accepts data from the sorter and outputs it.
 */

class CZMQBuiltRingItemEditorApp :  public CBuiltRingItemEditorApp
{
private:

    // Data source objects.

    CRingItemZMQSourceElement* m_pSourceElement;
    CThreadedProcessingElement* m_pSourceThread;

    // Stuff needed to support the sorter.

    CTransport*                 m_pSortServer;
    CReceiver*                  m_pSortReceiver;
    CTransport*                 m_pSortSource;
    CSender*                    m_pSortSender;
    CRingItemSorter*            m_pSortElement;
    CThreadedProcessingElement* m_pSortThread;

    // Stuff for the ultimate data sink:

    CTransport*                 m_pSortClient;
    CReceiver*                  m_pSortData;
    CTransport*                 m_pRingSink;
    CSender*                    m_pRingSender;
    CDataSinkElement*           m_pSinkElement;
    CThreadedProcessingElement* m_pSinkThread;

    std::vector<CThreadedProcessingElement*> m_workers;    
public:
    CZMQBuiltRingItemEditorApp(gengetopt_args_info args);
    virtual ~CZMQBuiltRingItemEditorApp();
    
    virtual void operator()();
private:
    void setupApplication();
    void setupWorkers();
};

#endif