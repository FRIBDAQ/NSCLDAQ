add_subdirectory(figures)

# install-exec-local

install(PROGRAMS MCFD16USB.tcl MCFD16RC.tcl MCFD16Gui.tcl
  MCFD16CommandLogger.tcl MCFD16Memorizer.tcl ScriptHeaderGenerator.tcl
  MCFD16GuiApp.tcl MCFD16Factory.tcl MCFD16ChannelNames.tcl
  DESTINATION TclLibs/mesytec)
install(PROGRAMS MCFD16Control.tcl DESTINATION bin RENAME MCFD16Control)
nscldaq_install_tcl_index(TclLibs/mesytec -verbose "*.tcl")

# check-TESTS

nscldaq_add_tcl_test(NAME tclTests)
