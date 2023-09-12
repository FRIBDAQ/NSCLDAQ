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
* @file     CAENVx2750PhaTrigger.h
* @brief    Trigger for a single module
* @author   Ron Fox
*
*/
#ifndef CAENV2750PHATRIGGER_H
#include <CEventTrigger.h>      // base class.

namespace caen_nscldaq {
class VX2750EventSegment;                // referenced class.

/**
 * @class CAENV2750PhaTrigger
 *     Trigger class for NSCLDAQ and the
 *     CAENV2x50 modules.
 *     We contain the support class for the module we trigger.
 */
class  CAENVX2750PhaTrigger : public CEventTrigger
{
private:
    VX2750EventSegment&    m_module;                      // Module we poll.
public:
    CAENVX2750PhaTrigger(VX2750EventSegment& module);
    bool operator()();
    VX2750EventSegment& getModule();
};

}

#endif
