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
* @file     VX2750EventBuiltEventProcessor.h
* @brief    Event built event processor for VX27x0 modules.
* @author   Ron Fox
*
*/
#ifndef VX2750EVENTBUILTEVENTPROCESSOR_H
#define VX2750EVENTBUILTEVENTPROCESSOR_H3

#include "CEventBuilderEventProcessor.h"
#include <vector>
#include <string>

namespace caen_spectcl {
    class VX2750ModuleUnpacker;
    class VX2750EventProcessor;
    /**
     * @class VX2750EventBuildtEventProcessor
     *     We need to derive a class from CEventBuilderEventProcessor
     *     to be sure the individual module unpackers get reset at the
     *     beginning of event processing.  In addition, we'll provide
     *     a convenience method to put together the module unpacker and
     *     the event processor binding them to a source id.
     *
     *  @note We don't make the base class public because  we want to forbid
     *        our direct clients from calling addEventProcessor in the base class.
     */
    class VX2750EventBuiltEventProcessor : public CEventBuilderEventProcessor {
    private:
        std::vector<VX2750EventProcessor*> m_eventProcessors;
        std::vector<VX2750EventProcessor*> m_createdEventProcessors;
        std::vector<VX2750ModuleUnpacker*> m_createdModuleUnpackers;
        
    public:
        VX2750EventBuiltEventProcessor(std::string baseName);
        virtual ~VX2750EventBuiltEventProcessor();
        
        // Maintaining the event processor list.  Note the base class has no
        // method for removing event processors and that's ok because
        // the set of sources can only be changed by re-doing readout e.g.
        
        void addEventProcessor(unsigned sourceId, VX2750EventProcessor& processor);
        void addEventProcessor(unsigned sourceId, VX2750ModuleUnpacker& unpacker);
        void addEventProcessor(
            unsigned sourceId,
            const std::string& moduleName, const std::string paramBasename
        );
        
        //  Override the base class operator() so we can reset the
        // module unpackers:
        
        virtual Bool_t operator()(const Address_t pEvent,
                              CEvent& rEvent,
                              CAnalyzer& rAnalyzer,
                              CBufferDecoder& rDecoder);
        
    };
}                      // caen_spectcl namespace.

#endif