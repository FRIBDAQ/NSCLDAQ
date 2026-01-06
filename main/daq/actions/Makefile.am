lib_LTLIBRARIES=libActions.la

libActions_la_SOURCES=Actions.cpp
libActions_la_CPPFLAGS = -I@top_srcdir@/base/thread @LIBTCLPLUS_CFLAGS@ \
	@PIXIE_CPPFLAGS@
libActions_la_LIBADD  = @top_builddir@/base/thread/libdaqthreads.la @LIBEXCEPTION_LDFLAGS@
include_HEADERS = Actions.h

install-exec-local:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/Stager
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(prefix)/TclLibs/Stager; done
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/Stager *.tcl" | $(TCLSH_CMD)


#  This is here just to let us shake out the compilation errors in Actions.h before
#  committing to installing it.  testCompile.cpp just includes Actions.h

testCompile.o: testCompile.cpp
	$(CXX) $(CXXFLAGS) @PIXIE_CPPFLAGS@ -c -I@srcdir@ @srcdir@/testCompile.cpp

TCLPACKAGES=actions.tcl defaultActions.tcl

TCLTESTS =    actions.test tclTests.tcl


check_PROGRAMS=testacts

testacts_SOURCES=TestActionPackage.cpp
testacts_CPPFLAGS=$(libActions_la_CPPFLAGS)
testacts_LDADD = @builddir@/libActions.la $(libActions_la_LIBADD) @TCL_LDFLAGS@

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST = $(TCLPACKAGES) $(TCLTESTS) $(CXXPACKAGES) testCompile.cpp

check: check-TESTS
