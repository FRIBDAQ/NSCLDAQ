#
#  readoutREST - REST control of Readout programs: Tcl server/client
#  packages and python programs/modules.
#

set(_rdo_python_programs rdo_control.py rdo_runFromKv.py rdo_titleFromKv.py
  rdo_PyRunControl.py)
set(_rdo_server_scripts ReadoutControl.tcl ReadoutStatus.tcl ReadoutParameters.tcl)
set(_rdo_library_scripts ReadoutREST.tcl ReadoutRESTClient.tcl ReadoutRESTUI.tcl
  rdo_utils.tcl)
set(_rdo_share_scripts rest_init_script.tcl)
set(_rdo_python_modules readoutRestClient.py rdo_utils.py
  RunInfo.py RunInfoController.py
  RunState.py RunStateController.py
  Eventlog.py EventlogController.py
  ReadoutStatistics.py ReadoutStatisticsController.py
  TimedRun.py TimedRunController.py
  ReadoutStatus.py ReadoutStatusController.py)

# install-exec-hook

foreach(_f IN LISTS _rdo_python_programs)
  get_filename_component(_name ${_f} NAME_WE)
  install(PROGRAMS ${_f} DESTINATION bin RENAME ${_name})
endforeach()
install(FILES ${_rdo_library_scripts} DESTINATION TclLibs/readoutREST)
nscldaq_install_tcl_index(TclLibs/readoutREST -verbose ${_rdo_library_scripts})
install(FILES ${_rdo_server_scripts} DESTINATION TclLibs/readoutREST/server)
install(FILES ${_rdo_share_scripts} DESTINATION share/scripts)
install(PROGRAMS ${_rdo_python_modules} DESTINATION pythonLibs/nscldaq/readoutREST)

# check: test_scripts is empty so there are no tests.
