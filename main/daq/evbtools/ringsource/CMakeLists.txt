nscldaq_gengetopt(rfcmdline options.ggo)

set(_ringsource_includes
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/daq/format)

nscldaq_add_executable(ringFragmentSource
  SOURCES Main.cpp CRingFragmentSource.h CRingFragmentSource.cpp)
nscldaq_use_gengetopt(ringFragmentSource rfcmdline)
target_include_directories(ringFragmentSource PRIVATE ${_ringsource_includes})
target_link_libraries(ringFragmentSource PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  EventBuilderClient NSCLDAQ::UFMT dataformat DataFlow NSCLDAQ::Exception dl)

# Tests

nscldaq_add_executable(evbtools_ringsource_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp fragsrctests.cpp Asserts.h CRingFragmentSource.cpp)
target_include_directories(evbtools_ringsource_unittests PRIVATE ${_ringsource_includes})
target_link_libraries(evbtools_ringsource_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit NSCLDAQ::UFMT dataformat dl)

nscldaq_add_test(NAME unittests COMMAND evbtools_ringsource_unittests)
