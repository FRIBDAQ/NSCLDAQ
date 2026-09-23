#
#  sumscaler - sums scalers over runs.
#

nscldaq_gengetopt(options options.ggo ARGS --unamed-opts)

set(_scalersum_includes
  ${PROJECT_SOURCE_DIR}/daq/IO ${PROJECT_SOURCE_DIR}/daq/format)

nscldaq_add_executable(sumscaler
  SOURCES main.cpp App.cpp CRun.cpp CIncrementalChannel.cpp
    CCumulativeChannel.cpp)
nscldaq_use_gengetopt(sumscaler options)
target_include_directories(sumscaler PRIVATE ${_scalersum_includes})
target_link_libraries(sumscaler PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  daqio dataformat NSCLDAQ::Exception)

# install-exec-hook

install(PROGRAMS makeNameFile.tcl DESTINATION bin RENAME makeNameFile)
install(PROGRAMS pyMakeNameFile.py DESTINATION bin RENAME pyMakeNameFile)

# Tests

nscldaq_add_executable(scalersum_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp channelTests.cpp runTests.cpp CRun.cpp
    CCumulativeChannel.cpp CIncrementalChannel.cpp)
target_include_directories(scalersum_unittests PRIVATE ${_scalersum_includes})
target_link_libraries(scalersum_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders NSCLDAQ::CppUnit
  daqio dataformat NSCLDAQ::Exception)
nscldaq_add_test(NAME unittests COMMAND scalersum_unittests)
