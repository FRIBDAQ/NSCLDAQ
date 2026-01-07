#-------------- The library libdataformat

lib_LTLIBRARIES	=	libdataformat.la libTclRingBuffer.la
COMPILATION_FLAGS	= -I@top_srcdir@/base/dataflow	\
		@LIBTCLPLUS_CFLAGS@	\
		-I@top_srcdir@/base/headers	\
		-I@top_srcdir@/base/uri		\
		-I@top_srcdir@/base/os @PIXIE_CPPFLAGS@

libdataformat_la_SOURCES = CRingSelectionPredicate.cpp	\
			   CDesiredTypesPredicate.cpp	\
			   CAllButPredicate.cpp	\
			CRingItem.cpp \
			CRingStateChangeItem.cpp	\
			CRingScalerItem.cpp		\
			CRingTextItem.cpp		\
			CRingPhysicsEventCountItem.cpp \
			CRingFragmentItem.cpp       \
			CPhysicsEventItem.cpp       \
                        CDataFormatItem.cpp          \
                        CGlomParameters.cpp              \
                        CUnknownFragment.cpp            \
			CRingItemFactory.cpp		\
			ringitem.c			\
      RingItemComparisons.cpp \
      CAbnormalEndItem.cpp CBufferedRingItemConsumer.cpp \
	CRingBufferChunkAccess.cpp

libdataformat_la_CPPFLAGS=$(COMPILATION_FLAGS)

include_HEADERS	= CRingSelectionPredicate.h	\
			  CDesiredTypesPredicate.h	\
			  CAllButPredicate.h	\
			 CRingItem.h \
			CRingStateChangeItem.h	\
			CRingScalerItem.h	\
			CRingTextItem.h	\
			CRingPhysicsEventCountItem.h	\
			CRingFragmentItem.h           \
			CPhysicsEventItem.h	\
			CRingItemFactory.h	\
                        CDataFormatItem.h        \
                        CGlomParameters.h       \
                        CUnknownFragment.h      \
			DataFormat.h	\
      RingItemComparisons.h \
      CAbnormalEndItem.h CBufferedRingItemConsumer.h \
	CRingBufferChunkAccess.h




libdataformat_la_LDFLAGS	= -version-info $(SOVERSION)	\
				-Wl,"-rpath-link=$(libdir)" -lrt \
				$(THREADLD_FLAGS)

libdataformat_la_LIBADD	= @top_builddir@/base/dataflow/libDataFlow.la \
			@top_builddir@/base/uri/liburl.la		\
			@top_builddir@/base/os/libdaqshm.la		\
			@LIBEXCEPTION_LDFLAGS@


libdataformat_la_CPPFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS) \
	$(COMPILATION_FLAGS)


libTclRingBuffer_la_DEPENDENCIES=@builddir@/libdataformat.la
libTclRingBuffer_la_SOURCES=CTclRingCommand.cpp
libTclRingBuffer_la_LIBADD=	@builddir@/libdataformat.la     \
				@top_builddir@/base/dataflow/libDataFlow.la    \
				@top_builddir@/base/tcpip/libTcp.la		\
				@top_builddir@/base/os/libdaqshm.la 		\
				@LIBTCLPLUS_LDFLAGS@  @TCL_LDFLAGS@ @THREADLD_FLAGS@


libTclRingBuffer_la_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@ @THREADCXX_FLAGS@ \
				-I@top_srcdir@/base/dataflow \
				-I@top_srcdir@/daq/format \
				-I@top_srcdir@/base/os @PIXIE_CPPFLAGS@

noinst_HEADERS=CTclRingCommand.h

# Get the package installed:

INSTDIR=@prefix@/TclLibs/ringcommand


install-exec-hook:
	$(mkinstalldirs) $(INSTDIR) 
	install @libdir@/libTclRingBuffer.so $(INSTDIR)
	echo "package ifneeded TclRingBuffer 1.0 [list load [file join \$$dir libTclRingBuffer.so]]" >> $(INSTDIR)/pkgIndex.tcl

#------------------- Tests:

check_PROGRAMS = unittests ringTests

unittests_SOURCES	= TestRunner.cpp selecttest.cpp desiredtests.cpp	\
			  allbuttests.cpp ringitemtests.cpp teststate.cpp	\
			scalertests.cpp texttest.cpp physcounttests.cpp		\
			infotests.cpp phystests.cpp trigcounttests.cpp \
			scalerformattests.cpp  statechangetests.cpp dataformattests.cpp       \
			textformattests.cpp					\
                        fragmenttest.cpp glomparamtests.cpp factorytests.cpp \
                      physeventtests.cpp bufferedconstest.cpp rbchunktests.cpp zcopytests.cpp \
			formatprimitiveTests.cpp


unittests_LDADD		= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
			@top_builddir@/base/dataflow/libDataFlow.la 	\
			@top_builddir@/base/uri/liburl.la 	\
			@builddir@/libdataformat.la	\
			@top_builddir@/base/os/libdaqshm.la		\
			@LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS=$(COMPILATION_FLAGS)

unittests_LDFLAGS	= -Wl,"-rpath-link=$(libdir)"

ringTests_SOURCES = TestRunner.cpp ringTests.cpp
ringTests_CPPFLAGS = @LIBTCLPLUS_CFLAGS@ @CPPUNIT_CFLAGS@ @TCL_FLAGS@ \
			@PIXIE_CPPFLAGS@ @THREADCXX_FLAGS@ \
                       -I@top_srcdir@/daq/format -I@top_srcdir@/base/dataflow

ringTests_LDADD    =  @CPPUNIT_LDFLAGS@ @builddir@/libTclRingBuffer.la \
                       @top_builddir@/daq/format/libdataformat.la     \
                               @top_builddir@/base/dataflow/libDataFlow.la    \
			@top_builddir@/base/os/libdaqshm.la   \
                               @LIBTCLPLUS_LDFLAGS@  @TCL_LDFLAGS@ @THREADLD_FLAGS@

ringTests_DEPENDENCIES=@builddir@/libTclRingBuffer.la @builddir@/libdataformat.la


TESTS=./unittests ./ringTests

EXTRA_DIST		=	dataformat_user.xml dataformat_dev.xml
