nscldaq_add_library(UtilCommon SOURCES StringsToIntegers.cpp StringsToIntegers.h)
target_include_directories(UtilCommon PRIVATE ${PROJECT_SOURCE_DIR}/daq/format)
target_link_libraries(UtilCommon
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::Exception)
