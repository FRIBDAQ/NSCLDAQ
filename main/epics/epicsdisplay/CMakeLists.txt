nscldaq_configure_file(epicsdisplay.in epicsdisplay)

if(ENABLE_EPICS_TOOLS)
  # install-exec-local
  install(PROGRAMS displayepics.tcl DESTINATION TclLibs/epicsdisplay)
  install(PROGRAMS EpicsDisplay.tcl ${CMAKE_CURRENT_BINARY_DIR}/epicsdisplay
    DESTINATION bin)
  install(FILES add.html alarmconfig.html channelmenu.html config.html
    filemenu.html gui.html helpmenu.html overview.html
    DESTINATION TclLibs/data/epicsdisplay)
endif()
