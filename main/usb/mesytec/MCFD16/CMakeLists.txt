add_subdirectory(figures)

# install-exec-local

install(PROGRAMS MCFD16USB.tcl MCFD16RC.tcl MCFD16Gui.tcl
  MCFD16CommandLogger.tcl MCFD16Memorizer.tcl ScriptHeaderGenerator.tcl
  MCFD16GuiApp.tcl MCFD16Factory.tcl MCFD16ChannelNames.tcl
  DESTINATION TclLibs/mesytec)
install(PROGRAMS MCFD16Control.tcl DESTINATION bin RENAME MCFD16Control)
nscldaq_install_tcl_index(TclLibs/mesytec -verbose "*.tcl")

# check-TESTS

# Not registered as a test: Makefile.am defines a check-TESTS rule but no
# TESTS variable, so 'make check' never ran it.  The rule was:
#   TCLLIBPATH="@prefix@/TclLibs" ... tclsh @srcdir@/tclTests.tcl
