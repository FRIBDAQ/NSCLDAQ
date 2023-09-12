/*
*-------------------------------------------------------------
 
 CAEN SpA 
 Via Vetraia, 11 - 55049 - Viareggio ITALY
 +390594388398 - www.caen.it

------------------------------------------------------------

**************************************************************************
* @note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
*
* @file     VX2750EventProcessor.h
* @brief    Event processor for the VX2750EventProcessor.
* @author   Ron Fox
*
*/
#ifndef VX2750EVENTPROCESSOR_H
#define VX2750EVENTPROCESSOR_H

#include <EventProcessor.h>

namespace caen_spectcl {
    
    class VX2750ModuleUnpacker;
    
    /**
     * @class VX2750EventProcessor
     *    This is an event processor that expects to get the body of a ring
     *    item containinga a VX2x50 hit.  This will normally be called from
     *    something like a CEventBuilderEventProcessor where it's registered
     *    to process the source id that corresponds to the module it understands.
     *    Each VX2750EventProcessor encapsulates the VX2750ModuleUnpacker that
     *    actually does the event processing but we'll do a bit of error checking
     *    to ensure the data from the module, and only the data from the module,
     *    are processed.
     */
    class VX2750EventProcessor : public CEventProcessor {
    private:
        VX2750ModuleUnpacker* m_pUnpacker;
    public:
        // Canonicals:
        
        VX2750EventProcessor(VX2750ModuleUnpacker* pUnpacker);
        virtual ~VX2750EventProcessor();
        
        // operations on the object:
        
        VX2750ModuleUnpacker* getUnpacker();
        void                  resetUnpacker();
        
        // Interface to the CEventProcessor that we implement:
        
        virtual Bool_t operator()(
            const Address_t pEvent, CEvent& rEvent, CAnalyzer& rAnalyzer,
            CBufferDecoder& rDecoder
        );
        
    };
    
}                                   // caen_spectcl namespace


#endif