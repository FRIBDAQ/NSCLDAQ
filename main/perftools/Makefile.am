AM_CPPFLAGS = \
	-I@top_srcdir@/base/tcpip \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/daq/IO \
	-I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os \
	-I@top_srcdir@/base/headers \
	@LIBTCLPLUS_CFLAGS@

LDADD = \
	@top_builddir@/base/tcpip/libTcp.la \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/daq/IO/libdaqio.la \
	@top_builddir@/base/dataflow/libDataFlow.la \
	@top_builddir@/base/os/libdaqshm.la \
	@LIBEXCEPTION_LDFLAGS@ \
	-lrt

noinst_LTLIBRARIES = libMeterApi.la
libMeterApi_la_SOURCES = MeterApi.cpp MeterApi.h

# Install under $(bindir)/perftools to keep these separate from the normal
# NSCLDAQ programs.
perftoolsdir = $(bindir)/perftools
perftools_PROGRAMS = \
	metertest rdoperf fragsrcperf socksend sockperf \
	pipesend pipeperf bufferedoutperf ritemMaker runmaker checkevfiles \
	fragmaker evbfilecheck

metertest_SOURCES       = metertestmain.cpp
metertest_LDADD         = libMeterApi.la $(LDADD)
rdoperf_SOURCES         = rdoperf.cpp
rdoperf_LDADD           = libMeterApi.la $(LDADD)
fragsrcperf_SOURCES     = fragsrcperf.cpp
fragsrcperf_LDADD       = libMeterApi.la $(LDADD)
sockperf_SOURCES        = sockperf.cpp
sockperf_LDADD          = libMeterApi.la $(LDADD)
pipeperf_SOURCES        = pipeperf.cpp
pipeperf_LDADD          = libMeterApi.la $(LDADD)

socksend_SOURCES        = socksend.cpp
pipesend_SOURCES        = pipesend.cpp
bufferedoutperf_SOURCES = bufferedoutperf.cpp
ritemMaker_SOURCES      = ritemMaker.cpp
runmaker_SOURCES        = runmaker.cpp
checkevfiles_SOURCES    = checkevfiles.cpp

# fragmaker and evbfilecheck also need the unified format library. The core
# include dirs (AM_CPPFLAGS) come first so CRingItem.h, DataFormat.h, etc.
# resolve to the core headers; @UFMT_CPPFLAGS@ is appended for the unified-only
# headers fragment.h (fragmaker) and FragmentIndex.h (evbfilecheck).
fragmaker_SOURCES       = fragmaker.cpp
fragmaker_CPPFLAGS      = $(AM_CPPFLAGS) @UFMT_CPPFLAGS@
fragmaker_LDADD         = $(LDADD) @UFMT_LDFLAGS@

evbfilecheck_SOURCES    = evbfilecheck.cpp
evbfilecheck_CPPFLAGS   = $(AM_CPPFLAGS) @UFMT_CPPFLAGS@
evbfilecheck_LDADD      = $(LDADD) @UFMT_LDFLAGS@

noinst_HEADERS = utils.h
EXTRA_DIST = README meter.tcl meterserver.tcl
