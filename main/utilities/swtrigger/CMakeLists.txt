#
#  swtrigger - the software trigger/parallel processing framework
#  (libSwTrigger) and the programs built on it.  The MPI transports and
#  application strategies are only built when MPI is available.
#

#---------------------------------------------------------------------------
#  MPI conditional parts (automake 'if MPI').

set(_swt_mpi_transport_srcs "")
set(_swt_mpi_transport_hdrs "")
set(_swt_mpi_softwaretrigger_srcs "")
set(_swt_mpi_transformer_srcs "")
set(_swt_mpi_eventeditor_srcs "")
set(_swt_mpi_fulleventeditor_srcs "")
set(_swt_mpi "")
if(NSCLDAQ_HAVE_MPI)
  set(_swt_mpi_transport_srcs CMPITransport_mpi.cpp CMPIFanoutTransport_mpi.cpp
    CMPIFanoutClientTransport_mpi.cpp CRingItemMPIDataSource_mpi.cpp)
  set(_swt_mpi_transport_hdrs CMPITransport_mpi.h CMPIFanoutTransport_mpi.h
    CMPIFanoutClientTransport_mpi.h CRingItemMPIDataSource_mpi.h)
  set(_swt_mpi_softwaretrigger_srcs CMPIClassifierApp_mpi.cpp CMPIAppStrategy_mpi.cpp)
  set(_swt_mpi_transformer_srcs CMPITransformerApp_mpi.cpp CMPIAppStrategy_mpi.cpp)
  set(_swt_mpi_eventeditor_srcs CMPIBuiltRingItemEditorApp_mpi.cpp CMPIAppStrategy_mpi.cpp)
  set(_swt_mpi_fulleventeditor_srcs CMPIFullEventEditorApp_mpi.cpp CMPIAppStrategy_mpi.cpp)
  set(_swt_mpi NSCLDAQ::MPI)
endif()

#---------------------------------------------------------------------------
#  Flags shared by everything here (libSwTrigger_la_CPPFLAGS/LDFLAGS).

set(_swt_includes
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/factories
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder)
set(_swt_compile_libs NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders)
set(_swt_libs daqshm dataformat daqthreads url DataFlow NSCLDAQ::Exception
  NSCLDAQ::ZMQ NSCLDAQ::Threads ${_swt_mpi} NSCLDAQ::UFMT)

#---------------------------------------------------------------------------
#  gengetopt parsers (BUILT_SOURCES: all are generated, only
#  swtriggerflags and eventfilterflags are compiled into anything).

foreach(_ggo swtriggerflags eventfilterflags transformer eventeditor fulleventeditor)
  nscldaq_gengetopt(${_ggo} ${_ggo}.ggo)
endforeach()
add_custom_target(swtrigger_built_sources ALL)
add_dependencies(swtrigger_built_sources
  ggo_utilities_swtrigger_transformer
  ggo_utilities_swtrigger_eventeditor
  ggo_utilities_swtrigger_fulleventeditor)

#---------------------------------------------------------------------------
#  Libraries.

set(_swt_lib_srcs
  CReceiver.cpp CTestTransport.cpp
  CZMQTransport.cpp CZMQClientTransport.cpp CZMQServerTransport.cpp
  CSender.cpp CRingItemFileTransport.cpp CRingBufferTransport.cpp
  CDispatcher.cpp CClientRegistry.cpp CZMQRouterTransport.cpp
  CZMQDealerTransport.cpp CDataSourceElement.cpp CParallelWorker.cpp
  CDataSinkElement.cpp CThreadedProcessingElement.cpp
  CRingItemTransportFactory.cpp CRingItemDispatcher.cpp
  CZMQRingItemDispatcher.cpp CRingItemBlockSourceElement.cpp
  CRingItemZMQSourceElement.cpp
  CZMQRingItemSourceThread.cpp CZMQRingItemWorker.cpp
  CZMQRingItemThreadedWorker.cpp CZMQCommunicatorFactory.cpp
  CCommunicatorFactoryMaker.cpp CRingItemMarkingWorker.cpp
  CNullTransport.cpp CGather.cpp ${_swt_mpi_transport_srcs}
  CBuiltRingItemExtender.cpp CBuiltItemWorker.cpp CBuiltRingItemEditor.cpp)

nscldaq_add_library(SwTrigger SOURCES ${_swt_lib_srcs})
target_include_directories(SwTrigger PRIVATE ${_swt_includes})
target_link_libraries(SwTrigger
  PRIVATE ${_swt_compile_libs}
  PUBLIC ${_swt_libs})

nscldaq_add_library(TestClassifier SOURCES TestClassifier.cpp)
nscldaq_add_library(TestExtender SOURCES CTestExtender.cpp)
foreach(_lib TestClassifier TestExtender)
  target_include_directories(${_lib} PRIVATE ${_swt_includes})
  target_link_libraries(${_lib}
    PRIVATE ${_swt_compile_libs}
    PUBLIC SwTrigger ${_swt_libs})
endforeach()

nscldaq_install_headers(
  CTransport.h CReceiver.h CZMQTransport.h CZMQClientTransport.h
  CZMQServerTransport.h CSender.h CRingItemTransport.h CRingItemFileTransport.h
  CRingBufferTransport.h CDispatcher.h CClientRegistry.h CZMQRouterTransport.h
  CZMQDealerTransport.h CFanoutClientTransport.h CProcessingElement.h
  CDataSourceElement.h CParallelWorker.h CDataSinkElement.h
  CThreadedProcessingElement.h CRingItemTransportFactory.h
  CRingItemDispatcher.h CZMQRingItemDispatcher.h CRingItemBlockSourceElement.h
  CRingItemZMQSourceElement.h
  CZMQRingItemSourceThread.h CZMQRingItemWorker.h CProcessor.h
  CZMQRingItemThreadedWorker.h CCommunicatorFactory.h CZMQCommunicatorFactory.h
  CCommunicatorFactoryMaker.h CRingItemMarkingWorker.h CNullTransport.h
  CFanoutTransport.h CGather.h CBuiltRingItemExtender.h
  CBuiltItemWorker.h CBuiltRingItemEditor.h
  CFullEventEditor.h
  ${_swt_mpi_transport_hdrs})

#---------------------------------------------------------------------------
#  Programs.

nscldaq_add_executable(SoftwareTrigger
  SOURCES SoftwareTrigger.cpp CClassifierApp.cpp CZMQThreadedClassifierApp.cpp
    CRingItemSorter.cpp CRingBlockDataSink.cpp CZMQAppStrategy.cpp
    ${_swt_mpi_softwaretrigger_srcs})
nscldaq_use_gengetopt(SoftwareTrigger swtriggerflags)

nscldaq_add_executable(EventFilter
  SOURCES EventFilterMain.cpp CEventFilterApp.cpp)
nscldaq_use_gengetopt(EventFilter eventfilterflags)

nscldaq_add_executable(Transformer
  SOURCES Transformer.cpp ${_swt_mpi_transformer_srcs}
    CTransformerApp.cpp CRingItemSorter.cpp CRingBlockDataSink.cpp
    CZMQTransformerApp.cpp CZMQAppStrategy.cpp)
nscldaq_use_gengetopt(Transformer swtriggerflags)

nscldaq_add_executable(EventEditor
  SOURCES ${_swt_mpi_eventeditor_srcs}
    CTransformerApp.cpp CRingItemSorter.cpp CRingBlockDataSink.cpp
    EventEditor.cpp CBuiltRingItemEditorApp.cpp
    CZMQBuiltRingItemEditorApp.cpp CZMQAppStrategy.cpp)
nscldaq_use_gengetopt(EventEditor swtriggerflags)

nscldaq_add_executable(FullEventEditor
  SOURCES CFullEventEditor.cpp FullEventEditor.cpp CFullEventEditorApp.cpp
    CZMQFullEventEditorApp.cpp CRingItemSorter.cpp CRingBlockDataSink.cpp
    ${_swt_mpi_fulleventeditor_srcs} CZMQAppStrategy.cpp)
nscldaq_use_gengetopt(FullEventEditor swtriggerflags)

foreach(_prog SoftwareTrigger EventFilter Transformer EventEditor FullEventEditor)
  target_include_directories(${_prog} PRIVATE ${_swt_includes})
  target_link_libraries(${_prog} PRIVATE
    ${_swt_compile_libs} SwTrigger ${_swt_libs})
endforeach()
# All but EventFilter added -ldl.
foreach(_prog SoftwareTrigger Transformer EventEditor FullEventEditor)
  target_link_libraries(${_prog} PRIVATE dl)
endforeach()

#---------------------------------------------------------------------------
#  Unit tests.  The test program compiles the library sources itself.
#  There is no TESTS variable (only an unused UNITTESTS rule), so, as
#  before, the program is built but not run by the test suite.

nscldaq_add_executable(swtrigger_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp testxporttests.cpp zmqxporttests.cpp
    zmqsvrxporttests.cpp zmqclitests.cpp zmqdealertests.cpp zmqrtrtests.cpp
    zmqsendertests.cpp zmqreceivertests.cpp zmqdispattests.cpp
    ritemfxporttests.cpp rbufferxporttests.cpp ritemtransportfactoryTests.cpp
    dsrceltests.cpp pworkertests.cpp sinktests.cpp tprocesstests.cpp
    zmqworkertests.cpp
    ${_swt_lib_srcs})
target_include_directories(swtrigger_unittests PRIVATE ${_swt_includes})
target_link_libraries(swtrigger_unittests PRIVATE
  ${_swt_compile_libs} NSCLDAQ::CppUnit daqio ${_swt_libs})
