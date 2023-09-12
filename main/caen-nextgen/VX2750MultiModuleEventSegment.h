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
* @file     VX2750MultiModuleEventSegment.h
* @brief    Event segment with several VX2750EventSegments as elements.
* @author   Ron Fox
*
*/
#ifndef VX2750MULTIMODULEEVENTSEGMENT_H
#define VX2750MULTIMODULEEVENTSEGMENT_H

#include <CEventSegment.h>

class CExperiment;
namespace caen_nscldaq {
    // Forward class references:
    
    class VX2750MultiTrigger;
    
    /**
     * @class VX2750MultiModuleEventSegment
     *    Event segment that handles several modules of VX27x0 digitizers.
     *    Note that we assume triggering is being handled by a multitrigger
     *    object and we can use its concept of the triggered modules to actually
     *    do the readout.  We'll get called from the trigger loop and read
     *    _all_ the triggered modules. each as one event using the retrigger
     *    flag in the event segment by calling the experiment's haveMore
     *    until we're done.
     */
    class VX2750MultiModuleEventSegment : public CEventSegment {
    private:
        CExperiment*        m_pExperiment;
        VX2750MultiTrigger* m_pTrigger;
        bool                m_configChanged;
    public:
        VX2750MultiModuleEventSegment(CExperiment* pExperiment, VX2750MultiTrigger* pTrigger);
        virtual ~VX2750MultiModuleEventSegment();
        
        void setConfigChanged();
        
        void initialize();
        void disable();
        void onPause();
        void onResume();
        size_t read(void* pBuffer, size_t maxwords);
    };
}                         // caen_nscldaq namespace.

#endif
