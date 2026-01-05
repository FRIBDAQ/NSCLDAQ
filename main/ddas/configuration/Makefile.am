lib_LTLIBRARIES=libConfiguration.la
test_PROGRAMS = unittests

include_HEADERS = ConfigurationParser.h Configuration.h \
                    FirmwareVersionFileParser.h ModEvtFileParser.h \
                    HardwareRegistry.h

libConfiguration_la_SOURCES = ConfigurationParser.cpp Configuration.cpp \
                    FirmwareVersionFileParser.cpp ModEvtFileParser.cpp \
                    HardwareRegistry.cpp

unittests_SOURCES = TestRunner.cpp ConfigurationParserTest.cpp \
                    FirmwareVersionFileParserTest.cpp \
                    ModEvtLenFileParserTest.cpp ConfigurationTest.cpp \
                    HardwareRegistryTests.cpp Asserts.h NewParseTests.cpp
unittests_CPPFLAGS = -I@srcdir@ @CPPUNIT_CFLAGS@ \
	-DTOP_SRC_DIR="\"@top_srcdir@\"" -I@top_srcdir@/base/testutils
unittests_LDADD = @builddir@/libConfiguration.la @CPPUNIT_LIBS@

TESTS = unittests

EXTRA_DIST = cfgpixie16.dox modevtlen.dox 
