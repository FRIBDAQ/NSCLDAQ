EXTRA_DIST=tclhttpdlaunch.tcl embeddedLaunch.tcl

install-exec-local:
	$(mkinstalldirs) @bindir@
	$(INSTALL_PROGRAM) @srcdir@/tclhttpdlaunch.tcl @bindir@/tclhttpdlaunch
	$(mkinstalldirs) @prefix@/TclLibs/embeddedhttpd
	$(INSTALL_PROGRAM) @srcdir@/embeddedLaunch.tcl \
		@prefix@/TclLibs/embeddedhttpd
	echo pkg_mkIndex -verbose @prefix@/TclLibs/embeddedhttpd embeddedLaunch.tcl |DAQSHARE=@datarootdir@  @TCLSH_CMD@


