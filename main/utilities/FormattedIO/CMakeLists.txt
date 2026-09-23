nscldaq_add_library(daqformatio VERSION_INFO ${SOVERSION}
  SOURCES BufferIOV8.cpp RingIOV10.cpp RingIOV11.cpp)
target_include_directories(daqformatio PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/utilities/format
  ${PROJECT_SOURCE_DIR}/utilities/Buffer
  ${PROJECT_SOURCE_DIR}/base/headers)
target_link_libraries(daqformatio
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC daqio buffer dataformatv8 dataformatv10 dataformatv11 dataformat
    NSCLDAQ::Exception rt NSCLDAQ::Threads)

nscldaq_install_headers(BufferIOV8.h RingIOV10.h RingIOV11.h)

#------------------- Tests:

nscldaq_add_executable(formattedio_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp daq8test.cpp daq10test.cpp daq11test.cpp)
target_include_directories(formattedio_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities/FormattedIO
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/utilities/Buffer
  ${PROJECT_SOURCE_DIR}/utilities/format
  ${PROJECT_SOURCE_DIR}/base/testutils)
target_link_libraries(formattedio_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders
  daqformatio daqio buffer dataformatv8 dataformatv10 dataformatv11 dataformat
  NSCLDAQ::CppUnit NSCLDAQ::Exception)
nscldaq_add_test(NAME unittests COMMAND formattedio_unittests)
