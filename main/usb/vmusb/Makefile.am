SUBDIRS = 	vmusb 		\
		daqconfig	\
		ctlconfig \
		core \
		driverkit	\
		tcldrivers	\
		controlpanels	\
		devtests	\
		vmusbPackage \
		tclservertests \
		sis3316examples


bin_PROGRAMS = VMUSBReadout

VMUSBReadout_CPPFLAGS= \
  @THREADCXX_FLAGS@ \
	-I@top_srcdir@/usb/vmusb/vmusb \
	-I@top_srcdir@/usb/vmusb/core \
	-I@top_srcdir@/usb/vmusb/daqconfig \
	-I@top_srcdir@/usb/vmusb/ctlconfig \
	-I@top_srcdir@/usb/common/slowcontrols	\
	-I@top_srcdir@/usb/common/configurableobject \
	@LIBTCLPLUS_CFLAGS@ \
	-I@top_srcdir@/base/thread	\
	-I@top_srcdir@/base/headers	\
	-I@top_srcdir@/base/dataflow   \
	-I@top_srcdir@/servers/portmanager \
	-I@top_srcdir@/base/os		\
	@TCL_CPPFLAGS@ $(USBSWITCHES)  @PIXIE_CPPFLAGS@


VMUSBReadout_CCFLAGS=@THREADC_FLAGS@

VMUSBReadout_SOURCES = CTheApplication.cpp Globals.cpp 

nodist_VMUSBReadout_SOURCES = cmdline.c cmdline.h

BUILT_SOURCES = cmdline.h cmdline.c

noinst_HEADERS = CTheApplication.h Globals.h event.h



VMUSBReadout_LDADD	= @top_builddir@/usb/vmusb/daqconfig/libVMUSBDaqConfig.la	\
		@top_builddir@/usb/vmusb/ctlconfig/libVMUSBCtlConfig.la	\
		@top_builddir@/usb/vmusb/core/libVMUSBCore.la	\
		@top_builddir@/servers/portmanager/libPortManager.la \
		@top_builddir@/base/dataflow/libDataFlow.la \
		@top_builddir@/usb/vmusb/vmusb/libVMUSB.la	\
		@LIBTCLPLUS_LDFLAGS@	\
		@top_builddir@/base/thread/libdaqthreads.la	\
		@top_builddir@/base/os/libdaqshm.la	\
		$(TCL_LDFLAGS) \
		 $(USB_LIBS) \
		@THREADLD_FLAGS@

VMUSBReadout_LDFLAGS=-Wl,"-rpath=$(libdir)"

cmdline.c: cmdline.h

cmdline.h: commandline.ggo
	$(GENGETOPT) <@srcdir@/commandline.ggo \
		@output-dir=@builddir@/vmusb --set-version=@VERSION@


EXTRA_DIST=commandline.ggo vmusbReadout_user.xml vmusbReadout_man.xml \
	vmusbReadout_man1.xml s800trigger.jpg



install-exec-local:
	$(mkinstalldirs) @prefix@/share/html
	$(INSTALL_DATA) @srcdir@/s800trigger.jpg @prefix@/share/html
