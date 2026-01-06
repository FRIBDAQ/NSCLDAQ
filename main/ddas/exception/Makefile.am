lib_LTLIBRARIES = libDDASException.la

libDDASException_la_SOURCES = CXIAException.cpp
include_HEADERS = CXIAException.h
libDDASException_la_CPPFLAGS = @LIBTCLPLUS_CFLAGS@ \
	@PIXIE_CPPFLAGS@ \
	-I@top_builddir@/ddas
libDDASException_la_LIBADD = @LIBEXCEPTION_LDFLAGS@ \
	@PIXIE_LDFLAGS@

check_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp Asserts.h ExceptionTests.cpp
unittests_CPPFLAGS = $(libDDASException_la_CPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDADD = $(libDDASException_la_LIBADD) @CPPUNIT_LIBS@ @builddir@/libDDASException.la

TESTS = unittests
