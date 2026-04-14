#  Makefile template for chanlog : log epics channels to file or stdout.
#  This can only be built if Epics was detected.

if EPICS
bin_PROGRAMS = chanlog


chanlog_SOURCES = chanlog.cpp getinfo.cpp
noinst_HEADERS = getinfo.h
chanlog_LDADD   = @top_builddir@/base/os/libdaqshm.la \
	@top_builddir@/epics/epicslib/libEpicslib.la \
	$(EPICS_LDFLAGS) $(THREADLD_FLAGS) @TCL_LDFLAGS@

chanlog_CXXFLAGS= $(THREADCXX_FLAGS) $(AM_CXXFLAGS) $(EPICS_BIN) $(EPICS_INCLUDES) \
	-I@top_srcdir@/Headers \
	-I@top_srcdir@/epics/epicslib \
	-I@top_srcdir@/base/os \
	@PIXIE_CPPFLAGS@




endif
EXTRA_DIST = chanlog.xml
