#
#  DDASReadout, ddasSort and the ddasReadout driver script.
#

set(_ddasreadout_sharedir share/ddasreadout)

nscldaq_add_executable(DDASReadout
  SOURCES DDASReadoutMain.cpp CMyEventSegment.cpp CMyTrigger.cpp CMyBusy.cpp
    CMyScaler.cpp CMyEndCommand.cpp CSyncCommand.cpp CBootCommand.cpp
    CDDASStatisticsCommand.cpp)
target_include_directories(DDASReadout PRIVATE
  ${PROJECT_BINARY_DIR}/ddas
  ${PROJECT_SOURCE_DIR}/sbs/readout
  ${PROJECT_SOURCE_DIR}/sbs/nsclapi
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/ddas/configuration
  ${PROJECT_SOURCE_DIR}/ddas/booter
  ${PROJECT_SOURCE_DIR}/ddas/exception)
# CReadoutMain.h includes sbs/readout's gengetopt generated options.h.
target_include_directories(DDASReadout PRIVATE ${PROJECT_BINARY_DIR}/sbs/readout)
add_dependencies(DDASReadout ggo_sbs_readout_options)
target_compile_definitions(DDASReadout PRIVATE
  FIRMWARE_FILE="${prefix}/${_ddasreadout_sharedir}/DDASFirmwareVersions.txt"
  SORTING)
# DDASReadout_CPPFLAGS has -fPIC (instead of the default -fPIE).
set_property(TARGET DDASReadout PROPERTY POSITION_INDEPENDENT_CODE OFF)
target_compile_options(DDASReadout PRIVATE -fPIC)
target_link_libraries(DDASReadout PRIVATE
  SBSProductionReadout SBSVmeAPI daqthreads daqshm Configuration
  SystemBooter DDASException NSCLDAQ::Pixie NSCLDAQ::tclPlus NSCLDAQ::Tcl
  NSCLDAQ::Exception)

nscldaq_gengetopt(ddasSortOptions ddasSortOptions.ggo)

set(_ddassort_common_sources DDASSorter.cpp RawChannel.cpp ZeroCopyHit.cpp
  BufferArena.cpp ReferenceCountedBuffer.cpp HitManager.cpp)

nscldaq_add_executable(ddasSort SOURCES ddasSort.cpp ${_ddassort_common_sources})
nscldaq_use_gengetopt(ddasSort ddasSortOptions)
target_include_directories(ddasSort PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/format ${PROJECT_SOURCE_DIR}/base/dataflow)
target_link_libraries(ddasSort PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  dataformat DataFlow NSCLDAQ::Exception)

# Tests

nscldaq_add_executable(ddas_readout_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp hitmgrtests.cpp refcounttests.cpp arenatests.cpp
    rawchtests.cpp zcopyhittests.cpp sortertests.cpp testcommon.cpp
    ${_ddassort_common_sources})
target_include_directories(ddas_readout_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/format ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/testutils)
target_link_libraries(ddas_readout_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  dataformat DataFlow NSCLDAQ::Exception NSCLDAQ::CppUnit)
nscldaq_add_test(NAME unittests COMMAND ddas_readout_unittests)

# Configured files.

nscldaq_configure_file(DDASFirmwareVersions.txt.in DDASFirmwareVersions.txt)
nscldaq_configure_file(pixieserver.tcl.in pixieserver.tcl)

# install-exec-hook

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/DDASFirmwareVersions.txt
  ${CMAKE_CURRENT_BINARY_DIR}/pixieserver.tcl
  DESTINATION ${_ddasreadout_sharedir})
file(GLOB _ddasreadout_crate1 CONFIGURE_DEPENDS
  LIST_DIRECTORIES false ${CMAKE_CURRENT_SOURCE_DIR}/crate_1/*)
install(FILES ${_ddasreadout_crate1} DESTINATION ${_ddasreadout_sharedir}/crate_1)
install(PROGRAMS ddasReadout.tcl DESTINATION bin RENAME ddasReadout)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(ddas_readout_unittests)
