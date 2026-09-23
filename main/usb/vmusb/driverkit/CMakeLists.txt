#
#  usb/vmusb/driverkit: the kit users build VM-USB drivers with.
#

nscldaq_configure_file(Makefile-template.in Makefile-template)
nscldaq_configure_file(VMUSBDriverIncludes.in VMUSBDriverIncludes)

nscldaq_install_mkdir(vmusbdriver vmusbdriver/includes)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/VMUSBDriverIncludes
  drivertemplate.cpp
  DESTINATION vmusbdriver)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile-template
  DESTINATION vmusbdriver RENAME Makefile)
