bin_PROGRAMS=mvlcgenerate

mvlcgenerate_SOURCES=generate.cpp utilities.h utilities.cpp \
	TCLConfigParser.h TCLConfigParser.cpp


mvlcgenerate_CPPFLAGS=@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@
mvlcgenerate_LDFLAGS=@LIBTCLPLUS_LDFLAGS@ @TCL_LIBS@

BUILT_SOURCES = generate.cpp options.h options.c
nodist_mvlcgenerate_SOURCES=options.h options.c

options.c: options.h

options.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ --file-name=options <@srcdir@/options.ggo 


noinst_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp utiltests.cpp parsertests.cpp \
	utilities.cpp options.h options.c TCLConfigParser.cpp TCLConfigParserh
unittests_CPPFLAGS = @CPPUNIT_CFLAGS@ $(mvlcgenerate_CPPFLAGS)
unittests_LDFLAGS = @CPPUNIT_LIBS@ $(mvlcgenerate_LDFLAGS)

TESTS=unittests

EXTRA_DIST=options.ggo