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
* @file     DynamicMultiTrigger.cpp
* @brief    Implement the dynamic trigger.
* @author   Ron Fox
*
*/
#include "DynamicMultiTrigger.h"
#include <algorithm>

/**
 * constructor
 *    null
 */
DynamicMultiTrigger::DynamicMultiTrigger() {}

/**
 * destructor
 *   null - the client must have ensured that any dynamic trigger storage
 *   has been deleted.  Note that in practice, the trigger lifetime is
 *   the life of the program (unless, of course, this dynamic trigger is
 *   nested inside another dynamic trigger).
 */
DynamicMultiTrigger::~DynamicMultiTrigger() {}
/////////////////////////////////////////////////////////////////////////////////
// Trigger management.
/**
 * addTrigger
 *    Append a trigger to the set of triggers we're managing.
 * @param pTrigger - points to the trigger to add.    
 */
void
DynamicMultiTrigger::addTrigger(CEventTrigger* pTrigger) {
    m_triggers.push_back(pTrigger);
}
/**
 * removeTrigger
 *   Find and remove a trigger from the set we manage.  We don't delete
 *   the trigger as the client retains ownership.
 *
 *  @param pTrigger - pointer to the trigger to remove.
 *  @note:  It's a silent no-op if the pointer doesn't live in the vector.
 */
void
DynamicMultiTrigger::removeTrigger(CEventTrigger* pTrigger) {
    auto p = std::find(m_triggers.begin(), m_triggers.end(), pTrigger);
    if (p != m_triggers.end()) {
        m_triggers.erase(p);
    }
}
/**
 * getTriggers
 *    Return a readonly reference to the triggers:
 * @return const std::vector<CEventTrigger*>&
 */
const std::vector<CEventTrigger*>&
DynamicMultiTrigger::getTriggers() const {
    return m_triggers;
 
}
/**
 * clear
 *    Remove all triggers.. normally done before adding a brand new set of
 *    trigger objects.
 * @note Can't stress enough the caller retains ownership of the trigger objects.
 */
void
DynamicMultiTrigger::clear() {
    m_triggers.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Implementation of trigger operations.

/**
 * setup
 *     Initialize all trigger modules prior to starting the trigger loop.
 */
void
DynamicMultiTrigger::setup()
{
    for (auto p : m_triggers) {
        p->setup();
    }
}
/**
 * teardown
 *    Any post run teardown operaitons.
 */
void
DynamicMultiTrigger::teardown() {
    for (auto p : m_triggers) {
        p->teardown();
    }
}
/**
 * operator()
 *     Check triggers.. note that no short circuit is done._All_ triggers are polled.
 * @return bool  - if any trigger fired.
 */
bool
DynamicMultiTrigger::operator()() {
    bool triggered(false);
    
    for (auto p : m_triggers) {
        if ((*p)()) triggered = true;
    }
    
    return triggered;
}
