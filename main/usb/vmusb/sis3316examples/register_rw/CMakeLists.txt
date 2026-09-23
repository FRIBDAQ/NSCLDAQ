#
#  usb/vmusb/sis3316examples/register_rw: register_rw (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_register_rw OUTPUT_NAME register_rw DESTINATION sis3316
  SOURCES sis3316_class.cpp sis3316_ethernet_access_class.cpp sis3316_eth_register_read_write.cpp)
target_include_directories(sis3316_register_rw PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_register_rw PRIVATE VMUSB NSCLDAQ::Tcl)
