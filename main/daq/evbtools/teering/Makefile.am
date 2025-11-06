bin_PROGRAMS = teering

teering_SOURCES = teering.cpp tcmdline.c 

BUILT_SOURCES=tcmdline.c

tcmdline.c: tcmdline.h


tcmdline.h: @srcdir@/options.ggo
	$(GENGETOPT) --input=@srcdir@/options.ggo \
		--output-dir=@builddir@ -F tcmdline

teering_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder  \
	-I@top_srcdir@/daq/format \
	-I@top_srcdir@/base/dataflow \
	-I@top_srcdir@/base/os @PIXIE_CPPFLAGS@  @UFMT_CPPFLAGS@


teering_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@UFMT_LDFLAGS@ \
	@top_builddir@/daq/format/libdataformat.la \
	@top_builddir@/base/dataflow/libRingBuffer.la     \
	@top_builddir@/base/dataflow/libDataFlow.la	\
	@top_builddir@/base/os/libdaqshm.la		 \
	@LIBTCLPLUS_LDFLAGS@	\
	@THREADLD_FLAGS@

teering_LDFLAGS= -Wl,"-rpath=@libdir@"

EXTRA_DIST=options.ggo teering.xml config.h
