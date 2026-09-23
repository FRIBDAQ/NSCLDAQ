nscldaq_gengetopt(cmdline cmdline.ggo)

nscldaq_add_executable(unglom SOURCES unglom.cpp)
nscldaq_use_gengetopt(unglom cmdline)
target_include_directories(unglom PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/headers)
target_link_libraries(unglom PRIVATE NSCLDAQ::PixieHeaders
  dataformat NSCLDAQ::UFMT EventBuilder EventBuilderClient daqshm
  NSCLDAQ::tclPlus)
