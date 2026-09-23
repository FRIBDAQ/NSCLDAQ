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
