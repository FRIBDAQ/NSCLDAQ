nscldaq_add_library(DDASDataSimulator SOURCES DDASDataSimulator.cpp)
target_link_libraries(DDASDataSimulator
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC NSCLDAQ::UFMT NSCLDAQ::Exception NSCLDAQ::DDASFormat)

nscldaq_install_headers(DDASDataSimulator.h)

# Tests

nscldaq_add_executable(ddas_simulator_DDASDataSimulatorTests TEST
  OUTPUT_NAME DDASDataSimulatorTests
  SOURCES TestRunner.cpp DDASDataSimulatorTests.cpp)
target_link_libraries(ddas_simulator_DDASDataSimulatorTests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::UFMT NSCLDAQ::Exception NSCLDAQ::DDASFormat
  DDASDataSimulator NSCLDAQ::CppUnit)
nscldaq_add_test(NAME DDASDataSimulatorTests
  COMMAND ddas_simulator_DDASDataSimulatorTests)
