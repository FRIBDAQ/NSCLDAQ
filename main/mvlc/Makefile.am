bin_PROGRAMS=mvlcgenerate

# Library for generator...needed for extended drivers
# to link to.

lib_LTLIBRARIES=libmvlcGenerator.la

libmvlcGenerator_la_SOURCES = CReadoutHardware.cpp CReadoutHardware.h \
	CReadoutModule.cpp CReadoutModule.h \
	DeviceCommand.h DeviceCommand.cpp \
	TCLConfigParser.cpp TCLConfigParser.h \
	CVMUSBReadoutList.cpp  CVMUSBReadoutList.h \
	CVMUSB.cpp CVMUSB.h\
	CStack.cpp CStack.h

libmvlcGenerator_la_CPPFLAGS=@TCL_CPPFLAGS@ -I@top_srcdir@ \
	-I@top_srcdir@/usb/common/configurableobject \
	@LIBTCLPLUS_CFLAGS@

# Put public incldes in $DAQINC/mvlcgenerate

includedir=@prefix@/include/mvlc

# Done this way so we don't need a lib and we don't need to repeat ourselves for
# test sources

DEVICE_MODULES= CMarker.h CMarker.cpp \
	C785.h C785.cpp C3820.h C3820.cpp \
	CDelay.h CDelay.cpp \
	CMADC32.cpp CMADC32.h CMesytecBase.cpp CMesytecBase.h MADC32Registers.h \
	CCAENChain.h CCAENChain.cpp CMADCChain.h CMADCChain.cpp \
	CMADCScaler.h CMADCScaler.cpp CMTDC32.h CMTDC32.cpp \
	C3804.h C3804.cpp \
	CMQDC32RdoHdwr.h CMQDC32RdoHdwr.cpp CMQDC32StackBuilder.h CMQDC32StackBuilder.cpp MQDC32Registers.h \
	CMDPP16QDC.cpp  CMDPP16QDC.h  CMDPP32QDC.cpp  CMDPP32QDC.h  CMDPP32SCP.cpp  CMDPP32SCP.h  CMDPP.h  CMDPPQDC.h \
	C830.h C830.cpp CNADC2530.h CNADC2530.cpp CV977.h CV977.cpp \
	CV1495sc.h CV1495sc.cpp CV1729.h CV1729.cpp \
	CCAENV1x90Opcodes.h CCAENV1x90Registers.h CV1x90.cpp CV1x90.h \
	sis3316.h sis3316Registers.h \
	CXLM.h CXLM.cpp CXLMTimestamp.cpp CXLMTimestamp.h CXLMFERA.cpp CXLMFERA.h \
	VMUSBCommand.h VMUSBCommand.cpp VMUSBListCommand.h VMUSBListCommand.cpp \
	TclWrapper.h TclWrapper.cpp

main_modules=utilities.h utilities.cpp \
	 MVLCConfigParser.cpp \
	$(DEVICE_MODULES) \
	MVLCGenerator.h MVLCGenerator.cpp \
	vme_interface_class.h sis_vmusb_interface.h sis_vmusb_interface.cpp

mvlcgenerate_SOURCES=generate.cpp $(main_modules) 
mvlcgenerate_DEPENDENCIES=libmvlcGenerator.la

mvlcgenerate_CPPFLAGS= -I@top_srcdir@/usb/common/configurableobject \
	-I@top_srcdir@/base/headers -I@top_srcdir@/base/os \
	@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@ @YAML_CFLAGS@ \
	-DMVLC_TEMPLATE=\"@datarootdir@/mvlc/triggeronly.yaml\" \
	-DMVLC_TCLLIBDIR=\"@datarootdir@/mvlc/TclLibs\"

mvlcgenerate_LDFLAGS=@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@ @YAML_LIBS@ @builddir@/libmvlcGenerator.la

include_HEADERS=TCLConfigParser.h  \
	CVMUSBReadoutList.h  \
	CVMUSB.h  \
	MVLCConfigParser.h \
	DeviceCommand.h \
	CReadoutModule.h \
	CReadoutHardware.h


# Build SampleDriver example library -> TclLibs/mvlc
#  A bit of trickyness since we include <mvlc/stuff.h>
# So pointing -I at the top directory will work.
#


noinst_LTLIBRARIES=libSampleDriver.la
libSampleDriver_la_SOURCES=SampleDriver.cpp 

	

libSampleDriver_la_CPPFLAGS=@TCL_CPPFLAGS@ -I@top_srcdir@ \
	-I@top_srcdir@/usb/common/configurableobject \
	@LIBTCLPLUS_CFLAGS@
libSampleDriver_la_LDFALGS=@builddir@/libmvlcGenerator.la
libSampleDriver_DEPENDENCIES=libmvlcGenerator.la


BUILT_SOURCES = generate.cpp options.h options.c
nodist_mvlcgenerate_SOURCES=options.h options.c

options.c: options.h

options.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ --file-name=options <@srcdir@/options.ggo 


noinst_PROGRAMS = unittests

UNIT_TESTS=utiltests.cpp parsertests.cpp \
    stacktests.cpp devcommandtests.cpp rdomoduletests.cpp \
	stackmoduletests.cpp markertests.cpp v785tests.cpp 3820tests.cpp \
	delaytests.cpp madc32tests.cpp caenchaintests.cpp madcchaintests.cpp \
	madcscalertests.cpp mtdctests.cpp mqdctests.cpp mdpptests.cpp sis3804tests.cpp \
	v830tests.cpp hytectests.cpp v977tests.cpp v1495sctests.cpp v1729tests.cpp  \
	v1x90tests.cpp sisvmusbTests.cpp

unittests_SOURCES = TestRunner.cpp \
	$(UNIT_TESTS) \
	options.h options.c \
	$(main_modules)
unittests_CPPFLAGS = @CPPUNIT_CFLAGS@ $(mvlcgenerate_CPPFLAGS)
unittests_LDFLAGS = @CPPUNIT_LIBS@ $(mvlcgenerate_LDFLAGS)

TESTS=unittests

#
# These files are Tcl Drivers in the library we supply:

TCLDRIVERFILES=VMUSBDriverSupport.tcl \
	sis3300defs.tcl  sis330xdriver.tcl sis3300.tcl \
	ACAENV1290.tcl

EXTRA_DIST=options.ggo triggeronly.yaml Readme.md mvlc_translate.xml \
	SampleDriver.cpp SampleDriver.tcl \
	$(TCLDRIVERFILES)

install-exec-local:
	$(mkinstalldirs) @datarootdir@/mvlc
	$(mkinstalldirs) @datarootdir@/mvlc-examples
	$(INSTALL_DATA)  @srcdir@/triggeronly.yaml @datarootdir@/mvlc/triggeronly.yaml
	$(INSTALL_DATA)  @srcdir@/SampleDriver.cpp @datarootdir@/mvlc-examples
	$(INSTALL_SCRIPT) @srcdir@/SampleDriver.tcl @datarootdir@/mvlc-examples
	$(mkinstalldirs) @datarootdir@/mvlc/TclLibs
	$(INSTALL_SCRIPT) $(TCLDRIVERFILES) @datarootdir@/mvlc/TclLibs;
	MVLC_TRANSATOR=1 echo pkg_mkIndex -verbose @datarootdir@/mvlc/TclLibs  "*.tcl" | $(TCLSH_CMD)

