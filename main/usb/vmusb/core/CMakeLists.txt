#
#  usb/vmusb/core: libVMUSBCore - the readout framework of VMUSBReadout.
#

nscldaq_add_library(VMUSBCore
  SOURCES CAcquisitionThread.cpp CControlQueues.cpp COutputThread.cpp
    CRunState.cpp DataBuffer.cpp CSystemControl.cpp CBeginRun.cpp
    CEndRun.cpp CPauseRun.cpp CResumeRun.cpp CMonVar.cpp CInit.cpp
    CExit.cpp TclServer.cpp Server.cpp serverinstance.cpp
    CRunStateCommand.cpp CGetCommand.cpp CSetCommand.cpp
    CUpdateCommand.cpp CStatisticsCommand.cpp tclUtil.cpp)
target_include_directories(VMUSBCore PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/vmusb/vmusb
  ${PROJECT_SOURCE_DIR}/usb/vmusb/daqconfig
  ${PROJECT_SOURCE_DIR}/usb/vmusb/ctlconfig
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/usb/vmusb
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder)
target_link_libraries(VMUSBCore
  PRIVATE NSCLDAQ::TclHeaders NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
    NSCLDAQ::UFMTHeaders NSCLDAQ::VMUSBWorkarounds
  PUBLIC ConfigurableObject VMUSBDaqConfig VMUSBCtlConfig VMUSB
    NSCLDAQ::tclPlus daqthreads daqshm dataformat NSCLDAQ::Tcl
    NSCLDAQ::Threads NSCLDAQ::OpenSSL)

#  Tests.

nscldaq_add_executable(vmusb_core_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp monvartests.cpp monvartimertests.cpp
    monvarcmdtests.cpp CMonVar.cpp DataBuffer.cpp)
target_include_directories(vmusb_core_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/base/thread ${PROJECT_SOURCE_DIR}/daq/format)
target_link_libraries(vmusb_core_unittests PRIVATE
  NSCLDAQ::PixieHeaders daqthreads NSCLDAQ::tclPlus NSCLDAQ::Tcl
  NSCLDAQ::CppUnit)
nscldaq_add_test(NAME unittests COMMAND vmusb_core_unittests)
