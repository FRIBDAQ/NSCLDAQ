bin_PROGRAMS		=	eventlog
TCLSCRIPTS            = eventlog_wrapper.tcl
PYTHONSCRIPTS         = eventlog_wrapper.py
BUILT_SOURCES		= 	eventlogargs.c eventlogargs.h

eventlog_SOURCES	=	eventlog.cpp eventlogMain.cpp \
				RingChunk.h RingChunk.cpp

nodist_eventlog_SOURCES =       eventlogargs.c eventlogargs.h

noinst_HEADERS		=	eventlogMain.h


eventlog_CPPFLAGS       =	-I@top_srcdir@/base/headers		\
				@LIBTCLPLUS_CFLAGS@			\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/base/os		\
				@OPENSSL_INCLUDES@ @PIXIE_CPPFLAGS@

eventlog_LDADD		=	@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@LIBEXCEPTION_LDFLAGS@			\
				@top_builddir@/base/os/libdaqshm.la		\
				$(THREADLD_FLAGS) @OPENSSL_LDFLAGS@ @OPENSSL_LIBS@

eventlog_CXXFLAGS	=	$(THREADCXX_FLAGS) $(AM_CXXFLAGS)

# Gengetopt stuff.

eventlogargs.c: eventlogargs.h


eventlogargs.h: eventlogargs.ggo
	$(GENGETOPT) <@srcdir@/eventlogargs.ggo --file=eventlogargs \
			--set-version=@VERSION@

# Extra files to distribute.

EXTRA_DIST		=	eventlogargs.ggo eventlog.xml Asserts.h \
	$(TCLSCRIPTS) $(PYTHONSCRIPTS)

#  Need to clean the products of processing eventlogargs.ggo:

clean-local:
	rm -f eventlogargs.h eventlogargs.c


install-exec-local:
	for f in $(PYTHONSCRIPTS) ; do  \
          $(INSTALL_PROGRAM) @srcdir@/$$f  @bindir@/`basename $$f .py`; \
	done

#-------------------------------------------------------------
#
# Test programs.
#
check_PROGRAMS		=	fakeruns eventlogTests

fakeruns_SOURCES	= 	fakeruns.cpp

fakeruns_LDADD		=	@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@top_builddir@/base/os/libdaqshm.la              \
				@LIBEXCEPTION_LDFLAGS@
fakeruns_CPPFLAGS=-I@top_srcdir@/base/headers		\
				@LIBTCLPLUS_CFLAGS@			\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/base/os		\
				@OPENSSL_INCLUDES@ @PIXIE_CPPFLAGS@

eventlogTests_SOURCES = TestRunner.cpp eventlogTests.cpp chunkTests.cpp RingChunk.cpp
eventlogTests_CPPFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base/headers		\
				@LIBTCLPLUS_CFLAGS@			\
				-I@top_srcdir@/daq/format		\
			        -I@top_srcdir@/base/dataflow	\
				-I@top_srcdir@/base/os		\
				@OPENSSL_INCLUDES@ @PIXIE_CPPFLAGS@

eventlogTests_LDADD   = @CPPUNIT_LDFLAGS@ \
				@top_builddir@/daq/format/libdataformat.la	\
				@top_builddir@/base/dataflow/libDataFlow.la	\
				@LIBEXCEPTION_LDFLAGS@			\
				@top_builddir@/base/os/libdaqshm.la		\
				$(THREADLD_FLAGS)

TESTS=eventlogTests
