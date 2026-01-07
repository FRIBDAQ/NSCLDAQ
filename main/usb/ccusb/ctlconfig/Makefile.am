lib_LTLIBRARIES			= libCCUSBCtlConfig.la

libCCUSBCtlConfig_la_SOURCES = \
				  CCtlConfiguration.cpp \
          CModuleCommand.cpp	\
					DriverTemplate.cpp	\
					C894.cpp		\
					CPH7106.cpp		\
					CWatchCommand.cpp	\
					CCAEN894Creator.cpp     \
					CPH7106Creator.cpp	\
					CTclControlModule.cpp	\
					CTclModuleCreator.cpp   \
					CCCUSBControl.cpp	\
					CCCUSBControlCreator.cpp	

noinst_HEADERS			=	 \
				  CCtlConfiguration.h \
					CControlHardware.h	\
					CControlModule.h	\
					CModuleCommand.h	\
					DriverTemplate.h	\
					C894.h			\
					CPH7106.h		\
					CWatchCommand.h		\
					CModuleFactory.h	\
					CModuleCreator.h	\
					CCAEN894Creator.h	\
					CPH7106Creator.h	\
					CTclControlModule.h	\
					CTclModuleCreator.h     \
					CCCUSBControl.h         \
					CCCUSBControlCreator.h 


libCCUSBCtlConfig_la_CPPFLAGS	= -I@top_srcdir@/usb/ccusb		\
					-I@top_srcdir@/daq/format	\
					-I@top_srcdir@/usb/ccusb/ccusb	\
					-I@top_srcdir@/usb/ccusb/daqconfig \
					-I@top_srcdir@/usb/ccusb/rdothread \
					-I@top_srcdir@/usb/ccusb/router	\
					-I@top_srcdir@/usb/common/slowcontrols \
				-I@top_srcdir@/usb/common/configurableobject \
					@LIBTCLPLUS_CFLAGS@	\
					-I@top_srcdir@/base/headers	\
					-I@top_srcdir@/base/thread	\
					-I@top_srcdir@/base/os		\
					@TCL_FLAGS@ @THREADCXX_FLAGS@ \
					 @PIXIE_CPPFLAGS@

libCCUSBCtlConfig_la_LIBADD	= \
	@top_builddir@/usb/ccusb/ccusb/libCCUSB.la          \
	@top_builddir@/usb/ccusb/daqconfig/libCCUSBDaqConfig.la \
	@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@top_builddir@/base/tcpip/libTcp.la			\
	@TCL_LDFLAGS@						\
	@LIBTCLPLUS_LDFLAGS@					\
	@top_builddir@/base/os/libdaqshm.la			\
	@top_builddir@/base/thread/libdaqthreads.la		\
	@THREADLD_FLAGS@ 

install-data-local:
	$(mkinstalldirs) @prefix@/ccusbdriver
	$(mkinstalldirs) @prefix@/ccusbdriver/includes
	$(mkinstalldirs) @prefix@/ccusbdriver/example
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/ccusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/sampleDriver.cpp @prefix@/ccusbdriver/example
	$(INSTALL_DATA)  @builddir@/driver_Makefile @prefix@/ccusbdriver/example/Makefile

EXTRA_DIST=driver_Makefile.in sampleDriver.cpp

## UNIT TESTS
check_PROGRAMS = unittests

unittests_SOURCES = \
			TestRunner.cpp \
		cctlconfigurationtest.cpp \
		cccusbcontroltests.cpp

unittests_CPPFLAGS = \
	    -I@srcdir@ \
	    -I@top_srcdir@/base/headers \
	    -I@top_srcdir@/base/testutils \
	    -I@top_srcdir@/usb/ccusb/ccusb \
	    -I@top_srcdir@/usb/ccusb/ctlconfig \
	    -I@top_srcdir@/usb/ccusb \
	    -I@top_srcdir@/usb/common/slowcontrols \
	    -I@top_srcdir@/usb/common/configurableobject \
			@TCL_FLAGS@ \
			@LIBTCLPLUS_CFLAGS@ \
			$(CPPUNIT_INCLUDES)  @PIXIE_CPPFLAGS@

unittests_LDADD = \
	@builddir@/libCCUSBCtlConfig.la \
	@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
			@TCL_LDFLAGS@ \
			@LIBTCLPLUS_LDFLAGS@  \
			@CPPUNIT_LDFLAGS@

unittests_DEPENDENCIES = \
			@builddir@/libCCUSBCtlConfig.la 

TESTS = unittests

