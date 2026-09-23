#
#  usb/vmusb/tcldrivers: Tcl VM-USB device drivers (TclLibs/vmusbdrivers).
#

nscldaq_install_mkdir(TclLibs TclLibs/vmusbdrivers)
install(PROGRAMS sis330xdriver.tcl sis3300.tcl VMUSBDriverSupport.tcl
  vmusb.tcl AXLM72.tcl APpacXLM72.tcl ACrdcXLM72.tcl AXLM72Scaler.tcl
  ACAENV288.tcl ACAENN568B.tcl AGD16XLM72.tcl AXLM72Generic.tcl
  ALevel3XLM72Readout.tcl MDPP.tcl
  DESTINATION TclLibs/vmusbdrivers)
nscldaq_install_tcl_index(TclLibs/vmusbdrivers -verbose "*.tcl")

# Not registered as a test: Makefile.am defines a check-TESTS rule but no
# TESTS variable, so 'make check' never ran it.  The rule was:
#   TCLLIBPATH="@prefix@/TclLibs @prefix@/lib" ... tclsh @srcdir@/tclTests.tcl
