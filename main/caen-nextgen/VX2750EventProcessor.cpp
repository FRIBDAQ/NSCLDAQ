/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  VX2750EventProcessor.cpp
 *  @brief: Impelement the module enent processor.
 *  
 */

#include "VX2750EventProcessor.h"
#include "VX2750ModuleUnpacker.h"
#include <stdexcept>
#include <cstdint>
#include <iostream>

namespace caen_spectcl {
/**
 * constructor
 *    @param pUnpacker - pointer to the unpacker associated with this
 *                   module.  Note that ownership is not taken. The caller
 *                   must dispose of this object when done (likely the
 *                   object actually has program lifetime).
 */
VX2750EventProcessor::VX2750EventProcessor(VX2750ModuleUnpacker* pUnpacker) :
    m_pUnpacker(pUnpacker)
{}
/**
 * Destructor
 *    Since the unpacker is not owned by us this is a no-op.
 *    We can't own the unpacker because we don't know how it was
 *    created -- for all we know it's static.
 */
VX2750EventProcessor::~VX2750EventProcessor()
{}

/**
 * getUnpacker
 *    Provides a pointer to the unpacker.   This is usually used to provide
 *    access to the methods of the unpacker after an event has been decoded
 *    (e.g. in a subsequent stage of the event processing pipeline).
 * @return VX2750ModuleUnpacker*
 */
VX2750ModuleUnpacker*
VX2750EventProcessor::getUnpacker()
{
    return m_pUnpacker;
}
/**
 * resetUnpacker
 *    Prior to processing an event, the unpacker must be reset to clear the
 *    state retained from the previous event (each event consists of a set of hits
 *    and the hits for this module are all accumulated in the unpacker
 *    during event processing).
 *    There are a couple of strategies for doing this:
 *    1.  (preferred) Derive from CEventBuilderEventProcessor for the
 *        event processor that dispatches fragements to the specific module
 *        and reset the event processors prior to delegating the actual unpack
 *        to the base class.
 *    2. Provide an event processor whose sole function is to do this which executes
 *       before e.g. the standard CEventBuilderEventProcessor that dispatches th
 *       fragments to a module.
 *    The first option is less error prone, and encapsulates the reset with the
 *    code that dispatches event data to modules.
 */
void
VX2750EventProcessor::resetUnpacker()
{
    m_pUnpacker->reset();
}
/**
 *  operator()
 *     Called with a pointer to the ring item data body (that is past the
 *     body header if there is one) for our module.
 *     - Get the number of words that are supposed to be in the fragment body.
 *     - Invoke the unpacker to unpack the body.
 *     - If there's a match in the number of words it processed and the
 *       size of the event, return kfTRUE else output an error message and
 *       return kfFALSE, aborting event processing for this evenmt.
 *
 *   @param pEvent - pointer to the event.
 *   @note the remainder of the parameters are unused and therefore not documented here.
 *         See the SpecTcl documentation for a descripton of them.
 *   @return Bool_t - kfTRUE if event processing was successful else kfFALSE if not.
 */
Bool_t
VX2750EventProcessor::operator()(
    const Address_t pEvent, CEvent& rEvent, CAnalyzer& rAnalyzer,
    CBufferDecoder& rDecoder
)
{
    // Recasting pEvent as pointing to  uint32_t allows us to painlessly extract
    // the fragment body size
    
    const std::uint32_t* p = reinterpret_cast<std::uint32_t*>(pEvent);
    const std::uint8_t*   pEnd;
    std::uint32_t nWords = *p;
    
    try {
      pEnd = reinterpret_cast<const std::uint8_t*>(m_pUnpacker->unpackHit(p));
    }
    catch (std::exception& e) {
        std::cerr << "Exception caught in VX2750EventProcessor: " << e.what() << std::endl;
        return kfFALSE;
    }
    // Ensure the event processor procssed the right amount of data:
    
    const std::uint8_t* pStart = reinterpret_cast<const std::uint8_t*>(p);
    ptrdiff_t nBytes = pEnd-pStart;
    if(nBytes != (nWords*sizeof(uint16_t))) {
        std::cerr << "VX2750EventProcessor: Fragment had : " << nWords * sizeof(uint16_t)
            << " but the unpacker processed: " <<nBytes << " of them\n";
        return kfFALSE;
    }
    return kfTRUE;
}

}

  
