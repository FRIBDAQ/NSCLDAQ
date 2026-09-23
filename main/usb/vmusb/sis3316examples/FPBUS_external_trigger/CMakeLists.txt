#
#  usb/vmusb/sis3316examples/FPBUS_external_trigger: FPBUS_external_trigger (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_FPBUS_external_trigger OUTPUT_NAME FPBUS_external_trigger DESTINATION sis3316
  SOURCES sis3316_cern_root_class.cpp sis3316_class.cpp sis3316_ethernet_access_class.cpp sis3316_FP_Bus_external_trigger.cpp)
target_include_directories(sis3316_FPBUS_external_trigger PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_FPBUS_external_trigger PRIVATE NSCLDAQ::ROOT VMUSB NSCLDAQ::Tcl)
