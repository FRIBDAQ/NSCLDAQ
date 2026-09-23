#  install-exec-local

install(PROGRAMS burnpackage.tcl TopGUi.ui.tcl dvdburnoutput.ui.tcl
  DESTINATION TclLibs/dvdburn)
install(PROGRAMS dvdburn burngui DESTINATION bin)
nscldaq_install_tcl_index(TclLibs/dvdburn "*.tcl")
