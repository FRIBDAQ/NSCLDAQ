check_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp  modulecommandtest.cpp \
		c894test.cpp cph7106test.cpp \
		cccusbcontroltest.cpp\
		ctclmoduletest.cpp

unittests_LDADD = @top_builddir@/usb/ccusb/ccusb/libCCUSB.la \
		@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
		@top_builddir@/usb/ccusb/ctlconfig/libCCUSBCtlConfig.la \
		@top_builddir@/usb/ccusb/core/libCCUSBCore.la \
		@top_builddir@/base/tcpip/libTcp.la                   \
		@LIBTCLPLUS_LDFLAGS@                                  \
		@LIBEXCEPTION_LDFLAGS@                                \
		$(CPPUNIT_LDFLAGS) \
		@TCL_LDFLAGS@ \
		@top_builddir@/base/thread/libdaqthreads.la 

unittests_CPPFLAGS = -I@top_srcdir@/usb/ccusb/ccusb  \
		 -I@top_srcdir@/usb/common/configurableobject \
		 -I@top_srcdir@/usb/common/slowcontrols \
		 $(CPPUNIT_FLAGS) \
		 -I@top_srcdir@/usb/ccusb/ctlconfig \
		 -I@top_srcdir@/base/thread \
		 @LIBTCLPLUS_CFLAGS@ \
		 @TCL_FLAGS@ @PIXIE_CPPFLAGS@


TESTS = unittests


EXTRA_DIST = FakeGlobals.h
