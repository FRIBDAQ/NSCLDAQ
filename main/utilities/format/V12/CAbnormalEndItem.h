/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CAbnormalEndItem.h
# @brief  Define the wrapper class for the abnormal end run ring item.
# @author <fox@nscl.msu.edu>
*/
#ifndef DAQ_V12_CABNORMALENDITEM_H
#define DAQ_V12_CABNORMALENDITEM_H

#include "V12/CRingItem.h"

namespace DAQ {
  namespace V12 {

class CAbnormalEndItem : public CRingItem
{
private:
    uint64_t m_evtTimestamp;
    uint32_t m_sourceId;

public:
    CAbnormalEndItem() = default;
    CAbnormalEndItem(const CAbnormalEndItem& rhs) = default;
    CAbnormalEndItem& operator=(const CAbnormalEndItem& rhs) = default;
    CAbnormalEndItem(const CRawRingItem& rhs);
    virtual ~CAbnormalEndItem() = default;
    
    int operator==(const CAbnormalEndItem& rhs) const;
    int operator!=(const CAbnormalEndItem& rhs) const;

    // Formatting:
    
public:
    uint64_t getEventTimestamp() const;
    void setEventTimestamp(uint64_t tstamp);

    uint32_t getSourceId() const;
    void setSourceId(uint32_t id);

    uint32_t type() const;
    void setType(uint32_t type);

    uint32_t size() const;

    bool isComposite() const;
    bool mustSwap() const;

    void toRawRingItem(CRawRingItem& rawItem) const;

    virtual std::string typeName() const;
    virtual std::string toString() const;
};

  } // end of V12 namespace
} // end DAQ

#endif
