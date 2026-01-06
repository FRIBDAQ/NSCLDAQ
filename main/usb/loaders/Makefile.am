bin_PROGRAMS=ccusbloader vmusbloader xlmload

noinst_HEADERS=libxxusb.h skipHeader.h Asserts.h

ccusbloader_SOURCES=ccusb_flash.cpp skipHeader.cpp libxxusb.cpp

vmusbloader_SOURCES=vmusb_flash.cpp skipHeader.cpp libxxusb.cpp

ccusbloader_LDFLAGS=@USB_LIBS@

vmusbloader_LDFLAGS=@USB_LIBS@


xlmload_SOURCES = xlmloadparams.c xlmload.cpp 

xlmload_CPPFLAGS = $(TCLFLAGS) -I@top_srcdir@/usb/vmusb/core \
	-I@top_srcdir@/usb/vmusb/daqconfig \
	-I@top_srcdir@/usb/vmusb/vmusb -I@top_srcdir@/base/thread \
	@LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@ 
xlmload_LDFLAGS=@top_builddir@/usb/vmusb/vmusb/libVMUSBRemote.la \
	@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
	@top_builddir@/usb/vmusb/daqconfig/libVMUSBDaqConfig.la \
	@LIBTCLPLUS_LDFLAGS@ \
	@top_builddir@/usb/vmusb/VMUSBReadout-Globals.o \
	@top_builddir@/base/thread/libdaqthreads.la  \
	-Wl,"-rpath=@libdir@"

xlmloadparams.c : xlmloadparams.h

xlmloadparams.h: xlmloadparams.ggo
	$(GENGETOPT) < @srcdir@/xlmloadparams.ggo --output-dir=@builddir@

#-----------------------------------------------------------------------------
# Unit tests:
#

check_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp skiptest.cpp skipHeader.cpp
unittests_LDADD= $(CPPUNIT_LDFLAGS)
unittests_CPPFLAGS=-DSRCDIR='"@srcdir@/"' @PIXIE_CPPFLAGS@

# NOTE: These .bit/.bin files don't purport to be the latest they are
#       only here to support unit tests for skipHeader.
#

EXTRA_DIST=ccusb_0600_072012.bit  ccusb_0603_123117.bit  \
	vmeusb_0A00_031913.bin \
	ccusb_0602_071916.bit  ccusb_122917.bit \
	usbloaders.xml xlmloadparams.ggo


TESTS=./unittests
