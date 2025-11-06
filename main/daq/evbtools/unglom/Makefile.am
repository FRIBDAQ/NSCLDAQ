bin_PROGRAMS=unglom


BUILT_SOURCES = cmdline.c

unglom_SOURCES=unglom.cpp cmdline.c

unglom_CPPFLAGS=-I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/daq/format       \
	@LIBTCLPLUS_CFLAGS@	\
	-I@top_srcdir@/base/os		\
	-I@top_srcdir@/base/headers   @PIXIE_CPPFLAGS@ @UFMT_CPPFLAGS@

unglom_LDADD = @top_builddir@/daq/format/libdataformat.la  \
	@UFMT_LDFLAGS@ \
	@top_builddir@/daq/eventbuilder/libEventBuilder.la \
	@top_builddir@/daq/eventbuilder/libEventBuilderClient.la \
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBTCLPLUS_LDFLAGS@



cmdline.c: cmdline.ggo
	@GENGETOPT@ --input=@srcdir@/cmdline.ggo \
		--output-dir=@builddir@

EXTRA_DIST=cmdline.ggo unglom.xml config.h
