# install-exec-local

install(PROGRAMS ccusb.tcl CCUSBDriverSupport.tcl ACAENC671.tcl ACAENC808.tcl
  ALeCroy4448.tcl ATrigger2367.tcl APhillips71xx.tcl ALeCroy4300B.tcl
  AOrtecCCF8200.tcl
  DESTINATION TclLibs/ccusbdrivers)
nscldaq_install_tcl_index(TclLibs/ccusbdrivers -verbose "*.tcl")

# Not registered as a test: Makefile.am defines a check-TESTS rule but no
# TESTS variable, so 'make check' never ran it.  The rule was:
#   echo pkg_mkIndex -verbose @srcdir@ MockCCUSB.tcl | tclsh
#   TCLLIBPATH="@prefix@/TclLibs @libdir@ @srcdir@" ... tclsh @srcdir@/tclTests.tcl
