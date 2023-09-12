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
* @file     VX2750MultiModuleEventSegment.cpp
* @brief    Implement the event segment.
* @author   Ron Fox
*
*/
#include "VX2750MultiModuleEventSegment.h"

#include "VX2750MultiTrigger.h"
#include "VX2750EventSegment.h"
#include <CExperiment.h>


namespace caen_nscldaq {
    /**
     * constructor
     * Our job is simple, just initialize the internal data from the
     * parameters.
     *    @param pExperment - pointer to the experiment object
     *    @param pTrigger   - pointer to the trigger object.
     */
    VX2750MultiModuleEventSegment::VX2750MultiModuleEventSegment(
        CExperiment* pExperiment, VX2750MultiTrigger* pTrigger
    ) : m_pExperiment(pExperiment), m_pTrigger(pTrigger),
        m_configChanged(false)
    {}
    
    /**
     * destructor
     *   Ownership of our data remains with our clients:
     */
    VX2750MultiModuleEventSegment::~VX2750MultiModuleEventSegment()
    {}
    /**
     * setConfigChanged
     *    Sets the config changed flag true indicating the modules must be
     *    hardware initialized.
     */
    void
    VX2750MultiModuleEventSegment::setConfigChanged() {
        m_configChanged = true;
    }
    
    /**
     * initialize
     *    Just get the module pointers from the trigger, interate over
     *    them and initialize each one.
     *      This implies, for a synchronized set, the order is important with
     *      the master module needing to be last in the list.
     */
    void
    VX2750MultiModuleEventSegment::initialize()
    {
        auto modules = m_pTrigger->getModules();
        for (auto p : modules) {
            if (m_configChanged) {
                p->hwInit();
            }
            p->initialize();
        }
        m_configChanged = false;
    }
    /**
     * disable
     *    As above but call disable for each item:
     */
    void VX2750MultiModuleEventSegment::disable()
    {
        auto modules = m_pTrigger->getModules();
        for (auto p : modules) {
            p->disable();
        }
    }
    /**
     * onPause
     *    This is just disable:
     */
    void VX2750MultiModuleEventSegment::onPause()
    {
        disable();
    }
    /**
     * onResume:
     *    Similarly, this is is just enable.
     */
    void  VX2750MultiModuleEventSegment::onResume()
    {
        initialize();
    }
    /**
     * read:
     *    While the set of triggered modules is not empty, get the last triggered
     *    module, read it, remove it from the trigger set and
     *    if the result is a nonempty trigger list, retrigger.
     * @param pBuffer - pointer to where the data will be stired,
     * @param maxwords - Maximum number of words that can be stored.
     * @return size_t - number of words read into pBuffr (word == uint16_t).
     */
    size_t
    VX2750MultiModuleEventSegment::read(void* pBuffer, size_t maxwords)
    {
        size_t nRead = 0;
        std::vector<VX2750EventSegment*>& triggered = m_pTrigger->getTriggeredModules();
        if (!triggered.empty()) {
            // Defensive.
            
            VX2750EventSegment* pSeg = triggered.back();
            nRead = pSeg->read(pBuffer, maxwords);
            triggered.pop_back();
            
            if (!triggered.empty()) {
                m_pExperiment->haveMore();
            }
            
        }
        return nRead;
    }
    
}