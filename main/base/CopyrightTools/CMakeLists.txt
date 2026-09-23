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

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(CopyrightTools_test)
