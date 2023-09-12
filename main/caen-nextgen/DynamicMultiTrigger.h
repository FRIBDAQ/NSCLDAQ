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
* @file     DynamicMultiTrigger.h
* @brief    This encapsulates a trigger that can change with time.
* @author   Ron Fox
* @note While this is quite general, in this example we're going to just
*  stock it with a single VX2750MultiTrigger object that, in turn, will
*  manage the triggers and readout for the modules we have.  Note thate
*  those trigger classes have a bit more oomph to them.
*/
#ifndef DYNAMICMULTITRIGGER_H
#define DYNAMICMULTITRIGGER_H
#include <CEventTrigger.h>
#include <vector>

/**
 * @class DynamicMultiTrigger
 *     A trigger that can be reconfigured at run time.
 *     The client maintains ownership of the actual triggers and must
 *     manage their storage.
 */
class DynamicMultiTrigger : public CEventTrigger {
private:
    std::vector<CEventTrigger*> m_triggers;
public:
    DynamicMultiTrigger();
    virtual ~DynamicMultiTrigger();
    
    // Trigger management:
    
    void addTrigger(CEventTrigger* pTrigger);
    void removeTrigger(CEventTrigger* pTrigger);
    const std::vector<CEventTrigger*>& getTriggers() const;
    void clear();
    
    // Trigger interface:
    
    virtual void setup();
    virtual void teardown();
    virtual bool operator()();
    
};

#endif