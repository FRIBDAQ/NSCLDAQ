lib_LTLIBRARIES = libLicense.la
libLicense_la_SOURCES = CopyrightNotice.cpp

COMPILATION_FLAGS =  -I@top_srcdir@/base/headers @PIXIE_CPPFLAGS@

libLicense_la_CXXFLAGS = $(COMPILATION_FLAGS) $(THREADCXX_FLAGS) $(AM_CXXFLAGS) 


include_HEADERS = CopyrightNotice.h
libLicense_la_LDFLAGS =  -version-info $(SOVERSION) \
	-Wl,"-rpath-link=$(libdir)"	\
	$(THREADLD_FLAGS)

TESTS = ./test

check_PROGRAMS = test

test_SOURCES       = test.cpp

test_LDADD = @builddir@/libLicense.la

test_CXXFLAGS=$(COMPILATION_FLAGS)


EXTRA_DIST = copyrightnotice.xml addlicense.py addscriptlicense.py
