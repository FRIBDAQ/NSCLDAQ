#
#  pyscaler - python scaler display.  Modules go in
#  pythonLibs/nscldaq/pyscaler, the main program is bin/pyScaler.
#

set(_pyscaler_modules channel.py configfile.py datasource.py RunInfo.py
  ScalerGui.py ScalerPage.py ScalerPlot.py source_manager.py)

# install-exec-hook

install(PROGRAMS ${_pyscaler_modules} DESTINATION pythonLibs/nscldaq/pyscaler)
install(PROGRAMS main.py DESTINATION bin RENAME pyScaler)

# TESTS = channel.py configfile.py  (self tests in their __main__ blocks)

foreach(_t channel.py configfile.py)
  nscldaq_add_test(NAME ${_t}
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/${_t})
endforeach()
