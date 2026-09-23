#
#  usb/vmusb: VM-USB support and the VMUSBReadout program.
#

add_subdirectory(vmusb)
add_subdirectory(daqconfig)
add_subdirectory(ctlconfig)
add_subdirectory(core)
add_subdirectory(driverkit)
add_subdirectory(tcldrivers)
add_subdirectory(controlpanels)
add_subdirectory(devtests)
add_subdirectory(vmusbPackage)
add_subdirectory(tclservertests)
add_subdirectory(sis3316examples)
add_subdirectory(SIS3316Readout)

#  VMUSBReadout.

nscldaq_gengetopt(cmdline commandline.ggo ARGS --set-version=${VERSION})

nscldaq_add_executable(VMUSBReadout SOURCES CTheApplication.cpp Globals.cpp)
nscldaq_use_gengetopt(VMUSBReadout cmdline)
target_include_directories(VMUSBReadout PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/vmusb/vmusb
  ${PROJECT_SOURCE_DIR}/usb/vmusb/core
  ${PROJECT_SOURCE_DIR}/usb/vmusb/daqconfig
  ${PROJECT_SOURCE_DIR}/usb/vmusb/ctlconfig
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/servers/portmanager
  ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(VMUSBReadout PRIVATE
  NSCLDAQ::VMUSBWorkarounds NSCLDAQ::TclOnlyHeaders NSCLDAQ::PixieHeaders
  VMUSBDaqConfig VMUSBCtlConfig VMUSBCore PortManager DataFlow VMUSB
  NSCLDAQ::tclPlus daqthreads daqshm NSCLDAQ::Tcl NSCLDAQ::USB
  NSCLDAQ::Threads)

#  install-exec-local

nscldaq_install_mkdir(share/html)
install(FILES s800trigger.jpg DESTINATION share/html)
