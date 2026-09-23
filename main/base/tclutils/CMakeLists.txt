#  libWait - a Tcl loadable extension.  install-exec-hook moved it from lib
#  to TclLibs so it is installed straight there.

nscldaq_add_library(Wait VERSION_INFO ${SOVERSION} DESTINATION TclLibs
  SOURCES WaitPackage.cpp)
target_include_directories(Wait PRIVATE ${PROJECT_SOURCE_DIR}/base/CopyrightTools)
target_link_libraries(Wait
  PRIVATE NSCLDAQ::TclOnlyHeaders NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::tclPlus NSCLDAQ::Tcl License m)

#  Tcl packages -> TclLibs/tclutils (install-exec-local).

nscldaq_install_mkdir(TclLibs)
install(PROGRAMS Utils.tcl BlockCompleter.tcl TclSourceFilter.tcl Process.tcl
    Orphan.tcl observer.tcl ssh.tcl OS.tcl UniqueInstance.tcl versionUtils.tcl
    Log.tcl pollmanager.tcl elimTclLibpath.tcl
  DESTINATION TclLibs/tclutils)
nscldaq_install_tcl_index(TclLibs/tclutils -verbose "*.tcl")

#  install-exec-hook: index the (moved) libWait.so.

nscldaq_install_tcl_index(TclLibs -verbose "*.so")

# Tests

nscldaq_add_tcl_test(NAME tclTests
  TCLLIBPATH ${prefix}/TclLibs ${libdir} ${CMAKE_CURRENT_SOURCE_DIR})
