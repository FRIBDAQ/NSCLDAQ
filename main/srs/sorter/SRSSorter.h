#ifndef SRSSORTER_H
#define SRSSORTER_H

#include <string>
#include <CDataSink.h>
#include <CRingItem.h>


class SRSSorter {
public:
    SRSSorter();
    SRSSorter(uint16_t maxHits, int dtHits);
    ~SRSSorter ();

    void sort(uint8_t* data, const uint64_t hitTimeStamp, int sid, CDataSink& sink, size_t nBytes);
    void reset();

private:

    void newRingItem(const uint64_t hitTimeStamp, int fecId, CDataSink& sink);
    void appendRingItem(int fecId, uint8_t* data);
    void deleteRingItem(int fecId);

    static const int HitAndMarkerSizeExtended{12};
    static const int MaxFECs{16};
    static const size_t m_packetSize{17936}; //8968//26904 carefull with that limit, it should be higher than maxHits bytes

    struct event {
        CRingItem* pRingItem{nullptr};
        uint64_t timestamp{0};
        uint16_t nHits{0};
        bool discard{false};

        void reset(){
            if (pRingItem != nullptr){
                delete pRingItem;
            }
            pRingItem = nullptr;
            timestamp = 0;
            nHits = 0;
            discard = false;
        }
    };
    event m_event[MaxFECs];

    // m_maxHits: cutoff on the number of hits per fec
    // m_dtHits: timestamp window for srs hits (ts in [0, m_dtHits]), should be a positive value
    uint16_t m_maxHits;
    int m_dtHits;


};

#endif
