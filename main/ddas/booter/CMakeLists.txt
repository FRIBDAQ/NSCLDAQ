nscldaq_add_library(SystemBooter SOURCES SystemBooter.cpp)
target_include_directories(SystemBooter PRIVATE
  ${PROJECT_SOURCE_DIR}/ddas/configuration
  ${PROJECT_BINARY_DIR}/ddas
  ${PROJECT_SOURCE_DIR}/ddas/exception)
target_compile_definitions(SystemBooter PRIVATE FIRMWARE_PATH="${firmwaredir}")
target_link_libraries(SystemBooter
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC Configuration DDASException NSCLDAQ::Pixie NSCLDAQ::Exception)

nscldaq_install_headers(SystemBooter.h)

# boot and boot_extcl are built from the same source.

nscldaq_add_executable(boot SOURCES boot.cpp)
target_include_directories(boot PRIVATE
  ${PROJECT_BINARY_DIR}/ddas ${PROJECT_SOURCE_DIR}/ddas/configuration)
target_link_libraries(boot PRIVATE
  SystemBooter Configuration NSCLDAQ::Pixie dl)

nscldaq_add_executable(boot_extcl SOURCES boot.cpp)
target_include_directories(boot_extcl PRIVATE
  ${PROJECT_SOURCE_DIR}/ddas/configuration)
target_link_libraries(boot_extcl PRIVATE
  SystemBooter Configuration NSCLDAQ::Pixie dl)

# Tests

nscldaq_add_executable(ddas_booter_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp SystemBooterTest.cpp)
target_include_directories(ddas_booter_unittests PRIVATE
  ${PROJECT_BINARY_DIR}/ddas ${PROJECT_SOURCE_DIR}/ddas/configuration)
target_link_libraries(ddas_booter_unittests PRIVATE
  SystemBooter Configuration NSCLDAQ::Pixie NSCLDAQ::CppUnit)
nscldaq_add_test(NAME unittests COMMAND ddas_booter_unittests)
