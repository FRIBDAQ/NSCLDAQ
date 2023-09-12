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
* @file     CAENVX2750PhaTrigger.cpp
* @brief    Implement the trigger class.
* @author   Ron Fox
*
*/
#include "CAENVX2750PhaTrigger.h"
#include "VX2750EventSegment.h"
#include "VX2750Pha.h"
namespace caen_nscldaq {
/**
 * constructor:
 *    @param module - the module we'll check for data.
 */

CAENVX2750PhaTrigger::CAENVX2750PhaTrigger(VX2750EventSegment& module) :
    m_module(module)
{}

/**
 * operator()
 *    @return bool - true if the module has data to read.
 */
bool
CAENVX2750PhaTrigger::operator()()
{
    return m_module.getModule()->hasData();
}
/**
 * return reference to the module:
 */
VX2750EventSegment&
CAENVX2750PhaTrigger::getModule()
{
    return m_module;
}
}
