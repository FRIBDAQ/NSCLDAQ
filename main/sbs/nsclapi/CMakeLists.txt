nscldaq_add_library(SBSVmeAPI VERSION_INFO 1:0:0
  SOURCES SBSBIT3API.cpp Locking.cpp VmeModule.cpp MmapError.cpp)
target_include_directories(SBSVmeAPI PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/sbs/driver/include
  ${PROJECT_SOURCE_DIR}/sbs/driver/dd)
target_link_libraries(SBSVmeAPI
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::tclPlus btp)

nscldaq_install_headers(CVMEInterface.h SBSBit3API.h CVME.h CVMEptr.h
  VmeModule.h MmapError.h)

# Tests (AM_LDFLAGS = -lpthread applies to unittests).

nscldaq_add_executable(sbs_nsclapi_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp LockingTest.cpp)
target_link_libraries(sbs_nsclapi_unittests PRIVATE
  NSCLDAQ::PixieHeaders SBSVmeAPI NSCLDAQ::CppUnit NSCLDAQ::TclOnly pthread)
nscldaq_add_test(NAME unittests COMMAND sbs_nsclapi_unittests)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(sbs_nsclapi_unittests)
