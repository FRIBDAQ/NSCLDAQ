bin_PROGRAMS=mvlcgenerate

mvlcgenerate_SOURCES=generate.cpp utilities.h utilities.cpp

BUILT_SOURCES = generate.cpp options.h options.c
nodist_mvlcgenerate_SOURCES=options.h options.c

options.c: options.h

options.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ --file-name=options <@srcdir@/options.ggo 

EXTRA_DIST=options.ggo