bin_PROGRAMS=mvlcgenerate


# Done this way so we don't need a lib and we don't need to repeat ourselves for
# test sources

DEVICE_MODULES=CStack.h CStack.cpp CMarker.h CMarker.cpp \
	C785.h C785.cpp C3820.h C3820.cpp \
	CDelay.h CDelay.cpp \
	CMADC32.cpp CMADC32.h CMesytecBase.cpp CMesytecBase.h MADC32Registers.h \
	CCAENChain.h CCAENChain.cpp CMADCCHain.h CMADCChain.cpp \
	CMADCScaler.h CMADCScaler.cpp CMTDC32.h CMTDC32.cpp \
	C3804.h C3804.cpp \
	CMQDC32RdoHdwr.h CMQDC32RdoHdwr.cpp CMQDC32StackBuilder.h CMQDC32StackBuilder.cpp MQDC32Registers.h \
	CMDPP16QDC.cpp  CMDPP16QDC.h  CMDPP32QDC.cpp  CMDPP32QDC.h  CMDPP32SCP.cpp  CMDPP32SCP.h  CMDPP.h  CMDPPQDC.h \
	C830.h C830.cpp CNADC2530.h CNADC2530.cpp CV977.h CV977.cpp \
	CV1495sc.h CV1495sc.cpp CV1729.h CV1729.cpp \
	CCAENV1x90Opcodes.h CCAENV1x90Registers.h CV1x90.cpp CV1x90.h 

main_modules=utilities.h utilities.cpp \
	TCLConfigParser.h TCLConfigParser.cpp \
	CVMUSBReadoutList.h CVMUSBReadoutList.cpp \
	CVMUSB.h CVMUSB.cpp \
	$(DEVICE_MODULES) \
	DeviceCommand.h DeviceCommand.cpp \
	CReadoutModule.h CReadoutModule.cpp \
	CReadoutHardware.h CReadoutHardware.cpp \
	MVLCConfigParser.h MVLCConfigParser.cpp \
	MVLCGenerator.h MVLCGenerator.cpp \
	vme_interface_class.h sis_vmusb_interface.h sis_vmusb_interface.cpp

mvlcgenerate_SOURCES=generate.cpp $(main_modules) 


mvlcgenerate_CPPFLAGS= -I@top_srcdir@/usb/common/configurableobject \
	@LIBTCLPLUS_CFLAGS@ @TCL_FLAGS@ @YAML_CFLAGS@ \
	-DMVLC_TEMPLATE=\"@datarootdir@/mvlc/triggeronly.yaml\" 

mvlcgenerate_LDFLAGS=@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
	@LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@ @YAML_LIBS@

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
	v1x90tests.cpp
	
unittests_SOURCES = TestRunner.cpp \
	$(UNIT_TESTS) \
	options.h options.c \
	$(main_modules)
unittests_CPPFLAGS = @CPPUNIT_CFLAGS@ $(mvlcgenerate_CPPFLAGS)
unittests_LDFLAGS = @CPPUNIT_LIBS@ $(mvlcgenerate_LDFLAGS)

TESTS=unittests

EXTRA_DIST=options.ggo triggeronly.yaml

install-exec-local:
	$(mkinstalldirs) @datarootdir@/mvlc
	$(INSTALL_DATA)  @srcdir@/triggeronly.yaml @datarootdir@/mvlc/triggeronly.yaml
	