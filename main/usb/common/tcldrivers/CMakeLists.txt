#
#  usb/common/tcldrivers: the XXUSBSupport Tcl package.
#

nscldaq_install_mkdir(bin TclLibs/XXUSBSupport share/html)
install(PROGRAMS DriverSupport.tcl readoutscript.tcl controlscript.tcl
  Globals.tcl xxusb.tcl tclTests.tcl
  DESTINATION TclLibs/XXUSBSupport)
nscldaq_install_tcl_index(TclLibs/XXUSBSupport -verbose "*.tcl")

# Not registered as a test: Makefile.am defines a check-TESTS rule but no
# TESTS variable, so 'make check' never ran it.  The rule was:
#   TCLLIBPATH="@prefix@/TclLibs @prefix@/lib" ... tclsh @srcdir@/tclTests.tcl
