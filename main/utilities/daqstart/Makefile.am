TESTABLEUNITS=CSink.cpp CFileSink.cpp CFileSinkCreator.cpp CSinkFactory.cpp \
	CMonitoredProgram.cpp CLocalMonitoredProgram.cpp
bin_PROGRAMS 	= daqstart

BUILT_SOURCES=cmdline.c cmdline.h

daqstart_SOURCES=  daqstart.cpp CDAQStart.cpp $(TESTABLEUNITS)
nodist_daqstart_SOURCES=cmdline.c cmdline.h

noinst_HEADERS = cmdline.h CDAQStart.h  CFileSinkCreator.h \
		CMonitoredProgram.h  CSinkCreator.h \
		CFileSink.h  CLocalMonitoredProgram.h  CSink.h \
		CSinkFactory.h grep.h


COMPILATION_FLAGS  =-I@top_srcdir@/base/os -I@top_srcdir@/base/factories \
	-I@top_srcdir@/base/headers @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@

daqstart_CPPFLAGS	= $(COMPILATION_FLAGS)

daqstart_LDADD = @LIBEXCEPTION_LDFLAGS@ \
	@top_builddir@/base/os/libdaqshm.la  \
	$(THREADLD_FLAGS) @TCL_LDFLAGS@

daqstart_CXXFLAGS=$(THREADCXX_FLAGS) -D_GNU_SOURCE $(AM_CXXFLAGS)


cmdline.c: cmdline.h
	$(GENGETOPT) --unamed-opts <@top_srcdir@/utilities/daqstart/daqstart.ggo

cmdline.h: daqstart.ggo
	$(GENGETOPT) --unamed-opts <@top_srcdir@/utilities/daqstart/daqstart.ggo

EXTRA_DIST=daqstart.ggo popup.tcl popup.py  daqstart.xml Makefile.am config.h


install-exec-local:
	$(INSTALL_SCRIPT) @srcdir@/popup.py $(bindir)/PopUp


check_PROGRAMS = unittests

unittests_SOURCES = $(TESTABLEUNITS) CFileSinkTests.cpp SinkCreatorTests.cpp \
	FactoryTests.cpp LocalMonitoredProgram.cpp  grep.cpp TestRunner.cpp

unittests_LDADD = $(CPPUNIT_LDFLAGS) @LIBEXCEPTION_LDFLAGS@ \
	@top_builddir@/base/os/libdaqshm.la @TCL_LDFLAGS@
unittests_CPPFLAGS=$(COMPILATION_FLAGS)


TESTS = ./unittests

