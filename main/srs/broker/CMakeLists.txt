nscldaq_add_library(UDPBrokerDerived SOURCES UDPBrokerDerived.cpp)
target_include_directories(UDPBrokerDerived PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities/udpBroker
  ${PROJECT_SOURCE_DIR}/base/udp
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/srs/maps
  ${PROJECT_SOURCE_DIR}/srs/sorter)
target_link_libraries(UDPBrokerDerived
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC UDPBrokerBase Udp daqio dataformat SRSMaps SRSSorter NSCLDAQ::Exception)

nscldaq_install_headers(UDPBrokerDerived.h datagramHeader.h pcapHeader.h)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(UDPBrokerDerived)
