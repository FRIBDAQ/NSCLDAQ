#
#  CC-USB C++ library and its SWIG Tcl wrappers.
#

# COMPILATION_FLAGS shared by the three libraries.

set(_ccusb_includes
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/tcpip)

## Normal C++ library containing CCCUSB*

nscldaq_add_library(CCUSB
  SOURCES CCCUSB.cpp CCCUSBusb.cpp CCCUSBRemote.cpp CMockCCUSB.cpp
    CCCUSBReadoutList.cpp CLoggingReadoutList.cpp)
target_include_directories(CCUSB PRIVATE ${_ccusb_includes})
target_link_libraries(CCUSB
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC daqshm daqthreads NSCLDAQ::Tcl NSCLDAQ::tclPlus Tcp
    NSCLDAQ::Exception NSCLDAQ::Threads NSCLDAQ::USB)

nscldaq_install_headers(CCCUSB.h CCCUSBusb.h CCCUSBRemote.h CMockCCUSB.h
  CCCUSBReadoutList.h CLoggingReadoutList.h)

## SWIG wrapper generation

foreach(_w CCCUSB CCCUSBReadoutList)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_w}_wrap.cxx
    COMMAND ${SWIG} -c++ -debug-classes -tcl -namespace -pkgversion 1.0
      -outdir ${CMAKE_CURRENT_BINARY_DIR}
      -o ${CMAKE_CURRENT_BINARY_DIR}/${_w}_wrap.cxx
      ${CMAKE_CURRENT_SOURCE_DIR}/${_w}.i
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_w}.h ${CMAKE_CURRENT_SOURCE_DIR}/${_w}.i
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "swig ${_w}.i"
    VERBATIM)
endforeach()

## SWIG Wrapper containing cvmusb::CCCUSB

nscldaq_add_library(CCCUSB SOURCES ${CMAKE_CURRENT_BINARY_DIR}/CCCUSB_wrap.cxx)
target_include_directories(CCCUSB PRIVATE ${_ccusb_includes})
target_link_libraries(CCCUSB
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC CCUSB daqshm daqthreads Tcp NSCLDAQ::tclPlus NSCLDAQ::Exception
    NSCLDAQ::Tcl NSCLDAQ::USB NSCLDAQ::Threads)

## SWIG Wrapper containing cvmusbreadoutlist::CCCUSBReadoutList

nscldaq_add_library(CCCUSBReadoutList
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/CCCUSBReadoutList_wrap.cxx)
target_include_directories(CCCUSBReadoutList PRIVATE ${_ccusb_includes})
target_link_libraries(CCCUSBReadoutList
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::USBHeaders
    NSCLDAQ::PixieHeaders
  PUBLIC CCUSB daqshm NSCLDAQ::Threads)

## install-data-local

file(GLOB _ccusb_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${_ccusb_headers} DESTINATION ccusbdriver/includes)
install(CODE [=[
  file(APPEND "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/pkgIndex.tcl"
    "package ifneeded cccusb 1.0 [list load [file join $dir libCCCUSB.so]]\n")
  file(APPEND "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/pkgIndex.tcl"
    "package ifneeded cccusbreadoutlist 1.0 [list load [file join $dir libCCCUSBReadoutList.so]]\n")
]=])

## -------------------------------------------
##          TESTS
## -------------------------------------------

## Manual test programs for CCUSBusb

nscldaq_add_executable(ccusb_ccusb_tests TEST OUTPUT_NAME tests
  SOURCES TestRunner.cpp regTests.cpp)
target_include_directories(ccusb_ccusb_tests PRIVATE ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(ccusb_ccusb_tests PRIVATE
  NSCLDAQ::PixieHeaders NSCLDAQ::CppUnit CCUSB NSCLDAQ::Tcl NSCLDAQ::tclPlus
  daqthreads Tcp NSCLDAQ::Exception NSCLDAQ::USB NSCLDAQ::Threads)

## Manual test program for CCCUSBRemote

nscldaq_add_executable(ccusb_ccusb_remotetests TEST OUTPUT_NAME remotetests
  SOURCES TestRunner.cpp remoteRegTests.cpp)
target_include_directories(ccusb_ccusb_remotetests PRIVATE ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(ccusb_ccusb_remotetests PRIVATE
  NSCLDAQ::PixieHeaders CCUSB NSCLDAQ::CppUnit Tcp daqthreads NSCLDAQ::tclPlus
  NSCLDAQ::Exception NSCLDAQ::Tcl NSCLDAQ::USB NSCLDAQ::Threads)

## These are tests that run automatically at check-TESTS

nscldaq_add_executable(ccusb_ccusb_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp rdolistTests.cpp loggingrdolistTests.cpp
    mockccusbTests.cpp)
target_include_directories(ccusb_ccusb_unittests PRIVATE ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(ccusb_ccusb_unittests PRIVATE
  CCUSB NSCLDAQ::CppUnit Tcp daqthreads NSCLDAQ::Tcl NSCLDAQ::tclPlus)

nscldaq_add_executable(ccusb_ccusb_camactests TEST OUTPUT_NAME camactests
  SOURCES TestRunner.cpp camacTests.cpp)
target_include_directories(ccusb_ccusb_camactests PRIVATE ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(ccusb_ccusb_camactests PRIVATE
  NSCLDAQ::PixieHeaders CCUSB NSCLDAQ::CppUnit Tcp daqthreads NSCLDAQ::Tcl
  NSCLDAQ::tclPlus)

nscldaq_add_test(NAME unittests COMMAND ccusb_ccusb_unittests)
