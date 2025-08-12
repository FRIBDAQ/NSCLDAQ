#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


#  This application requires epics:

if EPICS
LIBDIR=@prefix@/TclLibs

bin_PROGRAMS        = controlpush

BUILT_SOURCES = cmdline.h cmdline.c

APPITEMS=  CApplication.cpp CChannelList.cpp \
			CStrings.cpp \
			CLookupVisitor.cpp  CUnitChannel.cpp \
			CBuildChannelData.cpp \
			CChannel.cpp
controlpush_SOURCES = Main.cpp $(APPITEMS)
nodist_controlpush_SOURCES=cmdline.c cmdline.h


noinst_HEADERS=  CApplication.h CChannelList.h \
			CChannelVisitor.h CStrings.h \
			CLookupVisitor.h CChannel.h CUnitChannel.h \
			CBuildChannelData.h

controlpush_LDADD   =   @top_builddir@/base/tcpip/libTcp.la	\
			@LIBEXCEPTION_LDFLAGS@	\
			@top_builddir@/epics/epicslib/libEpicslib.la \
			@top_builddir@/base/os/libdaqshm.la	\
			@top_builddir@/base/thread/libdaqthreads.la \
			@top_builddir@/base/CopyrightTools/libLicense.la \
			$(EPICS_LDFLAGS)  $(THREADLD_FLAGS) @TCL_LDFLAGS@
controlpush_LDFLAGS= -Wl,"-rpath=$(libdir)"

COMPILATION_FLAGS            =	@LIBTCLPLUS_CFLAGS@		\
			-I@top_srcdir@/base/CopyrightTools	\
			-I@top_srcdir@/base/headers		\
			-I@top_srcdir@/base/tcpip	\
			-I@top_srcdir@/base/os	\
			-I@top_srcdir@/base/factories  \
			-I@top_srcdir@/base/thread    \
			-I@top_srcdir@/epics/epicslib \
			$(EPICS_BIN) $(EPICS_INCLUDES)  @PIXIE_CPPFLAGS@

controlpush_CPPFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS) $(COMPILATION_FLAGS)


cmdline.c: options.ggo
	$(GENGETOPT) --unamed-opts <@srcdir@/options.ggo \
		--output-dir=@builddir@

cmdline.h: cmdline.c




noinst_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp ChannelListTests.cpp  \
		LookupVisitorTest.cpp \
		TestChannel.cpp TestUnitChannel.cpp \
		$(APPITEMS)

unittests_LDADD = $(CPPUNIT_LDFLAGS) $(controlpush_LDADD)
unittests_CPPFLAGS=$(COMPILATION_FLAGS)


#
#  Not really a good testable thing unless we put up our own 
#  ioc with known channels/properties.

# TESTS=./unittests

clean-local:
	rm -f cmdline.c rmcline.h

endif
EXTRA_DIST = options.ggo  Channels.dat  controlpush.xml