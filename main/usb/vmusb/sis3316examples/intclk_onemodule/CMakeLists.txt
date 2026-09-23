#
#  usb/vmusb/sis3316examples/intclk_onemodule: intclk_onemodule (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_intclk_onemodule OUTPUT_NAME intclk_onemodule DESTINATION sis3316
  SOURCES get_configuration_parameter_appl.cpp sis3316_cern_root_class.cpp sis3316_class.cpp sis3316_ethernet_access_class.cpp sis3316_InternalClk_appl_oneModule.cpp)
target_include_directories(sis3316_intclk_onemodule PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_intclk_onemodule PRIVATE NSCLDAQ::ROOT VMUSB NSCLDAQ::Tcl)
