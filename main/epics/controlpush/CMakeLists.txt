#  This application requires epics:

if(ENABLE_EPICS_TOOLS)
  nscldaq_gengetopt(cmdline options.ggo ARGS --unamed-opts)

  set(_controlpush_appitems CApplication.cpp CChannelList.cpp CStrings.cpp
    CLookupVisitor.cpp CUnitChannel.cpp CBuildChannelData.cpp CChannel.cpp)

  # COMPILATION_FLAGS
  set(_controlpush_includes
    ${PROJECT_SOURCE_DIR}/base/CopyrightTools
    ${PROJECT_SOURCE_DIR}/base/headers
    ${PROJECT_SOURCE_DIR}/base/tcpip
    ${PROJECT_SOURCE_DIR}/base/os
    ${PROJECT_SOURCE_DIR}/base/factories
    ${PROJECT_SOURCE_DIR}/base/thread
    ${PROJECT_SOURCE_DIR}/epics/epicslib)
  set(_controlpush_libs
    Tcp NSCLDAQ::Exception Epicslib daqshm daqthreads License
    NSCLDAQ::EPICS NSCLDAQ::Threads NSCLDAQ::Tcl)

  nscldaq_add_executable(controlpush SOURCES Main.cpp ${_controlpush_appitems})
  nscldaq_use_gengetopt(controlpush cmdline)
  target_include_directories(controlpush PRIVATE ${_controlpush_includes})
  target_link_libraries(controlpush PRIVATE
    NSCLDAQ::tclPlusHeaders NSCLDAQ::EPICSBin NSCLDAQ::PixieHeaders
    ${_controlpush_libs})

  # Not run as a test (TESTS is commented out: needs an IOC with known
  # channels), but built for 'make check'.

  nscldaq_add_executable(controlpush_unittests TEST OUTPUT_NAME unittests
    SOURCES TestRunner.cpp ChannelListTests.cpp LookupVisitorTest.cpp
      TestChannel.cpp TestUnitChannel.cpp ${_controlpush_appitems})
  # CApplication.cpp includes the generated cmdline.h (cmdline.c itself
  # was not linked into unittests).
  add_dependencies(controlpush_unittests ggo_epics_controlpush_cmdline)
  target_include_directories(controlpush_unittests PRIVATE ${_controlpush_includes})
  target_link_libraries(controlpush_unittests PRIVATE
    NSCLDAQ::tclPlusHeaders NSCLDAQ::EPICSBin NSCLDAQ::PixieHeaders
    NSCLDAQ::CppUnit ${_controlpush_libs})
endif()
