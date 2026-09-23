# install-exec-local

install(PROGRAMS ccusbcamac.tcl DESTINATION TclLibs/ccusbpuretcl)
install(CODE [=[
  file(WRITE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/TclLibs/ccusbpuretcl/pkgIndex.tcl"
    "package ifneeded ccusbcamac 1.0 [list source [file join $dir ccusbcamac.tcl]]\n")
]=])
