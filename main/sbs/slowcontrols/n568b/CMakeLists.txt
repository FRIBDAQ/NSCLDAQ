# install-exec-local:

nscldaq_install_mkdir(TclLibs/v568 bin TclLibs/data/n568)
nscldaq_install_code([[chmod o+rx "$DESTDIR_PREFIX/TclLibs/v568" "$DESTDIR_PREFIX/bin" "$DESTDIR_PREFIX/TclLibs/data/n568"]])
install(PROGRAMS n568b.tcl n568Panel.tcl DESTINATION TclLibs/v568)
foreach(_app loadshaper n568Control)
  install(PROGRAMS ${_app}.tcl DESTINATION bin RENAME ${_app})
endforeach()
install(FILES gui.html overview.html close.html config.html exit.html
  filemenu.html helpmenu.html new.html read.html write.html
  DESTINATION TclLibs/data/n568)
nscldaq_install_tcl_index(TclLibs/v568 -verbose "*.tcl")
