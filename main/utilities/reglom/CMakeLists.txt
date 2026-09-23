#
#  reglom - Unglom, timecheck and reglom plus the offlinereglom GUI.
#

set(_reglom_includes
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/IO)
set(_reglom_libs
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  dataformat daqio DataFlow url NSCLDAQ::Exception)

nscldaq_gengetopt(reglomopts reglomopts.ggo ARGS -u)

nscldaq_add_executable(Unglom
  SOURCES Unglom.cpp CUnglom.cpp Main.cpp)
target_include_directories(Unglom PRIVATE ${_reglom_includes})
target_link_libraries(Unglom PRIVATE ${_reglom_libs} NSCLDAQ::UFMT)

nscldaq_add_executable(timecheck
  SOURCES Timecheck.cpp CTimeChecker.cpp Main.cpp)
target_include_directories(timecheck PRIVATE ${_reglom_includes})
target_link_libraries(timecheck PRIVATE ${_reglom_libs})

nscldaq_add_executable(reglom SOURCES reglomMain.cpp CMerge.cpp)
nscldaq_use_gengetopt(reglom reglomopts)
target_include_directories(reglom PRIVATE ${_reglom_includes}
  ${PROJECT_SOURCE_DIR}/base/uri)
target_link_libraries(reglom PRIVATE ${_reglom_libs} NSCLDAQ::UFMT)

# install-exec-local (PYPROGRAMS is empty)

install(PROGRAMS offlinereglom.py DESTINATION bin RENAME offlinereglom)
install(PROGRAMS OutputWindow.py DESTINATION pythonLibs/nscldaq)
