nscldaq_add_executable(SRSReadout
  SOURCES CReadoutAppSRS.cpp CEventSegmentSRS.cpp CTriggerSRS.cpp
    CEndCommandSRS.cpp CTcpClient.cpp)
target_include_directories(SRSReadout PRIVATE
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/sbs/readout
  ${PROJECT_BINARY_DIR}/sbs/readout
  ${PROJECT_SOURCE_DIR}/sbs/nsclapi
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/base/udp
  ${PROJECT_SOURCE_DIR}/utilities/udpBroker
  ${PROJECT_SOURCE_DIR}/srs/broker
  ${PROJECT_SOURCE_DIR}/srs/maps
  ${PROJECT_SOURCE_DIR}/srs/sorter)
# sbs/readout's headers include its gengetopt generated options.h (which the
# Makefile generated into @top_srcdir@/sbs/readout).
add_dependencies(SRSReadout ggo_sbs_readout_options)
target_compile_definitions(SRSReadout PRIVATE SORTING)
target_link_libraries(SRSReadout PRIVATE
  SBSProductionReadout SBSVmeAPI daqthreads daqshm UDPBrokerBase
  UDPBrokerDerived SRSMaps SRSSorter NSCLDAQ::tclPlus NSCLDAQ::Tcl)
