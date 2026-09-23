#  Configured files (AC_CONFIG_FILES).

nscldaq_configure_file(InstallRoot.tcl.in InstallRoot.tcl)
nscldaq_configure_file(ReadoutShell.tcl.in ReadoutShell.tcl)

#  The SecretTk program.

nscldaq_add_executable(SecretTk SOURCES SecretTk.c)
target_include_directories(SecretTk PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/CopyrightTools)
target_compile_definitions(SecretTk PRIVATE HOME="${prefix}")
target_link_libraries(SecretTk PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::Tcl)

#  install-exec-hook:

set(_readoutgui_tclpackages
  Configuration.tcl DAQParameters.tcl ExpFileSystemConfig.tcl
  InitializeConfiguration.tcl directories.tcl DataSourceManager.tcl
  RunstateMachine.tcl rdoCalloutsBundle.tcl eventLogBundle.tcl
  DataSourceUI.tcl ui.tcl sshProvider.tcl sshPrompt.tcl janusSshProvider.tcl
  janusSshPrompt.tcl s800Provider.tcl ReadoutGui.tcl state.tcl
  DataSourceMonitor.tcl remoteControl.tcl ReadoutGuiClient.tcl
  readoutGuiProvider.tcl error.png delayProvider.tcl offlineProvider.tcl
  multilogger.tcl stageareaValidation.tcl ReadoutGUIAppLauncher.tcl
  evbparameters.tcl evbsourceprompt.tcl sourceprompt.tcl BundleManager.tcl
  runStateObserver.tcl)

nscldaq_install_mkdir(bin TclLibs/Stager share/html)
install(PROGRAMS ${_readoutgui_tclpackages} DESTINATION TclLibs/Stager)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/InstallRoot.tcl DESTINATION TclLibs/Stager)
# (TCLAPPS is empty so the for loop over it installed nothing.)
install(PROGRAMS RunReadout.sh DESTINATION TclLibs)
nscldaq_install_tcl_index(TclLibs/Stager -verbose "*.tcl")
install(FILES RunState.jpg RunState.eps readoutshell.jpg readoutshell.eps
  eventlogsettings.jpg eventlogsettings.eps DESTINATION share/html)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ReadoutShell.tcl DESTINATION bin
  RENAME ReadoutShell)
install(PROGRAMS start.bash DESTINATION bin)
install(PROGRAMS evbwizard.tcl DESTINATION bin RENAME evbwizard)

#  check-TESTS:  A=1234 is for the state.test restore-6

nscldaq_add_tcl_test(NAME tclTests ENVIRONMENT A=1234)
