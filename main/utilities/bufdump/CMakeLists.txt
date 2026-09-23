nscldaq_add_library(Stringifier SOURCES CUnifiedFormatter.cpp CUnifiedFormatter.h)
target_link_libraries(Stringifier PUBLIC NSCLDAQ::UFMT)

nscldaq_gengetopt(dumperargs dumperargs.ggo
  ARGS --unamed-opts=OPTIONS --set-version=${VERSION})

nscldaq_add_executable(dumper SOURCES BufdumpMain.cpp dumper.cpp)
nscldaq_use_gengetopt(dumper dumperargs)
target_include_directories(dumper PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/utilities/common)
target_link_libraries(dumper PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  Stringifier UtilCommon url daqio dataformat DataFlow NSCLDAQ::Exception
  daqshm NSCLDAQ::Threads)

#  noinst: explores v12 run offset time dumping.

nscldaq_add_executable(bufdump_divisor_check NOINST OUTPUT_NAME divisor_check
  SOURCES divisor_check.cpp CUnifiedFormatter.cpp)
target_link_libraries(bufdump_divisor_check PRIVATE NSCLDAQ::UFMT)
