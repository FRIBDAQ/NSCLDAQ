#
#  usb/vmusb/sis3316examples/FPBUS_read_temps: FP_readtemps (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_FP_readtemps OUTPUT_NAME FP_readtemps DESTINATION sis3316
  SOURCES sis3316_class.cpp sis3316_ethernet_access_class.cpp sis3316_FP_Bus_read_temperature.cpp)
target_include_directories(sis3316_FP_readtemps PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_FP_readtemps PRIVATE NSCLDAQ::ROOT VMUSB NSCLDAQ::Tcl)
