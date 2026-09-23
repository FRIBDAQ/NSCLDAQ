#
#  usb/vmusb/sis3316examples/internaltrigger_externalgate_dt: inttrig_extgate_dt (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_inttrig_extgate_dt OUTPUT_NAME inttrig_extgate_dt DESTINATION sis3316
  SOURCES sis3316_cern_root_class.cpp sis3316_class.cpp sis3316_ethernet_access_class.cpp sis3316_internalTrigger_externalGate_deadTime.cpp)
target_include_directories(sis3316_inttrig_extgate_dt PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_inttrig_extgate_dt PRIVATE NSCLDAQ::ROOT VMUSB NSCLDAQ::Tcl)
