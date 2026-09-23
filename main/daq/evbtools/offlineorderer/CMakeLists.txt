# figures is only in DIST_SUBDIRS (not SUBDIRS) so, as before, it is not
# built or installed.

# install-exec-local:

nscldaq_install_mkdir(bin TclLibs/OfflineEVB share/html)
install(PROGRAMS InputPipeline.tcl HoistPipeline.tcl OutputPipeline.tcl
  JobBuilderUI.tcl JobConfigUI.tcl ConfigurationErrorUI.tcl
  MissingSourceUI.tcl JobProcessor.tcl RunProcessor.tcl RunStatusUI.tcl
  FileListWidget.tcl OfflineOrdererUI.tcl GlobalConfigUI.tcl
  EVBConfigure.tcl Job.tcl
  DESTINATION TclLibs/OfflineEVB)
install(PROGRAMS offlineorderer.tcl DESTINATION bin RENAME offlineorderer)
nscldaq_install_tcl_index(TclLibs/OfflineEVB -verbose "*.tcl")

# check-TESTS:

nscldaq_add_tcl_test(NAME tclTests)
