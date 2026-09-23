nscldaq_add_library(Configuration
  SOURCES ConfigurationParser.cpp Configuration.cpp
    FirmwareVersionFileParser.cpp ModEvtFileParser.cpp HardwareRegistry.cpp)

nscldaq_install_headers(ConfigurationParser.h Configuration.h
  FirmwareVersionFileParser.h ModEvtFileParser.h HardwareRegistry.h)

# Tests

nscldaq_add_executable(ddas_configuration_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp ConfigurationParserTest.cpp
    FirmwareVersionFileParserTest.cpp ModEvtLenFileParserTest.cpp
    ConfigurationTest.cpp HardwareRegistryTests.cpp NewParseTests.cpp)
target_include_directories(ddas_configuration_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/base/testutils)
target_compile_definitions(ddas_configuration_unittests PRIVATE
  TOP_SRC_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(ddas_configuration_unittests PRIVATE
  Configuration NSCLDAQ::CppUnit)
nscldaq_add_test(NAME unittests COMMAND ddas_configuration_unittests)
