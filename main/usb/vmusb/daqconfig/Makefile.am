lib_LTLIBRARIES		= libVMUSBDaqConfig.la



libVMUSBDaqConfig_la_SOURCES	= 	CADCCommand.cpp		\
				CConfiguration.cpp	\
				CScalerCommand.cpp	\
				CCAENChainCommand.cpp	\
				CStackCommand.cpp	\
				CCAENV830Command.cpp	\
				CMarkerCommand.cpp	\
				CMADCScalerCommand.cpp	\
				CMADCChainCommand.cpp   \
				CHINPCommand.cpp \
				CPSDCommand.cpp \
				CV1729Command.cpp	\
				CV1495scCommand.cpp	\
				CUserCommand.cpp	\
				CAddTclDriver.cpp	\
				CVMUSBCommand.cpp	\
	      			CReadoutModule.cpp	\
				C3820.cpp		\
				C785.cpp		\
				CCAENChain.cpp		\
				CStack.cpp		\
				C830.cpp		\
				CMarker.cpp		\
				C3804.cpp		\
				CNADC2530.cpp		\
				CMesytecBase.cpp	\
				CMADC32.cpp		\
				CMTDC32.cpp		\
				CMADCScaler.cpp		\
				CV1x90.cpp		\
				CV977.cpp		\
				CXLM.cpp		\
				CMASE.cpp		\
				CMADCChain.cpp	\
				CPSD.cpp	\
				CV1729.cpp	\
				CV1495sc.cpp	\
				CTclModule.cpp \
				CVMUSBControl.cpp \
				CDelay.cpp \
				CCBD8210CamacBranchDriver.cpp \
				@top_srcdir@/usb/common/devices/CCrateController.cpp \
				CCBD8210CrateController.cpp \
				CCBD8210ReadoutList.cpp \
				CCBDCamacBranch.cpp \
				@top_srcdir@/usb/common/devices/CBCNAF.cpp \
				@top_srcdir@/usb/common/devices/CCNAF.cpp \
				@top_srcdir@/usb/common/devices/CNAF.cpp \
				CULMTrigger.cpp \
				CLeCroy4300B.cpp \
				CLeCroy4434.cpp \
				CXLMFERA.cpp \
				CXLMTimestamp.cpp \
				CLeCroy2551.cpp \
				CMQDC32StackBuilder.cpp \
				CMQDC32RdoHdwr.cpp	\
				CDeviceCommand.cpp \
				CMDPP16QDC.cpp \
				CMDPP32QDC.cpp \
				CMDPP32SCP.cpp  \
				CMDPP32PADC.cpp  \
				CSIS3316.cpp \
				sis3316_class.cpp \
			s800TriggerRegisters.cpp s800TriggerRegisters.h \
				CS800TriggerNew.cpp CS800TriggerNew.h




noinst_HEADERS		=	CADCCommand.h  		\
				CConfiguration.h 	\
				CScalerCommand.h	\
				CCAENChainCommand.h	\
				CStackCommand.h		\
				CCAENV830Command.h	\
				CMarkerCommand.h	\
				CMADCScalerCommand.h	\
				CMADCChainCommand.h	\
				CHINPCommand.h \
				CPSDCommand.h		\
				CV1729Command.h		\
				CV1495scCommand.h	\
				CUserCommand.h		\
				CAddTclDriver.h		\
				CVMUSBCommand.h		\
				CReadoutModule.h	\
				CReadoutHardware.h	\
				C3820.h			\
				C785.h			\
				CCAENChain.h		\
				CStack.h		\
				C830.h			\
				CMarker.h		\
				C3804.h			\
				CNADC2530.h		\
				CMesytecBase.h		\
				CMADC32.h		\
				CMTDC32.h		\
				CMADCScaler.h		\
				CV1x90.h		\
				CV977.h			\
				CXLM.h			\
				CMASE.h			\
				CMADCChain.h		\
				MADC32Registers.h	\
				CPSD.h			\
				CV1729.h		\
				CV1495sc.h		\
				CCAENV1x90Opcodes.h	\
				CCAENV1x90Registers.h	\
				CTclModule.h		\
				CVMUSBControl.h \
				CDelay.h \
				CDeviceCommand.h \
				CCBD8210CamacBranchDriver.h \
				@top_srcdir@/usb/common/devices/CCrateController.h \
				CCBD8210CrateController.h \
				CCBD8210ReadoutList.h \
				CCBDCamacBranch.h \
				@top_srcdir@/usb/common/devices/CBCNAF.h \
				@top_srcdir@/usb/common/devices/CCNAF.h \
				@top_srcdir@/usb/common/devices/CNAF.h \
				CXLMFERA.h \
				CXLMTimestamp.h \
				CMQDC32StackBuilder.h \
				CMQDC32RdoHdwr.h \
				MQDC32Registers.h \
				CMDPP.h \
				CMDPP16QDC.h \
				CMDPP32QDC.h \
				CMDPP32SCP.h  \
				CMDPP32PADC.h  \
				CSIS3316.h \
				sis3316_class.h \
				sis3316.h \
				sis3316Registers.h


libVMUSBDaqConfig_la_CXXFLAGS=@THREADCXX_FLAGS@ @VMUSB_WORKAROUNDS@

libVMUSBDaqConfig_la_CPPFLAGS = -I@top_srcdir@/usb/common/devices	\
				-I@top_srcdir@/usb/common/configurableobject \
				-I@top_srcdir@/usb/vmusb		\
				-I@top_srcdir@/usb/vmusb/vmusb		\
				-I@top_srcdir@/usb/common/devices	\
				-I@top_srcdir@/base/thread              \
				@LIBTCLPLUS_CFLAGS@			\
				-I@top_srcdir@/base/headers		\
				-I@top_srcdir@/base/os		\
				$(TCL_FLAGS)  @PIXIE_CPPFLAGS@ \
				@JSON_CFLAGS@ \
	-DDEFAULT_S800REGFILE=\"@datarootdir@/s800triggers/RegisterFile.json\"


libVMUSBDaqConfig_la_LIBADD	= \
				@top_builddir@/usb/vmusb/vmusb/libVMUSB.la \
				@top_builddir@/usb/common/configurableobject/libConfigurableObject.la \
				@top_builddir@/base/thread/libdaqthreads.la \
				@LIBTCLPLUS_LDFLAGS@	\
				$(TCL_LDFLAGS) @THREADLD_FLAGS@ @JSON_LIBS@

install-data-local:
	$(mkinstalldirs) @prefix@/vmusbdriver
	$(mkinstalldirs) @prefix@/vmusbdriver/includes
	$(mkinstalldirs) @prefix@/share/s800trigger
	$(INSTALL_DATA)  @srcdir@/*.h @prefix@/vmusbdriver/includes
	$(INSTALL_DATA)  @srcdir@/RegisterFile.json @prefix@/share/s800trigger


noinst_PROGRAMS=cmdtests

cmdtests_SOURCES=TestRunner.cpp Asserts.h ad811test.cpp \
	v830tests.cpp madcscaler.cpp \
	markertests.cpp sis3820test.cpp v1495tests.cpp v1729tests.cpp \
	sis3804tests.cpp hytectests.cpp madctests.cpp tdc1x90tests.cpp \
	v977tests.cpp masetests.cpp mtdctests.cpp delaytests.cpp \
	xlmferatests.cpp cbdtests.cpp xlmtimestamptests.cpp mqdctests.cpp \
	cbdcratetests.cpp ulmtriggertests.cpp lrs4300tests.cpp lrs4434tests.cpp \
	lrs2551tests.cpp mdpp32qdctests.cpp s800triggerregistertests.cpp


cmdtests_CXXFLAGS=@CPPUNIT_CFLAGS@ $(libVMUSBDaqConfig_la_CXXFLAGS) \
	$(libVMUSBDaqConfig_la_CPPFLAGS) @PIXIE_CPPFLAGS@ \
	-DSOURCE_DIR=\"@srcdir@\" @JSON_CFLAGS@
cmdtests_LDFLAGS=@CPPUNIT_LDFLAGS@ @JSON_LIBS@
cmdtests_LDADD=@builddir@/libVMUSBDaqConfig.la \
	$(libVMUSBDaqConfig_la_LIBADD)

TESTS=cmdtests

EXTRA_DIST = CCamacCompat.hpp RegisterFile.json
