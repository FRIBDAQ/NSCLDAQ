// /*
//     This software is Copyright by the Board of Trustees of Michigan
//     State University (c) Copyright 2022.

//     You may use this software under the terms of the GNU public license
//     (GPL).  The terms of this license are described at:

//      http://www.gnu.org/licenses/gpl.txt

//      Authors:
//              Simon Giraud
// 	     FRIB
// 	     Michigan State University
// 	     East Lansing, MI 48824-1321
// */

// Simon - 10/2024 - Purposes: 
// (1) Group srs hits for each fec (source) based on hit timestamp.
// (2) Do sanity checks on the timestamp order and max hits per fec. 


#include "SRSSorter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <memory>
#include <CDataSink.h>
#include <DataFormat.h>
#include <CRingItem.h>


SRSSorter::SRSSorter() : m_maxHits(960), m_dtHits(2) {}//960

SRSSorter::SRSSorter(uint16_t maxHits = 960, int dtHits = 2) {//960
    m_maxHits = maxHits;
    m_dtHits = dtHits;
}


SRSSorter::~SRSSorter() {
}


void SRSSorter::reset() {
    for(int fecId = 0; fecId < MaxFECs; fecId++)
        m_event[fecId].reset();
}


// Hits (data) come here one by one.
// Inputs also sid and sink because for the moment the sorter will just group, into a RI,
// hits from the same fec (sink). Then, the event builder will sort the RI from different fec.  
void SRSSorter::sort(uint8_t* data, const uint64_t hitTimeStamp, int sid, CDataSink& sink, size_t nBytes){

    int fecId = sid - 10;

    int64_t tsDiff = hitTimeStamp - m_event[fecId].timestamp;

    // Sanity checks
    // (1) Skip hit/trig marker if smaller timesamp.
    if (tsDiff < 0){
        printf("SRSSorter::sort - tsDiff < 0 - currentTs: %llu prevTs : %llu \n", hitTimeStamp, m_event[fecId].timestamp);
        return;
    }
    // (2) If too many hits, delete the RI and dont't put in sink (discard).
    if (m_event[fecId].nHits > m_maxHits){
        printf("SRSSorter::sort - nHits > m_maxHits,  ts %llu \n", hitTimeStamp);
        m_event[fecId].nHits = 0;
        // Delete RI with too many hits
        deleteRingItem(fecId);
        // To not fill the (deleted) RI
        m_event[fecId].discard = true;
    }

    // Passed sanity checks, create RI 
    if (tsDiff > m_dtHits || hitTimeStamp == 0){
        newRingItem(hitTimeStamp, fecId, sink);
        m_event[fecId].timestamp = hitTimeStamp;
        m_event[fecId].nHits = 0;
        m_event[fecId].discard = false;
    }

    // nBytes is set to 0 for trig marker in UDPBrokerDerived.
    // Avoid appenning trig marker to RI.
    // Note: If no hit with corresponding trig marker, the RI has null body size.
    if (nBytes > 0 && !m_event[fecId].discard){
        appendRingItem(fecId, data);
    }

    return;
}


void SRSSorter::newRingItem(const uint64_t hitTimeStamp, int fecId, CDataSink& sink){

    if (m_event[fecId].pRingItem != nullptr) {
        sink.putItem(*m_event[fecId].pRingItem);
        deleteRingItem(fecId);
        m_event[fecId].pRingItem = new CRingItem(PHYSICS_EVENT, hitTimeStamp, fecId + 10, 0, m_packetSize + 1024);
        m_event[fecId].pRingItem->setBodyCursor(m_event[fecId].pRingItem->getBodyCursor());
    } else {
        m_event[fecId].pRingItem = new CRingItem(PHYSICS_EVENT, hitTimeStamp, fecId + 10, 0, m_packetSize + 1024);
        m_event[fecId].pRingItem->setBodyCursor(m_event[fecId].pRingItem->getBodyCursor());
    }

    return;
}



void SRSSorter::appendRingItem(int fecId, uint8_t* data){
    uint16_t oldSize = m_event[fecId].pRingItem->getBodySize();
    // Copy the 8 bytes of data into the CRingItem's body
    memcpy(m_event[fecId].pRingItem->getBodyCursor(), data, HitAndMarkerSizeExtended);
    // Update cursor after copy
    m_event[fecId].pRingItem->setBodyCursor(reinterpret_cast<uint8_t*>(m_event[fecId].pRingItem->getBodyCursor()) + HitAndMarkerSizeExtended);
    m_event[fecId].pRingItem->updateSize();
    
    //printf("Simon - SRSSorter::appendRingItem - body size - old: %lu new : %lu\n", oldSize, m_event[fecId].pRingItem->getBodySize());

    // if (oldSize < m_event[fecId].pRingItem->getBodySize()){
        m_event[fecId].nHits = m_event[fecId].nHits + 1;
    // }
    // else {
    //     printf("Simon - SRSSorter::appendRingItem - body size unchanged - old: %lu new : %lu\n", oldSize, m_event[fecId].pRingItem->getBodySize());
    // }
}


void SRSSorter::deleteRingItem(int fecId){

    if (m_event[fecId].pRingItem != nullptr) {
        delete m_event[fecId].pRingItem;
    }
    m_event[fecId].pRingItem = nullptr;

    return;
}