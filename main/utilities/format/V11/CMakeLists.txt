#  The C++ sources also got -I@top_srcdir@/base/os and the libtcl++ flags
#  (via _CXXFLAGS); ringitem.c only the -I.. and Pixie flags.  They are
#  applied to the whole target here, which is harmless.

nscldaq_add_library(dataformatv11
  SOURCES CRingItem.cpp CAbnormalEndItem.cpp CRingScalerItem.cpp
    CRingStateChangeItem.cpp CPhysicsEventItem.cpp
    CRingPhysicsEventCountItem.cpp CRingTextItem.cpp CRingFragmentItem.cpp
    CGlomParameters.cpp CUnknownFragment.cpp CDataFormatItem.cpp
    CRingItemFactory.cpp RingItemComparisons.cpp ringitem.c)
target_include_directories(dataformatv11 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(dataformatv11
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC daqshm NSCLDAQ::tclPlus NSCLDAQ::TclOnly)

nscldaq_install_headers(CRingItem.h CAbnormalEndItem.h CRingScalerItem.h
  CRingStateChangeItem.h CPhysicsEventItem.h CRingPhysicsEventCountItem.h
  CRingTextItem.h CRingFragmentItem.h CGlomParameters.h CUnknownFragment.h
  CDataFormatItem.h CRingItemFactory.h RingItemComparisons.h DataFormatV11.h
  DESTINATION include/V11)

# Tests

nscldaq_add_executable(formatv11_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp ringitemtests.cpp teststate.cpp scalertests.cpp
    texttest.cpp physcounttests.cpp physeventtests.cpp formatoutputtests.cpp
    dataformattests.cpp fragmenttest.cpp glomparamtests.cpp factorytests.cpp)
target_include_directories(formatv11_unittests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/testutils)
target_link_libraries(formatv11_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit dataformatv11 daqshm NSCLDAQ::Exception NSCLDAQ::TclOnly)
nscldaq_add_test(NAME unittests COMMAND formatv11_unittests)
