#--------------- The ring library....

set(_dataflow_includes
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/servers/portmanager
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/base/os)
set(_dataflow_defs BINDIR="${bindir}")

nscldaq_add_library(DataFlow VERSION_INFO ${SOVERSION}
  SOURCES CRingBuffer.cpp CTestRingBuffer.cpp CRemoteAccess.cpp
    CRingMaster.cpp CZCopyRingBuffer.cpp)
target_include_directories(DataFlow PRIVATE ${_dataflow_includes})
target_compile_definitions(DataFlow PRIVATE ${_dataflow_defs})
target_link_libraries(DataFlow
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  PUBLIC PortManager url daqshm NSCLDAQ::Threads)

nscldaq_install_headers(CRingBuffer.h CTestRingBuffer.h CRingMaster.h
  CRemoteAccess.h CZCopyRingBuffer.h)

nscldaq_add_library(RingBuffer VERSION_INFO ${SOVERSION}
  SOURCES ringPackage.cpp CRingCommand.cpp)
target_include_directories(RingBuffer PRIVATE ${_dataflow_includes})
target_compile_definitions(RingBuffer PRIVATE ${_dataflow_defs})
target_link_libraries(RingBuffer
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC DataFlow NSCLDAQ::tclPlus NSCLDAQ::Tcl NSCLDAQ::Threads)

nscldaq_add_library(PyRingBuffer SOURCES pyCRingBuffer.cpp)
target_link_libraries(PyRingBuffer
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC DataFlow NSCLDAQ::Python NSCLDAQ::Exception)

#---------------- Compiled programs.

nscldaq_gengetopt(ringtostdoutsw ringtostdoutsw.ggo ARGS --unamed-opts)
nscldaq_gengetopt(stdintoringsw stdintoringsw.ggo ARGS --unamed-opts)

nscldaq_add_executable(ringtostdout SOURCES ringtostdout.cpp stdintoringUtils.cpp)
nscldaq_use_gengetopt(ringtostdout ringtostdoutsw)
target_include_directories(ringtostdout PRIVATE ${_dataflow_includes})
target_compile_definitions(ringtostdout PRIVATE ${_dataflow_defs})
target_link_libraries(ringtostdout PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  DataFlow NSCLDAQ::Exception daqshm NSCLDAQ::Threads)

nscldaq_add_executable(stdintoring SOURCES stdintoring.cpp stdintoringUtils.cpp)
nscldaq_use_gengetopt(stdintoring stdintoringsw)
target_include_directories(stdintoring PRIVATE ${_dataflow_includes})
target_compile_definitions(stdintoring PRIVATE ${_dataflow_defs})
target_link_libraries(stdintoring PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  DataFlow daqshm NSCLDAQ::Exception NSCLDAQ::Threads)

#---------------- Tests

nscldaq_add_executable(dataflow_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp StaticTests.cpp TransferTests.cpp testcommon.cpp
    DifferenceTests.cpp BlockingTests.cpp InfoTests.cpp ManageTest.cpp
    WhilePredTest.cpp crmastertests.cpp RemoteTests.cpp stdintoringTests.cpp
    stdintoringUtils.cpp)
nscldaq_use_gengetopt(dataflow_unittests stdintoringsw)
target_include_directories(dataflow_unittests PRIVATE ${_dataflow_includes})
target_compile_definitions(dataflow_unittests PRIVATE ${_dataflow_defs})
target_link_libraries(dataflow_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit DataFlow daqshm NSCLDAQ::Exception)

foreach(_prog producer consumer)
  nscldaq_add_executable(dataflow_${_prog} TEST OUTPUT_NAME ${_prog} SOURCES ${_prog}.cpp)
  target_include_directories(dataflow_${_prog} PRIVATE ${_dataflow_includes})
  target_compile_definitions(dataflow_${_prog} PRIVATE ${_dataflow_defs})
  target_link_libraries(dataflow_${_prog} PRIVATE
    NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
    DataFlow daqshm NSCLDAQ::Exception)
endforeach()

nscldaq_add_test(NAME unittests COMMAND dataflow_unittests)

#-------------- Installation of the scripts, Tcl and python packages.

install(PROGRAMS RingMaster.tcl DESTINATION bin RENAME RingMaster)
install(TARGETS RingBuffer LIBRARY DESTINATION TclLibs)
nscldaq_install_tcl_index(TclLibs -verbose "*.so")
install(PROGRAMS ringutils.tcl DESTINATION TclLibs/ring)
nscldaq_install_tcl_index(TclLibs/ring -verbose "*.tcl")
install(PROGRAMS pyRingBuffer.py DESTINATION bin RENAME ringbuffer)
install(PROGRAMS pyRingMaster.py DESTINATION pythonLibs/nscldaq RENAME RingMaster.py)
nscldaq_install_copy(lib/libPyRingBuffer.so pythonLibs/nscldaq/RingBuffer.so)

# $(AM_CXXFLAGS) was in this target's CPPFLAGS, which also apply to its C
# sources (gengetopt/C files); nscldaq_add_* only applies it to C++.
target_compile_options(ringtostdout PRIVATE $<$<COMPILE_LANGUAGE:C>:-fno-strict-aliasing>)
