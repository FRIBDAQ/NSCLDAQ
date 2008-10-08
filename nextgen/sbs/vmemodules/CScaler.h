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


//////////////////////////CScaler.h file//////////////////////////////////

#ifndef __CSCALER_H  
#define __CSCALER_H
#ifndef __STL_VECTOR
#include <vector>
#define __STL_VECTOR
#endif                               

                               
/*!
   ABC for scaler modules.
   A scaler module is a collection  of 
   high performance counters which can be
   sequentially read and cleared.
 */		
class CScaler   
{ 
private:

public:
	// Constructors, destructors and other cannonical operations: 

  CScaler ();                      //!< Default constructor.
  CScaler(const CScaler& rhs); //!< Copy constructor.
  ~ CScaler ( ) { } //!< Destructor.
  
  CScaler& operator= (const CScaler& rhs); //!< Assignment
  int         operator==(const CScaler& rhs) const; //!< Comparison for equality.
  int         operator!=(const CScaler& rhs) const {
    return !(operator==(rhs));
  }
  
  // Selectors for class attributes:
public:
  
  // Mutators:
protected:  
  
  // Class operations:
public:
  virtual   void Initialize ()   = 0;
  virtual   void Read (std::vector<unsigned long>& Scalers)   = 0;
  virtual   void Clear ()   = 0;
  virtual   unsigned int size ()   = 0;
  
};

#endif
