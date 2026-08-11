bin_PROGRAMS=DDASReadout ddasSort

SHAREDIR=@prefix@/share/ddasreadout

DDASReadout_SOURCES= DDASReadoutMain.cpp DDASReadoutMain.h 	\
	CMyEventSegment.cpp CMyEventSegment.h 			\
	CMyTrigger.cpp CMyTrigger.h 				\
	CMyBusy.cpp CMyBusy.h 					\
	CMyScaler.cpp CMyScaler.h 				\
	CMyEndCommand.cpp CMyEndCommand.h 			\
	CSyncCommand.cpp CSyncCommand.h 			\
	CBootCommand.cpp CBootCommand.h 			\
	CDDASStatisticsCommand.cpp CDDASStatisticsCommand.h

DDASReadout_CPPFLAGS=-I@top_builddir@/ddas 				\
	-I@top_srcdir@/sbs/readout 					\
	-I@top_srcdir@/sbs/nsclapi					\
	-I@top_srcdir@/base/dataflow					\
	-I@top_srcdir@/base/os						\
	-I@top_srcdir@/daq/format					\
	-I@top_srcdir@/ddas/configuration				\
	-I@top_srcdir@/ddas/booter					\
	-I@top_srcdir@/ddas/exception					\
	@PIXIE_CPPFLAGS@						\
	@LIBTCLPLUS_CFLAGS@						\
	@TCL_FLAGS@         						\
	-fPIC -DSORTING 							

DDASReadout_LDFLAGS= \
	@top_builddir@/sbs/readout/libSBSProductionReadout.la   \
	@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la		\
	@top_builddir@/base/thread/libdaqthreads.la		\
	@top_builddir@/base/os/libdaqshm.la			\
	@top_builddir@/ddas/configuration/libConfiguration.la	\
	@top_builddir@/ddas/booter/libSystemBooter.la		\
	@top_builddir@/ddas/exception/libDDASException.la 	\
	@PIXIE_LDFLAGS@						\
	@LIBTCLPLUS_LDFLAGS@ 					\
	@TCL_LDFLAGS@						\
	@LIBEXCEPTION_LDFLAGS@

ddasSort_SOURCES=ddasSort.cpp 					\
	ddasSortOptions.c ddasSortOptions.h 			\
	DDASSorter.cpp DDASSorter.h 				\
	RawChannel.h RawChannel.cpp 				\
	ZeroCopyHit.h ZeroCopyHit.cpp 				\
	BufferArena.h BufferArena.cpp 				\
	ReferenceCountedBuffer.h ReferenceCountedBuffer.cpp 	\
	HitManager.h HitManager.cpp

ddasSort_CPPFLAGS=-I@top_srcdir@/daq/format 			\
	-I@top_srcdir@/base/dataflow 				\
	@LIBTCLPLUS_CFLAGS@ 					\
	@PIXIE_CPPFLAGS@

ddasSort_LDFLAGS=@top_builddir@/daq/format/libdataformat.la 	\
	@top_builddir@/base/dataflow/libDataFlow.la 		\
	@LIBEXCEPTION_LDFLAGS@

BUILT_SOURCES=ddasSortOptions.c ddasSortOptions.h

ddasSortOptions.c: ddasSortOptions.h

ddasSortOptions.h:  ddasSortOptions.ggo
	$(GENGETOPT) <@srcdir@/ddasSortOptions.ggo --file=ddasSortOptions --output-dir=@builddir@

#
# Unit tests
#

check_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp Asserts.h 				\
	hitmgrtests.cpp refcounttests.cpp arenatests.cpp rawchtests.cpp \
	zcopyhittests.cpp sortertests.cpp testcommon.cpp testcommon.h 	\
	DDASSorter.cpp DDASSorter.h 					\
	HitManager.cpp HitManager.h 					\
	ZeroCopyHit.h ZeroCopyHit.cpp 					\
	RawChannel.h RawChannel.cpp 					\
	ReferenceCountedBuffer.h ReferenceCountedBuffer.cpp 		\
	BufferArena.h BufferArena.cpp 

unittests_CXXFLAGS=$(ddasSort_CPPFLAGS) @CPPUNIT_CFLAGS@ -I@top_srcdir@/base/testutils
unittests_LDFLAGS=$(ddasSort_LDFLAGS) @CPPUNIT_LDFLAGS@
unittests_LDADD=@CPPUNIT_LIBS@

TESTS=./unittests

#
# Install pixieserver in @prefix@/share/readout
# Install the crate_1 dir in @prefix@/share/readout/crate_1
# Install ddasReadout driver in @bindir@
#

install-exec-hook:
	$(mkinstalldirs) $(SHAREDIR)
	$(mkinstalldirs) $(SHAREDIR)/crate_1
	$(INSTALL_DATA) pixieserver.tcl $(SHAREDIR)
	$(INSTALL_DATA) @srcdir@/crate_1/* $(SHAREDIR)/crate_1
	$(mkinstalldirs) $(bindir)
	@INSTALL_SCRIPT@ @srcdir@/ddasReadout.tcl @bindir@/ddasReadout

clean-local:
	rm -f @builddir@/ddasSortOptions.{h,c}

EXTRA_DIST=crate_1 ddasSortOptions.ggo ddasReadout.tcl readout.xml sorter.xml readoutdriver.xml readout.dox pixieserver.tcl.in
