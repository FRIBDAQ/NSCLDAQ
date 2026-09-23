#
#  usb/vmusb/tclservertests: tests of the slow controls server modules.
#

nscldaq_add_executable(vmusb_tclservertests_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp mxdcrcbustest.cpp modulecommandtest.cpp
    cgdgtest.cpp cv6533test.cpp cchicotriggertest.cpp
    ctclcontrolmoduletest.cpp cxlmcontrolstest.cpp cvmusbmoduletest.cpp
    cwienermdgg16test.cpp cmdgg16controltest.cpp)
target_include_directories(vmusb_tclservertests_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/vmusb/vmusb
  ${PROJECT_SOURCE_DIR}/usb/vmusb/ctlconfig
  ${PROJECT_SOURCE_DIR}/usb/vmusb/core
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(vmusb_tclservertests_unittests PRIVATE
  NSCLDAQ::PixieHeaders VMUSBCore VMUSBCtlConfig VMUSB ConfigurableObject
  NSCLDAQ::tclPlus NSCLDAQ::CppUnit NSCLDAQ::Tcl daqthreads)
nscldaq_add_test(NAME unittests COMMAND vmusb_tclservertests_unittests)
