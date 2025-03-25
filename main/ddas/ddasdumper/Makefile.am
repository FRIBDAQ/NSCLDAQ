bin_PROGRAMS = ddasdumper.bin
lib_LTLIBRARIES = libddasrootformat.la
include_HEADERS = DDASRootEvent.h DDASRootHit.h

BUILT_SOURCES = dumperargs.h dumperargs.cpp

ddasdumper_bin_SOURCES = dumperargs.cpp dumperargs.h ddasdumper.cpp 	\
	DataSource.cpp DataSource.h					\
	FdDataSource.cpp FdDataSource.h					\
	StreamDataSource.cpp StreamDataSource.h				\
	RootFileDataSink.cpp RootFileDataSink.h
ddasdumper_bin_CPPFLAGS = -I@top_srcdir@/base/uri 			\
	-I@top_srcdir@/daq/IO						\
	-I@top_srcdir@/daq/eventbuilder					\
	@UFMT_CPPFLAGS@ @DDASFMT_CPPFLAGS@                              \
	@LIBTCLPLUS_CFLAGS@ @ROOT_CFLAGS@
ddasdumper_bin_LDADD = @builddir@/libddasrootformat.la 			\
	@top_builddir@/base/uri/liburl.la				\
	@top_builddir@/daq/IO/libdaqio.la				
ddasdumper_bin_LDFLAGS = @UFMT_LDFLAGS@ @DDASFMT_LDFLAGS@               \
	@LIBEXCEPTION_LDFLAGS@ @ROOT_LDFLAGS@


libddasrootformat_la_SOURCES = DDASRootEvent.cpp DDASRootHit.h LinkDef.h
libddasrootformat_la_CPPFLAGS = -I@top_srcdir@ @DDASFMT_CPPFLAGS@ @ROOT_CFLAGS@
libddasrootformat_la_LIBADD = @DDASFMT_LDFLAGS@
libddasrootformat_la_LDFLAGS = @ROOT_LDFLAGS@
nodist_libddasrootformat_la_SOURCES = ddasrootformatDictionary.cpp ddasrootformatDictionary.h


dumperargs.cpp: dumperargs.h
	cp @srcdir@/dumperargs.c @srcdir@/dumperargs.cpp
dumperargs.h: dumperargs.ggo
	$(GENGETOPT) < @srcdir@/dumperargs.ggo --file=dumperargs --output-dir=@srcdir@

ddasrootformatDictionary.cpp: ddasrootformatDictionary.h
ddasrootformatDictionary_rdict.pcm: ddasrootformatDictionary.h
ddasrootformatDictionary.h: DDASRootHit.h DDASRootEvent.h LinkDef.h
	LD_LIBRARY_PATH=@ROOTSYS@/lib @ROOTSYS@/bin/rootcint -f ddasrootformatDictionary.cpp -rml libddasrootformat.so -rmf libddasrootformat.rootmap -I@srcdir@ @DDASFMT_CPPFLAGS@ -c -p DDASRootHit.h DDASRootEvent.h LinkDef.h


install-exec-local:
	$(mkinstalldirs) @prefix@/lib
	$(mkinstalldirs) @prefix@/bin
	$(INSTALL_PROGRAM) ddasrootformatDictionary_rdict.pcm @prefix@/lib/.
	$(INSTALL_DATA) libddasrootformat.rootmap @prefix@/lib/.
	$(INSTALL_PROGRAM) ddasdumper @prefix@/bin/.


clean-local:
	rm -f @srcdir@/dumperargs.{h,c,cpp}
	rm -f ddasrootformatDictionary*
	rm -f libddasrootformat.rootmap


EXTRA_DIST = ddasdumper.in dumperargs.ggo ddasdumper.dox

