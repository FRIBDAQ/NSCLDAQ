#  install-exec-local

install(PROGRAMS sequencerGui.tcl controller.tcl ReadoutControl.tcl pkgIndex.tcl
  DESTINATION TclLibs/sequencer)
install(PROGRAMS ReadoutCallouts.tcl sequencerActions.tcl
  DESTINATION examples/sequencer)
