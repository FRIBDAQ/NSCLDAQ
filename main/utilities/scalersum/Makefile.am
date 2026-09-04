
bin_PROGRAMS = sumscaler

sumscaler_SOURCES=   main.cpp App.cpp App.h  CRun.h CRun.cpp \
	CChannel.h CIncrementalChannel.h CIncrementalChannel.cpp \
	CCumulativeChannel.h CCumulativeChannel.cpp
nodist_sumscaler_SOURCES=options.c options.h

sumscaler_CPPFLAGS=-I@top_srcdir@/daq/IO 			\
		    -I@top_srcdir@/daq/format			\
		    @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@
sumscaler_LDFLAGS = @top_builddir@/daq/IO/libdaqio.la 	\
    @top_builddir@/daq/format/libdataformat.la			\
    @LIBEXCEPTION_LDFLAGS@

BUILT_SOURCES	= options.c options.h

options.c: options.h

options.h: options.ggo
	$(GENGETOPT) < @srcdir@/options.ggo --file=options \
	    --unamed-opts

install-exec-hook:
	$(INSTALL_SCRIPT) @srcdir@/makeNameFile.tcl @bindir@/makeNameFile
	$(INSTALL_SCRIPT) @srcdir@/pyMakeNameFile.py @bindir@/pyMakeNameFile

clean-local:
	-rm -rf options.h options.c

check_PROGRAMS=unittests

unittests_SOURCES=Asserts.h TestRunner.cpp channelTests.cpp runTests.cpp \
	CRun.cpp CCumulativeChannel.cpp CIncrementalChannel.cpp 

unittests_CPPFLAGS=$(sumscaler_CPPFLAGS) @CPPUNIT_CFLAGS@
unittests_LDFLAGS=$(sumscaler_LDFLAGS)   @CPPUNIT_LDFLAGS@

TESTS = unittests

EXTRA_DIST = pyMakeNameFile.py makeNameFile.tcl options.ggo scalersum.xml
