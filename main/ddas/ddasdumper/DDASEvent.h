/**
 * @file DDASEvent.h
 * @brief Defines a class to encapsulate the information in a built DDAS event.
 */ 

// DDASEvent.h
//
// A class to aggregate separate ddaschannel objects.
//
// Author: Jeromy Tompkins
// Date  : 5/6/2013   
//

#ifndef DDASEVENT_H
#define DDASEVENT_H

#include <vector>

#include <TObject.h>

#include "ddaschannel.h"

/**
 * @addtogroup libddaschannel libddaschannel.so
 * @brief DDAS data format for ROOT I/O e.g. produced by the ddasdumper 
 * program.
 * @{
 */

/** 
 * @class DDASEvent DDASEvent.h
 * @brief Encapsulates a Built DDAS event 
 *
 * @details
 * Any data that was written to disk downstream of the NSCLDAQ event
 * builder will have a "built" structure. What that means is that the body
 * of the physics event item will contain data from more than one DDAS 
 * event. The DDASEvent class represents this type of data. It provides
 * access to the events that make it up through the ddaschannel objects
 * it owns and then also provides some useful methods for getting
 * data from the event as a whole.
 */
class DDASEvent : public TObject
{
private:
    std::vector<ddaschannel*> m_data; ///< Extensible array of primitive ddaschannel objects

public:
    /** @brief Default constructor. */
    DDASEvent();
    /** @brief Copy constructor. */
    DDASEvent(const DDASEvent& obj);
    /** 
     * @brief Assignment operator.
     * @return Reference to the object.
     */
    DDASEvent& operator=(const DDASEvent& obj);
    /** 
     * @brief Destructor.
     */
    ~DDASEvent();
    /** 
     * @brief Access internal, extensible array of channel data.
     * @return Reference to the data.
     */ 
    std::vector<ddaschannel*>& GetData() { return m_data;}
    /**
     * @brief Return the number of hits in this event.
     * @return The number of hits in the event (size of the event vector).
     */
    UInt_t GetNEvents() const { return m_data.size(); }
    /** 
     * @brief Append channel data to event.
     * @param channel Pointer to a ddaschannel object.
     */
    void AddChannelData(ddaschannel* channel);
    /** 
     * @brief Get timestamp of first channel datum.
     * @return Timestamp of the first element in the data vector.
     */
    Double_t GetFirstTime() const;
    /** 
     * @brief  Get timestamp of last channel datum.
     * @return Timestamp of the last element in the data vector.
     */
    Double_t GetLastTime() const;        
    /** @brief Get time difference between first and last channel data */
    Double_t GetTimeWidth() const;
    /** @brief Clear data vector and reset event */ 
    void Reset();

    ClassDef(DDASEvent,1);
};

/** @} */

#endif
