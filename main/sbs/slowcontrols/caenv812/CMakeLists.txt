# install-exec-local:

nscldaq_install_mkdir(TclLibs/caenv812 bin TclLibs/data/caenv812)
nscldaq_install_code([[chmod o+rx "$DESTDIR_PREFIX/TclLibs/caenv812" "$DESTDIR_PREFIX/bin" "$DESTDIR_PREFIX/TclLibs/data/caenv812"]])
install(PROGRAMS CFD812.tcl CFDState.tcl caenv812Gui.tcl
  DESTINATION TclLibs/caenv812)
foreach(_app caenv812control loadcfd)
  install(PROGRAMS ${_app}.tcl DESTINATION bin RENAME ${_app})
endforeach()
install(PROGRAMS caenv812control.tcl loadcfd.tcl DESTINATION bin)
install(FILES close.html exit.html filemenu.html gui.html helpmenu.html
  new.html read.html write.html config.html overview.html
  DESTINATION TclLibs/data/caenv812)
nscldaq_install_tcl_index(TclLibs/caenv812 "*.tcl")
