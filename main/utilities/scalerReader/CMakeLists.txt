#
#  scalerReader
#

nscldaq_add_executable(scalerReader
  SOURCES scalerReader.cpp scalerReaderMain.cpp)
target_include_directories(scalerReader PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/servers/portmanager
  ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(scalerReader PRIVATE
  NSCLDAQ::tclPlusHeaders
  PortManager dataformat daqio DataFlow url daqshm
  NSCLDAQ::Exception NSCLDAQ::Threads)
