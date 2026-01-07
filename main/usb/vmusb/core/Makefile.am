lib_LTLIBRARIES		= libVMUSBCore.la
libVMUSBCore_la_SOURCES	= CAcquisitionThread.cpp 	\
			CControlQueues.cpp \
			COutputThread.cpp \
			CRunState.cpp \
			DataBuffer.cpp  \
			CSystemControl.cpp \
			CBeginRun.cpp \
			CEndRun.cpp \
			CPauseRun.cpp \
			CResumeRun.cpp \
			CMonVar.cpp \
			CInit.cpp \
			CExit.cpp  \
			TclServer.cpp \
			Server.cpp \
			serverinstance.cpp \
			CRunStateCommand.cpp  \
			CGetCommand.cpp  \
			CSetCommand.cpp  \
			CUpdateCommand.cpp  \
			CStatisticsCommand.cpp \
			tclUtil.cpp

noinst_HEADERS		= CAcquisitionThread.h		\
			CControlQueues.h \
			COutputThread.h \
			CRunState.h \
			DataBuffer.h \
			CSystemControl.h \
			CBeginRun.h \
			CEndRun.h \
			CPauseRun.h \
			CResumeRun.h \
			CMonVar.h	\
			CInit.h \
			CExit.h  \
			TclServer.h \
			server.h \
			serverinstance.h \
			CRunStateCommand.h \
			CGetCommand.h  \
			CSetCommand.h  \
			CUpdateCommand.h  \
			CStatisticsCommand.h \
			tclUtil.h

libVMUSBCore_la_CPPFLAGS = \
		  @THREADCXX_FLAGS@ \
			-I@top_srcdir@/usb/vmusb/vmusb		\
			-I@top_srcdir@/usb/vmusb/daqconfig	\
			-I@top_srcdir@/usb/vmusb/ctlconfig	\
			-I@top_srcdir@/usb/common/configurableobject \
			-I@top_srcdir@/usb/common/slowcontrols	\
			-I@top_srcdir@/usb/vmusb	\
			@TCL_FLAGS@ \
			@LIBTCLPLUS_CFLAGS@		\
			-I@top_srcdir@/base/headers	\
			-I@top_srcdir@/base/dataflow	\
			-I@top_srcdir@/base/thread	\
			-I@top_srcdir@/base/os \
			-I@top_srcdir@/daq/format	\
			-I@top_srcdir@/daq/eventbuilder \
			@OPENSSL_INCLUDES@ @PIXIE_CPPFLAGS@ \
			@UFMT_CPPFLAGS@ @VMUSB_WORKAROUNDS@

# add -DREPORT_ERRORS to INCLUDES below for verbose
# error reporting info.



libVMUSBCore_la_LIBADD	= \
	@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@top_builddir@/usb/vmusb/daqconfig/libVMUSBDaqConfig.la	\
	@top_builddir@/usb/vmusb/ctlconfig/libVMUSBCtlConfig.la	\
  @top_builddir@/usb/vmusb/vmusb/libVMUSB.la		\
	@LIBTCLPLUS_LDFLAGS@					\
			@top_builddir@/base/thread/libdaqthreads.la \
			@top_builddir@/base/os/libdaqshm.la	\
			@top_builddir@/daq/format/libdataformat.la	\
			@TCL_LDFLAGS@ \
			@THREADLD_FLAGS@   \
			@OPENSSL_LDFLAGS@ @OPENSSL_LIBS@


#-----------------------------------------------------------------------------------
# Tests:

check_PROGRAMS = unittests

unittests_SOURCES  = TestRunner.cpp \
	monvartests.cpp \
	monvartimertests.cpp \
	monvarcmdtests.cpp	\
	CMonVar.cpp DataBuffer.cpp Asserts.h
unittests_LDADD    = @top_builddir@/base/thread/libdaqthreads.la   \
	$(LIBTCLPLUS_LDFLAGS) $(TCL_LDFLAGS)  $(CPPUNIT_LDFLAGS)

unittests_CPPFLAGS = -I@top_srcdir@/base/thread	\
	-I@top_srcdir@/daq/format		\
	$(LIBTCLPLUS_CFLAGS) $(CPPUNIT_CFLAGS) $(TCL_FLAGS)  @PIXIE_CPPFLAGS@


TESTS=unittests
