#  COMPILATION_FLAGS shared by the library and the Readout program.

set(_sbsrdo_includes
  ${PROJECT_SOURCE_DIR}/sbs/vmemodules
  ${PROJECT_SOURCE_DIR}/sbs/nsclapi
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/servers/portmanager
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder
  ${PROJECT_SOURCE_DIR}/base/os)
set(_sbsrdo_defs HAVE_VME_MAPPING HAVE_SBSVME_INTERFACE)

#  options.c/options.h (BUILT_SOURCES).  The Makefile generated them in the
#  source directory; here they are in the binary directory.  CReadoutMain.h
#  includes options.h, so users of this directory's headers elsewhere need
#  ${PROJECT_BINARY_DIR}/sbs/readout and a dependency on
#  ggo_sbs_readout_options.

nscldaq_gengetopt(options options.ggo)

nscldaq_add_library(SBSProductionReadout VERSION_INFO 1:0:0
  SOURCES PacketUtils.h PacketUtils.cpp CReadoutMain.cpp CTCLAuthorizer.cpp
    CAuthorizedTclServer.cpp CExperiment.cpp RunState.cpp CComposite.cpp
    CScaler.cpp CEventSegment.cpp CScalerBank.cpp CCompoundEventSegment.cpp
    CEventTrigger.cpp CNullTrigger.cpp CTimedTrigger.cpp
    CCAENV262Trigger.cpp CV977Trigger.cpp CTriggerLoop.cpp
    CRunControlPackage.cpp CBeginCommand.cpp CPauseCommand.cpp
    CResumeCommand.cpp CEndCommand.cpp CInitCommand.cpp CDocumentedPacket.cpp
    CDocumentedPacketManager.cpp CReadoutException.cpp
    CInvalidPacketStateException.cpp CEventPacket.cpp CVarList.cpp
    CDocumentedVars.cpp CVariableBuffers.cpp CBusy.cpp CCAENV262Busy.cpp
    CV977Busy.cpp CStatisticsCommand.h CStatisticsCommand.cpp
    CRunStateCommand.h CRunStateCommand.cpp)
nscldaq_use_gengetopt(SBSProductionReadout options)
target_include_directories(SBSProductionReadout PRIVATE ${_sbsrdo_includes})
target_compile_definitions(SBSProductionReadout PRIVATE ${_sbsrdo_defs})
target_link_libraries(SBSProductionReadout
  PRIVATE NSCLDAQ::PixieHeaders NSCLDAQ::UFMTHeaders
  PUBLIC SBSDeviceSupport SBSVmeAPI dataformat PortManager DataFlow daqshm
    btp NSCLDAQ::tclPlus NSCLDAQ::Tcl NSCLDAQ::Threads)

#  noinst Readout program.

nscldaq_add_executable(sbsreadout_Readout NOINST OUTPUT_NAME Readout
  SOURCES Skeleton.cpp)
add_dependencies(sbsreadout_Readout ggo_sbs_readout_options)
target_include_directories(sbsreadout_Readout PRIVATE ${_sbsrdo_includes})
target_compile_definitions(sbsreadout_Readout PRIVATE ${_sbsrdo_defs})
target_link_libraries(sbsreadout_Readout PRIVATE
  NSCLDAQ::PixieHeaders NSCLDAQ::UFMTHeaders NSCLDAQ::TclHeaders
  SBSProductionReadout SBSDeviceSupport daqthreads daqshm NSCLDAQ::Threads
  NSCLDAQ::tclPlus)

#  Configured files (AC_CONFIG_FILES).

nscldaq_configure_file(UserMakefile.in UserMakefile)
nscldaq_configure_file(SBSRdoMakeIncludes.in SBSRdoMakeIncludes)

#  install-exec-local:

nscldaq_install_mkdir(skeletons/sbs etc include/sbsreadout
  TclLibs/readoutStateHook)
install(PROGRAMS Skeleton.cpp Skeleton.h DESTINATION skeletons/sbs)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/UserMakefile
  DESTINATION skeletons/sbs RENAME Makefile)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/SBSRdoMakeIncludes DESTINATION etc)
file(GLOB _sbsrdo_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
nscldaq_install_headers(${_sbsrdo_headers} ${CMAKE_CURRENT_BINARY_DIR}/options.h
  DESTINATION include/sbsreadout)
install(PROGRAMS readoutStateHook.tcl DESTINATION TclLibs/readoutStateHook)
install(CODE "file(WRITE \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/TclLibs/readoutStateHook/pkgIndex.tcl\"
  \"package ifneeded ReadoutStateHook 1.0 [list source [file join \\\$dir readoutStateHook.tcl]]\\n\")")
