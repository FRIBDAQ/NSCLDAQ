#-------------- The library libdataformat

lib_LTLIBRARIES	=	libdaqformatio.la

libdaqformatio_la_SOURCES = BufferIOV8.cpp \
                            RingIOV10.cpp \
				RingIOV11.cpp
include_HEADERS	= BufferIOV8.h \
                  RingIOV10.h \
		RingIOV11.h

libdaqformatio_la_CPPFLAGS	=  \
                -I@top_srcdir@/daq/IO \
                -I@top_srcdir@/utilities/format \
                -I@top_srcdir@/utilities/Buffer \
		-I@top_srcdir@/base/headers \
    @LIBTCLPLUS_CFLAGS@	

libdaqformatio_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt \
				$(THREADLD_FLAGS)

libdaqformatio_la_LIBADD	= \
			@top_builddir@/daq/IO/libdaqio.la \
                        @top_builddir@/utilities/Buffer/libbuffer.la \
			@top_builddir@/utilities/format/V8/libdataformatv8.la \
			@top_builddir@/utilities/format/V10/libdataformatv10.la \
			@top_builddir@/utilities/format/V11/libdataformatv11.la \
			@top_builddir@/daq/format/libdataformat.la	\
			@LIBEXCEPTION_LDFLAGS@


libdaqformatio_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)


------------------- Tests:

check_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp  \
                            daq8test.cpp \
										daq10test.cpp \
										daq11test.cpp

unittests_LDADD		= \
			@builddir@/libdaqformatio.la \
			@top_builddir@/daq/IO/libdaqio.la \
                        @top_builddir@/utilities/Buffer/libbuffer.la \
			@top_builddir@/utilities/format/V8/libdataformatv8.la \
			@top_builddir@/utilities/format/V10/libdataformatv10.la \
			@top_builddir@/utilities/format/V11/libdataformatv11.la \
			@top_builddir@/daq/format/libdataformat.la		\
			$(CPPUNIT_LDFLAGS) 		\
			@LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS=  \
		 -I@top_srcdir@/utilities/FormattedIO \
		 -I@top_srcdir@/daq/IO \
                 -I@top_srcdir@/utilities/Buffer \
			-I@top_srcdir@/utilities/format \
		   -I@top_srcdir@/base/testutils \
 			@LIBTCLPLUS_CFLAGS@
unittests_CXXFLAGS = $(AM_CXXFLAGS)

unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"

TESTS=./unittests


