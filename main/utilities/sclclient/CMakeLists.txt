set(_sclclient_includes
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/servers/portmanager
  ${PROJECT_SOURCE_DIR}/base/os)

nscldaq_gengetopt(sclclientargs sclclientargs.ggo)

nscldaq_add_executable(sclclient
  SOURCES TclServerConnection.cpp TcpClient.cpp sclclientMain.cpp sclclient.cpp)
nscldaq_use_gengetopt(sclclient sclclientargs)
target_include_directories(sclclient PRIVATE ${_sclclient_includes})
target_link_libraries(sclclient PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  PortManager dataformat daqio DataFlow url daqshm NSCLDAQ::Exception
  NSCLDAQ::Threads)

nscldaq_add_executable(sclclient_testData NOINST OUTPUT_NAME testData
  SOURCES testData.cpp)
target_include_directories(sclclient_testData PRIVATE ${_sclclient_includes})
target_link_libraries(sclclient_testData PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  PortManager dataformat DataFlow url daqshm NSCLDAQ::Exception
  NSCLDAQ::Threads)
