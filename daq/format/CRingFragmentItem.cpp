/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include "CRingFragmentItem.h"
#include "DataFormat.h"
#include <string.h>


/*-------------------------------------------------------------------------------------
 *   Canonical methods
 */

/**
 * constructor 
 *
 *  Create a new ring fragmen from the data supplied:
 *
 * @param timestamp   - The event builder timestamp.
 * @param source      - Source id of fragment creator.
 * @param payloadSize - Size of the fragment payload.
 * @param pBody       - Pointer to payload.
 * @param barrier     - Barrier type (defaults to 0 which is not a barrier).
 */
CRingFragmentItem::CRingFragmentItem(uint64_t timestamp, uint32_t source, uint32_t payloadSize, 
				     const void* pBody, uint32_t barrier) :
  CRingItem(EVB_FRAGMENT, bodySize(payloadSize)) 

{
  init(payloadSize);
  m_pFragment->s_timestamp = timestamp;
  m_pFragment->s_sourceId  = source;
  m_pFragment->s_barrierType = barrier;
  m_pFragment->s_payloadSize  = payloadSize;
  copyPayload(pBody);

}

/**
 * constructor 
 * 
 * Given a reference to a ring item create a CRingFragment item:
 * - If the type of the ring item is not EVB_FRAGMENT, throw std::bad_cast.
 * - Otherwise create this from the ring item.
 *
 * @param item - Reference to a generic ring item.
 */
CRingFragmentItem::CRingFragmentItem(const CRingItem& rhs) throw(std::bad_cast) :
  CRingItem(rhs)
{
  if (type() != EVB_FRAGMENT) {
    throw std::bad_cast();
  }
  pEventBuilderFragment pItem =
    reinterpret_cast<pEventBuilderFragment>(const_cast<CRingItem&>(rhs).getItemPointer());
  init(pItem->s_payloadSize);

  m_pFragment->s_timestamp = pItem->s_timestamp;
  m_pFragment->s_sourceId  = pItem->s_sourceId;
  m_pFragment->s_barrierType = pItem->s_barrierType;
  m_pFragment->s_payloadSize = pItem->s_payloadSize;
  copyPayload(pItem->s_body);

}

/**
 * Copy constructor.
 *
 * Construct this as an identical copy of an existing item.
 * 
 * @param rhs - const reference to what we are copying.
 */
CRingFragmentItem::CRingFragmentItem(const CRingFragmentItem& rhs) :
  CRingItem(rhs)
{
  pEventBuilderFragment pItem = const_cast<pEventBuilderFragment>(rhs.m_pFragment);
  init(pItem->s_payloadSize);

  m_pFragment->s_timestamp = pItem->s_timestamp;
  m_pFragment->s_sourceId  = pItem->s_sourceId;
  m_pFragment->s_barrierType = pItem->s_barrierType;
  m_pFragment->s_payloadSize = pItem->s_payloadSize;
  copyPayload(pItem->s_body);

}
/**
 * destructor
 *
 * Base class does everything we need:
 */
CRingFragmentItem::~CRingFragmentItem() {}

/**
 * assignment
 *
 *  This is pretty much like copy construction.
 *
 *
 * @param rhs - const reference to what we are copying.
 * @return CRingFragmentItem& reference to *this.
 *  
 */
CRingFragmentItem&
CRingFragmentItem::operator=(const CRingFragmentItem& rhs) 
{
  if (&rhs != this) {
    CRingItem:: operator=(rhs);
    pEventBuilderFragment pItem = const_cast<pEventBuilderFragment>(rhs.m_pFragment);
    init(pItem->s_payloadSize);
    
    m_pFragment->s_timestamp = pItem->s_timestamp;
    m_pFragment->s_sourceId  = pItem->s_sourceId;
    m_pFragment->s_barrierType = pItem->s_barrierType;
    m_pFragment->s_payloadSize = pItem->s_payloadSize;
    copyPayload(pItem->s_body);
  }
  return *this;
}
/**
 * operator==
 *
 *   There's a really good chance taht two items are equal if they have
 *   the same timestamp, sourcid, size and barrier value
 *
 * @param rhs - Const reference to the item to compare to *this.
 * @return int - nonzero for equality zero otherwise.
 */
int
CRingFragmentItem::operator==(const CRingFragmentItem& rhs) const
{
  const pEventBuilderFragment pFrag = rhs.m_pFragment;

  return (m_pFragment->s_timestamp == pFrag->s_timestamp)              &&
    (m_pFragment->s_sourceId       == pFrag->s_sourceId)               &&
    (m_pFragment->s_barrierType    == pFrag->s_barrierType)            &&
    (m_pFragment->s_payloadSize    == pFrag->s_payloadSize);

}
/**
 * operator!=
 *   
 * This is just the logical inverse of operator==
 * @param rhs - Const reference to the item to compare to *this.
 * @return int - nonzero for inequality zero otherwise.
 */
int
CRingFragmentItem::operator!=(const CRingFragmentItem& rhs) const
{
  return !(operator==(rhs));
}

/*------------------------------------------------------------------------------
 * getters:
 */

/**
 * Return the current object's timestamp.
 *
 * @return uint64_t
 */
uint64_t
CRingFragmentItem::timestamp() const
{
  return m_pFragment->s_timestamp;
}
/**
 * source
 *
 * Return the source id.
 *
 * @return uint32_t
 */
uint32_t 
CRingFragmentItem::source() const
{
  return m_pFragment->s_sourceId;
}
/**
 * return the size of thefragment payload
 * 
 * @return size_t
 */
size_t
CRingFragmentItem::payloadSize() const
{
  return static_cast<size_t>(m_pFragment->s_payloadSize);
}
/**
 * Return a const pointer to the payload.
 *
 * @return const void*
 */
const void*
CRingFragmentItem::payloadPointer() const
{
  return reinterpret_cast<const void*>(m_pFragment->s_body);
}
/**
 * return the barrier type:
 *
 * @return int32_t
 */
uint32_t
CRingFragmentItem::barrierType() const
{
  return m_pFragment->s_barrierType;
}
/*---------------------------------------------------------------------------
 * Private utilities:
 */

/**
 * bodySize
 *
 *  Returns the full size of the body of the event given the size of the payload.
 *
 * @param payloadSize Number of bytes in the payload.
 *
 * @return size_t
 */
size_t
CRingFragmentItem::bodySize(size_t payloadSize) const
{
  return sizeof(EventBuilderFragment) + (payloadSize-1) - sizeof(RingItemHeader);
}
/**
 * copyPayload
 *
 *  Copies the payload source to our payload destination.
 *  - It is assumed the destination size is big enough.
 *  - The cursor is reset after the copy.
 *  - The size is updated after the copy.
 *  - The payload size element is used to determine the data size.
 *
 * @param pPayloadSource - Pointer to the buffer containing the payload.
 *
 */
void
CRingFragmentItem::copyPayload(const void* pPayloadSource)
{
  memcpy(m_pFragment->s_body, pPayloadSource, m_pFragment->s_payloadSize);

  size_t       s = bodySize(m_pFragment->s_payloadSize);
  uint8_t* pBody = reinterpret_cast<uint8_t*>(getBodyPointer());
  pBody          += s;
  
  setBodyCursor(pBody);
  updateSize();
}
/**
 * init
 *
 * Initialize the size and various pointers in the base class.  Ensures the buffer is big enough
 * to take the data.  This should not be done with a fragment that has any data/meaning.
 *
 *  @param size Size of payload
 */
void
CRingFragmentItem::init(size_t size)
{
  size_t n = bodySize(size);

  deleteIfNecessary();
  newIfNecessary(size);

  uint8_t* pCursor = reinterpret_cast<uint8_t*>(getBodyPointer());
  m_pFragment      = reinterpret_cast<pEventBuilderFragment>(pCursor);
  pCursor         += n;
  setBodyCursor(pCursor);
  updateSize();

}
