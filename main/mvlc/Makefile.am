#
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
# The IMPORT_DIR symbol defines where modules are imoprted from.
# this is normally @top_srcdir@/usb/vmusb/daqconfig
#

bin_PROGRAMS=mvlcgenerate mvlc_xlmload mvlc_sets800run

# I had an out of tree build fail in a weird way that
# makes me believe this is not fully parallel safe.
# Specifically a link of the tests claimed#
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
IMPORTED_MODULES= \
	CMQDC32RdoHdwr.cpp \
	CMQDC32StackBuilder.cpp \
	C785.cpp \
	C830.cpp \
	C3804.cpp \
	C3820.cpp \
	CCAENChain.cpp \
	CDelay.cpp \
	CMesytecBase.cpp \
	CMADC32.cpp \
	CMADCChain.cpp  \
	CMADCScaler.cpp \
	CMarker.cpp \
	CMDPP16QDC.cpp \
	CMDPP32QDC.cpp  \
	CMDPP32SCP.cpp  \
	CMTDC32.cpp \
	CNADC2530.cpp \
	CV1x90.cpp  \
	CV977.cpp \
	CV1495sc.cpp \
	CV1729.cpp \
	CXLM.cpp \
	CXLMTimestamp.cpp \
	CXLMFERA.cpp \
	s800TriggerRegisters.cpp \
	CS800TriggerNew.cpp \
	CS800TriggerScalers.cpp 

BUILT_SOURCES = options.h options.c xlm_options.h xlm_options.c \
	$(IMPORTED_MODULES) 

$(IMPORTED_MODULES):
	for f in $(IMPORTED_MODULES) ; do cp $(IMPORT_DIR)/$$f @builddir@; done

nodist_mvlcgenerate_SOURCES=$(IMPORTED_MODULES) options.h options.c


LOCAL_MODULES= \
	VMUSBCommand.h VMUSBCommand.cpp VMUSBListCommand.h \
	VMUSBListCommand.cpp \
	TclWrapper.h TclWrapper.cpp

main_modules=utilities.h utilities.cpp \
	 MVLCConfigParser.cpp \
	$(LOCAL_MODULES) \
	MVLCGenerator.h MVLCGenerator.cpp \
	vme_interface_class.h sis_vmusb_interface.h sis_vmusb_interface.cpp

mvlcgenerate_SOURCES=generate.cpp $(main_modules) 
mvlcgenerate_DEPENDENCIES=@builddir@/libmvlcGenerator.la

mvlcgenerate_CPPFLAGS= -I@srcdir@ -I@top_srcdir@/usb/common/configurableobject \
	-I@top_srcdir@/base/headers -I@top_srcdir@/base/os \
	@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@ @YAML_CFLAGS@ \
	-DMVLC_TEMPLATE=\"@datarootdir@/mvlc/triggeronly.yaml\" \
	-DMVLC_TCLLIBDIR=\"@datarootdir@/mvlc/TclLibs\" \
	-DMVLC_GENERATOR -I@top_srcdir@/usb/vmusb/daqconfig \
	@JSON_CFLAGS@ \
	-DDEFAULT_S800REGFILE=\"@datarootdir@/s800triggers/RegisterFile.json\"

mvlcgenerate_LDFLAGS=@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@LIBTCLPLUS_LDFLAGS@ \
	@TCL_LDFLAGS@ \
	@YAML_LIBS@\
	@builddir@/libmvlcGenerator.la \
	@JSON_LIBS@

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


options.c: options.h

options.h: options.ggo
	$(GENGETOPT) --output-dir=@builddir@ --file-name=options <@srcdir@/options.ggo 


#----------------------------------- build mvlc_xlmload

nodist_mvlc_xlmload_SOURCES=xlm_option.h xlm_options.c

mvlc_xlmload_SOURCES = xlmload.cpp \
	CMVLCDirect.h CMVLCDirect.cpp \
	CXLM.cpp $(libmvlcGenerator_la_SOURCES)



mvlc_xlmload_CPPFLAGS=@MVLC_CPPFLAGS@ \
	-I@top_srcdir@/usb/common/configurableobject \
	-I@top_srcdir@/base/headers -I@top_srcdir@/base/os \
	-I$(IMPORT_DIR) -DMVLC_GENERATOR \
	-I@top_srcdir@/usb/vmusb/daqconfig \
	@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@

mvlc_xlmload_LDFLAGS= \
	@MVLC_LDFLAGS@ \
	@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@




xlm_options.c: xlm_options.h

xlm_options.h: xlmload_options.ggo
	$(GENGETOPT) --output-dir=@builddir@ --file-name=xlm_options <@srcdir@/xlmload_options.ggo

mvlc_xlmload_DEPENDENCIES=@builddir@/libmvlcGenerator.la xlm_options.c

#  Slow controls client to set the run number in the trigger module:


mvlc_sets800run_SOURCES=sets800run.cpp $(IMPORT_DIR)/s800TriggerRegisters.h \
	CVMUSBReadoutList.cpp CVMUSBReadoutList.h

nodist_mvlc_sets800run_SOURCES=s800TriggerRegisters.cpp

mvlc_sets800run_CPPFLAGS=-I@top_srcdir@/usb/vmusb/daqconfig \
	@MVLC_CPPFLAGS@ @JSON_CFLAGS@ -DMVLC_GENERATOR \
	-DDEFAULT_S800REGFILE=\"@datarootdir@/s800triggers/RegisterFile.json\"

mvlc_sets800run_LDFLAGS=@JSON_LIBS@ \
	-L@MVLC_LIB@ -lslowControlsClient -Wl,-rpath=@MVLC_LIB@

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
	$(IMPORTED_MODULES) $(LOCAL_MODULES) \
	$(libmvlcGenerator_la_SOURCES) sis_vmusb_interface.cpp \
	MVLCConfigParser.cpp utilities.cpp



unittests_CPPFLAGS = @CPPUNIT_CFLAGS@ $(mvlcgenerate_CPPFLAGS)
unittests_LDFLAGS = \
	@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@LIBTCLPLUS_LDFLAGS@ \
	@TCL_LDFLAGS@ \
	@YAML_LIBS@\@CPPUNIT_LIBS@ @JSON_LIBS@

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

