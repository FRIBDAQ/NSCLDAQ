lib_LTLIBRARIES = libdaqshm.la

libdaqshm_la_SOURCES = daqshm.cpp os.cpp io.cpp CTimeout.cpp CSemaphore.cpp \
	CPosixBlockingRecordLock.cpp CBufferedOutput.cpp NSCLDAQLog.cpp \
	CRingBlockReader.cpp CRingFileBlockReader.cpp CPagedOutput.cpp \
	CElapsedTime.cpp utils.cpp

include_HEADERS      = daqshm.h os.h io.h CTimeout.h CSemaphore.h \
	CPosixBlockingRecordLock.h CBufferedOutput.h NSCLDAQLog.h \
	CRingBlockReader.h CRingFileBlockReader.h CPagedOutput.h \
	CElapsedTime.h utils.h


noinst_HEADERS	     = Asserts.h

libdaqshm_la_LIBADD = @top_builddir@/base/thread/libdaqthreads.la \
        @THREADLD_FLAGS@ -lcrypt @LIBEXCEPTION_LDFLAGS@ \
	@BOOST_LDFLAGS@ @BOOST_LOG_LIB@

COMPILATION_FLAGS   = @PIXIE_CPPFLAGS@ \
	@THREADCXX_FLAGS@ @LIBTCLPLUS_CFLAGS@ \
	@BOOST_CPPFLAGS@ @BOOST_EXTRA_CPPFLAGS@ -I@top_srcdir@/base/thread


libdaqshm_la_CPPFLAGS=$(COMPILATION_FLAGS)


EXTRA_DIST=shm.xml

#----------------------------------------
#
# Tests:

check_PROGRAMS    = unittests
unittests_SOURCES = TestRunner.cpp createTests.cpp removeTests.cpp \
	attachTests.cpp \
        detachTests.cpp timeoutTests.cpp semaphoretests.cpp \
	closeunusedtests.cpp \
	testBufferedOutput.cpp logtest.cpp poutputtests.cpp testiov.cpp \
	eltest.cpp

unittests_CPPFLAGS=$(COMPILATION_FLAGS)

unittests_LDADD   = @builddir@/libdaqshm.la \
	@top_builddir@/base/thread/libdaqthreads.la \
	@LIBTCLPLUS_LDFLAGS@			\
	@CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@ \
	@BOOST_LDFLAGS@ @BOOST_LOG_LIB@ @TCL_LDFLAGS@

TESTS=./unittests
