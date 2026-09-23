nscldaq_gengetopt(tcmdline options.ggo)

nscldaq_add_executable(teering SOURCES teering.cpp)
nscldaq_use_gengetopt(teering tcmdline)
target_include_directories(teering PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(teering PRIVATE NSCLDAQ::PixieHeaders
  EventBuilderClient NSCLDAQ::UFMT dataformat RingBuffer DataFlow daqshm
  NSCLDAQ::tclPlus NSCLDAQ::Threads)
# (-Wl,-rpath=@libdir@ is covered by CMAKE_INSTALL_RPATH.)
