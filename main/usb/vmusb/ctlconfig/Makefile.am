lib_LTLIBRARIES			= libVMUSBCtlConfig.la libUserDriver.la
bin_PROGRAMS = sets800triggerrun

COMPILATION_FLAGS=@THREADCXX_FLAGS@ -I@top_srcdir@/usb/vmusb/vmusb	\
					-I@top_srcdir@/usb/common/configurableobject \
					-I@top_srcdir@/usb/common/slowcontrols \
					-I@top_srcdir@/daq/format  \
					-I@top_srcdir@/usb/vmusb/daqconfig \
					-I@top_srcdir@/usb/vmusb/core	\
					-I@top_srcdir@/usb/vmusb	\
					@LIBTCLPLUS_CFLAGS@	\
					-I@top_srcdir@/base/headers \
					-I@top_srcdir@/base/thread	\
					-I@top_srcdir@/base/os		\
					@TCL_FLAGS@ @PIXIE_CPPFLAGS@

libVMUSBCtlConfig_la_CPPFLAGS	= $(COMPILATION_FLAGS)

libVMUSBCtlConfig_la_SOURCES	= \
					CCtlConfiguration.cpp \
					CWatchCommand.cpp	\
					CMonCommand.cpp         \
					CModuleCommand.cpp	\
					CGDG.cpp		\
					CV812.cpp		\
					CVMUSBModule.cpp	\
					CV6533.cpp		\
					CJtecgdgCreator.cpp	\
					CV812Creator.cpp	\
					CVMUSBCreator.cpp	\
					CV6533Creator.cpp	\
					CXLMControls.cpp	\
					CXLMControlsCreator.cpp \
					CMxDCRCBus.cpp          \
					CMxDCRCBusCreator.cpp   \
					ChicoTrigger.cpp         \
					CChicoTriggerCreator.cpp \
					CWienerMDGG16.cpp        \
					CMDGG16Control.cpp       \
					CMDGG16ControlCreator.cpp \
					CTclModuleCreator.cpp     \
					CTclControlModule.cpp	  \
					CMarkerObject.cpp \
					CMarkerCreator.cpp \
					CMxDCReset.cpp \
					CMxDCResetCreator.cpp

noinst_HEADERS			=	\
					CCtlConfiguration.h \
					CWatchCommand.h		\
				  CMonCommand.h \
				  CModuleCommand.h \
					CModuleFactory.h	\
					CModuleCreator.h	\
					CControlHardware.h	\
					CControlModule.h	\
					CGDG.h			\
					CV812.h			\
					CV6533.h		\
					CVMUSBModule.h		\
					CJtecgdgCreator.h       \
					CV812Creator.h		\
					CVMUSBCreator.h		\
					CV6533Creator.h		\
					CTclModuleCreator.h	\
					CXLMControls.h	        \
					CXLMControlsCreator.h  \
					CMxDCRCBus.h           \
					CMxDCRCBusCreator.h    \
					ChicoTrigger.h         \
					CChicoTriggerCreator.h \
					CWienerMDGG16.h        \
					CMDGG16Control.h       \
					CMDGG16ControlCreator.h \
					CTclControlModule.h	 \
					CMarkerObject.h \
					CMarkerCreator.h \
					CMxDCReset.h \
					CMxDCResetCreator.h



libVMUSBCtlConfig_la_LIBADD	= \
					@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
					@top_builddir@/usb/vmusb/daqconfig/libVMUSBDaqConfig.la \
					@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
					@LIBTCLPLUS_LDFLAGS@	\
					@top_builddir@/base/os/libdaqshm.la	\
					@top_builddir@/base/thread/libdaqthreads.la \
					@TCL_LDFLAGS@ @THREADLD_FLAGS@

libUserDriver_la_SOURCES  = CUserDriverSample.cpp

libUserDriver_la_CPPFLAGS = $(COMPILATION_FLAGS)

install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	$(mkinstalldirs) @prefix@/vmusbdriver/example
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/vmusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/CUserDriverSample.cpp @prefix@/vmusbdriver/example

# S800 trigger run number setter.

sets800triggerrun_SOURCES=sets800triggerrun.cpp \
	@srcdir@/../daqconfig/s800TriggerRegisters.cpp

sets800triggerrun_CPPFLAGS = @JSON_CFLAGS@ -I@srcdir@/../daqconfig \
	-I@srcdir@/../vmusb \
	-DDEFAULT_S800REGFILE=\"@datarootdir@/s800triggers/RegisterFile.json\"

sets800triggerrun_LDFLAGS=@JSON_LIBS@ @builddir@/../vmusb/libVMUSBRemote.la \
	@builddir@/../vmusb/libCVMUSBReadoutList.la

EXTRA_DIST = WienerMDGG16Registers.h

## UNIT TESTS
noinst_PROGRAMS = unittests

unittests_SOURCES = \
			TestRunner.cpp \
		cctlconfigurationtest.cpp

unittests_CPPFLAGS = \
	    -I@top_srcdir@/base/headers \
	    -I@top_srcdir@/base/testutils \
	    -I@top_srcdir@/usb/vmusb/vmusb \
	    -I@top_srcdir@/usb/common/slowcontrols \
	    -I@top_srcdir@/usb/common/configurableobject \
			-I@srcdir@ \
			@TCL_FLAGS@ \
			@LIBTCLPLUS_CFLAGS@ \
			$(CPPUNIT_INCLUDES) @PIXIE_CPPFLAGS@

unittests_LDADD = \
			@builddir@/libVMUSBCtlConfig.la \
			@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
			@TCL_LDFLAGS@ \
			@LIBTCLPLUS_LDFLAGS@  \
			@CPPUNIT_LDFLAGS@

unittests_DEPENDENCIES = \
			@builddir@/libVMUSBCtlConfig.la 

check-TESTS: 
		./unittests

