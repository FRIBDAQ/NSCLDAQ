#
#  usb/vmusb/devtests: device driver tests.
#

nscldaq_add_executable(vmusb_devtests_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp cvmusbtest.cpp cmqdc32test.cpp cmqdc32stacktest.cpp)
target_include_directories(vmusb_devtests_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../daqconfig
  ${CMAKE_CURRENT_SOURCE_DIR}/../vmusb
  ${CMAKE_CURRENT_SOURCE_DIR}/../core
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(vmusb_devtests_unittests PRIVATE
  NSCLDAQ::TclOnlyHeaders VMUSBDaqConfig ConfigurableObject VMUSB
  NSCLDAQ::CppUnit NSCLDAQ::Threads NSCLDAQ::tclPlus NSCLDAQ::Tcl daqthreads)
# There is no TESTS variable in the Makefile.am, so 'make check' built
# unittests but never ran its check-TESTS rule; it is not registered as a
# test here either.
