nscldaq_add_executable(ccusbloader SOURCES ccusb_flash.cpp skipHeader.cpp libxxusb.cpp)
target_link_libraries(ccusbloader PRIVATE NSCLDAQ::USB)

nscldaq_add_executable(vmusbloader SOURCES vmusb_flash.cpp skipHeader.cpp libxxusb.cpp)
target_link_libraries(vmusbloader PRIVATE NSCLDAQ::USB)

#  xlmload links ../vmusb's VMUSBReadout-Globals.o; compile that same
#  source here with VMUSBReadout's flags.

add_library(loaders_VMUSBReadout_Globals OBJECT
  ${PROJECT_SOURCE_DIR}/usb/vmusb/Globals.cpp)
# Compiled as part of a program there, so not PIC.
set_property(TARGET loaders_VMUSBReadout_Globals PROPERTY POSITION_INDEPENDENT_CODE OFF)
target_include_directories(loaders_VMUSBReadout_Globals PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/vmusb
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
target_link_libraries(loaders_VMUSBReadout_Globals PRIVATE
  NSCLDAQ::Threads NSCLDAQ::VMUSBWorkarounds NSCLDAQ::tclPlusHeaders
  NSCLDAQ::TclOnlyHeaders NSCLDAQ::USBHeaders NSCLDAQ::PixieHeaders)

nscldaq_gengetopt(xlmloadparams xlmloadparams.ggo)

nscldaq_add_executable(xlmload SOURCES xlmload.cpp)
nscldaq_use_gengetopt(xlmload xlmloadparams)
target_include_directories(xlmload PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/vmusb/core
  ${PROJECT_SOURCE_DIR}/usb/vmusb/daqconfig
  ${PROJECT_SOURCE_DIR}/usb/vmusb/vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(xlmload PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  VMUSBRemote VMUSB VMUSBDaqConfig NSCLDAQ::tclPlus
  loaders_VMUSBReadout_Globals daqthreads)

#-----------------------------------------------------------------------------
# Unit tests:

nscldaq_add_executable(loaders_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp skiptest.cpp skipHeader.cpp)
target_compile_definitions(loaders_unittests PRIVATE
  SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/")
target_link_libraries(loaders_unittests PRIVATE
  NSCLDAQ::PixieHeaders NSCLDAQ::CppUnit)

nscldaq_add_test(NAME unittests COMMAND loaders_unittests)
