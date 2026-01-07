bin_PROGRAMS=evts2frags

evts2frag_COMMON_CFLAGS = -I@top_srcdir@/daq/format \
	-I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/base/os -I@top_srcdir@/base/thread @LIBTCLPLUS_CFLAGS@ \
	@PIXIE_CPPFLAGS@  @UFMT_CPPFLAGS@

evts2frags_SOURCES=Main.cpp CFragmentMaker.cpp CEvents2Fragments.cpp \
	evts2frags.c 
evts2frags_CPPFLAGS=$(evts2frag_COMMON_CFLAGS)
evts2frags_LDADD=@top_builddir@/daq/format/libdataformat.la \
	@UFMT_LDFLAGS@ \
	@top_builddir@/base/os/libdaqshm.la \
	@top_builddir@/base/thread/libdaqthreads.la \
	@LIBEXCEPTION_LDFLAGS@


# The command line def:

BUILT_SOURCES=evts2frags.c evts2frags.h

evts2frags.c: evts2frags.h

evts2frags.h: @srcdir@/evts2frags.ggo
	@GENGETOPT@ --input=@srcdir@/evts2frags.ggo \
		--output-dir=@builddir@ --file-name=evts2frags


EXTRA_DIST=evts2frags.ggo config.h


# Tests

noinst_HEADERS=CFragmentMaker.h CEvents2Fragments.h

check_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp headerbuildertests.cpp apptests.cpp \
	Asserts.h \
	CFragmentMaker.cpp CEvents2Fragments.cpp


unittests_CPPFLAGS=$(evts2frag_COMMON_CFLAGS)
unittests_LDADD=@CPPUNIT_LDFLAGS@ \
	@top_builddir@/daq/format/libdataformat.la \
	@UFMT_LDFLAGS@ \
	@top_builddir@/base/os/libdaqshm.la \
	@top_builddir@/base/thread/libdaqthreads.la @LIBEXCEPTION_LDFLAGS@


TESTS=unittests
