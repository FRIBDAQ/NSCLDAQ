/**
 * This is a test program that does not get installed nor run at test time.
 * It is used to explor and trace the source of a problem that was observed when
 * dumping items with non-zero time divisors.  It appeared like time offsets were dumped
 * as if the divisor was 1 no matter what it actually was.
 * What we do is construct a v11 unified format and hand it an item crafted with a divisor that is
 * not one...and ask it to be formatted.
 * 
 * we then do the same thing for a V12 item... note that v10, I don't think had divisors.
 */
#include "CUnifiedFormatter.h"
//#include <v11/DataFormat.h>
#include <v12/DataFormat.h>
#include <string.h>         // for titles.
#include <time.h>
#include <iostream>


static void testV11StateChange() {
    ufmt::v12::StateChangeItem item;
    item.s_header.s_type = ::ufmt::v12::BEGIN_RUN;
    item.s_header.s_size = 
        sizeof(::ufmt::v12::RingItemHeader) + 
        sizeof(uint32_t) +                     // No body header.
        sizeof(::ufmt::v12::StateChangeItemBody);
    item.s_body.u_noBodyHeader.s_mbz = 0;             // v11 this is zero not sizeof(uint32_t).
    auto& body = (item.s_body.u_noBodyHeader.s_body); // Fill in the body.

    body.s_runNumber = 1234;
    body.s_timeOffset = 120;
    body.s_Timestamp = time(nullptr);
    body.s_offsetDivisor = 2;                   // so 60 seconds into the run.
    strcpy(body.s_title, "This is a title");

    CUnifiedFormatter  fmt(12, "bodies");

    std::cerr<< fmt(&item) << std::endl;
}


void testV12StateChange() {}
int main(int argc, char** argv) {
    testV11StateChange();
    testV12StateChange();
}
