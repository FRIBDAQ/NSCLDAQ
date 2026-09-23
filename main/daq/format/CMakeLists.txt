#-------------- The library libdataformat

set(_format_includes
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/base/os)

nscldaq_add_library(dataformat VERSION_INFO ${SOVERSION}
  SOURCES CRingSelectionPredicate.cpp CDesiredTypesPredicate.cpp
    CAllButPredicate.cpp CRingItem.cpp CRingStateChangeItem.cpp
    CRingScalerItem.cpp CRingTextItem.cpp CRingPhysicsEventCountItem.cpp
    CRingFragmentItem.cpp CPhysicsEventItem.cpp CDataFormatItem.cpp
    CGlomParameters.cpp CUnknownFragment.cpp CRingItemFactory.cpp
    ringitem.c RingItemComparisons.cpp CAbnormalEndItem.cpp
    CBufferedRingItemConsumer.cpp CRingBufferChunkAccess.cpp)
target_include_directories(dataformat PRIVATE ${_format_includes})
target_link_libraries(dataformat
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  PUBLIC DataFlow url daqshm NSCLDAQ::Exception rt NSCLDAQ::Threads)

nscldaq_install_headers(CRingSelectionPredicate.h CDesiredTypesPredicate.h
  CAllButPredicate.h CRingItem.h CRingStateChangeItem.h CRingScalerItem.h
  CRingTextItem.h CRingPhysicsEventCountItem.h CRingFragmentItem.h
  CPhysicsEventItem.h CRingItemFactory.h CDataFormatItem.h
  CGlomParameters.h CUnknownFragment.h DataFormat.h RingItemComparisons.h
  CAbnormalEndItem.h CBufferedRingItemConsumer.h CRingBufferChunkAccess.h)

#-------------- The Tcl ring command package libTclRingBuffer

nscldaq_add_library(TclRingBuffer SOURCES CTclRingCommand.cpp)
target_include_directories(TclRingBuffer PRIVATE
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(TclRingBuffer
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC dataformat DataFlow Tcp daqshm NSCLDAQ::tclPlus NSCLDAQ::Tcl
    NSCLDAQ::Threads)

# install-exec-hook: get the package installed in TclLibs/ringcommand.

nscldaq_install_copy(lib/libTclRingBuffer.so TclLibs/ringcommand/libTclRingBuffer.so)
install(CODE "file(APPEND \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/TclLibs/ringcommand/pkgIndex.tcl\"
  \"package ifneeded TclRingBuffer 1.0 [list load [file join \\\$dir libTclRingBuffer.so]]\\n\")")

#------------------- Tests:

nscldaq_add_executable(format_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp selecttest.cpp desiredtests.cpp allbuttests.cpp
    ringitemtests.cpp teststate.cpp scalertests.cpp texttest.cpp
    physcounttests.cpp infotests.cpp phystests.cpp trigcounttests.cpp
    scalerformattests.cpp statechangetests.cpp dataformattests.cpp
    textformattests.cpp fragmenttest.cpp glomparamtests.cpp factorytests.cpp
    physeventtests.cpp bufferedconstest.cpp rbchunktests.cpp zcopytests.cpp
    formatprimitiveTests.cpp)
target_include_directories(format_unittests PRIVATE ${_format_includes})
target_link_libraries(format_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders NSCLDAQ::CppUnit
  DataFlow url dataformat daqshm NSCLDAQ::Exception)

nscldaq_add_executable(format_ringTests TEST OUTPUT_NAME ringTests
  SOURCES TestRunner.cpp ringTests.cpp)
target_include_directories(format_ringTests PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/format ${PROJECT_SOURCE_DIR}/base/dataflow)
target_link_libraries(format_ringTests PRIVATE
  NSCLDAQ::PixieHeaders NSCLDAQ::CppUnit TclRingBuffer dataformat DataFlow
  daqshm NSCLDAQ::tclPlus NSCLDAQ::Tcl NSCLDAQ::Threads)

nscldaq_add_test(NAME unittests COMMAND format_unittests)
nscldaq_add_test(NAME ringTests COMMAND format_ringTests)

# $(AM_CXXFLAGS) was in this target's CPPFLAGS, which also apply to its C
# sources (gengetopt/C files); nscldaq_add_* only applies it to C++.
target_compile_options(dataformat PRIVATE $<$<COMPILE_LANGUAGE:C>:-fno-strict-aliasing>)
