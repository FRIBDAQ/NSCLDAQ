#  Configured files (AC_CONFIG_FILES).

nscldaq_configure_file(Makefile_Skeleton.in Makefile_Skeleton)
nscldaq_configure_file(startOrderer.in startOrderer)

#  BUILT_SOURCES: cmdline.h/cmdline.c from options.ggo.  Only the header is
#  used here (GetOpt.cpp includes it); cmdline.c is not compiled into
#  anything (the user's framework skeleton builds its own).

nscldaq_gengetopt(cmdline options.ggo)

set(_evb_includes
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/tcpip
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/servers/portmanager
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/thread)

# Compile-only part of COMPILATION_FLAGS.
set(_evb_compile_deps
  NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::UFMTHeaders)

set(_evb_lib_sources
  OrdererPackage.cpp CFragmentHandlerCommand.cpp CFragmentHandler.cpp
  CInputStatsCommand.cpp COrdererOutput.cpp COutputStatsObserver.cpp
  COutputStatsCommand.cpp CDataLateStatistics.cpp CDLateStatsCommand.cpp
  CLateScriptObserver.cpp COnLateDataCommand.cpp CBarrierStatsCommand.cpp
  CBarrierStats.cpp CIncompleteBarrierStats.cpp CCompleteBarrierCallback.cpp
  CBarrierTraceCommand.cpp CPartialBarrierCallback.cpp CSourceCommand.cpp
  CDeadSourceCommand.cpp CReviveSocketCommand.cpp CFlushCommand.cpp
  CResetCommand.cpp CConfigure.cpp CDuplicateTimeStatCommand.cpp
  COutOfOrderTraceCommand.cpp CXonXOffCallbackCommand.cpp COutputThread.cpp
  CSortThread.cpp BarrierAbortCommand.cpp COutOfOrderStatsCommand.h
  COutOfOrderStatsCommand.cpp)

#-------------- Programs

nscldaq_add_executable(frag2ring SOURCES frag2ring.cpp CFragReader.cpp CFragWriter.cpp)
target_include_directories(frag2ring PRIVATE ${_evb_includes})
target_link_libraries(frag2ring PRIVATE ${_evb_compile_deps}
  dataformat DataFlow daqshm NSCLDAQ::Threads)

nscldaq_add_executable(Orderer SOURCES Orderer.cpp ${_evb_lib_sources})
target_include_directories(Orderer PRIVATE ${_evb_includes})
target_link_libraries(Orderer PRIVATE NSCLDAQ::PixieHeaders
  NSCLDAQ::UFMT NSCLDAQ::tclPlus daqshm daqthreads NSCLDAQ::Tcl NSCLDAQ::Threads)

#-------------- Libraries

nscldaq_add_library(EventBuilder SOURCES ${_evb_lib_sources})
target_include_directories(EventBuilder PRIVATE ${_evb_includes})
target_link_libraries(EventBuilder
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::UFMT NSCLDAQ::tclPlus daqthreads daqshm NSCLDAQ::Tcl
    NSCLDAQ::Threads)

nscldaq_add_library(EventBuilderClient
  SOURCES CEventOrderClient.cpp CEVBClientApp.cpp CEVBFrameworkApp.cpp
    EVBFramework.cpp GetOpt.cpp fragio.cpp)
target_include_directories(EventBuilderClient PRIVATE ${_evb_includes})
target_link_libraries(EventBuilderClient
  PRIVATE ${_evb_compile_deps} NSCLDAQ::Threads
  PUBLIC Tcp PortManager daqshm daqthreads)
add_dependencies(EventBuilderClient ggo_daq_eventbuilder_cmdline)

nscldaq_install_headers(CEventOrderClient.h CEVBClientApp.h EVBFramework.h
  CEVBFrameworkApp.h GetOpt.h fragio.h)

#-------------- install-exec-hook

nscldaq_install_mkdir(bin TclLibs/EventBuilder TclLibs/EventBuilderlib
  TclLibs/EventBuilder/REST)

foreach(_f EVBMonitor EVBShutdown EVBRest)
  install(PROGRAMS ${_f}.tcl DESTINATION bin RENAME ${_f})
endforeach()

install(PROGRAMS EVBStatisticsREST.tcl DESTINATION TclLibs/EventBuilder/REST)

install(PROGRAMS ConnectionManager.tcl callbackManager.tcl eventOrderer.tcl
  connectionList.tcl barriers.tcl GUI.tcl inputStatistics.tcl late.tcl
  outputStatistics.tcl utility.tcl latePopup.tcl DuplicateTimestamp.tcl
  OutOfOrderUI.tcl evbui.tcl EVBStatisticsManager.tcl EVBREST.tcl
  EVBRestClient.tcl EVBRestUI.tcl EVBRestControllers.tcl
  DESTINATION TclLibs/EventBuilder)
install(FILES red.gif DESTINATION TclLibs/EventBuilder)

install(CODE "
  set(_link \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/TclLibs/EventBuilderlib/libEventBuilder.so\")
  file(REMOVE \"\${_link}\")
  message(STATUS \"Installing: \${_link}\")
  file(CREATE_LINK \"\${CMAKE_INSTALL_PREFIX}/lib/libEventBuilder.so\" \"\${_link}\" SYMBOLIC)
")

nscldaq_install_tcl_index(TclLibs/EventBuilder -verbose "*.tcl")
nscldaq_install_tcl_index(TclLibs/EventBuilderlib -verbose "*.so")

nscldaq_install_mkdir(skeletons/eventbuilder)
install(FILES GetOpt.cpp Main.cpp options.ggo DESTINATION skeletons/eventbuilder)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Makefile_Skeleton
  DESTINATION skeletons/eventbuilder RENAME Makefile)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/startOrderer DESTINATION bin)

install(PROGRAMS evbRdoCallouts.tcl EVBStateCallouts.tcl ringSourceMgr.tcl
  endrunmon.tcl DESTINATION TclLibs/EVBCallouts)
nscldaq_install_tcl_index(TclLibs/EVBCallouts -verbose "*.tcl" evbRdoCallouts.tcl "*.so")

#-------------------------------------------
#
# Tests:

nscldaq_add_executable(eventbuilder_outputtests TEST OUTPUT_NAME outputtests
  SOURCES TestRunner.cpp outputTests.cpp COrdererOutput.cpp)
target_include_directories(eventbuilder_outputtests PRIVATE ${_evb_includes})
target_link_libraries(eventbuilder_outputtests PRIVATE
  NSCLDAQ::PixieHeaders NSCLDAQ::UFMTHeaders NSCLDAQ::Threads
  daqshm PortManager Tcp daqthreads NSCLDAQ::tclPlus NSCLDAQ::CppUnit NSCLDAQ::Tcl)

nscldaq_add_executable(eventbuilder_transmittests TEST OUTPUT_NAME transmittests
  SOURCES TestRunner.cpp transmittests.cpp CEventOrderClient.cpp)
target_include_directories(eventbuilder_transmittests PRIVATE ${_evb_includes})
target_link_libraries(eventbuilder_transmittests PRIVATE
  NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders NSCLDAQ::UFMTHeaders NSCLDAQ::Threads
  daqshm PortManager Tcp daqthreads NSCLDAQ::tclPlus NSCLDAQ::CppUnit)

nscldaq_add_executable(eventbuilder_ordertests TEST OUTPUT_NAME ordertests
  SOURCES TestRunner.cpp orderTests.cpp duptscmdtest.cpp configcmdtests.cpp
    tclflowtest.cpp fragalloctest.cpp evbclienttests.cpp CFragmentHandler.cpp
    CDuplicateTimeStatCommand.cpp CConfigure.cpp CXonXOffCallbackCommand.cpp
    COutputThread.cpp CSortThread.cpp CEventOrderClient.cpp)
target_include_directories(eventbuilder_ordertests PRIVATE ${_evb_includes})
target_link_libraries(eventbuilder_ordertests PRIVATE NSCLDAQ::PixieHeaders
  NSCLDAQ::UFMT daqthreads dataformat PortManager daqshm Tcp NSCLDAQ::CppUnit
  NSCLDAQ::Threads NSCLDAQ::tclPlus NSCLDAQ::Tcl)

nscldaq_add_executable(eventbuilder_fragcmdtests TEST OUTPUT_NAME fragcmdtests
  SOURCES TestRunner.cpp fragcmdtests.cpp CFragmentHandlerCommand.cpp
    fragcmdstubs.cpp)
target_include_directories(eventbuilder_fragcmdtests PRIVATE ${_evb_includes})
target_link_libraries(eventbuilder_fragcmdtests PRIVATE
  NSCLDAQ::PixieHeaders NSCLDAQ::UFMTHeaders NSCLDAQ::Threads
  NSCLDAQ::CppUnit NSCLDAQ::tclPlus NSCLDAQ::Tcl)

# check-TESTS ran these (ordertests is built but not run):

nscldaq_add_test(NAME transmittests COMMAND eventbuilder_transmittests)
nscldaq_add_test(NAME outputtests COMMAND eventbuilder_outputtests)
nscldaq_add_test(NAME fragcmdtests COMMAND eventbuilder_fragcmdtests)
nscldaq_add_tcl_test(NAME tclTests)
