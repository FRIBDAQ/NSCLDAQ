nscldaq_add_library(Tcp
  SOURCES CSocket.cpp CTCPBadSocketState.cpp CTCPConnectionFailed.cpp
    CTCPConnectionLost.cpp CTCPNoSuchHost.cpp CTCPNoSuchService.cpp)
target_include_directories(Tcp PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(Tcp
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::Exception daqshm)

nscldaq_install_headers(CSocket.h CTCPBadSocketState.h CTCPConnectionFailed.h
  CTCPConnectionLost.h CTCPNoSuchHost.h CTCPNoSuchService.h)
