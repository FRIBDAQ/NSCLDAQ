nscldaq_add_executable(ccusb_devtests_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp cccusbtest.cpp)
target_include_directories(ccusb_devtests_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/usb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/ccusb/daqconfig
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ccusb)
target_link_libraries(ccusb_devtests_unittests PRIVATE
  NSCLDAQ::PrefixInclude
  CCUSB CCUSBDaqConfig daqthreads Tcp ConfigurableObject NSCLDAQ::tclPlus
  NSCLDAQ::Exception NSCLDAQ::Tcl NSCLDAQ::CppUnit NSCLDAQ::Threads)

# check-TESTS: ./unittests
nscldaq_add_test(NAME unittests COMMAND ccusb_devtests_unittests)
