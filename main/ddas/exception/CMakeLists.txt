nscldaq_add_library(DDASException SOURCES CXIAException.cpp)
target_include_directories(DDASException PRIVATE ${PROJECT_BINARY_DIR}/ddas)
target_link_libraries(DDASException
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC NSCLDAQ::Exception NSCLDAQ::Pixie)

nscldaq_install_headers(CXIAException.h)

# Tests

nscldaq_add_executable(ddas_exception_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp ExceptionTests.cpp)
target_include_directories(ddas_exception_unittests PRIVATE ${PROJECT_BINARY_DIR}/ddas)
target_link_libraries(ddas_exception_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::Exception NSCLDAQ::Pixie NSCLDAQ::CppUnit
  DDASException)
nscldaq_add_test(NAME unittests COMMAND ddas_exception_unittests)
