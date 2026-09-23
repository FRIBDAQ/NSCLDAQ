#
#  usb/vmusb/vmusb: the VM-USB interface libraries and their Tcl (swig)
#  wrappers.
#

#  COMPILATION_FLAGS in the Makefile.am (USBSWITCHES and TCL_FLAGS are only
#  compile flags there; libraries that also linked them add the full target).

set(_vmusb_includes
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/tcpip
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/thread
  ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(_vmusb_compile_deps
  NSCLDAQ::USBHeaders NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders
  NSCLDAQ::PixieHeaders)

nscldaq_add_library(VMUSB
  SOURCES CVMUSBusb.cpp CVMUSBReadoutList.cpp CVMUSBEthernet.cpp
    CVMUSBFactory.cpp CVMUSB.cpp CMockVMUSB.cpp CLoggingReadoutList.cpp
    sis_vmusb_interface.cpp)
target_include_directories(VMUSB PRIVATE ${_vmusb_includes})
target_link_libraries(VMUSB
  PRIVATE ${_vmusb_compile_deps}
  PUBLIC Tcp daqthreads NSCLDAQ::tclPlus daqshm NSCLDAQ::USB NSCLDAQ::Threads)

nscldaq_add_library(VMUSBRemote
  SOURCES CVMUSBReadoutList.cpp CVMUSBRemote.cpp CVMUSBEthernet.cpp)
target_include_directories(VMUSBRemote PRIVATE ${_vmusb_includes})
target_link_libraries(VMUSBRemote
  PRIVATE ${_vmusb_compile_deps}
  PUBLIC VMUSB NSCLDAQ::Tcl Tcp NSCLDAQ::Exception NSCLDAQ::Threads)

#  Swig interface generation.

foreach(_mod CVMUSB CVMUSBReadoutList)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_mod}_wrap.cxx
    COMMAND ${SWIG} -c++ -outdir ${CMAKE_CURRENT_BINARY_DIR}
      -o ${CMAKE_CURRENT_BINARY_DIR}/${_mod}_wrap.cxx
      -debug-classes -tcl -namespace -pkgversion 1.0
      ${CMAKE_CURRENT_SOURCE_DIR}/${_mod}.i
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_mod}.i
      ${CMAKE_CURRENT_SOURCE_DIR}/${_mod}.h
    COMMENT "swig ${_mod}.i"
    VERBATIM)
  nscldaq_add_library(${_mod}
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${_mod}_wrap.cxx)
  target_include_directories(${_mod} PRIVATE ${_vmusb_includes})
  target_link_libraries(${_mod}
    PRIVATE ${_vmusb_compile_deps}
    PUBLIC VMUSB NSCLDAQ::Threads)
endforeach()

nscldaq_install_headers(CVMUSB.h CVMUSBReadoutList.h CVMUSBusb.h
  CVMUSBEthernet.h CVMUSBRemote.h CVMUSBFactory.h
  DESTINATION include/vmusb)

#  IVP tests - a bin_PROGRAM (installed as bin/tests).

nscldaq_add_executable(vmusb_tests OUTPUT_NAME tests
  SOURCES TestRunner.cpp regTests.cpp vmeTests.cpp)
target_include_directories(vmusb_tests PRIVATE ${PROJECT_SOURCE_DIR}/base/thread)
# tests_CPPFLAGS had an explicit -std=c++11.
set_target_properties(vmusb_tests PROPERTIES CXX_STANDARD 11 CXX_EXTENSIONS OFF)
target_link_libraries(vmusb_tests PRIVATE
  NSCLDAQ::PrefixInclude NSCLDAQ::PixieHeaders
  daqthreads NSCLDAQ::CppUnit VMUSB NSCLDAQ::Tcl NSCLDAQ::tclPlus)

#  The 'remotetests' rule of the Makefile.am was never part of any build
#  target (and links against the installed libraries) so is not translated.

#  Unit tests.

nscldaq_add_executable(vmusb_vmusb_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp vmusbrdolisttests.cpp loggingrdolisttests.cpp
    mockvmusbtests.cpp)
target_link_libraries(vmusb_vmusb_unittests PRIVATE
  NSCLDAQ::PrefixInclude NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit VMUSB NSCLDAQ::Tcl NSCLDAQ::tclPlus)
nscldaq_add_test(NAME unittests COMMAND vmusb_vmusb_unittests)

#  install-data-local: the driver kit headers and registration of the swig
#  packages in lib/pkgIndex.tcl (usb/CMakeLists.txt rewrites that file
#  later in the install).

nscldaq_install_mkdir(vmusbdriver vmusbdriver/includes)
file(GLOB _vmusb_kit_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CVMUSB*.h)
install(FILES ${_vmusb_kit_headers} DESTINATION vmusbdriver/includes)
install(CODE [=[
  set(_idx "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/pkgIndex.tcl")
  message(STATUS "Registering swig packages with lib's pkgIndex.tcl")
  file(APPEND "${_idx}" "package ifneeded cvmusbreadoutlist 1.0 [list load [file join $dir libCVMUSBReadoutList.so]]\n")
  file(APPEND "${_idx}" "package ifneeded cvmusb 1.0 [list load [file join $dir libCVMUSB.so]]\n")
]=])
