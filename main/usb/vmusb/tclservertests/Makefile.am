noinst_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp \
		mxdcrcbustest.cpp   \
		modulecommandtest.cpp \
		cgdgtest.cpp  \
		cv6533test.cpp  \
		cchicotriggertest.cpp \
		ctclcontrolmoduletest.cpp \
		cxlmcontrolstest.cpp \
		cvmusbmoduletest.cpp \
		cwienermdgg16test.cpp \
		cmdgg16controltest.cpp 


unittests_LDADD = @top_builddir@/usb/vmusb/core/libVMUSBCore.la \
		@top_builddir@/usb/vmusb/ctlconfig/libVMUSBCtlConfig.la \
		@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
		@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
		@LIBTCLPLUS_LDFLAGS@ \
		$(CPPUNIT_LDFLAGS) \
		@TCL_LDFLAGS@ \
		@top_builddir@/base/thread/libdaqthreads.la 

unittests_CPPFLAGS = -I@top_srcdir@/usb/vmusb/vmusb  \
		 -I@top_srcdir@/usb/vmusb/ctlconfig  \
		 -I@top_srcdir@/usb/vmusb/core  \
		 -I@top_srcdir@/usb/common/configurableobject \
		 -I@top_srcdir@/usb/common/slowcontrols \
		 $(CPPUNIT_FLAGS) \
		 -I@top_srcdir@/base/thread \
		 @LIBTCLPLUS_CFLAGS@ \
		 @TCL_FLAGS@ @PIXIE_CPPFLAGS@


check-TESTS : $(UNITTEST_MODULES)
		./unittests


EXTRA_DIST = FakeGlobals.h
