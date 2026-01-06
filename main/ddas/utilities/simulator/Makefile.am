lib_LTLIBRARIES = libDDASDataSimulator.la
include_HEADERS = DDASDataSimulator.h
libDDASDataSimulator_la_SOURCES = DDASDataSimulator.cpp
libDDASDataSimulator_la_CPPFLAGS = @UFMT_CPPFLAGS@ @LIBTCLPLUS_CFLAGS@ \
	@DDASFMT_CPPFLAGS@
libDDASDataSimulator_la_LIBADD = @UFMT_LDFLAGS@ @LIBEXCEPTION_LDFLAGS@ \
	@DDASFMT_LDFLAGS@

##
# Test suite
#

check_PROGRAMS = DDASDataSimulatorTests
noinst_HEADERS = Asserts.h
DDASDataSimulatorTests_SOURCES = TestRunner.cpp DDASDataSimulatorTests.cpp
DDASDataSimulatorTests_CPPFLAGS = ${libDDASDataSimulator_la_CPPFLAGS} 	\
	@CPPUNIT_CFLAGS@ 
DDASDataSimulatorTests_LDADD = ${libDDASDataSimulator_la_LIBADD} 	\
	@builddir@/libDDASDataSimulator.la 				\
	@CPPUNIT_LDFLAGS@
TESTS = DDASDataSimulatorTests
