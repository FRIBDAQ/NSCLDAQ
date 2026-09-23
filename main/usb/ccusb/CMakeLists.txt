add_subdirectory(ccusb)
add_subdirectory(daqconfig)
add_subdirectory(ctlconfig)
add_subdirectory(core)
add_subdirectory(driverkit)
add_subdirectory(tcldrivers)
add_subdirectory(controlpanels)
add_subdirectory(tclpackage)
add_subdirectory(devtests)
add_subdirectory(tclservertests)

#
#  The CC-USB readout program.
#

nscldaq_gengetopt(cmdline commandline.ggo ARGS --set-version=${VERSION})

nscldaq_add_executable(CCUSBReadout SOURCES CTheApplication.cpp Globals.cpp)
nscldaq_use_gengetopt(CCUSBReadout cmdline)
target_include_directories(CCUSBReadout PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/ccusb/daqconfig
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ctlconfig
  ${PROJECT_SOURCE_DIR}/usb/ccusb/core
  ${PROJECT_SOURCE_DIR}/usb/common/slowcontrols
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/servers/portmanager)
target_link_libraries(CCUSBReadout PRIVATE
  NSCLDAQ::PixieHeaders
  CCUSB CCUSBDaqConfig CCUSBCtlConfig CCUSBCore PortManager DataFlow
  daqthreads ConfigurableObject daqshm NSCLDAQ::tclPlus NSCLDAQ::Tcl
  NSCLDAQ::Threads)

# install-data-local

file(GLOB _ccusb_readout_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${_ccusb_readout_headers} DESTINATION ccusbdriver/include)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(CCUSBReadout)
