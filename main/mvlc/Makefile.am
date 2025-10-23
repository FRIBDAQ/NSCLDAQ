##
#  A bit about this makefile.  once it's fully done,
# The device support compiled modules will all come from
# main/usb/vmusb/daqconfig.  The modules in the IMPORTED_MODULES
# variable have been imported.  When compiled
# with this makefile, those modules have the preprocessor symbol
# MVLC_GENERATOR defined and ifedefs are sprinkled through the modules
# so that they can work correctly in both frameworks.
#  This comes at a cost, however.  Specifically if not done
# carefully, changes to main/usb/vmusb/daqconfig modules can trigger
# compilation errors here.  Such problems are usually remedied
# by appropriate #ifdefery.  This means that you may need to udnerstand
# how this framework differs from the VMUSBRedout framework.

bin_PROGRAMS=mvlcgenerate mvlc_xlmload

# I had an out of tree build fail in a weird way that
# makes me believe this is not fully parallel safe.
# Specifically a link of the tests claimed
# that the libmvlcGenerator.la was not present
# but after the dust settled it was...and  clean
# build serial and parallel worked....

NOTPARALLEL:

#------------------ build the generator:

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

IMPORT_DIR=@top_srcdir@/usb/vmusb/daqconfig
IMPORTED_MODULES=$(IMPORT_DIR)/CMQDC32RdoHdwr.h \
	$(IMPORT_DIR)/CMQDC32RdoHdwr.cpp \
	$(IMPORT_DIR)/CMQDC32StackBuilder.h \
	$(IMPORT_DIR)/CMQDC32StackBuilder.cpp \
	$(IMPORT_DIR)/MQDC32Registers.h \
	$(IMPORT_DIR)/C785.cpp $(IMPORT_DIR)/C785.h \
	$(IMPORT_DIR)/C830.cpp $(IMPORT_DIR)/C830.h \
	$(IMPORT_DIR)/C3804.cpp $(IMPORT_DIR)C3804.h \
	$(IMPORT_DIR)/C3820.cpp $(IMPORT_DIR)C3820.h \
	$(IMPORT_DIR)/CCAENChain.h $(IMPORT_DIR)/CCAENChain.cpp \
	$(IMPORT_DIR)/CCAENV1x90Opcodes.h $(IMPORT_DIR)/CCAENV1x90Registers.h\
	$(IMPORT_DIR)/CDelay.h $(IMPORT_DIR)/CDelay.cpp \
	$(IMPORT_DIR)/CMesytecBase.cpp $(IMPORT_DIR)/CMesytecBase.h \
	$(IMPORT_DIR)/CMADC32.cpp $(IMPORT_DIR)/CMADC32.h  


LOCAL_MODULES= CMarker.h CMarker.cpp \
	MADC32Registers.h \
	CMADCChain.h CMADCChain.cpp \
	CMADCScaler.h CMADCScaler.cpp CMTDC32.h CMTDC32.cpp \
	CMDPP16QDC.cpp  CMDPP16QDC.h  CMDPP32QDC.cpp  CMDPP32QDC.h \
	CMDPP32SCP.cpp  CMDPP32SCP.h  CMDPP.h  CMDPPQDC.h \
	CNADC2530.h CNADC2530.cpp CV977.h CV977.cpp \
	CV1495sc.h CV1495sc.cpp CV1729.h CV1729.cpp \
	CCAENV1x90Opcodes.h CCAENV1x90Registers.h CV1x90.cpp CV1x90.h \
	sis3316.h sis3316Registers.h \
	CXLM.h CXLM.cpp CXLMTimestamp.cpp CXLMTimestamp.h \
	CXLMFERA.cpp CXLMFERA.h \
	VMUSBCommand.h VMUSBCommand.cpp VMUSBListCommand.h \
	VMUSBListCommand.cpp \
	TclWrapper.h TclWrapper.cpp

main_modules=utilities.h utilities.cpp \
	 MVLCConfigParser.cpp \
	$(LOCAL_MODULES) \
	$(IMPORTED_MODULES) \
	MVLCGenerator.h MVLCGenerator.cpp \
	vme_interface_class.h sis_vmusb_interface.h sis_vmusb_interface.cpp

mvlcgenerate_SOURCES=generate.cpp $(main_modules) 
mvlcgenerate_DEPENDENCIES=@builddir@/libmvlcGenerator.la

mvlcgenerate_CPPFLAGS= -I@srcdir@ -I@top_srcdir@/usb/common/configurableobject \
	-I@top_srcdir@/base/headers -I@top_srcdir@/base/os \
	@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@ @YAML_CFLAGS@ \
	-DMVLC_TEMPLATE=\"@datarootdir@/mvlc/triggeronly.yaml\" \
	-DMVLC_TCLLIBDIR=\"@datarootdir@/mvlc/TclLibs\" \
	-DMVLC_GENERATOR -I@top_srcdir@/usb/vmusb/daqconfig

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
libSampleDriver_la_LDFLAGS=@builddir@/libmvlcGenerator.la
libSampleDriver_la_DEPENDENCIES=libmvlcGenerator.la


BUILT_SOURCES = generate.cpp options.h options.c xlm_options.h xlm_options.c
nodist_mvlcgenerate_SOURCES=options.h options.c

options.c: options.h

options.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ --file-name=options <@srcdir@/options.ggo 


#----------------------------------- build mvlc_xlmload

mvlc_xlmload_SOURCES = xlmload.cpp \
	CVMUSB.h CVMUSB.cpp CVMUSBReadoutList.h CVMUSBReadoutList.cpp \
	CMVLCDirect.h CMVLCDirect.cpp CXLM.h CXLM.cpp \
	xlm_options.h xlm_options.c

mvlc_xlmload_DEPENDENCIES=xlm_options.c

mvlc_xlmload_CPPFLAGS=@MVLC_CPPFLAGS@ \
	-I@top_srcdir@/usb/common/configurableobject \
	-I@top_srcdir@/base/headers -I@top_srcdir@/base/os

mvlc_xlmload_LDFLAGS=@builddir@/libmvlcGenerator.la \
	@MVLC_LDFLAGS@ \
	@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@



xlm_options.c: xlm_options.h

xlm_options.h: xlmload_options.ggo
	$(GENGETOPT) --output-dir=@builddir@ --file-name=xlm_options <@srcdir@/xlmload_options.ggo

mvlc_xlmload_DEPENDENCIES=@builddir@/libmvlcGenerator.la

#------------------------------ build/runtests

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

TCLDRIVERFILES=VMUSBDriverSupport.tcl Utils.tcl \
	sis3300defs.tcl  sis330xdriver.tcl sis3300.tcl \
	ACAENV1290.tcl ACAENV785.tcl AXLM72.tcl ACrdcXLM72.tcl \
	AGD16XLM72.tcl ALevel3XLM72Readout.tcl AMesytecMADC32.tcl \
	APpacXLM72.tcl ATimeStampXLM72.tcl AXLM72Generic.tcl \
	AXLM72Scaler.tcl


EXTRA_DIST=options.ggo xlmload_options.ggo \
	triggeronly.yaml Readme.md mvlc_translate.xml \
	SampleDriver.cpp SampleDriver.tcl \
	$(TCLDRIVERFILES)

install-exec-local:
	$(mkinstalldirs) @datarootdir@/mvlc
	$(mkinstalldirs) @datarootdir@/mvlc-examples
	$(INSTALL_DATA)  @srcdir@/triggeronly.yaml @datarootdir@/mvlc/triggeronly.yaml
	$(INSTALL_DATA)  @srcdir@/SampleDriver.cpp @datarootdir@/mvlc-examples
	$(INSTALL_SCRIPT) @srcdir@/SampleDriver.tcl @datarootdir@/mvlc-examples
	$(mkinstalldirs) @datarootdir@/mvlc/TclLibs
	for f in $(TCLDRIVERFILES) ; do $(INSTALL_SCRIPT) @srcdir@/$$f @datarootdir@/mvlc/TclLibs; done
	MVLC_TRANSATOR=1 echo pkg_mkIndex -verbose @datarootdir@/mvlc/TclLibs  "*.tcl" | $(TCLSH_CMD)

