nscldaq_add_executable(ccusb_tclservertests_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp modulecommandtest.cpp c894test.cpp cph7106test.cpp
    cccusbcontroltest.cpp ctclmoduletest.cpp)
target_include_directories(ccusb_tclservertests_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ctlconfig
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(ccusb_tclservertests_unittests PRIVATE
  NSCLDAQ::PixieHeaders
  CCUSB ConfigurableObject CCUSBCtlConfig CCUSBCore Tcp NSCLDAQ::tclPlus
  NSCLDAQ::Exception NSCLDAQ::CppUnit NSCLDAQ::Tcl daqthreads)

nscldaq_add_test(NAME unittests COMMAND ccusb_tclservertests_unittests)
