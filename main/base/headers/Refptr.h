/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef REFPTR_H
#define REFPTR_H
#pragma interface
//
//   This file contains template classes to provide automated memory 
//   management via smart pointers.
//   Each smart pointer points to a reference counted smart pointer and
//   delegates the pointer operations to it.
//


//
// CReferenceCounted:
//   Contains a reference counted pointer.
//

#include <stdio.h>
template <class T>
class CReferenceCounted
{
private:
  unsigned m_ReferenceCount;
  T*       m_Pointer;

  // Copy construction and assignment will not be allowed:

  template <class U>
  CReferenceCounted<T>& operator=(const CReferenceCounted<U>& rhs);
  template <class U>
  CReferenceCounted(const CReferenceCounted<U>& rhs);
public:
  CReferenceCounted(T* p) : 
    m_ReferenceCount(1),
    m_Pointer(p) {}
  ~CReferenceCounted()  { delete m_Pointer; }
  T& operator*()        { return *m_Pointer; }
  T* get()              { return m_Pointer; }
  void set(T* newp) { m_Pointer = newp; }
  void reference()      
    { m_ReferenceCount++; }
  void dereference()    
    { m_ReferenceCount--; }
  int  norefs()         { return !m_ReferenceCount; }
  int  refcount()       { return m_ReferenceCount; }  
};


template <class T>
class CRefcountedPtr
{
private:
  CReferenceCounted<T> *m_Object;
public:
  CRefcountedPtr() : m_Object(0) {}
  CRefcountedPtr(T* p) :		// Constructor
    m_Object(new CReferenceCounted<T>(p)) {}
  ~CRefcountedPtr() {		// Destructor
    if(m_Object) {
      m_Object->dereference();
      if(m_Object->norefs()) {
	delete m_Object;
      }
    }
  }
  template <class U>
  CRefcountedPtr(const CRefcountedPtr<U>& rhs) {  // Copy constructor.
    m_Object = rhs.m_Object;
    if(m_Object) {
      m_Object->reference();
    }
  }

  CRefcountedPtr&  operator=(const CRefcountedPtr& rhs) 
  {				// Assignment operator. 
    if(this == &rhs) return *this;

    if(m_Object) {
      m_Object->dereference();
      if(m_Object->norefs()) delete m_Object;
      m_Object = 0;
    }
    if(rhs.m_Object) {
      m_Object = rhs.m_Object;
      m_Object->reference();
    }
    return *this;
  }
  int refcount() const { 
     if(m_Object) {
       return m_Object->refcount();
     }
     else {
       return -1;
     }
  }
  // Operations:
  //
  template <class U>
  int operator==(const CRefcountedPtr<U>& rhs) const {
    return (m_Object == rhs.m_Object);
  }
  T& operator*() { return m_Object->operator*(); }
  T* operator->() {return m_Object->get(); }
  T& operator[](int n) { return m_Object->operator[](n); }
  T* get()             { return m_Object->get(); }
  void set(T* newp) { m_Object->set(newp); }  // Repoint.   
};


#endif
