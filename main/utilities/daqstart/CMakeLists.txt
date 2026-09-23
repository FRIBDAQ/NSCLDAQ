set(_daqstart_testable CSink.cpp CFileSink.cpp CFileSinkCreator.cpp
  CSinkFactory.cpp CMonitoredProgram.cpp CLocalMonitoredProgram.cpp)
set(_daqstart_includes
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/factories
  ${PROJECT_SOURCE_DIR}/base/headers)

nscldaq_gengetopt(cmdline daqstart.ggo ARGS --unamed-opts)

nscldaq_add_executable(daqstart
  SOURCES daqstart.cpp CDAQStart.cpp ${_daqstart_testable})
nscldaq_use_gengetopt(daqstart cmdline)
target_include_directories(daqstart PRIVATE ${_daqstart_includes})
# -D_GNU_SOURCE was in daqstart_CXXFLAGS, i.e. C++ sources only.
target_compile_definitions(daqstart PRIVATE $<$<COMPILE_LANGUAGE:CXX>:_GNU_SOURCE>)
target_link_libraries(daqstart PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::Exception daqshm NSCLDAQ::Threads NSCLDAQ::Tcl)

# install-exec-local

install(PROGRAMS popup.py DESTINATION bin RENAME PopUp)

# Tests

nscldaq_add_executable(daqstart_unittests TEST OUTPUT_NAME unittests
  SOURCES ${_daqstart_testable} CFileSinkTests.cpp SinkCreatorTests.cpp
    FactoryTests.cpp LocalMonitoredProgram.cpp grep.cpp TestRunner.cpp)
target_include_directories(daqstart_unittests PRIVATE ${_daqstart_includes})
target_link_libraries(daqstart_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit NSCLDAQ::Exception daqshm NSCLDAQ::Tcl)
nscldaq_add_test(NAME unittests COMMAND daqstart_unittests)
