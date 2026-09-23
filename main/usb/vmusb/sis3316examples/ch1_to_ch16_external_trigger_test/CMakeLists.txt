#
#  usb/vmusb/sis3316examples/ch1_to_ch16_external_trigger_test: allchans_exttrgtest (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_allchans_exttrgtest OUTPUT_NAME allchans_exttrgtest DESTINATION sis3316
  SOURCES get_configuration_parameters.cpp sis3316_cern_root_class.cpp sis3316_ch1_to_ch16_external_trigger_test.cpp sis3316_class.cpp sis3316_ethernet_access_class.cpp)
target_include_directories(sis3316_allchans_exttrgtest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_allchans_exttrgtest PRIVATE NSCLDAQ::ROOT VMUSB NSCLDAQ::Tcl)
