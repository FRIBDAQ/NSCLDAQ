#
#  usb/vmusb/sis3316examples/fwupdate: fwupdate (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_fwupdate OUTPUT_NAME fwupdate DESTINATION sis3316
  SOURCES sis3316_class.cpp sis3316_ethernet_access_class.cpp sis3316_eth_fpga_update.cpp)
target_include_directories(sis3316_fwupdate PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_fwupdate PRIVATE VMUSB NSCLDAQ::Tcl)
