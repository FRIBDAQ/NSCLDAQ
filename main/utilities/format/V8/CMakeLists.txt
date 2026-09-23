nscldaq_add_library(dataformatv8
  SOURCES bheader.cpp CV8Buffer.cpp CRawBuffer.cpp CPhysicsEventBuffer.cpp
    CScalerBuffer.cpp CPhysicsEventBodyParser.cpp CGenericBodyParser.cpp
    CControlBuffer.cpp CTextBuffer.cpp CVoidBuffer.cpp)
target_include_directories(dataformatv8 PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${PROJECT_SOURCE_DIR}/utilities/format
  ${PROJECT_SOURCE_DIR}/utilities/Buffer)
target_link_libraries(dataformatv8 PUBLIC NSCLDAQ::tclPlus buffer)

nscldaq_install_headers(DataFormatV8.h bheader.h CV8Buffer.h CRawBuffer.h
  CPhysicsEventBuffer.h CScalerBuffer.h CPhysicsEventBodyParser.h
  CGenericBodyParser.h CControlBuffer.h CTextBuffer.h CVoidBuffer.h
  format_cast.h
  DESTINATION include/V8)

# Tests

nscldaq_add_executable(formatv8_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp ccontrolbuffertest.cpp physicseventtest.cpp
    scalerbuffertest.cpp format_casttest.cpp rawbuffertest.cpp
    ctextbuffertest.cpp cvoidbuffertest.cpp cgenericbodyparsertest.cpp)
target_include_directories(formatv8_unittests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${PROJECT_SOURCE_DIR}/utilities/format
  ${PROJECT_SOURCE_DIR}/utilities/Buffer
  ${PROJECT_SOURCE_DIR}/base/testutils
  ${PROJECT_SOURCE_DIR}/base/headers)
target_link_libraries(formatv8_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders
  NSCLDAQ::CppUnit dataformatv8 buffer NSCLDAQ::Exception NSCLDAQ::Tcl)
nscldaq_add_test(NAME unittests COMMAND formatv8_unittests)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(dataformatv8)
