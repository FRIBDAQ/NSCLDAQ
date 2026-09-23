#
#  managercontrol - python GUI to control the manager (mg_control).
#

set(_mgcontrol_modules config.py cfgwizard.py maingui.py programlist.py
  loggerlist.py ReadoutStatus.py)

# install-data-local

install(PROGRAMS ${_mgcontrol_modules}
  DESTINATION pythonLibs/nscldaq/manager_control)
install(CODE "file(TOUCH \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/pythonLibs/nscldaq/manager_control/__init__.py\")")
install(PROGRAMS mg_control.py DESTINATION bin RENAME mg_control)
