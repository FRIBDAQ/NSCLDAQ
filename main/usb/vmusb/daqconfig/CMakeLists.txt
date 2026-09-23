#
#  usb/vmusb/daqconfig: readout configuration (device drivers) for
#  VMUSBReadout.
#

set(_daqconfig_includes
  ${PROJECT_SOURCE_DIR}/usb/common/devices
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/usb/vmusb
  ${PROJECT_SOURCE_DIR}/usb/vmusb/vmusb
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/os)
set(_daqconfig_defs
  DEFAULT_S800REGFILE="${datarootdir}/s800trigger/RegisterFile.json")

nscldaq_add_library(VMUSBDaqConfig
  SOURCES CADCCommand.cpp CConfiguration.cpp CScalerCommand.cpp
    CCAENChainCommand.cpp CStackCommand.cpp CCAENV830Command.cpp
    CMarkerCommand.cpp CMADCScalerCommand.cpp CMADCChainCommand.cpp
    CHINPCommand.cpp CPSDCommand.cpp CV1729Command.cpp CV1495scCommand.cpp
    CUserCommand.cpp CAddTclDriver.cpp CVMUSBCommand.cpp CReadoutModule.cpp
    C3820.cpp C785.cpp CCAENChain.cpp CStack.cpp C830.cpp CMarker.cpp
    C3804.cpp CNADC2530.cpp CMesytecBase.cpp CMADC32.cpp CMTDC32.cpp
    CMADCScaler.cpp CV1x90.cpp CV977.cpp CXLM.cpp CMASE.cpp CMADCChain.cpp
    CPSD.cpp CV1729.cpp CV1495sc.cpp CTclModule.cpp CVMUSBControl.cpp
    CDelay.cpp CCBD8210CamacBranchDriver.cpp CCBD8210CrateController.cpp
    CCBD8210ReadoutList.cpp CCBDCamacBranch.cpp CULMTrigger.cpp
    CLeCroy4300B.cpp CLeCroy4434.cpp CXLMFERA.cpp CXLMTimestamp.cpp
    CLeCroy2551.cpp CMQDC32StackBuilder.cpp CMQDC32RdoHdwr.cpp
    CDeviceCommand.cpp CMDPP16QDC.cpp CMDPP32QDC.cpp CMDPP32SCP.cpp
    CMDPP32PADC.cpp CSIS3316.cpp sis3316_class.cpp s800TriggerRegisters.cpp
    CS800TriggerNew.cpp CS800TriggerScalers.cpp)
target_include_directories(VMUSBDaqConfig PRIVATE ${_daqconfig_includes})
target_compile_definitions(VMUSBDaqConfig PRIVATE ${_daqconfig_defs})
target_link_libraries(VMUSBDaqConfig
  PRIVATE NSCLDAQ::VMUSBWorkarounds NSCLDAQ::tclPlusHeaders
    NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  PUBLIC VMUSB ConfigurableObject daqthreads NSCLDAQ::tclPlus NSCLDAQ::Tcl
    NSCLDAQ::Threads NSCLDAQ::JsonCpp)

#  install-data-local

nscldaq_install_mkdir(vmusbdriver vmusbdriver/includes share/s800trigger)
file(GLOB _daqconfig_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${_daqconfig_headers} DESTINATION vmusbdriver/includes)
install(FILES RegisterFile.json DESTINATION share/s800trigger)

#  Tests.

nscldaq_add_executable(vmusb_daqconfig_cmdtests TEST OUTPUT_NAME cmdtests
  SOURCES TestRunner.cpp ad811test.cpp v830tests.cpp madcscaler.cpp
    markertests.cpp sis3820test.cpp v1495tests.cpp v1729tests.cpp
    sis3804tests.cpp hytectests.cpp madctests.cpp tdc1x90tests.cpp
    v977tests.cpp masetests.cpp mtdctests.cpp delaytests.cpp
    xlmferatests.cpp cbdtests.cpp xlmtimestamptests.cpp mqdctests.cpp
    cbdcratetests.cpp ulmtriggertests.cpp lrs4300tests.cpp lrs4434tests.cpp
    lrs2551tests.cpp mdpp32qdctests.cpp s800triggerregistertests.cpp)
target_include_directories(vmusb_daqconfig_cmdtests PRIVATE ${_daqconfig_includes})
target_compile_definitions(vmusb_daqconfig_cmdtests PRIVATE ${_daqconfig_defs}
  SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(vmusb_daqconfig_cmdtests PRIVATE
  NSCLDAQ::VMUSBWorkarounds NSCLDAQ::PixieHeaders NSCLDAQ::CppUnit
  VMUSBDaqConfig VMUSB ConfigurableObject daqthreads NSCLDAQ::tclPlus
  NSCLDAQ::Tcl NSCLDAQ::Threads NSCLDAQ::JsonCpp)
nscldaq_add_test(NAME cmdtests COMMAND vmusb_daqconfig_cmdtests)
