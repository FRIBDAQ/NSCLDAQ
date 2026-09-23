#
#  ROOT format library (with its rootcint generated dictionary) and the
#  ddasdumper program.
#

# rootcint also writes the _rdict.pcm and the .rootmap which are installed.

set(_ddasroot_dict ${CMAKE_CURRENT_BINARY_DIR}/ddasrootformatDictionary.cpp)
set(_ddasroot_pcm ${CMAKE_CURRENT_BINARY_DIR}/ddasrootformatDictionary_rdict.pcm)
set(_ddasroot_rootmap ${CMAKE_CURRENT_BINARY_DIR}/libddasrootformat.rootmap)
add_custom_command(
  OUTPUT ${_ddasroot_dict} ${_ddasroot_pcm} ${_ddasroot_rootmap}
  COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${ROOTSYS}/lib
    ${ROOTSYS}/bin/rootcint -f ddasrootformatDictionary.cpp
    -rml libddasrootformat.so -rmf libddasrootformat.rootmap
    -I${CMAKE_CURRENT_SOURCE_DIR}
    -I${NSCLDAQ_DDASFMT_STAGE_PREFIX}/ddasformat/include
    -c -p DDASRootHit.h DDASRootEvent.h LinkDef.h
  DEPENDS DDASRootHit.h DDASRootEvent.h LinkDef.h ddasformat_incorp
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "rootcint ddasrootformatDictionary"
  VERBATIM)

nscldaq_add_library(ddasrootformat SOURCES DDASRootEvent.cpp ${_ddasroot_dict})
target_include_directories(ddasrootformat PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(ddasrootformat PUBLIC NSCLDAQ::DDASFormat NSCLDAQ::ROOT)

nscldaq_install_headers(DDASRootEvent.h DDASRootHit.h)

# ddasdumper.bin and its wrapper script ddasdumper.

nscldaq_gengetopt(dumperargs dumperargs.ggo)

nscldaq_add_executable(ddasdumper_bin OUTPUT_NAME ddasdumper.bin
  SOURCES ddasdumper.cpp DataSource.cpp FdDataSource.cpp
    StreamDataSource.cpp RootFileDataSink.cpp)
nscldaq_use_gengetopt(ddasdumper_bin dumperargs)
target_include_directories(ddasdumper_bin PRIVATE
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder)
target_link_libraries(ddasdumper_bin PRIVATE
  ddasrootformat url daqio
  NSCLDAQ::UFMT NSCLDAQ::DDASFormat NSCLDAQ::tclPlusHeaders NSCLDAQ::Exception
  NSCLDAQ::ROOT)

nscldaq_configure_file(ddasdumper.in ddasdumper)

# install-exec-local

install(PROGRAMS ${_ddasroot_pcm} DESTINATION lib)
install(FILES ${_ddasroot_rootmap} DESTINATION lib)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ddasdumper DESTINATION bin)
