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
# @file   CTclRingCommand.h
# @brief  Define the 'ring' command which gives limited access to ring data.
# @author <fox@nscl.msu.edu>
*/


#ifndef CTCLRINGCOMMAND_H
#define CTCLRINGCOMMAND_H

#include <TCLObjectProcessor.h>
#include <map>
#include <TCLObject.h>

class CTCLInterpreter;
class CRingBuffer;
class CRingItem;
class CRingSelectionPredicate;
class CTimeout;

/**
 * @class CTclRingCommand
 *
 * This class implements the ring command.  The ring command allows
 * Tcl scripts to establish a connection to an NSCL Ring buffer and to
 * get limited data types from that ringbuffer.  As this is part of the
 * scaler display program, the types of data that are accepted from the ring
 * are only:
 *
 * *  The state change types.
 * *  PERIODIC_SCALERS
 *
 * This can only be used for data in version 11.0 format.
 *
 *  Command format:
 *  \verbatim
 *     ring attach ringname
 *     ring detach ringname
 *     ring get ?-timeout usec? ringname  ?acceptable-types?
 * \endverbatim
 */
class CTclRingCommand : public CTCLObjectProcessor
{
private:
    std::map<std::string, CRingBuffer*> m_attachedRings;
    
public:
    CTclRingCommand(CTCLInterpreter& interp);
    virtual ~CTclRingCommand();
    
    virtual int operator()(
        CTCLInterpreter& interp, std::vector<CTCLObject>& objv
    );
    
protected:
    void attach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void detach(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void get(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // Local utilities.
private:
    CTCLObject formatBodyHeader(CTCLInterpreter& interp, CRingItem* pItem);
    void formatStateChangeItem(CTCLInterpreter& interp, CRingItem* pItem, CTCLObject& result);
    void formatScalerItem(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result);
    void formatStringItem(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result);
    void formatFormatItem(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result);
    void formatEvent(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result);
    void formatFragment(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result);
    void formatTriggerCount(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result);
    void formatGlomParams(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result);
    void formatAbnormalEnd(CTCLInterpreter& interp, CRingItem* pSpecificItem, CTCLObject& result);

    CRingItem* getFromRing(CRingBuffer& ring, CRingSelectionPredicate& predicate,
                           unsigned long timeout);
    CRingItem* getFromRing(CRingBuffer& ring, CTimeout& timeout);
    void setTiming(
        CTCLObject& result, int offset, int divisor, double seconds,
        int stamp
    );
};

#endif
