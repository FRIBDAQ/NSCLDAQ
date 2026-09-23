#  install-exec-local

install(PROGRAMS pybufdump.py DESTINATION bin RENAME pybufdump)
install(FILES samplePlugin.py DESTINATION share/pydumper)
install(PROGRAMS pyUI.py pybufdumpController.py
  DESTINATION pythonLibs/nscldaq/pydumper)
