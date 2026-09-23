nscldaq_configure_file(Makefile-template.in Makefile-template)
nscldaq_configure_file(CCUSBDriverIncludes.in CCUSBDriverIncludes)

# install-data-local

nscldaq_install_mkdir(ccusbdriver/includes)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CCUSBDriverIncludes drivertemplate.cpp
  DESTINATION ccusbdriver)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile-template
  DESTINATION ccusbdriver RENAME Makefile)
