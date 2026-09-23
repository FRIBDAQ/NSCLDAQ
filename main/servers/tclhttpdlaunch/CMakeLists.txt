#  install-exec-local

install(PROGRAMS tclhttpdlaunch.tcl DESTINATION bin RENAME tclhttpdlaunch)
install(PROGRAMS embeddedLaunch.tcl DESTINATION TclLibs/embeddedhttpd)

#  echo pkg_mkIndex -verbose @prefix@/TclLibs/embeddedhttpd embeddedLaunch.tcl |
#      DAQSHARE=@datarootdir@ @TCLSH_CMD@

nscldaq_install_code(
  "echo pkg_mkIndex -verbose \"$DESTDIR_PREFIX/TclLibs/embeddedhttpd\" embeddedLaunch.tcl | DAQSHARE=\"${datarootdir}\" \"${TCLSH_CMD}\"")
