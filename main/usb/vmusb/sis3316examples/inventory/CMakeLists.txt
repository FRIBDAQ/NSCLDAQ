#
#  usb/vmusb/sis3316examples/inventory: inventory (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_inventory OUTPUT_NAME inventory DESTINATION sis3316
  SOURCES inventory.cpp sis3316_class.cpp)
target_include_directories(sis3316_inventory PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_inventory PRIVATE VMUSB NSCLDAQ::Tcl)
