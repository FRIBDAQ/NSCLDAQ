nscldaq_add_library(CAENDPP_PSD
  SOURCES CCompoundTrigger.cpp CDPpPsdEventSegment.cpp
    COneOnlyEventSegment.cpp CPsdCompoundEventSegment.cpp CPsdTrigger.cpp
    PSDParameters.cpp CPSDMaker.cpp)
target_include_directories(CAENDPP_PSD PRIVATE
  ${PROJECT_SOURCE_DIR}/sbs/readout
  ${PROJECT_SOURCE_DIR}/base/pugi
  ${PROJECT_SOURCE_DIR}/base/iniparser)
target_link_libraries(CAENDPP_PSD PUBLIC
  NSCLDAQ::CAEN SBSProductionReadout pugixml)

nscldaq_install_headers(CCompoundTrigger.h CDPpPsdEventSegment.h
  COneOnlyEventSegment.h CPsdCompoundEventSegment.h CPsdTrigger.h
  PSDParameters.h CPSDMaker.h)

# install-exec-hook: the skeleton.  skel/Makefile is configured from
# skel/Makefile.in (AC_CONFIG_FILES).

nscldaq_configure_file(skel/Makefile.in ${CMAKE_CURRENT_BINARY_DIR}/skel/Makefile)

file(GLOB _dpppsd_skel CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/skel/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/skel/*.h)
install(FILES ${_dpppsd_skel} ${CMAKE_CURRENT_BINARY_DIR}/skel/Makefile
  DESTINATION skeletons/dpp-psd)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(CAENDPP_PSD)
