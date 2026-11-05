#  DAQ performance / diagnostic tools. Built only when --enable-perftools.
#
#  Most tools build against the in-tree core libraries. fragmaker and
#  evbfilecheck additionally use the unified format library (@UFMT_*@) for
#  fragment.h / FragmentIndex.h, which were previously pulled from SpecTcl.
#
#  Installed under $(bindir)/perftools to keep them separate from the normal
#  NSCLDAQ programs.
#
#  Command-line parsing is done with gengetopt: each tool's options live in
#  <tool>.ggo and are turned into <tool>_cmdline.c / .h at build time (see the
#  generated-parser rules below). The _cmdline suffix keeps the generated files
#  from colliding with the program's own <tool>.cpp.

AM_CPPFLAGS = \
	-I@top_srcdir@/base/tcpip \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/daq/IO \
	-I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os \
	-I@top_srcdir@/base/thread \
	-I@top_srcdir@/base/headers \
	@THREADCXX_FLAGS@ \
	@LIBTCLPLUS_CFLAGS@

LDADD = \
	@top_builddir@/base/tcpip/libTcp.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/daq/IO/libdaqio.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/os/libdaqshm.la \
	@top_builddir@/base/thread/libdaqthreads.la \
	@LIBEXCEPTION_LDFLAGS@ \
	-lrt

# Hardcode the install libdir into the tools so the installed programs find
# the core NSCLDAQ shared libraries at runtime without LD_LIBRARY_PATH.
# Matches the rest of the tree (controlpush, tclserver, portmanager, ...).
AM_LDFLAGS = -Wl,"-rpath=$(libdir)"

noinst_LTLIBRARIES = libMeterApi.la
libMeterApi_la_SOURCES = MeterApi.cpp MeterApi.h

perftoolsdir = $(bindir)/perftools
perftools_PROGRAMS = \
	metertest rdoperf fragsrcperf socksend sockperf \
	pipesend pipeperf bufferedoutperf ritemMaker runmaker checkevfiles \
	fragmaker evbfilecheck

metertest_SOURCES       = metertestmain.cpp
metertest_LDADD         = libMeterApi.la $(LDADD)

rdoperf_SOURCES         = rdoperf.cpp
nodist_rdoperf_SOURCES  = rdoperf_cmdline.c rdoperf_cmdline.h
rdoperf_LDADD           = libMeterApi.la $(LDADD)

fragsrcperf_SOURCES     = fragsrcperf.cpp
nodist_fragsrcperf_SOURCES = fragsrcperf_cmdline.c fragsrcperf_cmdline.h
fragsrcperf_LDADD       = libMeterApi.la $(LDADD)

sockperf_SOURCES        = sockperf.cpp
nodist_sockperf_SOURCES = sockperf_cmdline.c sockperf_cmdline.h
sockperf_LDADD          = libMeterApi.la $(LDADD)

pipeperf_SOURCES        = pipeperf.cpp
nodist_pipeperf_SOURCES = pipeperf_cmdline.c pipeperf_cmdline.h
pipeperf_LDADD          = libMeterApi.la $(LDADD)

socksend_SOURCES        = socksend.cpp
nodist_socksend_SOURCES = socksend_cmdline.c socksend_cmdline.h

pipesend_SOURCES        = pipesend.cpp
nodist_pipesend_SOURCES = pipesend_cmdline.c pipesend_cmdline.h

bufferedoutperf_SOURCES = bufferedoutperf.cpp
nodist_bufferedoutperf_SOURCES = bufferedoutperf_cmdline.c bufferedoutperf_cmdline.h

ritemMaker_SOURCES      = ritemMaker.cpp
nodist_ritemMaker_SOURCES = ritemMaker_cmdline.c ritemMaker_cmdline.h

runmaker_SOURCES        = runmaker.cpp
nodist_runmaker_SOURCES = runmaker_cmdline.c runmaker_cmdline.h

checkevfiles_SOURCES    = checkevfiles.cpp
nodist_checkevfiles_SOURCES = checkevfiles_cmdline.c checkevfiles_cmdline.h

# fragmaker and evbfilecheck also need the unified format library. AM_CPPFLAGS
# (the core include dirs) comes first so CRingItem.h, DataFormat.h, etc. resolve
# to the core headers; @UFMT_CPPFLAGS@ is appended for the unified-only headers
# fragment.h (fragmaker) and FragmentIndex.h (evbfilecheck).
fragmaker_SOURCES       = fragmaker.cpp
nodist_fragmaker_SOURCES = fragmaker_cmdline.c fragmaker_cmdline.h
fragmaker_CPPFLAGS      = $(AM_CPPFLAGS) @UFMT_CPPFLAGS@
fragmaker_LDADD         = $(LDADD) @UFMT_LDFLAGS@

evbfilecheck_SOURCES    = evbfilecheck.cpp
nodist_evbfilecheck_SOURCES = evbfilecheck_cmdline.c evbfilecheck_cmdline.h
evbfilecheck_CPPFLAGS   = $(AM_CPPFLAGS) @UFMT_CPPFLAGS@
evbfilecheck_LDADD      = $(LDADD) @UFMT_LDFLAGS@

noinst_HEADERS = utils.h StatsLog.h

#  ---- gengetopt generated command-line parsers --------------------------
#  <tool>.ggo -> <tool>_cmdline.c / <tool>_cmdline.h.  gengetopt is already
#  required to build the rest of NSCLDAQ. One gengetopt run makes both files,
#  so the .c just depends on the .h and the .h depends on the .ggo.

GENERATED_PARSERS = \
	rdoperf_cmdline.c rdoperf_cmdline.h \
	fragsrcperf_cmdline.c fragsrcperf_cmdline.h \
	sockperf_cmdline.c sockperf_cmdline.h \
	pipeperf_cmdline.c pipeperf_cmdline.h \
	socksend_cmdline.c socksend_cmdline.h \
	pipesend_cmdline.c pipesend_cmdline.h \
	bufferedoutperf_cmdline.c bufferedoutperf_cmdline.h \
	ritemMaker_cmdline.c ritemMaker_cmdline.h \
	runmaker_cmdline.c runmaker_cmdline.h \
	checkevfiles_cmdline.c checkevfiles_cmdline.h \
	fragmaker_cmdline.c fragmaker_cmdline.h \
	evbfilecheck_cmdline.c evbfilecheck_cmdline.h

BUILT_SOURCES = $(GENERATED_PARSERS)
CLEANFILES = $(GENERATED_PARSERS)

rdoperf_cmdline.c: rdoperf_cmdline.h
rdoperf_cmdline.h: $(srcdir)/rdoperf.ggo
	$(GENGETOPT) --input=$(srcdir)/rdoperf.ggo --output-dir=$(builddir) --file-name=rdoperf_cmdline

fragsrcperf_cmdline.c: fragsrcperf_cmdline.h
fragsrcperf_cmdline.h: $(srcdir)/fragsrcperf.ggo
	$(GENGETOPT) --input=$(srcdir)/fragsrcperf.ggo --output-dir=$(builddir) --file-name=fragsrcperf_cmdline

sockperf_cmdline.c: sockperf_cmdline.h
sockperf_cmdline.h: $(srcdir)/sockperf.ggo
	$(GENGETOPT) --input=$(srcdir)/sockperf.ggo --output-dir=$(builddir) --file-name=sockperf_cmdline

pipeperf_cmdline.c: pipeperf_cmdline.h
pipeperf_cmdline.h: $(srcdir)/pipeperf.ggo
	$(GENGETOPT) --input=$(srcdir)/pipeperf.ggo --output-dir=$(builddir) --file-name=pipeperf_cmdline

socksend_cmdline.c: socksend_cmdline.h
socksend_cmdline.h: $(srcdir)/socksend.ggo
	$(GENGETOPT) --input=$(srcdir)/socksend.ggo --output-dir=$(builddir) --file-name=socksend_cmdline

pipesend_cmdline.c: pipesend_cmdline.h
pipesend_cmdline.h: $(srcdir)/pipesend.ggo
	$(GENGETOPT) --input=$(srcdir)/pipesend.ggo --output-dir=$(builddir) --file-name=pipesend_cmdline

bufferedoutperf_cmdline.c: bufferedoutperf_cmdline.h
bufferedoutperf_cmdline.h: $(srcdir)/bufferedoutperf.ggo
	$(GENGETOPT) --input=$(srcdir)/bufferedoutperf.ggo --output-dir=$(builddir) --file-name=bufferedoutperf_cmdline

ritemMaker_cmdline.c: ritemMaker_cmdline.h
ritemMaker_cmdline.h: $(srcdir)/ritemMaker.ggo
	$(GENGETOPT) --input=$(srcdir)/ritemMaker.ggo --output-dir=$(builddir) --file-name=ritemMaker_cmdline

runmaker_cmdline.c: runmaker_cmdline.h
runmaker_cmdline.h: $(srcdir)/runmaker.ggo
	$(GENGETOPT) --input=$(srcdir)/runmaker.ggo --output-dir=$(builddir) --file-name=runmaker_cmdline

checkevfiles_cmdline.c: checkevfiles_cmdline.h
checkevfiles_cmdline.h: $(srcdir)/checkevfiles.ggo
	$(GENGETOPT) --input=$(srcdir)/checkevfiles.ggo --output-dir=$(builddir) --file-name=checkevfiles_cmdline

fragmaker_cmdline.c: fragmaker_cmdline.h
fragmaker_cmdline.h: $(srcdir)/fragmaker.ggo
	$(GENGETOPT) --input=$(srcdir)/fragmaker.ggo --output-dir=$(builddir) --file-name=fragmaker_cmdline

evbfilecheck_cmdline.c: evbfilecheck_cmdline.h
evbfilecheck_cmdline.h: $(srcdir)/evbfilecheck.ggo
	$(GENGETOPT) --input=$(srcdir)/evbfilecheck.ggo --output-dir=$(builddir) --file-name=evbfilecheck_cmdline

# The .ggo option descriptions and the empty config.h placeholder (needed so the
# generated parsers' "#include \"config.h\"" resolves here) go in the tarball.
EXTRA_DIST = \
	config.h \
	rdoperf.ggo fragsrcperf.ggo sockperf.ggo pipeperf.ggo \
	socksend.ggo pipesend.ggo bufferedoutperf.ggo ritemMaker.ggo \
	runmaker.ggo checkevfiles.ggo fragmaker.ggo evbfilecheck.ggo

# Install the Tcl meter server and its widget alongside the tools (in
# $(bindir)/perftools) so perftests can be run against an NSCLDAQ
# installation with no source tree. dist_ keeps them in the tarball too.
dist_perftools_SCRIPTS = meter.tcl meterserver.tcl
