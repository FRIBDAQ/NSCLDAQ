#
#  The pixieplugin Tcl package (TclLibs/pixieserver).  definitions.tcl is
#  configured; the pkgIndex.tcl is written rather than made by pkg_mkIndex.
#

nscldaq_configure_file(definitions.tcl.in definitions.tcl)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
  "package ifneeded pixieplugin 1.0 [list source [file join \$dir pixieplugin.tcl]]\n")

# install-exec-local

install(FILES pixieplugin.tcl ${CMAKE_CURRENT_BINARY_DIR}/definitions.tcl
  DESTINATION TclLibs/pixieserver)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl DESTINATION TclLibs/pixieserver)
