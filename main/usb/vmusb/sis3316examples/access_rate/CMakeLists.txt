#
#  usb/vmusb/sis3316examples/access_rate: ethernet_access_rate (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_ethernet_access_rate OUTPUT_NAME ethernet_access_rate DESTINATION sis3316
  SOURCES sis3316_class.cpp sis3316_eth_access_rate_test.cpp sis3316_ethernet_access_class.cpp)
target_include_directories(sis3316_ethernet_access_rate PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb)
target_link_libraries(sis3316_ethernet_access_rate PRIVATE NSCLDAQ::ROOT)
