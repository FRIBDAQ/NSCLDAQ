nscldaq_gengetopt(evts2frags evts2frags.ggo)

set(_evts2frags_includes
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/thread)

nscldaq_add_executable(evts2frags
  SOURCES Main.cpp CFragmentMaker.cpp CEvents2Fragments.cpp)
nscldaq_use_gengetopt(evts2frags evts2frags)
target_include_directories(evts2frags PRIVATE ${_evts2frags_includes})
target_link_libraries(evts2frags PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  dataformat NSCLDAQ::UFMT daqshm daqthreads NSCLDAQ::Exception)

# Tests

nscldaq_add_executable(evbtools_evts2frags_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp headerbuildertests.cpp apptests.cpp Asserts.h
    CFragmentMaker.cpp CEvents2Fragments.cpp)
target_include_directories(evbtools_evts2frags_unittests PRIVATE ${_evts2frags_includes})
target_link_libraries(evbtools_evts2frags_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders NSCLDAQ::CppUnit
  dataformat NSCLDAQ::UFMT daqshm daqthreads NSCLDAQ::Exception)

nscldaq_add_test(NAME unittests COMMAND evbtools_evts2frags_unittests)
