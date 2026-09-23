nscldaq_add_library(url SOURCES URL.cpp)
target_include_directories(url PRIVATE ${PROJECT_SOURCE_DIR}/base/headers)
target_link_libraries(url
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC NSCLDAQ::Exception NSCLDAQ::Threads)

nscldaq_install_headers(URL.h)
