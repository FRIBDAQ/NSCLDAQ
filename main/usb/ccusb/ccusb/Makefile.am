lib_LTLIBRARIES		= libCCUSB.la libCCCUSB.la libCCCUSBReadoutList.la
COMPILATION_FLAGS		= \
				-I@top_srcdir@/base/os \
				-I@top_srcdir@/base/thread \
				@LIBTCLPLUS_CFLAGS@ \
				@TCL_FLAGS@ \
				@USBSWITCHES@ \
				@THREADCXX_FLAGS@ \
				-I@top_srcdir@/base/tcpip  @PIXIE_CPPFLAGS@


libCCUSB_la_SOURCES	= CCCUSB.cpp \
			CCCUSBusb.cpp \
			CCCUSBRemote.cpp \
			CMockCCUSB.cpp \
			CCCUSBReadoutList.cpp \
			CLoggingReadoutList.cpp
libCCUSB_la_CPPFLAGS    = $(COMPILATION_FLAGS)
include_HEADERS         = CCCUSB.h \
			CCCUSBusb.h \
			CCCUSBRemote.h \
			CMockCCUSB.h \
			CCCUSBReadoutList.h \
			CLoggingReadoutList.h

## Normal C++ library containing CCCUSB*
libCCUSB_la_LIBADD	= \
		  @top_builddir@/base/os/libdaqshm.la \
		  @top_builddir@/base/thread/libdaqthreads.la \
			@TCL_LDFLAGS@ \
      @LIBTCLPLUS_LDFLAGS@ \
			@top_builddir@/base/tcpip/libTcp.la \
			@LIBEXCEPTION_LDFLAGS@ \
			@THREADLD_FLAGS@ \
	 		@USB_LIBS@  

## SWIG wrapper generation

libCCCUSB_la_SOURCES  = CCCUSB_wrap.cxx
libCCCUSB_la_CPPFLAGS =$(COMPILATION_FLAGS)
nodist_libCCCUSB_la_SOURECS=CCCUSB_wrap.cxx

libCCCUSBReadoutList_la_SOURCES = CCCUSBReadoutList_wrap.cxx
libCCCUSBReadoutList_la_CPPFLAGS= $(COMPILATION_FLAGS)
nodist_libCCCUSBReadoutList_la_SOURCES = CCCUSBReadoutList_wrap.cxx

BUILT_SOURCES = CCCUSB_wrap.cxx CCCUSBReadoutList_wrap.cxx

CCCUSB_wrap.cxx:	CCCUSB.h
	@SWIG@ -c++ -debug-classes  -tcl  -namespace  -pkgversion 1.0  \
	-outdir @builddir@ \
	@srcdir@/CCCUSB.i

CCCUSBReadoutList_wrap.cxx:  CCCUSBReadoutList.h
		@SWIG@ -c++ -debug-classes  -tcl  -namespace \
		-pkgversion 1.0  -outdir @builddir@ \
		@srcdir@/CCCUSBReadoutList.i



## SWIG Wrapper containing cvmusb::CCCUSB 
libCCCUSB_la_LIBADD = \
			@builddir@/libCCUSB.la \
			@top_builddir@/base/os/libdaqshm.la \
			@top_builddir@/base/thread/libdaqthreads.la \
			@top_builddir@/base/tcpip/libTcp.la         \
			@LIBTCLPLUS_LDFLAGS@                        \
			@LIBEXCEPTION_LDFLAGS@                      \
		 	@TCL_LDFLAGS@                               \
	 		@USB_LIBS@ \
			@THREADLD_FLAGS@

## SWIG Wrapper containing cvmusbreadoutlist::CCCUSBReadoutList
libCCCUSBReadoutList_la_LIBADD = \
			@builddir@/libCCUSB.la \
			@top_builddir@/base/os/libdaqshm.la \
			@THREADLD_FLAGS@ 

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes
	echo "package ifneeded cccusb 1.0 [list load [file join \$$dir libCCCUSB.so]]" >> @libdir@/pkgIndex.tcl
	echo "package ifneeded cccusbreadoutlist 1.0 [list load [file join \$$dir libCCCUSBReadoutList.so]]" >> @libdir@/pkgIndex.tcl



## -------------------------------------------
##          TESTS
## -------------------------------------------

check_PROGRAMS = unittests tests remotetests camactests

## These all link against installed libraries rather than
## the .la files produced before the installation. These are
## supposed to be production test programs.

## Manual test programs for CCUSBusb
tests_SOURCES = TestRunner.cpp \
		regTests.cpp
tests_LDADD = $(CPPUNIT_LDFLAGS) @builddir@/libCCUSB.la \
		@TCL_LDFLAGS@ \
		@LIBTCLPLUS_LDFLAGS@
tests_CPPFLAGS = -I@top_srcdir@/base/thread $(AM_CXXFLAGS)  @PIXIE_CPPFLAGS@
tests_LDFLAGS =  @top_builddir@/base/thread/libdaqthreads.la        \
			@top_builddir@/base/tcpip/libTcp.la         \
			@LIBTCLPLUS_LDFLAGS@                        \
			@LIBEXCEPTION_LDFLAGS@                      \
		   	@TCL_LDFLAGS@                               \
	 		@USB_LIBS@   @THREADLD_FLAGS@ @TCL_LDFLAGS@ \
		 -Wl,"-rpath=@libdir@" 


## Manual test program for CCCUSBRemote
remotetests_SOURCES = TestRunner.cpp \
			remoteRegTests.cpp
remotetests_LDADD = @builddir@/libCCUSB.la        \
		  $(CPPUNIT_LDFLAGS)			\
			@top_builddir@/base/tcpip/libTcp.la         \
			@top_builddir@/base/thread/libdaqthreads.la \
			@LIBTCLPLUS_LDFLAGS@                        \
			@LIBEXCEPTION_LDFLAGS@                      \
		   	@TCL_LDFLAGS@                               \
	 		@USB_LIBS@   @THREADLD_FLAGS@ @TCL_LDFLAGS@


remotetests_CPPFLAGS = $(AM_CXXFLAGS) -I@top_srcdir@/base/thread \
	 @PIXIE_CPPFLAGS@
remotetests_LDFLAGS = @top_builddir@/base/thread/libdaqthreads.la \
	-Wl,"-rpath=@libdir@" 


## These are tests that run automatically at check-TESTS
unittests_SOURCES  = TestRunner.cpp rdolistTests.cpp \
			 loggingrdolistTests.cpp \
			 mockccusbTests.cpp
unittests_LDADD    = @builddir@/libCCUSB.la        \
			$(CPPUNIT_LDFLAGS)                          \
			@top_builddir@/base/tcpip/libTcp.la         \
			@top_builddir@/base/thread/libdaqthreads.la \
			@TCL_LDFLAGS@                               \
			@LIBTCLPLUS_LDFLAGS@

unittests_CXXFLAGS = -I@top_srcdir@/base/thread \
			$(AM_CXXFLAGS)
unittests_LDFLAGS  = -Wl,"-rpath=@libdir@" 

## These are tests that run automatically at check-TESTS
camactests_SOURCES  = TestRunner.cpp camacTests.cpp 
camactests_LDADD    = @builddir@/libCCUSB.la        \
			$(CPPUNIT_LDFLAGS)                          \
			@top_builddir@/base/tcpip/libTcp.la         \
			@top_builddir@/base/thread/libdaqthreads.la \
			@TCL_LDFLAGS@                               \
			@LIBTCLPLUS_LDFLAGS@

camactests_CPPFLAGS = -I@top_srcdir@/base/thread \
			$(AM_CXXFLAGS) @PIXIE_CPPFLAGS@
camactests_LDFLAGS  = -Wl,"-rpath=@libdir@" 

TESTS = ./unittests


clean-local:
	$(RM) -f $(BUILT_SOURCES)


EXTRA_DIST=CCCUSB.i CCCUSBReadoutList.i Asserts.h
