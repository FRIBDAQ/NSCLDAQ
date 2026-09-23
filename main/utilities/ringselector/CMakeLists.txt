set(_ringselector_includes
  ${PROJECT_SOURCE_DIR}/utilities/common
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/thread)

nscldaq_gengetopt(parser parser.ggo)

nscldaq_add_executable(ringselector
  SOURCES ringselector.cpp RingSelectorMain.cpp RingBufferQueue.cpp COutputter.cpp)
nscldaq_use_gengetopt(ringselector parser)
target_include_directories(ringselector PRIVATE ${_ringselector_includes})
target_link_libraries(ringselector PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  UtilCommon dataformat DataFlow daqshm daqthreads NSCLDAQ::Exception
  NSCLDAQ::Threads)

# Tests (the StringsToIntegers tests live here too).

nscldaq_add_executable(ringselector_stringstointtests TEST
  OUTPUT_NAME stringstointtests SOURCES TestRunner.cpp str2int.cpp)
target_include_directories(ringselector_stringstointtests PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities/common ${PROJECT_SOURCE_DIR}/daq/format)
target_link_libraries(ringselector_stringstointtests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  UtilCommon NSCLDAQ::CppUnit NSCLDAQ::Exception)

nscldaq_add_executable(ringselector_selectortests TEST
  OUTPUT_NAME selectortests SOURCES TestRunner.cpp selectortests.cpp)
target_include_directories(ringselector_selectortests PRIVATE ${_ringselector_includes})
target_compile_definitions(ringselector_selectortests PRIVATE BINDIR="${bindir}")
target_link_libraries(ringselector_selectortests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit UtilCommon daqshm dataformat DataFlow NSCLDAQ::Exception)

nscldaq_add_test(NAME stringstointtests COMMAND ringselector_stringstointtests)
nscldaq_add_test(NAME selectortests COMMAND ringselector_selectortests)
