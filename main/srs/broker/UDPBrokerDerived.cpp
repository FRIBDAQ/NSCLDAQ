// /*
//     This software is Copyright by the Board of Trustees of Michigan
//     State University (c) Copyright 2022.

//     You may use this software under the terms of the GNU public license
//     (GPL).  The terms of this license are described at:

//      http://www.gnu.org/licenses/gpl.txt

//      Authors:
//              Ron Fox
//              Giordano Cerriza
//              Genie Jhang
//              Simon Giraud
// 	     FRIB
// 	     Michigan State University
// 	     East Lansing, MI 48824-1321
// */

#include "UDPBrokerDerived.h"
#include "pcapHeader.h"
#include "datagramHeader.h"
#include "BitMath.h"

#include <chrono>
#include <thread>
#include <ctime> //for state change RI
#include <iostream>
#include <stdexcept>
#include <memory>
// #include <CUDPClient.h>
#include <CDataSinkFactory.h>
#include <CDataSink.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <DataFormat.h>
#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <Exception.h>
#include <bitset>
#include <SRSMaps.h>
#include <SRSSorter.h>

#define NULL_TIMESTAMP UINT64_C(0xffffffffffffffff)
#define BARRIER_NOTBARRIER   0
#define BARRIER_START   1
#define BARRIER_END   2
#define BARRIER_SYNCH   3

#define EXTRA_DATA_ERR   9
#define EXTRA_DATA_MARKER   0
#define EXTRA_DATA_TRIG_MARKER   1
#define EXTRA_DATA_HIT   2
#define EXTRA_DATA_HIT_MARKER   3
#define EXTRA_DATA_PASS   4


UDPBrokerDerived::UDPBrokerDerived() {
}

UDPBrokerDerived::~UDPBrokerDerived() {
}


void UDPBrokerDerived::initialize(short port, std::string mapStr) {
  UDPBrokerBase::initialize(port); // Call base class initialization
  // Add derived class specificities
  // Set the detector channel mapping
  m_channelsMap = std::make_unique<SRSMaps>();
  m_channelsMap->setChannelsMap(mapStr);
  // Initialize the sorter
  m_sorter = std::make_unique<SRSSorter>();
  m_stopMainLoop = false;
  m_pauseMainLoop = true;
}

// sid is the FEC number + 10
void UDPBrokerDerived::addSink(std::string sinkType, int sid) {
    std::unique_ptr<CDataSink> dataSink;
    dataSink.reset(CDataSinkFactory().makeSink(sinkType)); 
    if (!dataSink) {
        throw std::runtime_error("UDPBrokerDerived::addSink - Failed to create data sink");
    }
    m_dataSinks[sid] = std::move(dataSink);
}

// Return non zero timestamp only when data, not for markers. 
// The markers are used to compute the data timestamp.
int UDPBrokerDerived::getExtraData(uint8_t fecId, uint8_t* data)
{
    int result = EXTRA_DATA_MARKER; 
    auto Data1Offset = 0;
    auto Data2Offset = Data1Offset + Data1Size;
    uint32_t data1 = htonl(*reinterpret_cast<uint32_t*>(&data[Data1Offset]));
    uint16_t data2 = htons(*reinterpret_cast<uint16_t*>(&data[Data2Offset]));
    uint8_t vmmid = MaxVMMs ;
    uint8_t vmmidHitMarker = MaxVMMs;
    uint16_t idx = MaxFECs * MaxVMMs;
    uint16_t idxHitMarker = MaxFECs * MaxVMMs;
    uint8_t triggerOffset = 0;
    uint16_t bcid = 0;
    // the source id given as fecId is fecId+10, so:
    fecId -= 10;

    int dataflag = (data2 >> 15) & 0x1;
    if(dataflag){
        // Enter here if data

        vmmid = (data1 >> 22) & 0x1F;
        //if (m_datagramCounter < 10)printf("dataflag True fecId %d m_triggerMode %d, m_dataEnded %d: m_markerCounter %d \n",
        // fecId, m_triggerMode, m_dataEnded, m_markerCounter);

        // if ext. trigger get marker at vmmid +16
        // if (m_triggerMode == 1){
        //     idx = (fecId - 1) * MaxVMMs + vmmid + 16;
        // }
        // else {
        //     idx = (fecId - 1) * MaxVMMs + vmmid;
        // }
        idx = (fecId - 1) * MaxVMMs + vmmid;
        triggerOffset = (data1 >> 27) & 0x1F;
        bcid = BitMath::gray2bin32(data1 & 0xFFF);

        // Note: chno takes only 6 bits (0-63) 
        uint8_t chno = (data2 >> 8) & 0x3f;
        //!!!!!!!! for debug !!!!!!!!!
        //if (chno > 10 ) chno = chno -10;
        //!!!!!!!! for debug !!!!!!!!!

        // Set mapped channel
        extraData.chnoMapped = m_channelsMap->getMappedChannel(fecId, vmmid, chno);


        // Every 65536 clock cycle a new marker is issued, every 4096 clock cycle triggerOffset is increased by 1, bcid max is 4095
        uint64_t fineTS = markerSRS[idx].fecTimestamp + triggerOffset*4096 + bcid;

        // With ext. trigger we don't need the triggerOffset and bcid
        if (m_triggerMode == 1){
            uint8_t idxTrigg = idx + 16;
            fineTS = markerSRS[idxTrigg].fecTimestamp; 
            if (fineTS == 0)
            {
                m_firstDataCounter++;
            }
            // Associate hit marker to current hit
            extraData.hitMarker = markerSRS[idx].hitMarker;

            //calc. drift time
            //uint8_t tdc = data2 & 0xff;
            // double timeFec = static_cast<double>(extraData.hitContinuousMarker)*22.5 + 
            //               static_cast<double>(triggerOffset)*4096.0*22.5;

            // double timeFec = static_cast<double>(triggerOffset)*4096.0*22.5;

            // double timeChip =
            //     static_cast<double>(bcid)*22.5 +
            //     (1.5 * 22.5 - static_cast<double>(tdc)*60.0 / 255.0);

            // double driftTime = (timeFec + timeChip - extraData.hitMarker*22.5);
            //double driftTime = (timeChip - extraData.hitMarker*22.5);

            // if (m_datagramCounter < 10)printf("In hit fecId: %d vmmid: %d idx : %d hitMarker: %llu timestamp: %llu driftTime %lf \n",
            //  fecId, vmmid, idx, extraData.hitMarker, fineTS, driftTime );
        }

        //uint8_t tdc = data2 & 0xff;
        //printf("SRS Data: fecId: %d, vmm: %d, channel: %d, channelMapped: %d, fecTimestamp: %llu, fineTS: %llu, bcid: %d, tdc: %d\n", fecId, vmmid, chno, extraData.chnoMapped, markerSRS[idx].fecTimestamp, fineTS, bcid, tdc);
        //if data come before the first markers set TS to 0 and these data will be skipped.
        if (m_triggerMode != 1 && markerSRS[idx].fecTimestamp == 0)
        {
            m_firstDataCounter++;
            fineTS = 0;
        }
        // Set the hit timestamp
        extraData.hitTimestamp = fineTS;
        return EXTRA_DATA_HIT;
    } else {
        // Enter here if marker

        vmmid = (data2 >> 10) & 0x1F;
        idx = (fecId - 1) * MaxVMMs + vmmid;
        extraData.chnoMapped = 0;
        uint64_t lower_10bit_data2 = data2 & 0x03FF;
        uint64_t upper_32bit_data1 = data1;
        uint64_t lower_10bit_data1 = data1 & 0x03FF;
        uint64_t timestamp_42bit = (upper_32bit_data1 << 10) + lower_10bit_data2;
        uint64_t hitMarker_20bit = (lower_10bit_data1 << 10) + lower_10bit_data2;
        uint32_t hitMarker = static_cast<uint32_t>(hitMarker_20bit);
        //if (m_datagramCounter < 10)printf("SRS Marker fecId %d vmmid %d, idx %d: timestamp lower 10bit %lu, timestamp upper 32 bit %lu, 42 bit timestamp %lu \n", fecId, vmmid, idx, timestamp_lower_10bit, timestamp_upper_32bit, timestamp_42bit);
        //if (m_datagramCounter < 10)printf("SRS hit Marker m_startedMarker[idx] %d fecId %d vmmid %d, idx %d: d1 lower 10bit %x, d2 lower 10 bit %x, 20 bit marker %x hitMarker %llu \n", m_startedMarker[idx], fecId, vmmid, idx, lower_10bit_data1, lower_10bit_data2, hitMarker_20bit, hitMarker);
        //if (m_datagramCounter < 10)printf("SRS trigg Marker m_startedMarker[idx] %d fecId %d vmmid %d, idx %d: lower 10bit %x, upper 32 bit %x, 42 bit timestamp %llu \n", m_startedMarker[idx], fecId, vmmid, idx, lower_10bit_data2, upper_32bit_data1, timestamp_42bit);


        //Check if hits end  
        if (m_triggerMode == 1 && timestamp_42bit == 0x3FFFFFFFFFF) {
            //if (m_datagramCounter < 10)printf("SRS Marker MAX fecId %d vmmid %d, idx %d: lower 10bit %x, upper 32 bit %x, 42 bit timestamp %llu \n", fecId, vmmid, idx, lower_10bit_data2, upper_32bit_data1, timestamp_42bit);
            // Get hit markers ready to be set with new values
            if (!m_dataEnded) for (size_t im = 0; im < MaxFECs * MaxVMMs; im++) markerSRS[im].hitMarker = 0;
            m_dataEnded = true;
            return EXTRA_DATA_PASS;
        }
        // if(markerSRS[idx].fecTimestamp > timestamp_42bit) {
        //     if (markerSRS[idx].fecTimestamp < 0x1FFFFFFF + timestamp_42bit) {
        //         m_markerErrCounter++;
        //         //printf( "ParserTimestampSeqErrors:  ts %lu, marker ts %lu \n", timestamp_42bit, markerSRS[idx].fecTimestamp);
        //     }
        // }
        if (m_triggerMode == 1 && m_startedMarker[idx]) {
            if (hitMarker != 0 && vmmid <= 15) {
                // requires m_startedMarker[idx] true so first trig markers are missed.
                markerSRS[idx].hitMarker = hitMarker;
                result = EXTRA_DATA_MARKER;
                //if (m_datagramCounter < 10)printf("SRS  set hit marker, idx %d: hitMarker %llu \n",  idx, hitMarker);
            }
            else if (timestamp_42bit != 0 && vmmid > 15) {
                markerSRS[idx].fecTimestamp = timestamp_42bit;
                //Give value to extraData.hitTimestamp, in case no hit follows, an empty RI with trigger ts will be send.
                extraData.hitTimestamp = timestamp_42bit;
                //if (m_datagramCounter < 10)printf("SRS  set hit timestamp, idx %d: timestamp %llu \n",  idx, timestamp_42bit);

                result = EXTRA_DATA_TRIG_MARKER;
            }
            else if ((vmmid > 15 && timestamp_42bit == 0)) {
                // Likely not markers, e.g. it passes here for the last two 6 bytes at end of a datagram
                //printf( "ParserTimestampSeqErrors:  ts == 0 for not first marker\n");
                extraData.hitTimestamp = 0;
                m_markerErrCounter++;
                result = EXTRA_DATA_ERR;
            }
        }
        else if (m_triggerMode != 1 && m_startedMarker[idx]) {
            if (timestamp_42bit != 0 && vmmid < 15) {
                markerSRS[idx].fecTimestamp = timestamp_42bit;
                //Give value to extraData.hitTimestamp, in case no hit follows, an empty RI with trigger ts will be send.
                extraData.hitTimestamp = timestamp_42bit;
                result = EXTRA_DATA_MARKER;
            }
            else if ((vmmid < 15 && timestamp_42bit == 0)) {
                // Likely not markers, e.g. it passes here for the last two 6 bytes at end of a datagram
                //printf( "ParserTimestampSeqErrors:  ts == 0 for not first marker\n");
                extraData.hitTimestamp = 0;
                m_markerErrCounter++;
                result = EXTRA_DATA_ERR;
            }
        }
        else if (!m_startedMarker[idx])  {
            // First markers after run start
            markerSRS[idx].fecTimestamp = 0;
            extraData.hitTimestamp = 0;
            m_startedMarker[idx] = true;
            m_dataEnded = true;
            result = EXTRA_DATA_PASS;
        }
    }
    return result;
}


/**
 * mainLoop
 *    Accepts datagrams from the server object and forwards them to the
 *    sink. Data sent looks like ring items of type FIRST_USER_ITEM_CODE
 *    (or PHYSICS_EVENT?)
 *    The body header will be filled in as:
 *    - timestamp from the routing header.
 *    - sourceid as (detectorId << 8) | (subdetectorId)
 *    - barrier type 0.
 *    The ring item payload will be the complete datagram including
 *    routing header preceded by the IP address and port of the sender.
 *
 */
void UDPBrokerDerived::mainLoop() {
    uint8_t datagram[65536];
    struct sockaddr_in senderAddr;
    socklen_t senderAddrLen = sizeof(senderAddr);

    markerSRS = new VMM3Marker[MaxFECs * MaxVMMs];

    while (!m_stopMainLoop) {
        if (m_pauseMainLoop) continue;

        int bytesReceived = recvfrom(UDPBrokerBase::getSocket(), datagram, sizeof(datagram), 0,
                                     (struct sockaddr*)&senderAddr, &senderAddrLen);//m_socket
        if (bytesReceived < 0) {
            perror("UDPBrokerDerived::mainLoop - Error receiving datagram");
            continue;
        }

        // Process received datagram
        in_addr_t from = senderAddr.sin_addr.s_addr;
        short fromPort = ntohs(senderAddr.sin_port);

        uint8_t* buffer = reinterpret_cast<uint8_t*>(datagram);
        // Get fecId from srs datagram header
        psrshdr hdr = reinterpret_cast<psrshdr>(buffer);
        uint8_t sid = hdr -> fecId;
        sid = sid + 10;

        auto it = m_dataSinks.find(sid);
        // Check if data sink for this sid
        if (it != m_dataSinks.end()) {
            std::unique_ptr<CDataSink>& dataSinks = it->second;
            // Extract all hits (data1&data2) from datagram, encapsulate the hits in ring items (one hit into one RI) and put RI in sink        
            makeRingItems(from, fromPort, *dataSinks, sid, buffer, bytesReceived);
        } else {
            std::cerr << "UDPBrokerDerived::mainLoop - no data sink for source id (fecId): " << sid << std::endl;
            continue;
        }

        m_datagramCounter++;

        // std::cout<<"Counters: dT, datagram, marker, markerErr, hit, firstData: "<<elapsed_time_s<<" "<<m_datagramCounter<<" "<<m_markerCounter<<" "<<m_markerErrCounter<<" "<<m_hitCounter<<" "<<m_firstDataCounter<<std::endl;
        //std::cout<<"Counters: datagram, marker, trig. marker, markerErr, hit, firstData: "<<m_datagramCounter<<" "<<m_markerCounter<<" "<<m_trigMarkerCounter<<" "<<m_markerErrCounter<<" "<<m_hitCounter<<" "<<m_firstDataCounter<<std::endl;

        if (m_stopMainLoop) {
            break;
        }
    }
    m_stopMainLoop = false;
    delete[] markerSRS;

}


/**
 * begin
 */
void UDPBrokerDerived::begin() {
    m_stopMainLoop = false;
    m_pauseMainLoop = false;
    m_hitCounter = 0;
    m_datagramCounter = 0;
    m_markerCounter = 0;
    m_trigMarkerCounter = 0;
    m_markerErrCounter = 0;
    m_firstDataCounter = 0;
    m_startChrono = true;
    m_start = std::chrono::high_resolution_clock::now();

    std::map<int, std::unique_ptr<CDataSink>>::iterator it = m_dataSinks.begin();
    while (it != m_dataSinks.end()) {
        CRingStateChangeItem* pBegin = new CRingStateChangeItem(
        NULL_TIMESTAMP, it->first, BARRIER_START, BEGIN_RUN,  m_runNumber, 0, std::time(nullptr), " ", 1000);
        
        it->second->putItem(*pBegin);
        delete pBegin;
        ++it;
    }
}


/**
 * end
 */
void UDPBrokerDerived::end() {
    m_pauseMainLoop = true;
    std::cout<<"UDPBrokerDerived - end"<<std::endl;

    m_end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::ratio<1, 1>> elapsed_time = m_end - m_start;
    uint32_t elapsed_time_s = static_cast<uint32_t>(elapsed_time.count());
    std::cout<<"UDPBrokerDerived - end function after chrono "<<elapsed_time_s<<std::endl;



    // // Schedule delayed end signal using a separate thread
    // std::thread delayed_execution([this, elapsed_time_s]() {
    //     std::cout<<"Simon in UDPBrokerDerived - end function thread before sleep "<<std::endl;
    //     // Wait for 30 seconds
    //     std::this_thread::sleep_for(std::chrono::seconds(15));
    //     std::cout<<"Simon in UDPBrokerDerived - end function thread after sleep "<<std::endl;


    //     std::map<int, std::unique_ptr<CDataSink>>::iterator it = m_dataSinks.begin();
    //     while (it != m_dataSinks.end()) {
    //         CRingStateChangeItem* pEnd = new CRingStateChangeItem(
    //             NULL_TIMESTAMP, it->first, BARRIER_NOTBARRIER, END_RUN,
    //             m_runNumber, elapsed_time_s, std::time(nullptr), " ", 1);

    //         it->second->putItem(*pEnd);
    //         std::cout<<"Simon in UDPBrokerDerived - end function thread after send RI "<<std::endl;

    //         delete pEnd;
    //         ++it;
    //     }
    // });
    // delayed_execution.detach();

    std::map<int, std::unique_ptr<CDataSink>>::iterator it = m_dataSinks.begin();
    while (it != m_dataSinks.end()) {
        CRingStateChangeItem* pEnd = new CRingStateChangeItem(
        NULL_TIMESTAMP, it->first, BARRIER_END, END_RUN,  m_runNumber, elapsed_time_s, std::time(nullptr), " ", 1);
        it->second->putItem(*pEnd);
        delete pEnd;
        ++it;
    }

    //clear markers 
    for (size_t idx = 0; idx < MaxFECs * MaxVMMs; idx++) {
        markerSRS[idx].fecTimestamp = 0;
        markerSRS[idx].hitMarker = 0;
        m_startedMarker[idx] = false;
    }
    m_sorter->reset();
}


/**
 * stop mainLoop
 */
void UDPBrokerDerived::stop() {
    m_stopMainLoop = true;
}

/**
 * pause mainLoop
 */
void UDPBrokerDerived::pause() {
    std::cout << "UDPBrokerDerived::pause() " << std::endl;
    m_pauseMainLoop = true;

    m_pause = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::ratio<1, 1>> elapsed_time = m_pause - m_start;
    uint32_t elapsed_time_s = static_cast<uint32_t>(elapsed_time.count());
    std::cout<<"UDPBrokerDerived - pause function after chrono "<<elapsed_time_s<<std::endl;

    std::map<int, std::unique_ptr<CDataSink>>::iterator it = m_dataSinks.begin();
    while (it != m_dataSinks.end()) {
        CRingStateChangeItem* pPause = new CRingStateChangeItem(
        NULL_TIMESTAMP, it->first, BARRIER_SYNCH, PAUSE_RUN,  m_runNumber, elapsed_time_s, std::time(nullptr), " ", 1);
        it->second->putItem(*pPause);
        delete pPause;
        ++it;
    }

}

/**
 * resume mainLoop
 */
void UDPBrokerDerived::resume() {
    std::cout << "UDPBrokerDerived::resume() " << std::endl;
    m_pauseMainLoop = false;

    m_resume = std::chrono::high_resolution_clock::now();

    // Time into run provided to state change item is the same as at pause
    std::chrono::duration<double, std::ratio<1, 1>> elapsed_time = m_pause - m_start;
    uint32_t elapsed_time_s = static_cast<uint32_t>(elapsed_time.count());
    std::cout<<"UDPBrokerDerived - resume function after chrono "<<elapsed_time_s<<std::endl;

    std::map<int, std::unique_ptr<CDataSink>>::iterator it = m_dataSinks.begin();
    while (it != m_dataSinks.end()) {
        CRingStateChangeItem* pResume = new CRingStateChangeItem(
        NULL_TIMESTAMP, it->first, BARRIER_SYNCH, RESUME_RUN,  m_runNumber, elapsed_time_s, std::time(nullptr), " ", 1);
        it->second->putItem(*pResume);
        delete pResume;
        ++it;
    }

}


//...
void UDPBrokerDerived::makeRingItems(in_addr_t from, short port, CDataSink& sink, int sid, uint8_t* buffer, size_t nBytes) {

    size_t bytes = nBytes;
    int readoutIndex = 0;

    while (bytes >= HitAndMarkerSize) {
        //printf("readoutIndex: %d, bytes %d\n", readoutIndex, bytes);

        // The datagram send from the slow controler has only srsHeader and data
        auto dataOffset = SRSHeaderSize + HitAndMarkerSize * readoutIndex;

        // Add mapped chno, and hit marker to data (+ 2 bytes + 4 bytes)
        std::unique_ptr<uint8_t[]> data(new uint8_t[HitAndMarkerSize + 6]);
        memcpy(data.get(), buffer + dataOffset, HitAndMarkerSize);

        extraData.hitTimestamp = 0;
        extraData.chnoMapped = 0;
        extraData.hitMarker = 0;
        // Set hitTimestamp and chnoMapped
        int proceed = getExtraData(sid, data.get());

        // write chno mapped to data
        uint16_t chnoMapped = extraData.chnoMapped;
        memcpy(data.get() + HitAndMarkerSize, &chnoMapped, sizeof(chnoMapped));

        // write hit marker to data
        uint32_t hitMarker = extraData.hitMarker;
        memcpy(data.get() + HitAndMarkerSize + sizeof(chnoMapped), &hitMarker, sizeof(hitMarker));
        // if (m_datagramCounter < 10)printf("chnoMapped %x timestamp %x hitMarker %x \n",chnoMapped,extraData.hitTimestamp,hitMarker);
        //if (m_datagramCounter < 10)printf("proceed %d chnoMapped %lu timestamp %llu hitMarker %llu \n",
        //proceed, chnoMapped, extraData.hitTimestamp, hitMarker);

        // testReadData(data.get());

        // Only for hits
        if (proceed == EXTRA_DATA_HIT && extraData.hitTimestamp > 0){

            //keep feeding the sorter, it will manage the add to RI and put into sink when it is time... 
            m_sorter->sort(data.get(), extraData.hitTimestamp, sid, sink, nBytes);

            //if (m_startChrono){
            //  m_start = std::chrono::high_resolution_clock::now();
            //  m_startChrono = false;
            //}
            m_hitCounter += 1;
        } 
        else if (proceed == EXTRA_DATA_TRIG_MARKER && extraData.hitTimestamp > 0) { // Only for trigger markers
            m_sorter->sort(data.get(), extraData.hitTimestamp, sid, sink, 0); //set nBytes to 0 to flag trigg marker
            m_trigMarkerCounter +=1 ; 
        }
        else if (proceed == EXTRA_DATA_MARKER) {
            m_markerCounter +=1 ; 
        }
        else {
            // ?
        }
        bytes -= HitAndMarkerSize;
        readoutIndex++;
    }
}



uint16_t UDPBrokerDerived::invertByteOrder(uint16_t data) {
    uint8_t lowerHalf = data & 0x00FF;
    uint8_t upperHalf = (data & 0xFF00) >> 8;
    return (lowerHalf << 8) | upperHalf;
}


void UDPBrokerDerived::testReadData(uint8_t* data){
    auto Data2Offset = Data1Size;
    auto Data3Offset = Data2Offset + Data2Size;
    uint16_t data2 = htons(*reinterpret_cast<uint16_t*>(&data[Data2Offset]));
    uint16_t data3 = htons(*reinterpret_cast<uint16_t*>(&data[Data3Offset]));
    // uint16_t data3 = ntohs(*reinterpret_cast<uint16_t*>(&data[Data3Offset]));
    int dataflag = (data2 >> 15) & 0x1;

    // printf("testReadData - dataflag: %d \n",dataflag);

    if (dataflag) {
        uint8_t chno = (data2 >> 8) & 0x3f;

        uint16_t chnoNew = invertByteOrder(data3);

        std::cout<<"bit rep of data2: "<<std::bitset<16>( data2 )<<std::endl;
        std::cout<<"bit rep of data3: "<<std::bitset<16>( data3 )<< " " <<(int)data3<<" "<<std::bitset<16>( chnoNew )<< " " <<(int)(chnoNew)<<std::endl;
        // std::cout<<"bit rep data2Copy : "<<std::bitset<16>(data2Copy)<<std::endl;
        // std::cout<<"bit rep data2Copy : "<<std::bitset<16>(data2Copy | ( (data2 >> 8 & 0xC0 | chnoCte) << 8) )<<std::endl;

        printf("testReadData - chno: %d \n",chno);
    }
    return;
}


void UDPBrokerDerived::setTriggerMode(int triggerIn, int invTrigger){
    if (invTrigger==1){
        m_triggerMode = 2;
    }
    else if (triggerIn==1){
        m_triggerMode = 1;
    }
    else {
        m_triggerMode = 0;
    }
    // std::cout<<"setTriggerMode - "<<triggerIn<<" "<<invTrigger<<std::endl;
}

void UDPBrokerDerived::setClockMode(int extClock){
    m_extClock = extClock;
    // std::cout<<"setClockMode - "<<extClock<<std::endl;
}

void UDPBrokerDerived::setClockPeriod(double period){
    m_clockPeriod = period;
    // std::cout<<"setClockPeriod - "<<period<<std::endl;
}

void UDPBrokerDerived::setRunNumber(uint32_t runNb){
    m_runNumber = runNb;
    // std::cout<<"setRunNumber - "<<runNb<<std::endl;
}

void UDPBrokerDerived::setSourceId(uint32_t sourceId){
    m_sourceId = sourceId;
    // std::cout<<"setSourceId - "<<sourceId<<std::endl;
}
