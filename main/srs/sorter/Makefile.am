lib_LTLIBRARIES = libSRSSorter.la

libSRSSorter_la_SOURCES = SRSSorter.cpp


libSRSSorter_la_CXXFLAGS = -I@top_srcdir@/daq/format \
                -I@top_srcdir@/daq/IO \
                @LIBTCLPLUS_CFLAGS@

libSRSSorter_la_LIBADD = @top_builddir@/daq/IO/libdaqio.la \
                   @top_builddir@/daq/format/libdataformat.la \
                   @LIBEXCEPTION_LDFLAGS@

include_HEADERS = SRSSorter.h