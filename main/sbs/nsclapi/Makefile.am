lib_LTLIBRARIES = libSBSVmeAPI.la
libSBSVmeAPI_la_SOURCES = SBSBIT3API.cpp Locking.cpp	\
	VmeModule.cpp MmapError.cpp

include_HEADERS = CVMEInterface.h SBSBit3API.h CVME.h \
		CVMEptr.h VmeModule.h MmapError.h

# Locking.cpp


libSBSVmeAPI_la_LDFLAGS =  -version-info 1:0:0

libSBSVmeAPI_la_LIBADD = @LIBTCLPLUS_LDFLAGS@	\
			@top_builddir@/sbs/driver/src/libbtp.la



libSBSVmeAPI_la_CPPFLAGS = -I@top_srcdir@/base/headers \
	@LIBTCLPLUS_CFLAGS@	\
	-I@top_srcdir@/sbs/driver/include	\
	-I@top_srcdir@/sbs/driver/dd @PIXIE_CPPFLAGS@

AM_LDFLAGS = -lpthread


check_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp LockingTest.cpp

unittests_CXXFLAGS = @CPPUNIT_CFLAGS@ @PIXIE_CPPFLAGS@

unittests_LDADD = @builddir@/libSBSVmeAPI.la @CPPUNIT_LDFLAGS@ @TCL_LIBS@

check-TESTS:
	./unittests

EXTRA_DIST = sbsVmeApi.xml  Asserts.h


check:  check-TESTS
