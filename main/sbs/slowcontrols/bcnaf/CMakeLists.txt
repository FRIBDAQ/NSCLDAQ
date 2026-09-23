# install-exec-local:

install(PROGRAMS bcnaf.tcl DESTINATION bin RENAME bcnaf)
install(PROGRAMS cesbcnaf.tcl DESTINATION bin RENAME cesbcnaf)
install(PROGRAMS wienerbcnaf.tcl DESTINATION bin RENAME wienerbcnaf)
install(PROGRAMS bcnafutils.tcl DESTINATION TclLibs/bcnafutils)
nscldaq_install_tcl_index(TclLibs/bcnafutils -verbose bcnafutils.tcl)
