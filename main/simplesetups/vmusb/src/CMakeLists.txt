#
#  Sample SpecTcl sources for the VM-USB simple setup.
#

nscldaq_configure_file(Example_Makefile.in Example_Makefile)

# share_DATA (sharedir = @prefix@/share/src/vmusb_simple_setup)
install(FILES
  CRawADCUnpacker.cpp CRawADCUnpacker.h
  CRawUnpacker.cpp CRawUnpacker.h
  MySpecTclApp.cpp MySpecTclApp.h
  SpecTclRC.tcl
  DESTINATION share/src/vmusb_simple_setup)

# install-data-local
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Example_Makefile
  DESTINATION share/src/vmusb_simple_setup RENAME Makefile)
