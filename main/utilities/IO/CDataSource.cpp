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
#include "CDataSource.h"
#include <CTimeout.h>

namespace DAQ {


////////////////////////////////////////////////////////////////////////////////
//
// Implemented canonicals:

CDataSource::CDataSource() : m_eof(false) {}
CDataSource::~CDataSource() {}


void CDataSource::read(char *pBuffer, size_t nBytes)
{
    CTimeout timeout(std::numeric_limits<size_t>::max());
    timedRead(pBuffer, nBytes, timeout);
}

} // end DAQ
