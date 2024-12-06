lib_LTLIBRARIES		= libVMUSB.la libVMUSBRemote.la libCVMUSB.la \
	libCVMUSBReadoutList.la

COMPILATION_FLAGS	= $(USBSWITCHES) -I@top_srcdir@/base/headers @LIBTCLPLUS_CFLAGS@  \
			-I@top_srcdir@/base/tcpip \
			-I@top_srcdir@/base/os	\
			-I@top_srcdir@/base/thread \
			  @TCL_FLAGS@ @THREADCXX_FLAGS@ @PIXIE_CPPFLAGS@ \
			-I@srcdir@/.. -I@top_srcdir@/base/os

libVMUSB_la_SOURCES 	=  CVMUSBusb.cpp CVMUSBReadoutList.cpp CVMUSBEthernet.cpp \
	CVMUSBFactory.cpp \
	CVMUSB.cpp \
	CMockVMUSB.cpp \
	CLoggingReadoutList.cpp sis_vmusb_interface.cpp

libVMUSB_la_CPPFLAGS=$(COMPILATION_FLAGS)

libVMUSBRemote_la_SOURCES = CVMUSBReadoutList.cpp \
	CVMUSBRemote.cpp \
	CVMUSBEthernet.cpp

libVMUSBRemote_la_CPPFLAGS=$(COMPILATION_FLAGS)


noinst_HEADERS		= CVMUSB.h   \
	CVMUSBReadoutList.h  \
	CVMUSBusb.h	\
	CVMUSBEthernet.h \
	CVMUSBRemote.h       \
	CVMUSBFactory.h \
	CMockVMUSB.h \
	CLoggingReadoutList.h \
	vme_interface_class.h sis_vmusb_interface.h


libVMUSB_CXXFLAGS=@THREADCXX_FLAGS@ -I@srcdir@/.. -I@top_srcdir@/base/os

libVMUSB_la_LIBADD       = @top_builddir@/base/tcpip/libTcp.la \
			@top_builddir@/base/thread/libdaqthreads.la \
			@LIBTCLPLUS_LDFLAGS@			\
			@top_builddir@/base/os/libdaqshm.la	\
				$(USB_LIBS)  

AM_CPPFLAGS		= $(USBSWITCHES) -I@top_srcdir@/base/headers @LIBTCLPLUS_CFLAGS@  \
			-I@top_srcdir@/base/tcpip \
			-I@top_srcdir@/base/os \
			  @TCL_FLAGS@



libVMUSBRemote_la_LIBADD = @builddir@/libVMUSB.la \
	@TCL_LDFLAGS@ \
	@top_builddir@/base/tcpip/libTcp.la \
	@LIBEXCEPTION_LDFLAGS@  @THREADLD_FLAGS@

#IVP tests:

bin_PROGRAMS = tests

tests_SOURCES = @srcdir@/TestRunner.cpp \
		@srcdir@/regTests.cpp \
		@srcdir@/vmeTests.cpp \
		@srcdir@/Asserts.h


tests_CPPFLAGS = -std=c++11 -g -I@prefix@/include -I@top_srcdir@/base/thread \
		$(CPPUNIT_INCLUDES)  @PIXIE_CPPFLAGS@
tests_LDADD = @top_builddir@/base/thread/libdaqthreads.la \
		$(CPPUNIT_LDFLAGS) \
		@builddir@/libVMUSB.la \
		 @TCL_LDFLAGS@ \
		 @LIBTCLPLUS_LDFLAGS@

#tests: $(TEST_MODULES)
#	$(CXX) -g -o tests -I@srcdir@ -I@prefix@/include $(CPPUNIT_INCLUDES) \
#	$(TEST_MODULES) $(CPPUNIT_LDFLAGS) -L@prefix@/lib -lVMUSB \
#	-Wl,"-rpath=@prefix@/lib" @TCL_LDFLAGS@ @LIBTCLPLUS_LDFLAGS@


REMOTE_TEST_MODULES = @srcdir@/TestRunner.cpp \
             @srcdir@/remoteRegTests.cpp \
             @srcdir@/remoteVMETests.cpp 

remotetests: $(REMOTE_TEST_MODULES)
	$(CXX) -g -o remotetests -I@srcdir@ -I@prefix@/include \
	$(CPPUNIT_INCLUDES)  @PIXIE_CPPFLAGS@ \
	$(REMOTE_TEST_MODULES) $(CPPUNIT_LDFLAGS) -L@prefix@/lib -lVMUSB \
	-lVMUSBRemote \
	-Wl,"-rpath=@prefix@/lib" @TCL_LDFLAGS@ @LIBTCLPLUS_LDFLAGS@

UNITTEST_MODULES = @srcdir@/TestRunner.cpp \
		@srcdir@/vmusbrdolisttests.cpp \
		@srcdir@/loggingrdolisttests.cpp \
		@srcdir@/mockvmusbtests.cpp

noinst_PROGRAMS = unittests
unittests_SOURCES = $(UNITTEST_MODULES)

unittests_CPPFLAGS = -I@srcdir@ -I@prefix@/include \
	$(CPPUNIT_INCLUDES)  @PIXIE_CPPFLAGS@
unittests_LDADD = $(CPPUNIT_LDFLAGS) \
	 	@builddir@/libVMUSB.la  \
		@TCL_LDFLAGS@ @LIBTCLPLUS_LDFLAGS@
unittests_LDFLAGS = -Wl,"-rpath=@prefix@/lib"

installcheck-local: unittests
	./unittests



install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	for f in @srcdir@/CVMUSB*.h; do $(INSTALL_DATA)  $$f  @prefix@/vmusbdriver/includes; done
	echo "Registering swig packages with lib's pkgIndex.tcl"
	echo "package ifneeded cvmusbreadoutlist 1.0 [list load [file join \$$dir libCVMUSBReadoutList.so]]" >> @libdir@/pkgIndex.tcl
	echo "package ifneeded cvmusb 1.0 [list load [file join \$$dir libCVMUSB.so]]" >> @libdir@/pkgIndex.tcl


clean-local:
	rm -f $(BUILT_SOURCES)


check-TESTS :  unittests
	./unittests

# Swig interface generation:

libCVMUSB_la_SOURCES = @srcdir@/CVMUSB_wrap.cxx
libCVMUSB_la_LIBADD  = @builddir@/libVMUSB.la @THREADLD_FLAGS@

libCVMUSB_la_CPPFLAGS=$(COMPILATION_FLAGS)

BUILT_SOURCES=@srcdir@/CVMUSB_wrap.cxx @srcdir@/CVMUSBReadoutList_wrap.cxx

@srcdir@/CVMUSB_wrap.cxx:	@srcdir@/CVMUSB.h
	@SWIG@ -c++ -outdir @srcdir@ -debug-classes -tcl -namespace -pkgversion 1.0 @srcdir@/CVMUSB.i

libCVMUSBReadoutList_la_SOURCES = @srcdir@/CVMUSBReadoutList_wrap.cxx
libCVMUSBReadoutList_la_LIBADD = @builddir@/libVMUSB.la @THREADLD_FLAGS@

libCVMUSBReadoutList_la_CPPFLAGS=$(COMPILATION_FLAGS)


@srcdir@/CVMUSBReadoutList_wrap.cxx: @srcdir@/CVMUSBReadoutList.h
	@SWIG@ -c++ -outdir @srcdir@ -debug-classes -tcl -namespace -pkgversion 1.0 \
	 @srcdir@/CVMUSBReadoutList.i 

#  Stuff that needs to go int the distro:
EXTRA_DIST = $(TEST_MODULES) $(UNITTEST_MODULES) @srcdir@/CVMUSB.i @srcdir@/CVMUSBReadoutList.i
