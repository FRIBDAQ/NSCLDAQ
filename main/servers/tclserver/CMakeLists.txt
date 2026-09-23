nscldaq_add_executable(tclserver
  SOURCES TclAuthorizer.cpp serverinstance.cpp tclAppInit.cpp tkAppInit.cpp
    Server.cpp)
# (-I@top_srcdir@/base/exception is dropped: that directory no longer exists.)
target_include_directories(tclserver PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/servers/portmanager
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/CopyrightTools)
target_link_libraries(tclserver PRIVATE
  NSCLDAQ::PixieHeaders NSCLDAQ::tclPlus PortManager License daqshm
  NSCLDAQ::Exception NSCLDAQ::Tcl NSCLDAQ::Threads)

#  install-exec-local

nscldaq_install_mkdir(TclLibs)
install(PROGRAMS serverPackage.tcl DESTINATION TclLibs/tclserver)
nscldaq_install_tcl_index(TclLibs/tclserver "*.tcl")
