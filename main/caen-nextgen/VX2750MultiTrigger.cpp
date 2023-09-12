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
* @file     VX2750MultiTrigger.cpp
* @brief    Implement the VX2750MultiTrigger class.
* @author   Ron Fox
*
*/
#include "VX2750MultiTrigger.h"
#include "CAENVX2750PhaTrigger.h"
#include "VX2750EventSegment.h"
#include "VX2750Pha.h"
#include <algorithm>


namespace caen_nscldaq {
    /**
     * constructor
     *     Constructs the object...nothing to  it -- literally.
     */
    VX2750MultiTrigger::VX2750MultiTrigger() {}
    
    /** Ditto for the destructors since we don't own the actual modules or their
     *  trigger objects.
     */
    VX2750MultiTrigger::~VX2750MultiTrigger() {}
    
    /**
     *  Add a new trigger to the list of  modules polled.
     *  @param pTrigger - trigger object to add... the caller continues to own
     *                    the module.
     */
    void
    VX2750MultiTrigger::addTrigger(CAENVX2750PhaTrigger* pTrigger)
    {
        m_triggers.push_back(pTrigger);    
    }
    /**
     * removeTrigger
     *    If the trigger passed in is in the trigger list, it is removed
     *    otherwise this silently does nothing.
     *
     *   @param pTrigger - pointer to the trigger to remove.
     */
    void
    VX2750MultiTrigger::removeTrigger(CAENVX2750PhaTrigger* pTrigger) {
        auto p = std::find(m_triggers.begin(), m_triggers.end(), pTrigger);
        if (p != m_triggers.end()) {
            m_triggers.erase(p);
        }
    }
    /**
     * getTriggers
     *    Returns a *copy* of the trigger list.
     * @return std::vector<VX2750PhaTrigger*>
     */
    std::vector<CAENVX2750PhaTrigger*>
    VX2750MultiTrigger::getTriggers() const
    {
        return m_triggers;          // Does a copy.
    }
    /**
     * getModules
     *   A bit more work than getTriggers - we must iterate through all
     *   triggers adding their modules to the result vector.
     *  @return std::vector<VX2750Pha*>
     */
    std::vector<VX2750EventSegment*>
    VX2750MultiTrigger::getModules() const
    {
        std::vector<VX2750EventSegment*> result;
        std::for_each(m_triggers.begin(), m_triggers.end(),
            [&result](CAENVX2750PhaTrigger* p) {
                result.push_back(&(p->getModule()));
            }
        );
        return result;
    }
    /**
     * getTriggeredModules
     *    Returns a vector of the modules that have triggered.
     * @return std::vector<VX2750PhaTrigger*>& Note this is not
     *       a const reference to allow a client to service a module and then
     *       remove it from the triggeredlist.
     */
    std::vector<VX2750EventSegment*>&
    VX2750MultiTrigger::getTriggeredModules() 
    {
        return m_triggeredModules;
    }
    /**
     * operator()
     *    Checks for a trigger.
     *    - Clear m_triggeredModules.
     *    - For each member trigger that returns true,
     *      put the triggered module into m_triggeredModules.
     * @return bool m_triggeredModules.size() > 0
     */
     bool
     VX2750MultiTrigger::operator()()
     {
        m_triggeredModules.clear();
        for (auto p : m_triggers) {
    
            if ((*p)()) {
	      m_triggeredModules.push_back(&(p->getModule()));
            }
        }
        return m_triggeredModules.size() > 0;
     }
}
