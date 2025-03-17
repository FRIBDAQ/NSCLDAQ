bin_PROGRAMS=DDASReadout
lib_LTLIBRARIES=libddastimestampextractor.la
SHAREDIR=@prefix@/share/ddasreadout

DDASReadout_SOURCES=Skeleton.cpp Skeleton.h 			\
	CMyEventSegment.cpp CMyEventSegment.h 			\
	CMyTrigger.cpp CMyTrigger.h 				\
	CMyBusy.cpp CMyBusy.h 					\
	CMyScaler.cpp CMyScaler.h 				\
	CMyEndCommand.cpp CMyEndCommand.h 			\
	CSyncCommand.cpp CSyncCommand.h 			\
	CBootCommand.cpp CBootCommand.h 			\
	RawChannel.cpp RawChannel.h 				\
	ReferenceCountedBuffer.cpp ReferenceCountedBuffer.h 	\
	BufferArena.cpp BufferArena.h 				\
	ZeroCopyHit.cpp ZeroCopyHit.h 				\
	ModuleReader.cpp ModuleReader.h 			\
	CHitManager.cpp CHitManager.h 				\
	CDDASStatisticsCommand.cpp CDDASStatisticsCommand.h

DDASReadout_CPPFLAGS= \
	-I@top_srcdir@ 							\
	-I@top_srcdir@/sbs/readout 					\
	-I@top_srcdir@/sbs/nsclapi					\
	-I@top_srcdir@/base/dataflow					\
	-I@top_srcdir@/base/os						\
	-I@top_srcdir@/daq/format					\
	-I@top_srcdir@/ddas/configuration				\
	-I@top_srcdir@/ddas/booter					\
	@PIXIE_CPPFLAGS@						\
	@LIBTCLPLUS_CFLAGS@						\
	@TCL_FLAGS@         						\
	-fPIC -DFIRMWARE_FILE=\"${SHAREDIR}/DDASFirmwareVersions.txt\" \
	-DSORTING

DDASReadout_LDFLAGS= \
	@top_builddir@/sbs/readout/libSBSProductionReadout.la   \
	@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la		\
	@top_builddir@/base/thread/libdaqthreads.la		\
	@top_builddir@/base/os/libdaqshm.la			\
	@top_builddir@/ddas/configuration/libConfiguration.la	\
	@top_builddir@/ddas/booter/libSystemBooter.la		\
	@PIXIE_LDFLAGS@						\
	@PLX_LDFLAGS@ 						\
	@LIBTCLPLUS_LDFLAGS@ 					\
	@TCL_LDFLAGS@

libddastimestampextractor_la_SOURCES=ddastimestamp.cpp
libddastimestampextractor_la_CPPFLAGS=$(DDASReadout_CPPFLAGS) $(CXXFLAGS)

#
#   *  Install the firmware versions file in @prefix@/share/readout
#   *  Install the crate_1 dir in @prefix@/share/readout/crate_1
#

install-exec-hook:
	$(mkinstalldirs) $(SHAREDIR)
	$(mkinstalldirs) $(SHAREDIR)/crate_1
	$(INSTALL_DATA) DDASFirmwareVersions.txt $(SHAREDIR)
	$(INSTALL_DATA) pixieserver.tcl $(SHAREDIR)
	$(INSTALL_DATA) @srcdir@/crate_1/* $(SHAREDIR)/crate_1
	$(INSTALL_DATA) @srcdir@/envset $(SHAREDIR)

EXTRA_DIST=DDASFirmwareVersions.txt.in envset README crate_1 pixieserver.tcl.in
