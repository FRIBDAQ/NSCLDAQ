#  Tcl widget packages -> TclLibs/widgets.

install(PROGRAMS mwutil.tcl notebook.tcl tablelist.tcl tablelistSortByColumn.tcl
    tablelistWidget.tcl tabnbook.tcl FrameSequencer.tcl FrameManager.tcl
    RingStatus.tcl ChannelLabel.tcl TransientLabel.tcl scaleControl.tcl
    nsclspinbox.tcl tkutils.tcl warning.tcl NSCLBgerror.tcl bells.tcl
    icon.tcl titledSpinBox.tcl bindDown.tcl meter.tcl blockimage.tcl
    dialogwrapper.tcl textprompter.tcl selectablelist.tcl OutputWindow.tcl
  DESTINATION TclLibs/widgets)
install(PROGRAMS ReDistribNotes DESTINATION TclLibs/widgets)
nscldaq_install_tcl_index(TclLibs/widgets -verbose "*.tcl")

# Tests

nscldaq_add_tcl_test(NAME tclTests)
