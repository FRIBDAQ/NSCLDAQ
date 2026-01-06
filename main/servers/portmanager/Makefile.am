#
#   Build the C++ library:
#


PYTHON_PACKAGES=PortManager.py
PYTHON_PKGINSTDIR=@prefix@/pythonLibs/nscldaq/portmanager

lib_LTLIBRARIES=libPortManager.la

COMPILATION_FLAGS = -I@top_srcdir@/base/headers @LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/os		\
		-DHOME=\"@prefix@\" @PIXIE_CPPFLAGS@

libPortManager_la_SOURCES = CPortManagerException.cpp \
			    CPortManager.cpp

include_HEADERS = CPortManagerException.h \
		  CPortManager.h

libPortManager_la_LDFLAGS = @LIBEXCEPTION_LDFLAGS@	\
			@top_builddir@/base/os/libdaqshm.la   \
			-version-info $(SOVERSION) \
			-Wl,"-rpath=$(libdir)"	\
			$(TCL_LDFLAGS)		\
			$(THREADLD_FLAGS)

libPortManager_la_CPPFLAGS = $(THREADCXX_FLAGS) 	\
				$(TCL_FLAGS)		\
				$(AM_CXXFLAGS) $(COMPILATION_FLAGS)



TCLPACKAGES=ConnectionManager.tcl \
	portAllocator.tcl  \
	PortManager.tcl

TCLAPPS=DaqPortManager.tcl

TCLTESTS=connectionmanager.test portallocator.test portmanager.test log.test tclTests.tcl

PYTHON_TESTS=@srcdir@/PortManagerTests.py


#
#   Install the TCL Packages:
#   We're counting on the top level makefile to build the package directory.
#
install-exec-local:
	$(mkinstalldirs) @prefix@/bin
	$(mkinstalldirs) @prefix@/TclLibs/PortManager
	$(mkinstalldirs) $(PYTHON_PKGINSTDIR)
	touch $(PYTHON_PKGINSTDIR)/__init__.py
	for f in  $(TCLPACKAGES); do @INSTALL_SCRIPT@ @srcdir@/$$f @prefix@/TclLibs/PortManager; done
	for f in  $(TCLAPPS); do @INSTALL_SCRIPT@ @srcdir@/$$f     @prefix@/bin/DaqPortManager; done
	for f in $(PYTHON_PACKAGES);  do $(INSTALL_SCRIPT) @srcdir@/$$f $(PYTHON_PKGINSTDIR); done
	echo "pkg_mkIndex @prefix@/TclLibs/PortManager *.tcl" | @TCLSH_CMD@
	$(INSTALL_PROGRAM) @srcdir@/lsservices.tcl @bindir@/lsservices


#
#  The distribution should contain the tcl files as well as their unit tests:
#
EXTRA_DIST =$(TCLPACKAGES) $(TCLAPPS) $(TCLTESTS) $(PYTHON_PACKAGES) $(PYTHON_TESTS) portmanager.xml lsservices.tcl

#
#  The tests are all TCL tests and can be unconditionally run.
#  We rely on the value of TCLSH_CMD from configure to know how to run the
#  shell.  TCLSH_CMD must represent a tcl shell of at least version 8.4.
#
check_PROGRAMS = cpptest

cpptest_SOURCES = cpptest.cpp
cpptest_LDADD= @builddir@/libPortManager.la @LIBEXCEPTION_LDFLAGS@
cpptest_CPPFLAGS = $(COMPILATION_FLAGS)


check-TESTS: cpptest
	@TCLSH_CMD@ @srcdir@/tclTests.tcl
	@builddir@/cpptest
	for f in $(PYTHON_TESTS); do PYTHONPATH=@prefix@/pythonLibs @PYTHON@ $$f ; done

check: check-TESTS
