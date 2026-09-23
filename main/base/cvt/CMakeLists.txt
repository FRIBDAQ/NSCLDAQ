nscldaq_add_library(cvt VERSION_INFO ${SOVERSION} SOURCES daqcvt.c)
target_link_libraries(cvt PUBLIC NSCLDAQ::Threads)

nscldaq_install_headers(daqcvt.h)
