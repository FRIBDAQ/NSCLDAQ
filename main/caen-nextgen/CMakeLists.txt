#
#  CAEN nextgen (VX2750) digitizer support (only added when
#  ENABLE_CAEN_NEXTGEN).
#

nscldaq_add_library(CaenVx2750
  SOURCES CAENVX2750PhaTrigger.cpp Dig2Device.cpp DynamicMultiTrigger.cpp
    TclConfiguredReadout.cpp VX2750EventSegment.cpp
    VX2750MultiModuleEventSegment.cpp VX2750MultiTrigger.cpp
    VX2750PHAConfiguration.cpp VX2750Pha.cpp VX2750TclConfig.cpp)
target_include_directories(CaenVx2750 PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/sbs/readout
  ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(CaenVx2750 PUBLIC
  ConfigurableObject SBSProductionReadout NSCLDAQ::BoostLog NSCLDAQ::tclPlus
  NSCLDAQ::Tcl NSCLDAQ::CAENNG NSCLDAQ::JsonCpp)

nscldaq_install_headers(CAENVX2750PhaTrigger.h Dig2Device.h
  DynamicMultiTrigger.h TclConfiguredReadout.h VX2750EventSegment.h
  VX2750MultiModuleEventSegment.h VX2750MultiTrigger.h
  VX2750PHAConfiguration.h VX2750Pha.h VX2750TclConfig.h)

# Test programs (check_PROGRAMS) - not in TESTS as they need hardware; see
# the caen_nextgen_ONLINE_TESTS target below.

foreach(_prog fejackettests triggertests configtests)
  if(_prog STREQUAL "fejackettests")
    set(_srcs TestRunner.cpp devtests.cpp vx2750phatests.cpp)
  elseif(_prog STREQUAL "triggertests")
    set(_srcs TestRunner.cpp triggertests.cpp)
  else()
    set(_srcs TestRunner.cpp configtests.cpp)
  endif()
  nscldaq_add_executable(caen_nextgen_${_prog} TEST OUTPUT_NAME ${_prog}
    SOURCES ${_srcs})
  target_include_directories(caen_nextgen_${_prog} PRIVATE
    ${PROJECT_SOURCE_DIR}/base/os)
  target_link_libraries(caen_nextgen_${_prog} PRIVATE
    CaenVx2750 NSCLDAQ::CAENNG NSCLDAQ::CppUnit NSCLDAQ::tclPlus NSCLDAQ::Tcl
    NSCLDAQ::JsonCpp NSCLDAQ::OpenSSL)
endforeach()
target_include_directories(caen_nextgen_configtests PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject)
target_link_libraries(caen_nextgen_configtests PRIVATE ConfigurableObject)

# ONLINE_TESTS: needs TEST_MODULE_CONNECTION and TEST_MODULE_ISUSB in the
# environment (they were make variables).

add_custom_target(caen_nextgen_ONLINE_TESTS
  COMMAND sh -c "\"$0\" \"$TEST_MODULE_CONNECTION\" \"$TEST_MODULE_ISUSB\""
    $<TARGET_FILE:caen_nextgen_fejackettests>
  COMMAND sh -c "\"$0\" \"$TEST_MODULE_CONNECTION\" \"$TEST_MODULE_ISUSB\""
    $<TARGET_FILE:caen_nextgen_triggertests>
  COMMAND sh -c "\"$0\" \"$TEST_MODULE_CONNECTION\" \"$TEST_MODULE_ISUSB\""
    $<TARGET_FILE:caen_nextgen_configtests>
  USES_TERMINAL
  VERBATIM)
add_dependencies(caen_nextgen_ONLINE_TESTS caen_nextgen_fejackettests
  caen_nextgen_triggertests caen_nextgen_configtests)

# tclreadout/Makefile is configured (AC_CONFIG_FILES) though what is
# installed is the source directory's contents (including Makefile.in).

nscldaq_configure_file(tclreadout/Makefile.in
  ${CMAKE_CURRENT_BINARY_DIR}/tclreadout/Makefile)

# install-data-local

file(GLOB _caenng_tclreadout CONFIGURE_DEPENDS LIST_DIRECTORIES false
  ${CMAKE_CURRENT_SOURCE_DIR}/tclreadout/*)
install(FILES ${_caenng_tclreadout} DESTINATION share/recipes/caen-nextgen)
