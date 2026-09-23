nscldaq_add_executable(FileAnalyzer
  SOURCES FileAnalyzer.cpp CSourceCounterFilter.cpp CSourceCounterFilter.h)
target_include_directories(FileAnalyzer PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities/filter
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder)
target_link_libraries(FileAnalyzer PRIVATE
  NSCLDAQ::tclPlusHeaders filter dataformat NSCLDAQ::UFMT DataFlow)
