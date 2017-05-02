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
*/

/**
* @file   CRingStatisticsMessage.cpp
* @brief  Implement formatting of ring statistics status messages.
* @author <fox@nscl.msu.edu>
* @note See the nscldaq redmine wiki at:
*       https://swdev-redmine.nscl.msu.edu/projects/sfnscldaq/wiki/NSCLDAQ_status_aggregation
*       for more information about all of this.
*/


#include "CStatusMessage.h"
#include <os.h>

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>

/**
 * CStatusDefinitions::RingStatistics::RingStatistics
 *    Construct a ring statistics object.  This sort of object accumulates
 *    the bits and pieces needed to send ring statistics messages and
 *    sends them.
 *
 *   @param socket - The ZMQ socket used to transport the messages.  This
 *                   code is agnostic about the type of the underlying
 *                   transport so long as the socket is in a state that
 *                   allows messages to be sent when endMessage is called.
 *  @param app     - Identifies the application.  This is put unmodified in the
 *                   message header part.
 */
CStatusDefinitions::RingStatistics::RingStatistics(ZmqSocket& sock, std::string app) :
    m_socket(sock),
    m_applicationName(app),
    m_msgOpen(false),
    m_producer(0)
{}

/**
 * CStatusDefinitions::RingStatistics::~RingStatistics
 *    Destroys the object. 
 */
CStatusDefinitions::RingStatistics::~RingStatistics()
{
    freeStorage();
}

/**
 * CStatusDefinitions::RingStatistics::startMessage
 *   Start a new message.
 *
 * @param ring - the name of the ring the messasge is associated with.l
 * @throw std::logic_error if there's an open message.
 */
void
CStatusDefinitions::RingStatistics::startMessage(std::string ring)
{
    if(m_msgOpen) {
        throw std::logic_error("Message is open");
    }
    
    m_ringName = ring;
    m_msgOpen  = true;
    
    // Newly opened messages don't have producers yet:
    
    freeStorage();
}
/**
 * CStatusDefinitions::RingStatistics::addProducer
 *    Add the producer to the message.  Note that the producer is optional,
 *    however you can only add one of them to a message else a logic_error
 *    is thrown.
 *
 *  @param command - Command words that make up the consumer program.
 *  @param ops     - Number of puts to the ring.
 *  @param bytes   - Number of bytes sent to the ring
 *  @param pid     - Pid of producer in the possibly remote host.
 */
void
CStatusDefinitions::RingStatistics::addProducer(
    std::vector<std::string> command, uint64_t ops, uint64_t bytes,
    pid_t pid
)
{
    // if there's a producer throw:
    
    if(m_producer) {
        throw std::logic_error("Ring statistic messages can only have one producer");
    }
    m_producer = makeClient(command, ops, bytes, 0, pid, true);
}
/**
 * CStatusDefinitions::RingStatistics::addConsumer
 *   Add a consumer to the message.
 *
 *   @param command  - Vector of command words.
 *   @param ops      - Number of get ops performed by this consumer.
 *   @param bytes    - Number of bytes gotten by this consumer.
 *   @param backlog  - Number of bytes backlogged for the consumer.
 *   @param pid      - pid of consumer in the possibly remote system.
 */
void
CStatusDefinitions::RingStatistics::addConsumer(
    std::vector<std::string> command, uint64_t ops, uint64_t bytes,
    uint64_t backlog, pid_t pid
)
{
    m_consumers.push_back(makeClient(command, ops, bytes, backlog, pid));  
}

/**
 * CStatusDefinitions::RingStatistics::endMessage()
 *   Indicates a message has been completely built up.  The message
 *   is sent to the peer through m_socket.  Once the message is sent, the
 *   storage associated with the message is released.
 */
void
CStatusDefinitions::RingStatistics::endMessage()
{
    /* Build/send the header. */
    
      // Fill in the header fields that are easy:
      
    Header hdr;
    formatHeader(
        hdr,
        MessageTypes::RING_STATISTICS, SeverityLevels::INFO,
        m_applicationName.c_str()
    );

    
       // Build a message around this struct and send it to the socket.
       // There's always at least one  more message part.
    
    zmq::message_t msgHeader(sizeof(hdr));
    std::memcpy(msgHeader.data(), &hdr, sizeof(hdr));
    
    m_socket->send(msgHeader, ZMQ_SNDMORE);          // We have at least the ring id part.
    
    RingStatIdentification* pId =
        CStatusDefinitions::makeRingid(m_ringName.c_str());
    size_t idSize = CStatusDefinitions::ringIdSize(pId);
    
        // If there is no producer and no consumers, there are no more message
        // parts:
        
    int flags = ((!m_producer) && (m_consumers.size() == 0)) ? 0 : ZMQ_SNDMORE;
    zmq::message_t msgId(idSize);
    std::memcpy(msgId.data(), pId, idSize);
    m_socket->send(msgId, flags);
    std::free(pId);                         // Done with that storage now.
   
    
    // If there's a producer send that message part.
    
    if (m_producer) {
        int flags = m_consumers.size() ? ZMQ_SNDMORE : 0; // The send flag.
        size_t prodSize = sizeClient(m_producer);
        zmq::message_t producer(prodSize);
        std::memcpy(producer.data(), m_producer, prodSize);
        m_socket->send(producer, flags);
    }
    
    // If there are consumers, send those message parts as well.
    
    for (auto p = m_consumers.begin(); p != m_consumers.end(); p++) {
        RingStatClient* pClient = *p;
        int flags = (*p == m_consumers.back()) ? 0 : ZMQ_SNDMORE;
        
        size_t s = sizeClient(pClient);
        zmq::message_t client(s);
        std::memcpy(client.data(), pClient, s);
        m_socket->send(client, flags);
    }
    
    // We did copy based data transmission so we can release the storage now
    // and we no longer have a message open:
    
    freeStorage();
    m_msgOpen = false;
}
/*----------------------------------------------------------------------------
 *  CStatusDefinitions::RingStatistics::addConsumer
 *    private methods
 */

/**
 * makeClient
 *    Create a client description.  Note that the pointer to the returned
 *    struct points to calloc's memory (not new) and must eventually be
 *    relcaimed with free().
 *
 *    @param command - Vector of command words that are the consumer program.
 *    @param ops     - Number of get/put operations performed by the client.
 *    @param bytes   - Number of bytes gotten/put by the client.
 *    @param backlog - Number of bytes of backlog (producers should put 0)
 *    @param pid     - Pid of client in possibly remote system.
 *    @param producer - true if this is a producer, false if a consumer.
 *                      defaults to false.
 *    @return CStatusDefinitions::RingStatClient*
 */
CStatusDefinitions::RingStatClient*
CStatusDefinitions::RingStatistics::makeClient(
    std::vector<std::string> command, uint64_t ops, uint64_t bytes,
    uint64_t backlog, pid_t pid,
    bool producer
)
{

    return CStatusDefinitions::makeRingClient(
        ops, bytes, backlog, pid, producer, command
    );
}
/**
 *  freeStorage
 *     Get rid of all dynamically allocated object storage.
 */
void
CStatusDefinitions::RingStatistics::freeStorage()
{
    std::free(m_producer);              // Kill off dynamic storage.
    m_producer = 0;
    
    while(!m_consumers.empty()) {
        std::free(m_consumers.front());
        m_consumers.pop_front();
    }
    
}
/**
 * sizeClient
 *   @param pClient - points to a ring client data structure.
 *   @return size_t - Number of bytes in that struct.
 *   @note remember the s_command field is dynamically sized.
 */
size_t
CStatusDefinitions::RingStatistics::sizeClient(RingStatClient* pClient)
{
    return CStatusDefinitions::ringClientSize(pClient);
}
