#
#  ringdiagnostics - python ring buffer diagnostics GUI.
#

# install-exec-local

install(PROGRAMS pidtocommand.py DESTINATION bin RENAME pidtocommand)
# Force recompilation of the python modules.
install(CODE "file(REMOVE_RECURSE \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/pythonLibs/nscldaq/__pycache__\")")
file(GLOB _ringdiag_py CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
install(PROGRAMS ${_ringdiag_py} DESTINATION pythonLibs/nscldaq)
install(PROGRAMS ringdiagnostics.py DESTINATION bin RENAME ringdiagnostics)
