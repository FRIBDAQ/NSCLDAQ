bin_PROGRAMS	=	dumper
lib_LTLIBRARIES    =   libStringifier.la


dumper_SOURCES	=	BufdumpMain.cpp		\
			dumper.cpp 

nodist_dumper_SOURCES= dumperargs.c dumperargs.h

noinst_HEADERS	=	BufdumpMain.h	

BUILT_SOURCES	=	dumperargs.c dumperargs.h


dumper_CPPFLAGS =	-I@srcdir@/.. 				\
				-I.. 					\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/uri			\
				@LIBTCLPLUS_CFLAGS@			\
				-I@top_srcdir@/daq/format		\
				-I@top_srcdir@/base/dataflow		\
                                -I@top_srcdir@/daq/IO		\
                                -I@top_srcdir@/base/os			\
				-I@top_srcdir@/utilities/common \
				@PIXIE_CPPFLAGS@

dumper_LDADD		=	 @builddir@/libStringifier.la \
				@top_builddir@/utilities/common/libUtilCommon.la 	\
				@top_builddir@/base/uri/liburl.la			\
                                @top_builddir@/daq/IO/libdaqio.la			\
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/uri/liburl.la	\
				@LIBEXCEPTION_LDFLAGS@				\
				@top_builddir@/base/os/libdaqshm.la            \
				$(THREADLD_FLAGS)
dumper_DEPENDENCIES = libStringifier.la 
libStringifier_la_SOURCES = CUnifiedFormatter.cpp CUnifiedFormatter.h

libStringifier_la_CPPFLAGS = @UFMT_CPPFLAGS@
libStringifier_la_LIBADD = @UFMT_LDFLAGS@


clean-local:
	rm -f dumperargs.o $(BUILT_SOURCES)

dumper_CXXFLAGS = $(THREADCXX_FLAGS) $(AM_CXXFLAGS)

dumperargs.c: dumperargs.h


dumperargs.h: dumperargs.ggo 
	$(GENGETOPT) < @srcdir@/dumperargs.ggo \
		--output-dir=@builddir@ --file=dumperargs --unamed-opts="OPTIONS" \
		--set-version=@VERSION@


#  This explores the problem I saw where v12 buffers did not dump
# with the right run offset times.

noinst_PROGRAMS=divisor_check

divisor_check_SOURCES=divisor_check.cpp CUnifiedFormatter.cpp

divisor_check_CPPFLAGS=-I@srcdir@ @UFMT_CPPFLAGS@
divisor_check_LDFLAGS=	@UFMT_LDFLAGS@

EXTRA_DIST=dumperargs.ggo dumper_user.xml dumper_man.xml

