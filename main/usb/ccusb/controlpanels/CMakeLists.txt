# install-exec-local

install(PROGRAMS ledcontrol.tcl ULMTriggerGUIPanel.tcl
  DESTINATION TclLibs/ccusbpanels)
nscldaq_install_tcl_index(TclLibs/ccusbpanels "*.tcl")
install(PROGRAMS ledph7106.tcl DESTINATION bin)
install(PROGRAMS ULMTriggerGUI.tcl DESTINATION bin RENAME ULMTriggerGUI)
install(FILES ulmtrigger_startup.png DESTINATION share/html/figures)
