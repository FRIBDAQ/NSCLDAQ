nscldaq_add_library(daqthreads VERSION_INFO ${SOVERSION}
  SOURCES dshwrappthreads.cpp SyncGuard.cpp Synchronizable.cpp Thread.cpp
    CMutex.cpp CCondition.cpp CSynchronizedThread.cpp CGaurdedObject.cpp
    CBufferQueue.cpp)
target_include_directories(daqthreads PRIVATE ${PROJECT_SOURCE_DIR}/base/headers)
target_compile_definitions(daqthreads PRIVATE USE_PTHREADS)
target_link_libraries(daqthreads
  PRIVATE NSCLDAQ::PixieHeaders NSCLDAQ::tclPlusHeaders
  PUBLIC NSCLDAQ::Exception NSCLDAQ::Threads)

nscldaq_install_headers(dshwrappthreads.h dshwrapthreads.h Runnable.h
  SyncGuard.h Synchronizable.h Thread.h CMutex.h CCondition.h
  CSynchronizedThread.h CGaurdedObject.h CBufferQueue.h CBufferQueue.cpp)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(daqthreads)
