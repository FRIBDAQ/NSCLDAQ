# install-exec-local ($(HELPDIR) is not defined in this Makefile.am):

nscldaq_install_mkdir(TclLibs/vhs bin)
nscldaq_install_code([[chmod o+rx "$DESTDIR_PREFIX/TclLibs/vhs" "$DESTDIR_PREFIX/bin"]])
install(PROGRAMS vhs.tcl VhsWidgets.tcl DESTINATION TclLibs/vhs)
install(PROGRAMS vhsPanel.tcl DESTINATION bin RENAME vhsPanel)
nscldaq_install_tcl_index(TclLibs/vhs "*.tcl")
