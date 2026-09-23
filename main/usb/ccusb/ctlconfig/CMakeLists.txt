nscldaq_configure_file(driver_Makefile.in driver_Makefile)

nscldaq_add_library(CCUSBCtlConfig
  SOURCES CCtlConfiguration.cpp CModuleCommand.cpp DriverTemplate.cpp
    C894.cpp CPH7106.cpp CWatchCommand.cpp CCAEN894Creator.cpp
    CPH7106Creator.cpp CTclControlModule.cpp CTclModuleCreator.cpp
    CCCUSBControl.cpp CCCUSBControlCreator.cpp)
target_include_directories(CCUSBCtlConfig PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/ccusb
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/ccusb/daqconfig
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/os)
# (usb/ccusb/rdothread and usb/ccusb/router were also on the include path
#  but no longer exist.)
target_link_libraries(CCUSBCtlConfig
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC CCUSB CCUSBDaqConfig ConfigurableObject Tcp NSCLDAQ::Tcl
    NSCLDAQ::tclPlus daqshm daqthreads NSCLDAQ::Threads)

# install-data-local

file(GLOB _ccusb_ctlconfig_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${_ccusb_ctlconfig_headers} DESTINATION ccusbdriver/includes)
install(FILES sampleDriver.cpp DESTINATION ccusbdriver/example)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/driver_Makefile
  DESTINATION ccusbdriver/example RENAME Makefile)

## UNIT TESTS

nscldaq_add_executable(ccusb_ctlconfig_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp cctlconfigurationtest.cpp cccusbcontroltests.cpp)
target_include_directories(ccusb_ctlconfig_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/testutils
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ctlconfig
  ${PROJECT_SOURCE_DIR}/usb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject)
target_link_libraries(ccusb_ctlconfig_unittests PRIVATE
  NSCLDAQ::PixieHeaders CCUSBCtlConfig ConfigurableObject NSCLDAQ::Tcl
  NSCLDAQ::tclPlus NSCLDAQ::CppUnit)

nscldaq_add_test(NAME unittests COMMAND ccusb_ctlconfig_unittests)
