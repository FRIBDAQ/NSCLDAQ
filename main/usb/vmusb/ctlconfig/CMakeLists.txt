#
#  usb/vmusb/ctlconfig: slow controls configuration for VMUSBReadout.
#

set(_ctlconfig_includes
  ${PROJECT_SOURCE_DIR}/usb/vmusb/vmusb
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/usb/vmusb/daqconfig
  ${PROJECT_SOURCE_DIR}/usb/vmusb/core
  ${PROJECT_SOURCE_DIR}/usb/vmusb
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/os)

nscldaq_add_library(VMUSBCtlConfig
  SOURCES CCtlConfiguration.cpp CWatchCommand.cpp CMonCommand.cpp
    CModuleCommand.cpp CGDG.cpp CV812.cpp CVMUSBModule.cpp CV6533.cpp
    CJtecgdgCreator.cpp CV812Creator.cpp CVMUSBCreator.cpp CV6533Creator.cpp
    CXLMControls.cpp CXLMControlsCreator.cpp CMxDCRCBus.cpp
    CMxDCRCBusCreator.cpp ChicoTrigger.cpp CChicoTriggerCreator.cpp
    CWienerMDGG16.cpp CMDGG16Control.cpp CMDGG16ControlCreator.cpp
    CTclModuleCreator.cpp CTclControlModule.cpp CMarkerObject.cpp
    CMarkerCreator.cpp CMxDCReset.cpp CMxDCResetCreator.cpp)
target_include_directories(VMUSBCtlConfig PRIVATE ${_ctlconfig_includes})
target_link_libraries(VMUSBCtlConfig
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  PUBLIC VMUSB VMUSBDaqConfig ConfigurableObject NSCLDAQ::tclPlus daqshm
    daqthreads NSCLDAQ::Tcl NSCLDAQ::Threads)

nscldaq_add_library(UserDriver SOURCES CUserDriverSample.cpp)
target_include_directories(UserDriver PRIVATE ${_ctlconfig_includes})
target_link_libraries(UserDriver
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
    NSCLDAQ::Threads)

#  S800 trigger run number setter (s800TriggerRegisters.cpp is copied from
#  ../daqconfig as the Makefile.am did).

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/s800TriggerRegisters.cpp
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/../daqconfig/s800TriggerRegisters.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/s800TriggerRegisters.cpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../daqconfig/s800TriggerRegisters.cpp
  VERBATIM)

nscldaq_add_executable(sets800triggerrun
  SOURCES sets800triggerrun.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/s800TriggerRegisters.cpp)
target_include_directories(sets800triggerrun PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../daqconfig ${CMAKE_CURRENT_SOURCE_DIR}/../vmusb)
target_compile_definitions(sets800triggerrun PRIVATE
  DEFAULT_S800REGFILE="${datarootdir}/s800triggers/RegisterFile.json")
target_link_libraries(sets800triggerrun PRIVATE
  NSCLDAQ::JsonCpp VMUSBRemote CVMUSBReadoutList)

#  install-data-local

nscldaq_install_mkdir(vmusbdriver vmusbdriver/includes vmusbdriver/example)
file(GLOB _ctlconfig_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${_ctlconfig_headers} DESTINATION vmusbdriver/includes)
install(FILES CUserDriverSample.cpp DESTINATION vmusbdriver/example)

#  Unit tests.

nscldaq_add_executable(vmusb_ctlconfig_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp cctlconfigurationtest.cpp)
target_include_directories(vmusb_ctlconfig_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/testutils
  ${PROJECT_SOURCE_DIR}/usb/vmusb/vmusb
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject)
target_link_libraries(vmusb_ctlconfig_unittests PRIVATE
  NSCLDAQ::PixieHeaders VMUSBCtlConfig ConfigurableObject NSCLDAQ::Tcl
  NSCLDAQ::tclPlus NSCLDAQ::CppUnit)
# Not registered as a test: Makefile.am defines a check-TESTS rule but no
# TESTS variable, so 'make check' never ran it.  The rule was:
#   ./unittests
