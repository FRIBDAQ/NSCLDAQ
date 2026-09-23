#
#  manager - the DAQ manager: Tcl server/library packages, python
#  programs and modules.
#

set(_mgr_program_scripts startServer.tcl)
set(_mgr_py_packages mg_database.py manager_client.py editablelist.py
  editablelist6.py mg_clientutils.py mg_kvclient.py OutputMonitorQt.py
  mg_configutils.py)
set(_mgr_pyprograms mg_logwizard.py mg_program_wizard.py mg_import.py
  mg_monitorOutput.py mg_mkconfig.py mg_container_wizard.py
  mg_cfgcontainers.py mg_cfgprogram.py mg_stateedit.py
  mg_seqedit.py mg_loggeredit.py mg_kvedit.py mg_authedit.py
  mg_config.py mg_startManager.py mg_kvget.py mg_shutdown.py
  mg_cfgEvlog.py mg_startloggers.py mg_readout_wizard.py)
set(_mgr_server_scripts mg_statusserver.tcl mg_programserver.tcl
  mg_kvserver.tcl mg_evlogserver.tcl)
set(_mgr_library_scripts mg_containerpackage.tcl mg_containeredit.tcl
  mg_programpackage.tcl mg_programedit.tcl mg_sequencepackage.tcl
  mg_kvstorepackage.tcl mg_auth.tcl
  mg_stateclient.tcl mg_stateclientui.tcl mg_clientutils.tcl
  mg_pgmstatusclient.tcl mg_programstatusclientui.tcl
  mg_kvclient.tcl mg_kvwidgets.tcl mg_clientui.tcl mg_serverutils.tcl
  mg_OutputModel.tcl mg_OutputController.tcl mg_evlogpackage.tcl
  mg_evlogEditViews.tcl mg_evlogEditModel.tcl mg_evlogEditController.tcl
  mg_loggerRESTClient.tcl)
set(_mgr_test_scripts container.test program.test sequence.test
  transitions.test loggerdb.test kv.test authtest.test evlog.test)

# install-exec-local

foreach(_f IN LISTS _mgr_program_scripts)
  get_filename_component(_name ${_f} NAME_WE)
  install(PROGRAMS ${_f} DESTINATION bin RENAME ${_name})
endforeach()
install(FILES ${_mgr_library_scripts} DESTINATION TclLibs/manager)
nscldaq_install_tcl_index(TclLibs/manager -verbose ${_mgr_library_scripts})
install(FILES ${_mgr_server_scripts} DESTINATION TclLibs/manager_server)
# Force recompilation.
install(CODE "file(REMOVE_RECURSE \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/pythonLibs/nscldaq/__pycache__\")")
install(PROGRAMS ${_mgr_py_packages} DESTINATION pythonLibs/nscldaq)
foreach(_f IN LISTS _mgr_pyprograms)
  get_filename_component(_name ${_f} NAME_WE)
  install(PROGRAMS ${_f} DESTINATION bin RENAME ${_name})
endforeach()

#  check: the manager tests need singularity (checked over ssh with
#  singexists.sh copied to ~) and the minimal.img container in ~; without
#  singularity they are skipped.  Same shell logic as the old check rule.

string(REPLACE ";" " " _mgr_test_list "${_mgr_test_scripts}")
nscldaq_add_test(NAME singularityTests
  COMMAND /bin/sh -c "\
install -c ${CMAKE_CURRENT_SOURCE_DIR}/singexists.sh ~ || exit 1; \
if test \"`ssh localhost ~/singexists.sh`\" = \"yes\" ; then \
  install -c -m 644 ${CMAKE_CURRENT_SOURCE_DIR}/minimal.img ~ || exit 1; \
  for script in ${_mgr_test_list}; do \
    DAQBIN=${bindir} SRCDIR=${CMAKE_CURRENT_SOURCE_DIR} TCLLIBPATH=${prefix}/TclLibs PYTHONPATH=${prefix}/pythonLibs \
      ${TCLSH_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/$script || exit 1; \
  done; \
else \
  echo No singularity manager tests will not be done; \
fi")
