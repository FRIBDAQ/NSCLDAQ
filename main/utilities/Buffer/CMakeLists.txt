nscldaq_add_library(buffer SOURCES ByteBuffer.cpp ByteOrder.cpp)
target_link_libraries(buffer PUBLIC NSCLDAQ::tclPlus)

nscldaq_install_headers(ByteBuffer.h ByteOrder.h BufferPtr.h Deserializer.h)

# Tests

nscldaq_add_executable(buffer_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp translatorptrtest.cpp byteordertest.cpp
    bytebuffertest.cpp deserializertest.cpp ByteBuffer.cpp)
target_include_directories(buffer_unittests PRIVATE ${PROJECT_SOURCE_DIR}/base/testutils)
target_link_libraries(buffer_unittests PRIVATE
  buffer NSCLDAQ::CppUnit NSCLDAQ::tclPlus NSCLDAQ::Exception NSCLDAQ::Tcl)
nscldaq_add_test(NAME unittests COMMAND buffer_unittests)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(buffer)
