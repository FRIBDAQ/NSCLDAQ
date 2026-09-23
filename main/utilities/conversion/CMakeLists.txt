nscldaq_add_library(Conversion
  SOURCES CTransformMediator.h C8p0to10p0Mediator.cpp C10p0to8p0Mediator.cpp
    C10p0to11p0Mediator.cpp CTransformMediator.hpp CTransform8p0to10p0.cpp
    CTransform10p0to8p0.cpp CTransform10p0to11p0.cpp CTransform11p0to10p0.cpp
    CTransformFactory.cpp CCompositePredicate.cpp)
target_include_directories(Conversion PRIVATE
  ${PROJECT_SOURCE_DIR}/utilities/format
  ${PROJECT_SOURCE_DIR}/utilities/filter
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/utilities/FormattedIO
  ${PROJECT_SOURCE_DIR}/utilities/Buffer
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/factories)
target_link_libraries(Conversion
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC dataformatv8 dataformatv10 dataformatv11 filter daqio daqformatio
    NSCLDAQ::tclPlus)

nscldaq_install_headers(CTransformMediator.h C8p0to10p0Mediator.h
  C10p0to8p0Mediator.h C10p0to11p0Mediator.h CTransform8p0to10p0.h
  CTransform10p0to8p0.h CTransform10p0to11p0.h CTransform11p0to10p0.h
  CTransformFactory.h CPredicate.h CCompositePredicate.h)

set(_conversion_includes
  ${PROJECT_SOURCE_DIR}/utilities/Buffer
  ${PROJECT_SOURCE_DIR}/utilities/filter
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/utilities/FormattedIO
  ${PROJECT_SOURCE_DIR}/utilities/format
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/factories)

#------------------- Tests:

nscldaq_add_executable(conversion_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp ctransform8p0to10p0tests.cpp
    ctransform10p0to8p0tests.cpp ctransform10p0to11p0tests.cpp
    ctransform11p0to10p0tests.cpp ctransformfactorytests.cpp
    c10p0to8p0mediatortests.cpp)
target_include_directories(conversion_unittests PRIVATE ${_conversion_includes}
  ${PROJECT_SOURCE_DIR}/base/testutils)
target_link_libraries(conversion_unittests PRIVATE
  NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  NSCLDAQ::CppUnit Conversion buffer dataformatv8 dataformatv10 dataformatv11
  daqio daqformatio filter NSCLDAQ::Exception NSCLDAQ::Tcl)
nscldaq_add_test(NAME unittests COMMAND conversion_unittests)

#------------------- format_converter

nscldaq_gengetopt(format_converter_options format_converter_options.ggo
  ARGS -a cmdLineOpts)

nscldaq_add_executable(format_converter SOURCES Main.h Main.cpp)
nscldaq_use_gengetopt(format_converter format_converter_options)
target_include_directories(format_converter PRIVATE ${_conversion_includes})
target_link_libraries(format_converter PRIVATE
  NSCLDAQ::PixieHeaders
  Conversion buffer dataformatv8 dataformatv10 dataformatv11 filter daqio
  daqformatio)

# install-exec-local

install(PROGRAMS convert_format.py DESTINATION bin RENAME convert_format)
