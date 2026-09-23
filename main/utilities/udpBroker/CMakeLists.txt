#
#  libUDPBrokerBase - base class for UDP brokers.
#

nscldaq_add_library(UDPBrokerBase SOURCES UDPBrokerBase.cpp)
target_include_directories(UDPBrokerBase PRIVATE
  ${PROJECT_SOURCE_DIR}/base/udp
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/IO)
target_link_libraries(UDPBrokerBase
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC Udp daqio dataformat NSCLDAQ::Exception)

nscldaq_install_headers(UDPBrokerBase.h datagramHeader.h pcapHeader.h)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(UDPBrokerBase)
