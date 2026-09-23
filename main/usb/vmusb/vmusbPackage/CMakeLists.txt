#
#  usb/vmusb/vmusbPackage: the vmusbvme Tcl package.
#

nscldaq_install_mkdir(TclLibs/vmusbtcl)
install(PROGRAMS vmusbPackage.tcl DESTINATION TclLibs/vmusbtcl)
install(CODE [=[
  set(_idx "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/TclLibs/vmusbtcl/pkgIndex.tcl")
  message(STATUS "Writing: ${_idx}")
  file(WRITE "${_idx}" "package ifneeded vmusbvme 1.0 [list source [file join $dir vmusbPackage.tcl]]\n")
]=])
