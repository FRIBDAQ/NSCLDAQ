noinst_PROGRAMS = unittests

unittests_CPPFLAGS=@TCL_CPPFLAGS@

unittests_SOURCES = TestRunner.cpp cvmusbtest.cpp cmqdc32test.cpp cmqdc32stacktest.cpp

unittests_DEPENDENCIES = @top_builddir@/usb/vmusb/daqconfig/libVMUSBDaqConfig.la \
	@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
	$(NSCLDAQ_LIB) 

unittests_LDADD = @top_builddir@/usb/vmusb/daqconfig/libVMUSBDaqConfig.la \
	@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
	$(NSCLDAQ_LIB) @CPPUNIT_LDFLAGS@ @THREADLD_FLAGS@

unittests_CXXFLAGS = @TCL_CPPFLAGS@ $(CPPUNIT_INCLUDES) $(THREADCXX_FLAGS) $(AM_CXXFLAGS) \
	 -I@top_srcdir@/usb/common/configurableobject \
	-I@srcdir@/../. -I@srcdir@/../daqconfig/. \
	-I@srcdir@/../vmusb/. -I@srcdir@/../core \
	-I@top_srcdir@/base/thread

unittests_LDFLAGS = @LIBTCLPLUS_LDFLAGS@  $(TCL_LDFLAGS) \
                  -Wl,"-rpath=$(libdir)" -Wl,"-rpath=$(daqdir)/lib" \
		@top_builddir@/base/thread/libdaqthreads.la

check-TESTS:
	./unittests 

EXTRA_DIST=TestRunner.cpp $(TESTS) Asserts.h
