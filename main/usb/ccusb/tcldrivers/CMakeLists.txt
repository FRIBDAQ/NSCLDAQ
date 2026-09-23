# install-exec-local

install(PROGRAMS ccusb.tcl CCUSBDriverSupport.tcl ACAENC671.tcl ACAENC808.tcl
  ALeCroy4448.tcl ATrigger2367.tcl APhillips71xx.tcl ALeCroy4300B.tcl
  AOrtecCCF8200.tcl
  DESTINATION TclLibs/ccusbdrivers)
nscldaq_install_tcl_index(TclLibs/ccusbdrivers -verbose "*.tcl")

# check-TESTS: index MockCCUSB.tcl in the source directory, then run the
# tests.

nscldaq_add_test(NAME mkindex_MockCCUSB
  COMMAND sh -c "echo 'pkg_mkIndex -verbose ${CMAKE_CURRENT_SOURCE_DIR} MockCCUSB.tcl' | ${TCLSH_CMD}")
nscldaq_add_tcl_test(NAME tclTests
  TCLLIBPATH ${prefix}/TclLibs ${libdir} ${CMAKE_CURRENT_SOURCE_DIR})
if(BUILD_TESTING)
  set_tests_properties(usb/ccusb/tcldrivers/mkindex_MockCCUSB PROPERTIES
    FIXTURES_SETUP ccusb_tcldrivers_mockindex)
  set_tests_properties(usb/ccusb/tcldrivers/tclTests PROPERTIES
    FIXTURES_REQUIRED ccusb_tcldrivers_mockindex)
endif()
