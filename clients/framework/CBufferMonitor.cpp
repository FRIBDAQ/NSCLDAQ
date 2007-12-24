/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#include <config.h>
#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif
#include <CBufferMonitor.h>




/*!
  \fn CEventMonitor::result CBufferMonitor::operator()() 

 Operation Type:
    override
 
 Purpose:
    Waits for a buffer to be received. Returns one of
    the following: 
    Occurred - a buffer was received into m_Buffer
    TimedOut - Timeouts were enabled and no buffer was
    received during the timeout interval.

*/
CEventMonitor::result
CBufferMonitor::operator() ()
{ 
  m_Buffer.SetTag(m_nTag);
  m_Buffer.SetMask(m_nMask);
  // If timeouts were enabled:
  if(getTimedWait()) {
    struct timeval timeout = getTimeout();
    // Accept a buffer
    m_Buffer.Accept(&timeout);
    if(m_Buffer.GetLen() != 0)
      return CEventMonitor::Occurred;
    else
      return CEventMonitor::TimedOut;
  }
  // Timeouts disabled
  else {
    m_Buffer.Accept();
    if(m_Buffer.GetLen() != 0)
      return CEventMonitor::Occurred;
    else
      return CEventMonitor::TimedOut;
  }
}

/*!
  \fn int CBufferMonitor::AddLink (const string& URL, int tag = COS_ALLBITS,
                                   int mask = COS_ALLBITS, bool fReliable=true)

 Operation Type:
    Mutator

 Purpose:
    Adds a link to the link manager. The link id is returned.
    On failure, a CLinkFailedException is thrown.
*/

int
CBufferMonitor::AddLink (const string& URL, int tag, int mask, 
			    bool fReliable)
{
  LinkInfo info;
  info.Tag = tag;
  info.Mask = mask;
  info.URL = URL;
  DAQURL daqurl = DAQURL(URL.c_str());
  info.linkid = daq_link_mgr.AddSink(daqurl, tag, mask, 
				     fReliable ? COS_RELIABLE 
				     : COS_UNRELIABLE);
  if(info.linkid == 0) {
    throw CLinkFailedException
      ("CBufferMonitor::AddLink Adding link to list", info.linkid);
  }
  m_lLinks.push_back(info);
  return info.linkid;
}

/*!
  \fn void CBufferMonitor::RemoveLink (int linkid)

 Operation Type:
    Mutator

 Purpose:
    If the specified link exists, it is removed from the
    link list and deleted from the spectrodaq link manager.
    If the link does not exist, a CNoSuchLinkException
    is thrown.
*/
void
CBufferMonitor::RemoveLink (int linkid)
{
  LinkIterator linkIt;
  for(linkIt = m_lLinks.begin(); linkIt != m_lLinks.end(); linkIt++) {
    if ((*linkIt).linkid == linkid) {
      if(!(daq_link_mgr.DeleteSink(linkid))) {
	throw CNoSuchLinkException
	  ("CBufferMonitor::RemoveLink Removing link from list", 
	   linkid);
      }
      m_lLinks.erase(linkIt);
    }
  }
  if(linkIt == m_lLinks.end())
    throw CNoSuchLinkException
      ("CBufferMonitor::RemoveLink Removing link from list", linkid);
}

/*!
  \fn void CBufferMonitor::RemoveLink (LinkIterator link)

 Operation Type:
    Mutator
 
 Purpose:
    Removes a link given the iterator to its link structure
    in the link list. If the iterator is end(), a
    CNoSuchLinkException is thrown.
*/
void
CBufferMonitor::RemoveLink (LinkIterator link)
{
  if(link != m_lLinks.end()) {
    if(!(daq_link_mgr.DeleteSink((*link).linkid))) {
      throw CNoSuchLinkException
	("CBufferMonitor::Remove Removing link from list", (*link).linkid);
    }
    m_lLinks.erase(link);
  }
  else {
    throw CNoSuchLinkException
      ("CBufferMonitor::Remove Removing link from list", (*link).linkid);
  }
}


/*!
  \fn LinkIterator CBufferMonitor::beginLinks()

 Operation Type:
    Selector
 
 Purpose:
    Returns an iterator to the beginning of the
    link list.
*/
LinkIterator
CBufferMonitor::beginLinks()
{
  return m_lLinks.begin();
}

/*!
  \fn LinkIterator CBufferMonitor::endLinks()

 Operation Type:
    Selector
 
 Purpose:
    Returns an iterator suitable for determining
    end of iteration through the link list.
*/
LinkIterator
CBufferMonitor::endLinks()
{
  return m_lLinks.end();
}

/*!


 Operation Type:
    Selector
    
 Purpose:
    Returns a pointer to the DAQ Buffer.

*/
DAQWordBufferPtr
CBufferMonitor::getBufferPointer (int nOffset)
{
  DAQWordBufferPtr p(&m_Buffer);
  p += nOffset;
  return p;
}

/*!
  \fn void CBufferMonitor::SetBufferTag (int tag = COS_ALLBITS)

 Operation Type:
    Mutator
 
 Purpose:
    Sets the tag matched on receives into the buffer.
    For each link/buffer pair, tags are used to determine
    which buffers routed through SpectroDaq will be
    received by a link or buffer.  The logic is that
    at each stage, the routed buffer's tag is anded with
    the receiving entity's mask.  If this is equal to the
    receiving entity's tag, the buffer is accepted.
    So, for a given link (link.mask, link.tag), and
    our buffer (buffer.mask, buffer.tag):
    A routed buffer rbuffer.tag is received when:
    
    ((rbuffer.tag & link.mask) == link.tag) &&
    ((rbuffer.tag & buffer.mask) == buffer.tag)
*/
void 
CBufferMonitor::SetBufferTag (int tag)
{
  try {
    m_Buffer.SetTag(tag);
    m_nTag = tag;
  }
  catch (DAQException& de) {
    throw de;
  }
}

/*!
  \fn void CBufferMonitor::SetBufferMask (int nMask)

 Operation Type:
    Mutator
 
 Purpose:
    Sets the receive mask associated with the buffer.
    See SetBufferTag for an explanation of tags and masks
    and how they interact with link tags and masks and the tag
    of the incomming buffer to determine receipt.
*/
void
CBufferMonitor::SetBufferMask (int nMask)
{
  try {
    m_Buffer.SetMask(nMask);
    m_nMask = nMask;
  }
  catch (DAQException& de) {
    throw de;
  }
}

/*!
  \fn string CBufferMonitor::DescribeSelf()

 Operation Type:
    Selector
 
 Purpose:
    Produces a desciprtion string of the object.  This includes
    1. Calling CEventManager::DescribeSelf()
    2. Putting out the tag and mask of the buffer.
    3. Listing the links and their information.
*/
string
CBufferMonitor::DescribeSelf()
{
  string Result;
  int count = 0;
  Result = CEventMonitor::DescribeSelf();
  if(m_Buffer.GetLen() > 0) {
    Result += "\n  Tag = ";
    char tagBuffer[20];
    sprintf(tagBuffer, "%d", m_Buffer.GetTag());
    Result += tagBuffer;
    char maskBuffer[20];
    sprintf(maskBuffer, "%d", m_Buffer.GetMask());
    Result += "\n  Mask = ";
    Result += maskBuffer;
    Result += "\n  Link info:";
    if(m_lLinks.size() > 0) {
      for(LinkIterator It = beginLinks(); It != endLinks(); It++) {
	Result += "\n    Link #";
	char countBuffer[256];
	sprintf(countBuffer, "%d", ++count);
	Result += countBuffer;
	Result += "\n      Tag =    ";
	char tagbuf[256], maskbuf[256], linkbuf[256];
	sprintf(tagbuf, "%d", (*It).Tag);
	Result += tagbuf;
	Result += "\n      Mask =   ";
	sprintf(maskbuf, "%d", (*It).Mask);
	Result += maskbuf;
	Result += "\n      URL =    ";
	Result += (*It).URL;
	Result += "\n      Linkid = ";
	sprintf(linkbuf, "%d", (*It).linkid);
	Result += linkbuf;
      }
    }
    else
      Result += "\n    No links currently exist";
  }
  else
    Result += "\n  There is no buffer";

  return Result;
}


