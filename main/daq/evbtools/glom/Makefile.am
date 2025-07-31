bin_PROGRAMS=glom


glom_SOURCES= glomMain.cpp CBufferedFragmentReader.cpp

nodist_glom_SOURCES=glom.c glom.h

BUILT_SOURCES = glom.c glom.h

glom_CPPFLAGS = -I@top_srcdir@/daq/eventbuilder \
	-I@top_srcdir@/daq/format          \
	-I@top_srcdir@/base/os @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@ \
	-I@top_srcdir@/base/thread @UFMT_CPPFLAGS@

glom_LDADD = @top_builddir@/daq/eventbuilder/libEventBuilderClient.la  \
	@top_builddir@/daq/format/libdataformat.la \
	@UFMT_LDFLAGS@ \
	@top_builddir@/base/os/libdaqshm.la @LIBEXCEPTION_LDFLAGS@

glom.c:  glom.h

glom.h: @srcdir@/glom.ggo
	@GENGETOPT@ --input=@srcdir@/glom.ggo \
		--output-dir=@builddir@ --file-name=glom

EXTRA_DIST=glom.ggo glom.xml config.h

##
# Make a buffered fragment reader library for testing
#

lib_LTLIBRARIES = libBufferedFragmentReader.la
libBufferedFragmentReader_la_SOURCES = CBufferedFragmentReader.cpp CBufferedFragmentReader.h
include_HEADERS = CBufferedFragmentReader.h
libBufferedFragmentReader_la_CPPFLAGS = @UFMT_CPPFLAGS@
libBufferedFragmentReader_la_LIBADD = @UFMT_LDFLAGS@ 
