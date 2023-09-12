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
* @file     VX2750EventBuiltEventProcessor.cpp
* @brief    Implement the event processor that handles event built data:
* @author   Ron Fox
*
*/
#include "VX2750EventBuiltEventProcessor.h"
#include "VX2750ModuleUnpacker.h"
#include "VX2750EventProcessor.h"


namespace caen_spectcl {
    /**
     * constructor
     *    For the most part we defer to the base class.  The base class
     *    will create a set of diagnostic parameters that can be used to
     *    ensure clocks remain synchronized.  We need to know the
     *    basename for those parameters and the clock frequency so that those
     *    items can be calibrated.  Note that since in our case a module is
     *    a data source and it could have multiple hits spread out across the
     *    event bulding window the expectation for the time difference spectra
     *    is that we'd see bands that are distributed across the event build window
     *    from 0.
     *
     *    Note that in our case we know that the timestamp used is in ns.
     *    Therefore clockMhz is 1000.
     *
     *    @param basename  - base name for the diagnostic parameters created by
     *                       our base class.  See CEventBuilderEventProcessor's
     *                       reference documentation at
     *                       https://docs.nscl.msu.edu/daq/newsite/spectcl-5.0/pgmref
     *                       for a descsription of these.
     *
     *    Several bits of local data are created which are needed both in event
     *    unpacking and in object destruction, because of the number of ways
     *    in which we've allowed client software to add event processors:
     *    - m_eventProcessors - is a vector of all event processors.
     *                          this is used by us to reset module unpackers
     *                          before passing event processing off to the
     *                          base class.
     *    - m_createdEventProcessors - is a vector of event processors we created
     *                          as a result of calls to addEventProcessor.
     *                          The event processors in this vector will be
     *                          deleted in the destructor.
     *    - m_createdModuleUnpackersis a vector of event processors we created
     *                          as a result of calls to addEventProcessor.
     *                          these will be deleted in the destructor.
     */
    VX2750EventBuiltEventProcessor::VX2750EventBuiltEventProcessor(
        std::string baseName
    )   : CEventBuilderEventProcessor(1000.0, baseName)
    {}
    /**
     * destructor
     *    delete the processors and unpackers we created:
     */
    VX2750EventBuiltEventProcessor::~VX2750EventBuiltEventProcessor()
    {
        for (auto p : m_createdEventProcessors) {
            delete p;
        }
        for (auto p : m_createdModuleUnpackers) {
            delete p;
        }
    }
    
    /**
     * addEventProcessor
     *    This overload just binds an existing event processor, produced and
     *    owned by the calling code to a source id.  The event processor is added
     *    in the base calss and to m_eventProcessors.  Ownership and lifetime
     *    are the responsibility of the caller.
     *  @param sourceId   - the source id that will be processed by this procssor.
     *  @param processor  - the event processor to call for that source id.
     *  @note it is the client software's responsibility not to double assign
     *        source ids. The base class will happily replace the processor we,
     *        will continue to assume the processor is alive and reset its module
     *        unpacker each event.
     */
    void
    VX2750EventBuiltEventProcessor::addEventProcessor(
        unsigned sourceId, VX2750EventProcessor& processor
    )
    {
        CEventBuilderEventProcessor::addEventProcessor(sourceId, processor);
        m_eventProcessors.push_back(&processor);
    }
    /**
     * addEventProcessor
     *    This overload adds an event processor given that the client has
     *    already produced and intends to own/manage lifetime of a module unpacker
     *    for this source id.  we:
     *    -  Create a new event processor encapsulating this unpacker.
     *    -  Add the event processor to our m_createdEventProcessors collection.
     *    -  Add the event processor to the base class.
     *    -  Add the event processor to the m_eventProcessors collection.
     */
    void
    VX2750EventBuiltEventProcessor::addEventProcessor(
        unsigned sourceId, VX2750ModuleUnpacker& unpacker
    )
    {
        VX2750EventProcessor* pProcessor = new VX2750EventProcessor(&unpacker);
        m_createdEventProcessors.push_back(pProcessor);
        addEventProcessor(sourceId, *pProcessor);            // Takes care of the rest of this.
    }
    /**
     * addEventProcessor
     *    This overload adds an event procssor given sufficient information to
     *    create the module unpacker for its module.
     * @param sid - the source id to bind the resulting event processor into.
     * @param moduleName - the name of the module to be processed.
     * @param paramBaseName - base name for the parameters creatd by this processor.
     */
    void
    VX2750EventBuiltEventProcessor::addEventProcessor(
        unsigned sourceId,
            const std::string& moduleName, const std::string paramBasename
    )
    {
        VX2750ModuleUnpacker* pUnpacker = new VX2750ModuleUnpacker(
            moduleName.c_str(), paramBasename.c_str()
        );
        m_createdModuleUnpackers.push_back(pUnpacker);
        addEventProcessor(sourceId, *pUnpacker);
    }
    
    /**
     * operator()
     *    This is called to process each event.
     *    What we need to do is iterate over the event processors we _have_
     *    and reset their module unpackers.  Once that's done, we can turn over
     *    control to the base class's operator().
     * @note we don't need to know the meaning of the parameters, however they
     *    are standard for CEventProcessor derived classes and, if you are
     *    curious look at the reference page for CEventProcessor at:
     *    https://docs.nscl.msu.edu/daq/newsite/spectcl-5.0/pgmref e.g.
     */
    Bool_t
    VX2750EventBuiltEventProcessor::operator()(
        const Address_t pEvent,
        CEvent& rEvent,
        CAnalyzer& rAnalyzer,
        CBufferDecoder& rDecoder
    )
    {
        for (auto p : m_eventProcessors) {
            p->resetUnpacker();
        }
        return CEventBuilderEventProcessor::operator()(
            pEvent, rEvent, rAnalyzer, rDecoder
        );
        
    }
    
}                              // caen_spectcl namespace.
