#-------------- The filter framework library.

nscldaq_gengetopt(filterargs filterargs.ggo ARGS --unamed-opts=OPTIONS)

nscldaq_add_library(filter VERSION_INFO ${SOVERSION}
  SOURCES CFilterMain.cpp CBaseMediator.cpp CMediator.cpp CFakeMediator.cpp
    CInfiniteMediator.cpp COneShotMediator.cpp COneShotHandler.cpp
    CCompositeFilter.cpp COneShotException.cpp CMediatorException.cpp
    CAbnormalEndRunFilterHandler.cpp CRunStatistics.cpp SatisfyTclPlus.cpp
    CFilterTestSource.cpp CFilterTestSource.h CFilterTestSink.cpp
    CFilterTestSink.h)
nscldaq_use_gengetopt(filter filterargs)
target_include_directories(filter PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities/common
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/headers)
target_link_libraries(filter
  PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  PUBLIC UtilCommon daqio dataformat DataFlow url daqshm NSCLDAQ::Exception
    rt NSCLDAQ::Threads)

nscldaq_install_headers(CFilterMain.h CBaseMediator.h CMediator.h
  CFakeMediator.h CInfiniteMediator.h COneShotMediator.h COneShotHandler.h
  CFilter.h CCompositeFilter.h CTransparentFilter.h CNullFilter.h
  COneShotException.h CMediatorException.h CFatalException.h
  CAbnormalEndRunFilterHandler.h CRunStatistics.h)

#------------------- Tests:

nscldaq_add_executable(filter_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp infinitemediatortests.cpp filtermaintests.cpp
    compositefiltertests.cpp transparentfiltertests.cpp
    oneshothandlertests.cpp abnormalendrunfiltertests.cpp testmultiple.cpp
    CTestFilter.h CTestFilter.cpp)
target_include_directories(filter_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities/filter
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/dataflow)
target_link_libraries(filter_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  filter daqio DataFlow url dataformat NSCLDAQ::CppUnit NSCLDAQ::Exception)

nscldaq_add_executable(filter_testapp TEST OUTPUT_NAME testapp SOURCES TestApp.cpp)
target_include_directories(filter_testapp PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/dataflow)
target_link_libraries(filter_testapp PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit DataFlow url dataformat filter NSCLDAQ::Exception)

nscldaq_add_test(NAME unittests COMMAND filter_unittests)

# AC_CONFIG_LINKS: the tests read this event file from the build directory.

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/run-0000-00.evt)
  file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/run-0000-00.evt
    ${CMAKE_CURRENT_BINARY_DIR}/run-0000-00.evt SYMBOLIC)
endif()

#-------------------- Filter Kit (install-data-local)

nscldaq_configure_file(FilterIncludes.in FilterIncludes)
nscldaq_configure_file(Makefile-template.in Makefile-template)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FilterIncludes
  CTemplateFilter.cpp SkeletonMain.cpp DESTINATION filterkit)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile-template
  DESTINATION filterkit RENAME Makefile)
