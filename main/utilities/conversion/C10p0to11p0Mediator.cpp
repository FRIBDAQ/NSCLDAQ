/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#include "C10p0to11p0Mediator.h"
#include <CDataSink.h>
#include <V11/CDataFormatItem.h>

namespace DAQ {
  namespace Transform {

    ///////////////////////////////////////////////////////////////////////////
    // C10p0to11p0MediatorCreator IMplementation

    std::unique_ptr<CBaseMediator> C10p0to11p0MediatorCreator::operator ()() const {
      return std::unique_ptr<CBaseMediator>(new C10p0to11p0Mediator());
    }



    ///////////////////////////////////////////////////////////////////////////
    // C10p0to11p0Mediator Implementation
    ///////////////////////////////////////////////////////////////////////////

    //
    C10p0to11p0Mediator::C10p0to11p0Mediator(std::shared_ptr<CDataSource> source,
                                           std::shared_ptr<CDataSink> sink)
      : CTransformMediator<CTransform10p0to11p0>(source, sink)
    {
    }


    //
    void C10p0to11p0Mediator::mainLoop()
    {

      outputRingFormat();

      CTransformMediator::mainLoop();
    }

    //
    void C10p0to11p0Mediator::outputRingFormat()
    {
      CDataSink& sink = *getDataSink();
      sink << V11::CDataFormatItem();
    }

  } // namespace Transform
} // namespace DAQ
