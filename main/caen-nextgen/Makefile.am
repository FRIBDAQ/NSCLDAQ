lib_LTLIBRARIES=libCaenVx2750.la

include_HEADERS=CAENVX2750PhaTrigger.h \
    Dig2Device.h \
    DynamicMultiTrigger.h \
    TclConfiguredReadout.h \
    VX2750EventSegment.h \
    VX2750MultiModuleEventSegment.h \
    VX2750MultiTrigger.h \
    VX2750PHAConfiguration.h \
    VX2750Pha.h \
    VX2750TclConfig.h

libCaenVx2750_la_CPPFLAGS=@BOOST_CPPFLAGS@ @BOOST_EXTRA_CPPFLAGS@ \
    @TCL_CPPFLAGS@ @CAENNG_CPPFLAGS@ -I@top_srcdir@/usb/common/configurableobject \
    -I@top_srcdir@/sbs/readout @JSON_CFLAGS@ @LIBTCLPLUS_CFLAGS@ -I@top_srcdir@/base/os

libCaenVx2750_la_LDFLAGS=@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
    @top_builddir@/sbs/readout/libSBSProductionReadout.la \
    @BOOST_LDFLAGS@ @BOOST_LOG_LIB@  \
    @LIBTCLPLUS_LDFLAGS@ \
    @TCL_LDFLAGS@ @CAENNG_LDFLAGS@ @JSON_LIBS@
    


libCaenVx2750_la_SOURCES= CAENVX2750PhaTrigger.cpp \
    Dig2Device.cpp \
    DynamicMultiTrigger.cpp \
    TclConfiguredReadout.cpp \
    VX2750EventSegment.cpp \
    VX2750MultiModuleEventSegment.cpp \
    VX2750MultiTrigger.cpp \
    VX2750PHAConfiguration.cpp \
    VX2750Pha.cpp \
    VX2750TclConfig.cpp 



noinst_HEADERS=Asserts.h


#  These are test programs but can only be run
#  If there is hardware to run them on.

noinst_PROGRAMS=fejackettests triggertests configtests

ONLINETESTSCPPFLAGS=@CAENNG_CPPFLAGS@ @CPPUNIT_CFLAGS@ @JSON_CFLAGS@ \
    -I@top_srcdir@/base/os
ONLINETESTSLDFLAGS=@builddir@/libCaenVx2750.la @CAENNG_LDFLAGS@ \
     @CPPUNIT_LDFLAGS@ @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@ @JSON_LIBS@ \
     @OPENSSL_LIBS@
ONLINEDEPENDENCIES=@builddir@/libCaenVx2750.la
fejackettests_SOURCES=TestRunner.cpp devtests.cpp vx2750phatests.cpp
fejackettests_CPPFLAGS=$(ONLINETESTSCPPFLAGS)
fejackettests_LDFLAGS=$(ONLINETESTSLDFLAGS)
fejackettests_DEPENDENCIES=$(ONLINEDEPENDENCIES)

triggertests_SOURCES=TestRunner.cpp triggertests.cpp
triggertests_CPPFLAGS=$(ONLINETESTSCPPFLAGS)
triggertests_LDFLAGS=$(ONLINETESTSLDFLAGS)
triggertests_DEPENDENCIES=$(ONLINEDEPENDENCIES)

configtests_SOURCES=TestRunner.cpp configtests.cpp
configtests_CPPFLAGS=$(ONLINETESTSCPPFLAGS) \
    -I@top_srcdir@/usb/common/configurableobject
configtests_LDFLAGS=@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
    $(ONLINETESTSLDFLAGS)
configtests_DEPENDENCIES=$(ONLINEDEPENDENCIES)    


install-data-local:
	$(mkinstalldirs) @datarootdir@/recipes
	$(mkinstalldirs) @datarootdir@/recipes/caen-nextgen
	$(INSTALL_DATA) @srcdir@/tclreadout/* \
			@datarootdir@/recipes/caen-nextgen
    

#
#  This target requires two environment/Makefile variables:
#    TEST_MODULE_ISUSB - Nonzero if the connection to the module
#       used for testing is a USB direct connection. 0 if ethernet.
#    TEST_MODULE_CONNECTION - the connection string to the module. 
#       For usb modules, this is the module id from the sticker on the PCB
#       For Ethernet modules, this is the IP address.
#
ONLINE_TESTS:
	@builddir@/fejackettests $(TEST_MODULE_CONNECTION) $(TEST_MODULE_ISUSB)
	@builddir@/triggertests  $(TEST_MODULE_CONNECTION) $(TEST_MODULE_ISUSB)
	@builddir@/configtests  $(TEST_MODULE_CONNECTION) $(TEST_MODULE_ISUSB)

EXTRA_DIST=tclreadout


