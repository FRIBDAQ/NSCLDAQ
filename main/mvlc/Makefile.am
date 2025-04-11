bin_PROGRAMS=mvlcgenerate


# Done this way so we don't need a lib and we don't need to repeat ourselves for
# test sources

main_modules=utilities.h utilities.cpp \
	TCLConfigParser.h TCLConfigParser.cpp \
	CVMUSBReadoutList.h CVMUSBReadoutList.cpp \
	CVMUSB.h CVMUSB.cpp \
	DeviceCommand.h DeviceCommand.cpp \
	CReadoutModule.h CReadoutModule.cpp \
	CReadoutHardware.h CReadoutHardware.cpp \
	CStack.h CStack.cpp CMarker.h CMarker.cpp \
	C785.h C785.cpp C3820.h C3820.cpp \
	MVLCConfigParser.h MVLCConfigParser.cpp \
	MVLCGenerator.h MVLCGenerator.cpp 

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

unittests_SOURCES = TestRunner.cpp utiltests.cpp parsertests.cpp \
    stacktests.cpp devcommandtests.cpp rdomoduletests.cpp \
	stackmoduletests.cpp markertests.cpp v785tests.cpp\
	options.h options.c \
	$(main_modules)
unittests_CPPFLAGS = @CPPUNIT_CFLAGS@ $(mvlcgenerate_CPPFLAGS)
unittests_LDFLAGS = @CPPUNIT_LIBS@ $(mvlcgenerate_LDFLAGS)

TESTS=unittests

EXTRA_DIST=options.ggo triggeronly.yaml

install-exec-local:
	$(mkinstalldirs) @datarootdir@/mvlc
	$(INSTALL_DATA)  @srcdir@/triggeronly.yaml @datarootdir@/mvlc/triggeronly.yaml
	