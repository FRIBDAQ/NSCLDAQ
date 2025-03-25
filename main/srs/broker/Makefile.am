lib_LTLIBRARIES = libUDPBrokerDerived.la

libUDPBrokerDerived_la_SOURCES = UDPBrokerDerived.cpp

libUDPBrokerDerived_la_CXXFLAGS = -I@top_srcdir@/utilities/udpBroker \
                -I@top_srcdir@/base/udp \
                -I@top_srcdir@/daq/format \
                -I@top_srcdir@/daq/IO \
                -I@top_srcdir@/srs/maps \
                -I@top_srcdir@/srs/sorter \
                @LIBTCLPLUS_CFLAGS@

libUDPBrokerDerived_la_LIBADD = @top_builddir@/utilities/udpBroker/libUDPBrokerBase.la \
                   @top_builddir@/base/udp/libUdp.la \
                   @top_builddir@/daq/IO/libdaqio.la \
                   @top_builddir@/daq/format/libdataformat.la \
                   @top_builddir@/srs/maps/libSRSMaps.la \
                   @top_builddir@/srs/sorter/libSRSSorter.la \
                   @LIBEXCEPTION_LDFLAGS@

include_HEADERS = UDPBrokerDerived.h datagramHeader.h pcapHeader.h

EXTRA_DIST = BitMath.h


