/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  MeterApi.h
 *  @brief: API to meter server.
 */
#ifndef METERAPI_H
#define METERAPI_H
#include <string>
class CSocket;

/**
 *  Instance of a meter.
*/
class Meter
{
private:
    CSocket*    m_pSocket;
    std::string m_host;
    int         m_port;
    std::string m_name;
    
public:
    Meter(
        const char* host, int port, const char* meter,
        double low, double high, bool log = false
    );
    virtual ~Meter();
    void set(double value);
    
private:
    void sendline(const char* msg);
    std::string service() const;
};


#endif