nscldaq_add_executable(FileAnalyzer
  SOURCES FileAnalyzer.cpp CSourceCounterFilter.cpp CSourceCounterFilter.h)
target_include_directories(FileAnalyzer PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities/filter
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder)
target_link_libraries(FileAnalyzer PRIVATE
  NSCLDAQ::tclPlusHeaders filter dataformat NSCLDAQ::UFMT DataFlow)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(FileAnalyzer)
