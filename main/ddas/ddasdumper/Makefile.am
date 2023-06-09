bin_PROGRAMS = ddasdumper.bin
lib_LTLIBRARIES = libddaschannel.la

BUILT_SOURCES = ddaschannelDictionary.h ddaschannelDictionary.cpp ddaschannelDictionary_rdict.pcm dumperargs.cpp

include_HEADERS = DDASEvent.h ddaschannel.h 

ddasdumper_bin_SOURCES = BufdumpMain.cpp \
	dumperargs.cpp  StringsToIntegers.cpp \
	Converter.cpp RootConverter2.cpp  dumper.cpp \
	dumperargs.h BufdumpMain.h StringsToIntegers.h \
	Converter.h RootConverter2.h  ddaschannel.cpp \
	DataSource.h DataSource.cpp FdDataSource.cpp FdDataSource.h \
	StreamDataSource.h StreamDataSource.cpp
ddasdumper_bin_CPPFLAGS = -I@top_srcdir@/ddas/format \
	-I@top_srcdir@/base/uri \
	@ROOT_CFLAGS@ @UFMT_CPPFLAGS@ -g
ddasdumper_bin_LDADD = @builddir@/libddaschannel.la \
	@top_builddir@/ddas/format/libddasformat.la \
	@top_builddir@/base/uri/liburl.la \
	@ROOT_LDFLAGS@
ddasdumper_bin_LDFLAGS = @UFMT_LDFLAGS@ -g

install-exec-local:
	$(mkinstalldirs) @prefix@/lib
	$(mkinstalldirs) @prefix@/bin
	$(INSTALL_PROGRAM) ddaschannelDictionary_rdict.pcm @prefix@/lib/.
	$(INSTALL_DATA) libddaschannel.rootmap @prefix@/lib/.
	$(INSTALL_PROGRAM) ddasdumper @prefix@/bin/.

clean-local:
	rm -f dumperargs.{h,cpp,c}
	rm -f ddaschannelDictionary*
	rm -f ddaschannelDictionary_rdict.pcm
	rm -f libddaschannel.rootmap

#  Use gengetopt to build dumperargs.{h,cpp} - command argument parser.
#  This stuff still needs a bit of work to be 100% out of tree-able.
#
dumperargs.cpp: dumperargs.h
	mv @srcdir@/dumperargs.c @srcdir@/dumperargs.cpp

dumperargs.h:  dumperargs.ggo
	$(GENGETOPT) --input=@srcdir@/dumperargs.ggo --file-name dumperargs --output-dir=@srcdir@

libddaschannel_la_SOURCES = ddaschannel.cpp DDASEvent.cpp LinkDef.h
libddaschannel_la_CPPFLAGS = -I@top_srcdir@ -I@top_srcdir@/ddas/format \
	@ROOT_CFLAGS@ @PIXIE_CPPFLAGS@
libddaschannel_la_LDFLAGS = @top_builddir@/ddas/format/libddasformat.la \
	@ROOT_LDFLAGS@ 
nodist_libddaschannel_la_SOURCES = ddaschannelDictionary.cpp ddaschannelDictionary.h

ddaschannelDictionary.cpp: ddaschannelDictionary.h

ddaschannelDictionary_rdict.pcm: ddaschannelDictionary.h

ddaschannelDictionary.h: ddaschannel.h DDASEvent.h LinkDef.h
	LD_LIBRARY_PATH=@ROOTSYS@/lib @ROOTSYS@/bin/rootcint -f ddaschannelDictionary.cpp -rml libddaschannel.so -rmf libddaschannel.rootmap -I@srcdir@  -c -p ddaschannel.h DDASEvent.h LinkDef.h 

noinst_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp \
		ddaschanneltest.cpp \
		Asserts.h

unittests_CXXFLAGS = -I@top_srcdir@ -I@top_srcdir@/ddas/format \
	@ROOT_CFLAGS@ \
	@CPPUNIT_CFLAGS@ @PIXIE_CPPFLAGS@
unittests_LDADD = @top_builddir@/ddas/format/libddasformat.la \
	@builddir@/libddaschannel.la \
	@ROOT_LDFLAGS@ \
	@CPPUNIT_LIBS@

TESTS = ./unittests

EXTRA_DIST=dumperargs.ggo  \
	dumperargs.h dumperargs.cpp \
	README_ddasdumper.txt
