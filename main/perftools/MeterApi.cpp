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

/** @file:  MeterApi.cpp
 *  @brief:Implement the meter api.
 */
#include "MeterApi.h"
#include <CSocket.h>

#include <sstream>
#include <string.h>

/**
 * construcgor
 *     open a connection to the meter server and create the meter.
 *
 * @param host - host on which the server is running
 * @param port - port on which the server is listening for connections.
 * @param name  - name of the meter.
 * @param low -  low limit of displayed values.
 * @param high - high limit of displayed values.
 * @param log  - bool true if log.
 */
Meter::Meter(
    const char* host, int port, const char* meter,
    double low, double high, bool log
) :
    m_pSocket(nullptr), m_host(host), m_port(port), m_name(meter)
{
    m_pSocket = new CSocket();
    m_pSocket->Connect(m_host, service());
    
    std::stringstream line;
    line << "meter " << m_name << " " << low << " " << high << "  "
        << (log ? "log" : "linear") << std::endl;
    sendline(line.str().c_str());
}
/**
 * destructor - close/delte the socket.
 */
Meter::~Meter()
{
    m_pSocket->Shutdown();
    delete m_pSocket;
}
/**
 * set
 *    set the meter value.
 *
 *  @value - meter value.
 */
void
Meter::set(double value)
{
    std::stringstream line;
    line << "value " << m_name << " " << value << std::endl;
    sendline(line.str().c_str());
}

////////////////////////////////////////////////////////////////

/**
 * sendline
 *    Send a line of data and fluhs.
 *
 *    @param msg - null terminated string to send.
 */
void
Meter::sendline(const char* msg)
{
    m_pSocket->Write(msg, strlen(msg));
    m_pSocket->Flush();
}
/**
 * service
 *    Return a stringified version of the service port.
 *
 *  @return std::string
 */
std::string
Meter::service() const
{
    std::stringstream s;
    s << m_port;
    return s.str();
}