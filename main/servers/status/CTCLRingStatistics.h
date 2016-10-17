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
# @file   CTCLRingStatistics.h
# @brief  TCL bindings to the ring statistics interface.
# @author <fox@nscl.msu.edu>
*/

#ifndef CTCLRINGSTATISTICS_H
#define CTCLRINGSTATISTICS_H
#include <TCLObjectProcessor.h>
#include <CStatusMessage.h>
#include <map>
#include <zmq.hpp>

#include <CStatusMessage.h>

class CTCLInterpreter;
class CTCLObject;


/**
 * @class CTCLRingStatistics
 *    Defines an object oriented Tcl interface to the
 *    CStatusDefinitions::RingStatistics class.
 *
 *    This is implemented as an outer class that handles construction, registration
 *    and destruction of instance of an inner class that actually implement
 *    the api.
 *    Note that we rely on the global external gpZmqContext to point to a
 *    zmq::context_t that was created at package initialization
 *    time.
*/
class CTCLRingStatistics : public CTCLObjectProcessor {
private:
    class RingStatistics;                         // Object instances.
    typdef std::map<std::string, RingStatistics*> ObjectRegistry;
    
private:
    ObjectRegistry m_registry;
    static unsigned       m_instanceNumber;
    static zmq::context_t m_zmqContext;
    // Canonicals:
    
public:
    CTCLRingStatistics(
        CTCLInterpreter& interp, const char* command="RingStatistics"
    );
    virtual ~CTCLRingStatistics();
    
    // Executes (constructor/destructor)
    
public:
    virtual int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // object constructors/destructors
    
private:
    void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    void destroy(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    
    // The nested class:
    
    class RingStatistics: public CTCLObjectProcessor {
        private:
            CStatusDefinitions::RingStatistics* m_pObject;
            
        public:    
            RingStatistics(
                CTCLInterpreter& interp, const char* command,
                CStatusDefinitions::RingStatistics* pApi)
            );
            virtual ~RingStatistics();
        public:
            int operator()(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
        private:
            void startMessage(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void addProducer(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void addConsumer(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            void endMessage(
                CTCLInterpreter& interp, std::vector<CTCLObject>& objv
            );
            //  Utilities:
        private:
            std::vector<std::string> stringVectorFromList(
                CTCLInterpreter& interp, CTCLObject& obj
            );
            uint64_t uint64FromObject(
                CTCLInterpreter& interp, CTCLObject& objv,
                const char* doing = "Getting a uint64_t from a command argument"
            );
    };
};

#endif