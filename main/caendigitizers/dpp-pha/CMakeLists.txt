nscldaq_add_library(CAENDPP_PHA
  SOURCES CAENPhaChannelParameters.cpp CAENPhaParameters.cpp
    CompassEventSegment.cpp CompassProject.cpp config.cpp PHAEventSegment.cpp
    PHATrigger.cpp CAENPha.cpp CAENPHAScalers.cpp
    CompassMultiModuleEventSegment.cpp CompassTrigger.cpp DPPConfig.cpp
    PHAMultiModuleSegment.cpp)
target_include_directories(CAENDPP_PHA PRIVATE
  ${PROJECT_SOURCE_DIR}/sbs/readout
  ${PROJECT_SOURCE_DIR}/base/pugi
  ${PROJECT_SOURCE_DIR}/base/iniparser)
target_link_libraries(CAENDPP_PHA PUBLIC
  NSCLDAQ::CAEN SBSProductionReadout pugixml iniparser)

nscldaq_install_headers(CAENPhaChannelParameters.h CAENPhaParameters.h
  CompassEventSegment.h CompassProject.h PHAEventSegment.h PHATrigger.h
  CAENPha.h CAENPHAScalers.h CompassMultiModuleEventSegment.h
  CompassTrigger.h DPPConfig.h PHAMultiModuleSegment.h CAENPhaBuffer.h)

# config.h is installed to include/ too, but not staged for
# NSCLDAQ::PrefixInclude: it would shadow the top level config.h there (the
# top level installs its config.h last, over this one, as before).

install(FILES config.h DESTINATION include)

# install-exec-hook: the skeleton.  skel/Makefile is configured from
# skel/Makefile.in (AC_CONFIG_FILES).

nscldaq_configure_file(skel/Makefile.in ${CMAKE_CURRENT_BINARY_DIR}/skel/Makefile)

file(GLOB _dpppha_skel CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/skel/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/skel/*.h)
install(FILES ${_dpppha_skel} ${CMAKE_CURRENT_BINARY_DIR}/skel/Makefile
  DESTINATION skeletons/dpp-pha)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(CAENDPP_PHA)
