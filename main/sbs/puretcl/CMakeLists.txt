# install-exec-local:

file(GLOB _puretcl_scripts CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.tcl)
install(PROGRAMS ${_puretcl_scripts} DESTINATION TclLibs/sbspuretcl)
install(PROGRAMS cratelocator.tcl DESTINATION bin RENAME cratelocator)
install(CODE "
  set(_idx \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/TclLibs/sbspuretcl/pkgIndex.tcl\")
  file(WRITE \${_idx} \"package ifneeded camac 1.1 [list source [file join \\\$dir camac.tcl]]\\n\")
  file(APPEND \${_idx} \"package ifneeded wienercamac 1.1 [list source [file join \\\$dir wienercamac.tcl]]\\n\")
  file(APPEND \${_idx} \"package ifneeded caennet 1.5 [list source [file join \\\$dir caennet.tcl]]\\n\")
  file(APPEND \${_idx} \"package ifneeded camacutils 1.0 [list source [file join \\\$dir camacutils.tcl]]\\n\")
")
