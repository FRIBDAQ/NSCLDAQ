#
#  usb/vmusb/controlpanels/cv6533: CAEN V6533 HV control panel.
#
#  Note: as in the Makefile.am the package index is built *before* the
#  package files are installed, so a fresh install has no pkgIndex.tcl here.
#

nscldaq_install_mkdir(TclLibs TclLibs/v6533)
nscldaq_install_tcl_index(TclLibs/v6533 -verbose "*.tcl")
install(PROGRAMS channel.tcl channelParams.tcl v6533.tcl DESTINATION TclLibs/v6533)
install(PROGRAMS v6533panel.tcl DESTINATION bin)
