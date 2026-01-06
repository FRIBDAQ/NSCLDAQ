check_PROGRAMS = unittests


TESTS = cccusbtest.cpp

unittests_SOURCES = TestRunner.cpp $(TESTS) 

unittests_LDADD =  \
		@top_builddir@/usb/ccusb/ccusb/libCCUSB.la            \
		@top_builddir@/usb/ccusb/daqconfig/libCCUSBDaqConfig.la  \
		@top_builddir@/base/thread/libdaqthreads.la		\
		@top_builddir@/base/tcpip/libTcp.la                   \
		@top_builddir@/usb/common/configurableobject/libConfigurableObject.la	\
		@LIBTCLPLUS_LDFLAGS@                                  \
		@LIBEXCEPTION_LDFLAGS@                                \
		@TCL_LDFLAGS@				              \
		@CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@


unittests_CPPFLAGS = \
		-I@top_srcdir@/usb/common/configurableobject	\
		@TCL_CPPFLAGS@ \
		$(CPPUNIT_INCLUDES) \
		$(THREADCXX_FLAGS) \
		$(AM_CXXFLAGS) \
		-I@top_srcdir@/usb/ccusb \
		-I@top_srcdir@/usb/ccusb/daqconfig \
		-I@top_srcdir@/usb/ccusb/ccusb \
		-I@prefix@/include

unittests_LDFLAGS =  -Wl,"-rpath=$(libdir)" -Wl,"-rpath=$(daqdir)/lib" 

check-TESTS:
	./unittests 

EXTRA_DIST=TestRunner.cpp $(TESTS) Asserts.h
