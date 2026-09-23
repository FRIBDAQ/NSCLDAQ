#
#  usb/vmusb/sis3316examples/rebootall: sis3316_rebootall (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_rebootall DESTINATION sis3316
  SOURCES sis3316_class.cpp sis3316_ethernet_access_class.cpp sis3316_reboot_all_modules.cpp)
target_include_directories(sis3316_rebootall PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_rebootall PRIVATE VMUSB NSCLDAQ::Tcl)
