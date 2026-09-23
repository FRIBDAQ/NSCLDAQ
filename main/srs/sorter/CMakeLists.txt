nscldaq_add_library(SRSSorter SOURCES SRSSorter.cpp)
target_include_directories(SRSSorter PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/format ${PROJECT_SOURCE_DIR}/daq/IO)
target_link_libraries(SRSSorter
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC daqio dataformat NSCLDAQ::Exception)

nscldaq_install_headers(SRSSorter.h)
