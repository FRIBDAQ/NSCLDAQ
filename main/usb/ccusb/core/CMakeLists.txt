nscldaq_add_library(CCUSBCore
  SOURCES CAcquisitionThread.cpp CControlQueues.cpp COutputThread.cpp
    CRunState.cpp DataBuffer.cpp CSystemControl.cpp CBeginRun.cpp
    CEndRun.cpp CPauseRun.cpp CResumeRun.cpp CInit.cpp CExit.cpp
    TclServer.cpp Server.cpp serverinstance.cpp CGetCommand.cpp
    CSetCommand.cpp CUpdateCommand.cpp CRunStateCommand.cpp tclUtil.cpp)
target_include_directories(CCUSBCore PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/ccusb/daqconfig
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ctlconfig
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/usb/ccusb
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder)
# add -DREPORT_ERRORS for verbose error reporting info.
target_link_libraries(CCUSBCore
  PRIVATE NSCLDAQ::PixieHeaders NSCLDAQ::UFMTHeaders
  PUBLIC CCUSBDaqConfig CCUSBCtlConfig ConfigurableObject CCUSB
    NSCLDAQ::tclPlus daqthreads daqshm dataformat NSCLDAQ::Tcl
    NSCLDAQ::Threads NSCLDAQ::OpenSSL)
