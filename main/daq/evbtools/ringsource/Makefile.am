bin_PROGRAMS = ringFragmentSource

ringFragmentSource_SOURCES=rfcmdline.c Main.cpp \
	CRingFragmentSource.h CRingFragmentSource.cpp

ringFragmentSource_CPPFLAGS=-I@top_srcdir@/daq/eventbuilder \
	@LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/daq/format @PIXIE_CPPFLAGS@ @UFMT_CPPFLAGS@

ringFragmentSource_LDFLAGS= \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@UFMT_LDFLAGS@ \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libDataFlow.la 	\
	@LIBEXCEPTION_LDFLAGS@ -ldl


BUILT_SOURCES = rfcmdline.c rfcmdline.h

EXTRA_DIST=  options.ggo manpages.xml config.h


rfcmdline.c: rfcmdline.h


rfcmdline.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ \
		--input=@srcdir@/options.ggo -F rfcmdline


check_PROGRAMS= unittests

unittests_SOURCES=TestRunner.cpp fragsrctests.cpp Asserts.h \
	CRingFragmentSource.cpp 
unittests_CPPFLAGS= $(ringFragmentSource_CPPFLAGS) @CPPUNIT_CFLAGS@ @UFMT_CPPFLAGS@
unittests_LDFLAGS= @CPPUNIT_LDFLAGS@ @UFMT_LDFLAGS@ @top_builddir@/daq/format/libdataformat.la \
	-ldl

TESTS=./unittests
