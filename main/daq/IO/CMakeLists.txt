#-------------- The library libdaqio

nscldaq_add_library(daqio VERSION_INFO ${SOVERSION}
  SOURCES CDataSource.cpp CFileDataSource.cpp CRingDataSource.cpp
    CFakeDataSource.cpp CDataSourceFactory.cpp CDataSink.cpp
    CFileDataSink.cpp CRingDataSink.cpp CTestSourceSink.cpp
    CLoggingDataSink.cpp CDataSinkFactory.cpp CDataSinkException.cpp)
target_include_directories(daqio PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/dataflow)
target_link_libraries(daqio
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  PUBLIC dataformat DataFlow url daqshm NSCLDAQ::Exception rt NSCLDAQ::Threads)

nscldaq_install_headers(CDataSource.h CFileDataSource.h CRingDataSource.h
  CFakeDataSource.h CDataSourceFactory.h CDataSink.h CFileDataSink.h
  CRingDataSink.h CTestSourceSink.h CLoggingDataSink.h CDataSinkFactory.h
  CDataSinkException.h)

#------------------- Tests:

nscldaq_add_executable(IO_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp filedatasinktests.cpp datasourcefactorytests.cpp
    datasinkfactorytests.cpp ringdatasinktests.cpp)
target_include_directories(IO_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities/filter
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/dataflow)
target_link_libraries(IO_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  daqio url dataformat DataFlow daqshm NSCLDAQ::CppUnit NSCLDAQ::Exception)

nscldaq_add_test(NAME unittests COMMAND IO_unittests)
