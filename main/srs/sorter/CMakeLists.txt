nscldaq_add_library(SRSSorter SOURCES SRSSorter.cpp)
target_include_directories(SRSSorter PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/format ${PROJECT_SOURCE_DIR}/daq/IO)
target_link_libraries(SRSSorter
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC daqio dataformat NSCLDAQ::Exception)

nscldaq_install_headers(SRSSorter.h)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(SRSSorter)
