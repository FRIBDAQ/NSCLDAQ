#-------------- The library libdataformat

lib_LTLIBRARIES	=	libdaqio.la

libdaqio_la_SOURCES = CDataSource.cpp \
		 CFileDataSource.cpp \
		 CRingDataSource.cpp \
		 CFakeDataSource.cpp \
		 CDataSourceFactory.cpp \
		 CDataSink.cpp \
		 CFileDataSink.cpp \
		 CRingDataSink.cpp \
		 CTestSourceSink.cpp \
		 CLoggingDataSink.cpp \
		 CDataSinkFactory.cpp \
		 CDataSinkException.cpp

include_HEADERS	=  CDataSource.h \
		 CFileDataSource.h \
		 CRingDataSource.h \
		 CFakeDataSource.h \
		 CDataSourceFactory.h \
		 CDataSink.h \
		 CFileDataSink.h \
		 CRingDataSink.h \
		 CTestSourceSink.h \
		 CLoggingDataSink.h \
		 CDataSinkFactory.h \
		 CDataSinkException.h


#                   COneShotMediator.h 

libdaqio_la_CPPFLAGS	=  \
		-I@top_srcdir@/daq/format \
		-I@top_srcdir@/base/uri		\
		-I@top_srcdir@/base/os \
		-I@top_srcdir@/base/headers \
    -I@top_srcdir@/base/dataflow \
    @LIBTCLPLUS_CFLAGS@	 @PIXIE_CPPFLAGS@

libdaqio_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt \
				$(THREADLD_FLAGS)

libdaqio_la_LIBADD	= \
			@top_builddir@/daq/format/libdataformat.la \
			@top_builddir@/base/dataflow/libDataFlow.la \
			@top_builddir@/base/uri/liburl.la		\
			@top_builddir@/base/os/libdaqshm.la		\
			@LIBEXCEPTION_LDFLAGS@


libdaqio_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS) @PIXIE_CPPFLAGS@


------------------- Tests:

check_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp  \
						filedatasinktests.cpp \
						datasourcefactorytests.cpp \
						datasinkfactorytests.cpp \
						ringdatasinktests.cpp

unittests_LDADD		= \
			@builddir@/libdaqio.la \
			@top_builddir@/base/uri/liburl.la 	\
			@top_builddir@/daq/format/libdataformat.la	\
			@top_builddir@/base/dataflow/libDataFlow.la 	\
			@top_builddir@/base/os/libdaqshm.la	\
			$(CPPUNIT_LDFLAGS) 		\
			@LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS=  -I@top_srcdir@/utilities/filter \
    @LIBTCLPLUS_CFLAGS@	\
		-I@top_srcdir@/base/uri		\
		-I@top_srcdir@/base/headers		\
    -I@top_srcdir@/daq/format \
    -I@top_srcdir@/base/dataflow @PIXIE_CPPFLAGS@
unittests_CXXFLAGS = $(AM_CXXFLAGS)

unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"

TESTS=./unittests


