#
#  usb/vmusb/sis3316examples/FPBUS_threads_allmodules: FPBUS_threads_allmodules (installed in ${prefix}/sis3316).
#

nscldaq_add_executable(sis3316_FPBUS_threads_allmodules OUTPUT_NAME FPBUS_threads_allmodules DESTINATION sis3316
  SOURCES get_configuration_parameter_appl.cpp sis3316_cern_root_class.cpp sis3316_class.cpp sis3316_ethernet_access_class.cpp sis3316_FP_BUS_Threads_appl_allModules.cpp)
target_include_directories(sis3316_FPBUS_threads_allmodules PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(sis3316_FPBUS_threads_allmodules PRIVATE NSCLDAQ::ROOT VMUSB NSCLDAQ::Tcl)
