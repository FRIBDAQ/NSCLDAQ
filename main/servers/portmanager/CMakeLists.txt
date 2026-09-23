#
#   Build the C++ library:
#

nscldaq_add_library(PortManager VERSION_INFO ${SOVERSION}
  SOURCES CPortManagerException.cpp CPortManager.cpp)
target_include_directories(PortManager PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers ${PROJECT_SOURCE_DIR}/base/os)
target_compile_definitions(PortManager PRIVATE HOME="${prefix}")
target_link_libraries(PortManager
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::Exception daqshm NSCLDAQ::Tcl NSCLDAQ::Threads)

nscldaq_install_headers(CPortManagerException.h CPortManager.h)

#
#   Install the Tcl packages, application and python package:
#

install(PROGRAMS ConnectionManager.tcl portAllocator.tcl PortManager.tcl
  DESTINATION TclLibs/PortManager)
install(PROGRAMS DaqPortManager.tcl DESTINATION bin RENAME DaqPortManager)
install(PROGRAMS PortManager.py DESTINATION pythonLibs/nscldaq/portmanager)
install(CODE "file(TOUCH \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/pythonLibs/nscldaq/portmanager/__init__.py\")")
nscldaq_install_tcl_index(TclLibs/PortManager "*.tcl")
install(PROGRAMS lsservices.tcl DESTINATION bin RENAME lsservices)

#
#  Tests: the Tcl tests, the C++ test and the python tests.
#

nscldaq_add_executable(portmanager_cpptest TEST OUTPUT_NAME cpptest
  SOURCES cpptest.cpp)
target_include_directories(portmanager_cpptest PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers ${PROJECT_SOURCE_DIR}/base/os)
target_compile_definitions(portmanager_cpptest PRIVATE HOME="${prefix}")
target_link_libraries(portmanager_cpptest PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders PortManager NSCLDAQ::Exception)

nscldaq_add_test(NAME tclTests COMMAND ${TCLSH_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/tclTests.tcl)
nscldaq_add_test(NAME cpptest COMMAND portmanager_cpptest)
nscldaq_add_test(NAME PortManagerTests
  COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/PortManagerTests.py
  ENVIRONMENT PYTHONPATH=${prefix}/pythonLibs)
