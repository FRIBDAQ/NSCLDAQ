#-------------- The library libdataformat

lib_LTLIBRARIES	=	libfilter.la

libfilter_la_SOURCES = CFilterMain.cpp \
                       CBaseMediator.cpp \
                       CMediator.cpp \
                       CFakeMediator.cpp \
                       CInfiniteMediator.cpp \
                       COneShotMediator.cpp \
                       COneShotHandler.cpp \
                       CCompositeFilter.cpp \
                       COneShotException.cpp \
                       CMediatorException.cpp \
                       CAbnormalEndRunFilterHandler.cpp \
			CRunStatistics.cpp  \
			 SatisfyTclPlus.cpp  \
			CFilterTestSource.cpp \
			 CFilterTestSource.h \
			CFilterTestSink.cpp CFilterTestSink.h


nodist_libfilter_la_SOURCES=filterargs.c filterargs.h

#                       COneShotMediator.cpp 

BUILT_SOURCES = filterargs.c filterargs.h


include_HEADERS	=  CFilterMain.h \
		CBaseMediator.h \
                   CMediator.h \
		 CFakeMediator.h \
                   CInfiniteMediator.h \
		 COneShotMediator.h \
                   COneShotHandler.h \
                   CFilter.h \
                   CCompositeFilter.h  \
                   CTransparentFilter.h \
                   CNullFilter.h \
                   COneShotException.h \
                   CMediatorException.h \
                   CFatalException.h \
                   CAbnormalEndRunFilterHandler.h \
		 CRunStatistics.h


#                   COneShotMediator.h 

libfilter_la_CPPFLAGS	= -I@top_srcdir@/utilities/common \
		-I@top_srcdir@/daq/IO \
		@LIBTCLPLUS_CFLAGS@	\
		-I@top_srcdir@/base/uri		\
		-I@top_srcdir@/daq/format \
		-I@top_srcdir@/base/os \
		-I@top_srcdir@/base/headers @PIXIE_CPPFLAGS@

libfilter_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt \
				$(THREADLD_FLAGS)

libfilter_la_LIBADD	= @top_builddir@/utilities/common/libUtilCommon.la \
			@top_builddir@/daq/IO/libdaqio.la \
			@top_builddir@/daq/format/libdataformat.la \
			@top_builddir@/base/dataflow/libDataFlow.la \
			@top_builddir@/base/uri/liburl.la		\
			@top_builddir@/base/os/libdaqshm.la		\
			@LIBEXCEPTION_LDFLAGS@


libfilter_la_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)



filterargs.c: filterargs.h

filterargs.h: filterargs.ggo 
	$(GENGETOPT) < @srcdir@/filterargs.ggo \
		--output-dir=@builddir@ --file=filterargs --unamed-opts="OPTIONS"



#------------------- Tests:

noinst_PROGRAMS = unittests testapp

unittests_SOURCES	= TestRunner.cpp  \
						infinitemediatortests.cpp \
						filtermaintests.cpp  \
						compositefiltertests.cpp \
						transparentfiltertests.cpp \
						oneshothandlertests.cpp \
						abnormalendrunfiltertests.cpp  \
						testmultiple.cpp \
						CTestFilter.h \
						CTestFilter.cpp


unittests_LDADD		= \
			@builddir@/libfilter.la	\
			@top_builddir@/daq/IO/libdaqio.la \
			@top_builddir@/base/dataflow/libDataFlow.la 	\
			@top_builddir@/base/uri/liburl.la 	\
			@top_builddir@/daq/format/libdataformat.la	\
			$(CPPUNIT_LDFLAGS) 		\
			@LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS=  -I@top_srcdir@/utilities/filter \
		 -I@top_srcdir@/daq/IO \
    @LIBTCLPLUS_CFLAGS@	\
		-I@top_srcdir@/base/uri		\
		-I@top_srcdir@/base/headers		\
    -I@top_srcdir@/daq/format \
    -I@top_srcdir@/base/dataflow @PIXIE_CPPFLAGS@
unittests_CXXFLAGS = $(AM_CXXFLAGS)

unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


testapp_SOURCES = TestApp.cpp
testapp_CPPFLAGS= -I@top_srcdir@/utilities \
    @LIBTCLPLUS_CFLAGS@	\
		-I@top_srcdir@/base/uri		\
    -I@top_srcdir@/daq/format \
    -I@top_srcdir@/base/dataflow @PIXIE_CPPFLAGS@

testapp_LDADD = -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
			@top_builddir@/base/dataflow/libDataFlow.la 	\
			@top_builddir@/base/uri/liburl.la 	\
			@top_builddir@/daq/format/libdataformat.la	\
			@builddir@/libfilter.la	\
			@LIBEXCEPTION_LDFLAGS@
testapp_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests

#-------------------- Filter Kit

install-data-local:
	$(mkinstalldirs) @prefix@/filterkit
	$(INSTALL_DATA) FilterIncludes \
		@srcdir@/CTemplateFilter.cpp \
    @srcdir@/SkeletonMain.cpp @prefix@/filterkit
	$(INSTALL_DATA) Makefile-template @prefix@/filterkit/Makefile

EXTRA_DIST = filterargs.ggo SkeletonMain.cpp CTemplateFilter.cpp \
             Makefile-template.in FilterIncludes.in config.h filter.xml
