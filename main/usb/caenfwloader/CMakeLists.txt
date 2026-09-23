nscldaq_add_executable(vmusbcaenupgrader SOURCES CAENComm_VMUSB.cpp)
target_include_directories(vmusbcaenupgrader PRIVATE
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/caenfw-common
  ${PROJECT_SOURCE_DIR}/usb/vmusb/vmusb)
target_link_libraries(vmusbcaenupgrader PRIVATE
  caenfwloadCommon VMUSB daqthreads NSCLDAQ::tclPlus NSCLDAQ::Tcl)
