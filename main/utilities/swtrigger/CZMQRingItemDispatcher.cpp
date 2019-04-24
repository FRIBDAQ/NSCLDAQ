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

/** @file:  CZMQRingItemDispatcher.cpp
 *  @brief: Implement CRingItemDispatcher
 */
#include "CZMQRingItemDispatcher.h"
#include "CZMQRouterTransport.h"
#include "CSender.h"

/**
 * constructor
 *    @param ringUri - ring buffer specification
 *    @param routerUri - URI on which the router accepts connections.
 */
CZMQRingItemDispatcher::CZMQRingItemDispatcher(
        const char* ringUri, const char* routerUri
) :
    CRingItemDispatcher(
        ringUri,
        new CSender(*(new CZMQRouterTransport(routerUri)))
    )
{}