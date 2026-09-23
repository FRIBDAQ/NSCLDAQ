set(_eventlog_includes
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/os)

nscldaq_gengetopt(eventlogargs eventlogargs.ggo ARGS --set-version=${VERSION})

nscldaq_add_executable(eventlog
  SOURCES eventlog.cpp eventlogMain.cpp RingChunk.h RingChunk.cpp)
nscldaq_use_gengetopt(eventlog eventlogargs)
target_include_directories(eventlog PRIVATE ${_eventlog_includes})
target_link_libraries(eventlog PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  dataformat DataFlow NSCLDAQ::Exception daqshm NSCLDAQ::Threads
  NSCLDAQ::OpenSSL)

# install-exec-local

install(PROGRAMS eventlog_wrapper.py DESTINATION bin RENAME eventlog_wrapper)

# Tests

nscldaq_add_executable(eventlog_fakeruns TEST OUTPUT_NAME fakeruns
  SOURCES fakeruns.cpp)
target_include_directories(eventlog_fakeruns PRIVATE ${_eventlog_includes})
target_link_libraries(eventlog_fakeruns PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::OpenSSLHeaders NSCLDAQ::PixieHeaders
  dataformat DataFlow daqshm NSCLDAQ::Exception)

nscldaq_add_executable(eventlog_eventlogTests TEST OUTPUT_NAME eventlogTests
  SOURCES TestRunner.cpp eventlogTests.cpp chunkTests.cpp RingChunk.cpp)
target_include_directories(eventlog_eventlogTests PRIVATE ${_eventlog_includes})
target_link_libraries(eventlog_eventlogTests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::OpenSSLHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit dataformat DataFlow NSCLDAQ::Exception daqshm
  NSCLDAQ::Threads)

nscldaq_add_test(NAME eventlogTests COMMAND eventlog_eventlogTests)
nscldaq_add_test(NAME evtlog_wrapper_tests.py
  COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/evtlog_wrapper_tests.py)
