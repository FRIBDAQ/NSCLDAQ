bin_PROGRAMS=Unglom timecheck reglom


common_cxxflags=-I@top_srcdir@/daq/eventbuilder -I@top_srcdir@/daq/format -I@top_srcdir@/daq/IO @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@
common_ldflags=@top_builddir@/daq/eventbuilder/libFragmentIndex.la @top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/daq/IO/libdaqio.la @top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/uri/liburl.la @LIBEXCEPTION_LDFLAGS@ -Wl,"-rpath=@libdir@"

Unglom_SOURCES=Unglom.cpp CUnglom.cpp CUnglom.h CRingItemDecoder.h Main.cpp
Unglom_CPPFLAGS=$(common_cxxflags)
Unglom_LDFLAGS=$(common_ldflags)


timecheck_SOURCES=Timecheck.cpp CTimeChecker.cpp CTimeChecker.h CRingItemDecoder.h Main.cpp
timecheck_CPPFLAGS=$(common_cxxflags)
timecheck_LDFLAGS=$(common_ldflags)


reglom_SOURCES=reglomMain.cpp CMerge.cpp CMerge.h reglomopts.c reglomopts.h
reglom_CPPFLAGS=$(common_cxxflags) -I@top_srcdir@/base/uri
reglom_LDFLAGS=$(common_ldflags)


BUILT_SOURCES=reglomopts.c 
EXTRA_DIST=reglomopts.ggo offlinereglom.tcl reglom_ref.xml


reglomopts.c: reglomopts.h

reglomopts.h: reglomopts.ggo
	@GENGETOPT@ -i @srcdir@/reglomopts.ggo -F reglomopts -u

install-exec-local:
	$(INSTALL_PROGRAM) @srcdir@/offlinereglom.tcl @bindir@/offlinereglom
