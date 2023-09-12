/**
 * @file DDASEvent.cpp
 * @brief Implementation of a class to encapsulate the information in a built
 * DDAS event.
 */ 

#include "DDASEvent.h"

ClassImp(DDASEvent)

DDASEvent::DDASEvent() : TObject(), m_data() {}

/**
 * Implement a deep copy.
 */
DDASEvent::DDASEvent(const DDASEvent& obj)
    : TObject(obj), m_data()
{
    // Create new copies of the ddaschannel events
    for (UInt_t i = 0; i < m_data.size(); ++i) {
        m_data[i] = new ddaschannel(*obj.m_data[i]);
    }
    
}

/**
 * Performs a deep copy of the data belonging to obj. There is no attempt 
 * to make this exception-safe.
 */
DDASEvent& DDASEvent::operator=(const DDASEvent& obj)
{
    // This assignment operator is simple at the expense of some safety. The
    // entirety of the data vector is deleted prior to assignment. If the
    // initialization of the ddaschannel objects threw an exception or caused
    // something else to happen that is bad, then it would be a big problem.
    // The possibility does exist for this to happen. Coding up a safer version
    // is just more complex, harder to understand, and will be slower.
    
    if (this!=&obj) {
        // Create new copies of the ddaschannel events
        for (UInt_t i=0; i<m_data.size(); ++i) {
            delete m_data[i];
        }
        m_data.resize(obj.m_data.size());
        for (UInt_t i=0; i<m_data.size(); ++i) {
            m_data[i] = new ddaschannel(*obj.m_data[i]);
        }
    }

    return *this;
}

/**
 * Deletes the objects stored in m_data and clear the vector.
 */
DDASEvent::~DDASEvent()
{
    Reset();
}

/**
 * Appends the pointer to the internal, extensible data array.
 * There is no check that the object pointed to by the argument
 * exists, so that it is the user's responsibility to implement.
 */
void DDASEvent::AddChannelData(ddaschannel* channel)
{
    m_data.push_back(channel);
}

/**
 * If data exists return timestamp of first element in the array. This should 
 * be the earliest unit of data stored by this object. If no data exists, 
 * returns 0.
 */ 
Double_t DDASEvent::GetFirstTime() const
{
    Double_t time = 0;
    if (m_data.size() > 0) { 
        time = m_data.front()->GetTime();
    }
    
    return time;
}

/**
 * If data exists return timestamp of last element in the array. This should 
 * be the most recent unit of data stored by this object. If no data exists, 
 * returns 0.
 */
Double_t DDASEvent::GetLastTime() const
{
    Double_t time = 0;
    if (m_data.size() > 0) { 
        time = m_data.back()->GetTime();
    }
    
    return time;
}

/**
 * Calculate and return the timestamp difference between the last and first
 * elements of the data vector. If the data vector is empty, returns 0.
 */
Double_t DDASEvent::GetTimeWidth() const
{
    return GetLastTime() - GetFirstTime();
}

/**
 * Deletes the ddaschannel data objects and resets the  size of the extensible 
 * data array to zero. 
 */
void DDASEvent::Reset()
{
    // Delete all of the object stored in m_data
    for (UInt_t i = 0; i < m_data.size(); ++i) {
        delete m_data[i];
    }
    
    // Clear the array and resize it to zero
    m_data.clear();
}
