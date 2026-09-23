nscldaq_add_library(dataformatv10
  SOURCES CRingItem.cpp CRingScalerItem.cpp
    CRingTimestampedRunningScalerItem.cpp CRingStateChangeItem.cpp
    CPhysicsEventItem.cpp CRingPhysicsEventCountItem.cpp CRingTextItem.cpp
    CRingFragmentItem.cpp CUnknownFragment.cpp CRingItemFactory.cpp
    ringitem.cpp)
target_include_directories(dataformatv10 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${PROJECT_SOURCE_DIR}/utilities/Buffer
  ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(dataformatv10
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC buffer daqshm NSCLDAQ::tclPlus NSCLDAQ::Exception NSCLDAQ::TclOnly)

nscldaq_install_headers(CRingItem.h CRingScalerItem.h
  CRingTimestampedRunningScalerItem.h CRingStateChangeItem.h
  CPhysicsEventItem.h CRingPhysicsEventCountItem.h CRingTextItem.h
  CRingFragmentItem.h CUnknownFragment.h CRingItemFactory.h CVoidItem.h
  DataFormatV10.h
  DESTINATION include/V10)

# Tests

nscldaq_add_executable(formatv10_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp ringitemtests.cpp teststate.cpp scalertests.cpp
    texttest.cpp physcounttests.cpp formatoutputtests.cpp physeventtests.cpp)
target_include_directories(formatv10_unittests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${PROJECT_SOURCE_DIR}/utilities/Buffer
  ${PROJECT_SOURCE_DIR}/base/testutils
  ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(formatv10_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit buffer dataformatv10 daqshm NSCLDAQ::TclOnly
  NSCLDAQ::Exception)
nscldaq_add_test(NAME unittests COMMAND formatv10_unittests)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(dataformatv10)
