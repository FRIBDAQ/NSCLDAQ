#
#  usb/common/tcldrivers: the XXUSBSupport Tcl package.
#

nscldaq_install_mkdir(bin TclLibs/XXUSBSupport share/html)
install(PROGRAMS DriverSupport.tcl readoutscript.tcl controlscript.tcl
  Globals.tcl xxusb.tcl tclTests.tcl
  DESTINATION TclLibs/XXUSBSupport)
nscldaq_install_tcl_index(TclLibs/XXUSBSupport -verbose "*.tcl")

nscldaq_add_tcl_test(NAME tclTests TCLLIBPATH ${prefix}/TclLibs ${prefix}/lib)
