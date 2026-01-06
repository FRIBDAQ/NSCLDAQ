bin_PROGRAMS = ScriptedReadout
lib_LTLIBRARIES=libScriptedRdo.la

COMPILATION_FLAGS =  $(VMEBUILDEADERS) \
	-I@top_srcdir@/base/headers 	\
	@LIBTCLPLUS_CFLAGS@	\
	-I@top_srcdir@/sbs/nsclapi 	\
	-I@top_srcdir@/sbs/vmemodules\
	-I@top_srcdir@/sbs/readout   \
	-I@top_srcdir@/base/os      \
	-I@top_srcdir@/base/factories \
	-DVME $(TCL_FLAGS) \
	-DINSTDIR=\"@prefix@\" -g -pthread @PIXIE_CPPFLAGS@


ScriptedReadout_SOURCES = Skeleton.cpp
ScriptedReadout_DEPENDENCIES = libScriptedRdo.la


libScriptedRdo_la_SOURCES = 	CScriptedSegment.cpp		\
	CBoolConfigParam.cpp \
	CCAENModule.cpp \
	CCAENV775.cpp \
	CCAENV775Creator.cpp \
	CCAENV785.cpp \
	CCAENV785Creator.cpp \
	CCAENV792.cpp \
	CCAENV792Creator.cpp \
	CCAENV830Creator.cpp \
	CCAENV830Module.cpp \
	CSIS3300Creator.cpp \
	CSIS3300Module.cpp   \
	CConfigurationParameter.cpp \
	CDigitizerDictionary.cpp \
	CDigitizerModule.cpp \
	CIntArrayParam.cpp \
	CIntConfigParam.cpp \
	CModuleCommand.cpp \
	CModuleCreator.cpp \
	CScalerBank.cpp \
	CScalerModule.cpp \
	CStringArrayparam.cpp \
	CStringConfigParam.cpp \
	CConfigurableObject.cpp \
	CReadableObject.cpp 	\
	CReadException.cpp	\
	CPacketCreator.cpp	\
	CEnumParameter.cpp	\
	CV1x90Creator.cpp CV1x90Module.cpp \
	CReadOrder.cpp	\
	CScriptedScalers.cpp	\
	ScriptedBundle.cpp

libScriptedRdo_la_CPPFLAGS=$(COMPILATION_FLAGS)



libScriptedRdo_la_LDFLAGS = -version-info $(SOVERSION)

ScriptedReadout_LDFLAGS = -Wl,"-rpath=@prefix@/lib"
ScriptedReadout_CPPFLAGS=$(COMPILATION_FLAGS)

noinst_HEADERS= CScriptedSegment.h	\
	CBoolConfigParam.h \
	CCAENModule.h \
	CCAENV775.h \
	CCAENV775Creator.h \
	CCAENV785.h \
	CCAENV785Creator.h \
	CCAENV792.h \
	CCAENV792Creator.h \
	CCAENV820Registers.h \
	CCAENV830Creator.h \
	CCAENV830Module.h \
	CSIS3300Creator.h \
	CSIS3300Module.h  \
	CConfigurationParameter.h \
	CDigitizerDictionary.h \
	CDigitizerModule.h \
	CIntArrayParam.h \
	CIntConfigParam.h \
	CModuleCommand.h \
	CModuleCreator.h \
	CReadOrder.h \
	CScalerBank.h \
	CScalerModule.h \
	CStringArrayparam.h \
	CStringConfigParam.h \
	CConfigurableObject.h	\
	CReadableObject.h	\
	CReadException.h	\
	CPacketCreator.h       \
	CEnumParameter.h	\
	CV1x90Creator.h CV1x90Module.h \
	Skeleton.h	\
	CScriptedScalers.h \
	ScriptedBundle.h









ScriptedReadout_LDADD = @top_builddir@/sbs/scripted/libScriptedRdo.la   \
	@top_builddir@/sbs/readout/libSBSProductionReadout.la		\
	@top_builddir@/sbs/vmemodules/libSBSDeviceSupport.la	\
	@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la		\
	@top_builddir@/base/thread/libdaqthreads.la	\
	@top_builddir@/base/os/libdaqshm.la	\
	@LIBTCLPLUS_LDFLAGS@ \
	$(TCL_LDFLAGS) \
	-lpthread


EXTRA_DIST=@srcdir@/Makefile.skel @srcdir@/scriptedreadout_Makefile.include @srcdir@/scripted.xml


#
#   Now the extra installation stuff:
#
#   We make:   @prefix@/contrib/scriptedReadout
#                           ..                  /include
#                          ..                   /skel
#                            ..                 /src
#
#  All the .cpp's in this directory go to .../src
#  All the .h's in this directory go to   .../include
#  ReadoutMain.cpp, skeleton.{cpp,h} goto .../skel
#  Makefile gets generated from Makefile.skel
#  and stuff we know from configure and put
#  in                                     .../skel
#
install-exec-local:
	$(mkinstalldirs)     @prefix@/skeletons
	$(mkinstalldirs)     @prefix@/skeletons/scriptedReadout
	$(mkinstalldirs)     @prefix@/skeletons/scriptedReadout/include
	$(mkinstalldirs)     @prefix@/skeletons/scriptedReadout/skel
	$(mkinstalldirs)     @prefix@/skeletons/scriptedReadout/src
	$(mkinstalldirs)     @prefix@/etc
	$(INSTALL_DATA)   @srcdir@/*.cpp @prefix@/skeletons/scriptedReadout/src
	$(INSTALL_DATA)   @srcdir@/*.h   @prefix@/skeletons/scriptedReadout/include
	$(INSTALL_DATA)    @srcdir@/Skeleton.cpp @srcdir@/Skeleton.h  \
				@prefix@/skeletons/scriptedReadout/skel
	echo INSTDIR=@prefix@                                 > @srcdir@/Makefile.user
	cat  @srcdir@/Makefile.skel                                    >> @srcdir@/Makefile.user
	$(INSTALL_DATA) @srcdir@/Makefile.user \
		@prefix@/skeletons/scriptedReadout/skel/Makefile
	$(INSTALL_DATA) @srcdir@/scriptedreadout_Makefile.include \
	        @prefix@/etc

##
#  Tests
#

check_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp factorytests.cpp \
	CModuleCommand.cpp  CDigitizerDictionary.cpp \
	CModuleCreator.cpp \
	CCAENModule.cpp CCAENV775.cpp CCAENV775Creator.cpp \
	CCAENV785.cpp CCAENV785Creator.cpp \
	CCAENV792.cpp CCAENV792Creator.cpp \
	CCAENV830Module.cpp CCAENV830Creator.cpp \
	CV1x90Module.cpp CV1x90Creator.cpp \
	CSIS3300Module.cpp CSIS3300Creator.cpp \
	CDigitizerModule.cpp CReadableObject.cpp CReadOrder.cpp \
	CReadException.cpp \
	CConfigurableObject.cpp CConfigurationParameter.cpp \
	CEnumParameter.cpp CIntArrayParam.cpp CIntConfigParam.cpp \
	CStringArrayparam.cpp CStringConfigParam.cpp CBoolConfigParam.cpp

unittests_CPPFLAGS=$(COMPILATION_FLAGS) @CPPUNIT_CFLAGS@ \
	@LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@
unittests_LDFLAGS= @CPPUNIT_LDFLAGS@  \
	@top_builddir@/sbs/readout/libSBSProductionReadout.la		\
	@top_builddir@/sbs/vmemodules/libSBSDeviceSupport.la    \
        @top_builddir@/sbs/nsclapi/libSBSVmeAPI.la              \
        @top_builddir@/base/thread/libdaqthreads.la     \
        @top_builddir@/base/os/libdaqshm.la     \
        @LIBTCLPLUS_LDFLAGS@ @TCL_LIBS@ 


TESTS=unittests
