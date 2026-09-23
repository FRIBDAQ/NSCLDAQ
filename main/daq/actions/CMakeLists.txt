nscldaq_add_library(Actions SOURCES Actions.cpp)
target_include_directories(Actions PRIVATE ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(Actions
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  PUBLIC daqthreads NSCLDAQ::Exception)

nscldaq_install_headers(Actions.h)

# install-exec-local:

nscldaq_install_mkdir(bin TclLibs/Stager share/html)
install(PROGRAMS actions.tcl defaultActions.tcl DESTINATION TclLibs/Stager)
nscldaq_install_tcl_index(TclLibs/Stager -verbose "*.tcl")

# (The testCompile.o rule was never used by any target.)

# Tests

nscldaq_add_executable(actions_testacts TEST OUTPUT_NAME testacts
  SOURCES TestActionPackage.cpp)
target_include_directories(actions_testacts PRIVATE ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(actions_testacts PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  Actions daqthreads NSCLDAQ::Exception NSCLDAQ::Tcl)

# check-TESTS:

nscldaq_add_tcl_test(NAME tclTests)
