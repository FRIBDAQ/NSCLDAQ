nscldaq_add_library(License VERSION_INFO ${SOVERSION} SOURCES CopyrightNotice.cpp)
target_include_directories(License PRIVATE ${PROJECT_SOURCE_DIR}/base/headers)
target_link_libraries(License
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::Threads)

nscldaq_install_headers(CopyrightNotice.h)

# Tests

nscldaq_add_executable(CopyrightTools_test TEST OUTPUT_NAME test SOURCES test.cpp)
target_include_directories(CopyrightTools_test PRIVATE ${PROJECT_SOURCE_DIR}/base/headers)
target_link_libraries(CopyrightTools_test PRIVATE NSCLDAQ::PixieHeaders License)
nscldaq_add_test(NAME test COMMAND CopyrightTools_test)
