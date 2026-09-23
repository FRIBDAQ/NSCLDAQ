nscldaq_add_library(daqshm
  SOURCES daqshm.cpp os.cpp io.cpp CTimeout.cpp CSemaphore.cpp
    CPosixBlockingRecordLock.cpp CBufferedOutput.cpp NSCLDAQLog.cpp
    CRingBlockReader.cpp CRingFileBlockReader.cpp CPagedOutput.cpp
    CElapsedTime.cpp utils.cpp)
target_include_directories(daqshm PRIVATE ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(daqshm
  PRIVATE NSCLDAQ::PixieHeaders NSCLDAQ::tclPlusHeaders
  PUBLIC daqthreads NSCLDAQ::Threads crypt NSCLDAQ::Exception NSCLDAQ::BoostLog)

nscldaq_install_headers(daqshm.h os.h io.h CTimeout.h CSemaphore.h
  CPosixBlockingRecordLock.h CBufferedOutput.h NSCLDAQLog.h
  CRingBlockReader.h CRingFileBlockReader.h CPagedOutput.h CElapsedTime.h
  utils.h)

# Tests

nscldaq_add_executable(os_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp createTests.cpp removeTests.cpp attachTests.cpp
    detachTests.cpp timeoutTests.cpp semaphoretests.cpp closeunusedtests.cpp
    testBufferedOutput.cpp logtest.cpp poutputtests.cpp testiov.cpp eltest.cpp)
target_include_directories(os_unittests PRIVATE ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(os_unittests PRIVATE
  NSCLDAQ::PixieHeaders daqshm daqthreads NSCLDAQ::tclPlus NSCLDAQ::CppUnit
  NSCLDAQ::Threads NSCLDAQ::BoostLog NSCLDAQ::Tcl)
nscldaq_add_test(NAME unittests COMMAND os_unittests)
