set(_compat_includes
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/os)

nscldaq_gengetopt(parser compatibilitybuffer.ggo)
nscldaq_gengetopt(loggeroptions loggeroptions.ggo)
nscldaq_gengetopt(btoroptions buffertoring.ggo)
nscldaq_gengetopt(test test.ggo)

nscldaq_add_executable(compatibilitybuffer
  SOURCES compatibilitybuffer.cpp CBufferConverter.cpp)
nscldaq_use_gengetopt(compatibilitybuffer parser)
target_include_directories(compatibilitybuffer PRIVATE ${_compat_includes})
target_link_libraries(compatibilitybuffer PRIVATE
  NSCLDAQ::PixieHeaders dataformat NSCLDAQ::UFMT daqshm)

nscldaq_add_executable(compatibilitylogger
  SOURCES compatibilitylogger.cpp CLogRun.cpp)
nscldaq_use_gengetopt(compatibilitylogger loggeroptions)
target_include_directories(compatibilitylogger PRIVATE ${_compat_includes})
target_link_libraries(compatibilitylogger PRIVATE
  NSCLDAQ::PixieHeaders NSCLDAQ::UFMTHeaders)

nscldaq_add_executable(BufferToRing SOURCES BufferToRing.cpp)
nscldaq_use_gengetopt(BufferToRing btoroptions)
target_include_directories(BufferToRing PRIVATE ${_compat_includes}
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder)
target_link_libraries(BufferToRing PRIVATE
  NSCLDAQ::PixieHeaders dataformat NSCLDAQ::UFMT daqshm dl)

nscldaq_add_executable(convert10to11 SOURCES Convert10to11.cpp)
target_include_directories(convert10to11 PRIVATE ${_compat_includes}
  ${PROJECT_SOURCE_DIR}/daq/IO)
target_link_libraries(convert10to11 PRIVATE
  NSCLDAQ::PixieHeaders dataformat daqshm NSCLDAQ::UFMT daqio)

#  Configured scripts (AC_CONFIG_FILES).

nscldaq_configure_file(eventlog-compat.in eventlog-compat)
nscldaq_configure_file(spectcldaq.in spectcldaq)
nscldaq_configure_file(s800toring.in s800toring)
nscldaq_configure_file(bufdump.in bufdump)

#  install-exec-local: generated scripts are in the build dir, the rest in
#  the source dir.

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/eventlog-compat
  ${CMAKE_CURRENT_BINARY_DIR}/spectcldaq DESTINATION bin)
install(PROGRAMS spectcldaq.server.tcl DESTINATION bin RENAME spectcldaq.server)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/s800toring netcat.tcl
  ${CMAKE_CURRENT_BINARY_DIR}/bufdump DESTINATION bin)

#  For testing (noinst):

nscldaq_add_executable(compatibility_testDataSource NOINST
  OUTPUT_NAME testDataSource SOURCES testDataSource.cpp)
nscldaq_use_gengetopt(compatibility_testDataSource test)
target_include_directories(compatibility_testDataSource PRIVATE
  ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(compatibility_testDataSource PRIVATE
  NSCLDAQ::PixieHeaders daqshm NSCLDAQ::tclPlus NSCLDAQ::TclOnly)
