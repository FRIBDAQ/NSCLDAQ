# install-exec-local:

nscldaq_install_mkdir(TclLibs/vhq bin TclLibs/data/vhq)
nscldaq_install_code([[chmod o+rx "$DESTDIR_PREFIX/TclLibs/vhq" "$DESTDIR_PREFIX/bin" "$DESTDIR_PREFIX/TclLibs/data/vhq"]])
install(PROGRAMS vhqPanel.tcl vhq202m.tcl DESTINATION TclLibs/vhq)
install(PROGRAMS vhqControl.tcl DESTINATION bin RENAME vhqControl)
install(FILES configfile.html configilimit.html configmenu.html
  configspeeds.html fileclose.html filemenu.html filenew.html filequit.html
  fileread.html filewrite.html gui.html helpmenu.html menubar.html
  overview.html
  DESTINATION TclLibs/data/vhq)
nscldaq_install_tcl_index(TclLibs/vhq "*.tcl")
