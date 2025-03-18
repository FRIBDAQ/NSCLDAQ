#ifndef CCOMPOUNDEVENTSEGMENT_H
#define CCOMPOUNDEVENTSEGMENT_H




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
#include "CComposite.h"
#include <CEventSegment.h>
#include <stddef.h>
#include <stdint.h>

/*!
  CCompound event segment is a CEventSegment that is also a Composite.
  The composite class is inherited but re-exposed via a type-safe adaptor
  that ensures we can only contain other event segments.

  The CEventSegment interface visits all of the contained elements
  and delegates to them sequentially.

  Since this is a CEventSegment, we can put other compound event segments
  into us providing a hierarchical assembly of event segments.
*/
class CCompoundEventSegment : public CComposite, public CEventSegment
{
public:
  typedef CComposite::objectIterator EventSegmentIterator;

  class CVisitor {
  public:
    virtual void operator()(CEventSegment* pSegment) = 0;
  };

public:
  // members inherited from CEventSegment:

  virtual void   initialize();
  virtual void   clear();
  virtual void   disable();
  virtual size_t read(void* pBuffer, size_t maxwords);
  virtual void   onBegin();
  virtual void   onEnd();
  virtual void   onPause();
  virtual void   onResume();



  // Type-safe adaptor to CComposite:

  virtual void AddEventSegment(CEventSegment*    pSegment);
  virtual void DeleteEventSegment(CEventSegment* pSegment);
  EventSegmentIterator begin();
  EventSegmentIterator end();
  virtual bool isComposite() const;

  void visit(CVisitor& visitor);
  
};
#endif