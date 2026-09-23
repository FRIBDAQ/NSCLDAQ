# install-exec-local:

install(PROGRAMS s800rdocallouts.tcl s800rctl.tcl DESTINATION TclLibs/s800)
install(PROGRAMS dummyrdo DESTINATION bin)
nscldaq_install_tcl_index(TclLibs/s800 "*.tcl")

# check-TESTS:

nscldaq_add_tcl_test(NAME tclTests)
