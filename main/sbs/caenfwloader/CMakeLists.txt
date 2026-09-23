nscldaq_add_executable(sbscaenupgrader SOURCES CAENComm_SBS.cpp)
target_include_directories(sbscaenupgrader PRIVATE
  ${PROJECT_SOURCE_DIR}/caenfw-common
  ${PROJECT_SOURCE_DIR}/sbs/driver/include)
target_link_libraries(sbscaenupgrader PRIVATE caenfwloadCommon btp)
