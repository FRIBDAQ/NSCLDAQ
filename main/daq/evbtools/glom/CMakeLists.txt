##
# Build glom with buffered fragment reader:
#

nscldaq_gengetopt(glom glom.ggo)

nscldaq_add_executable(glom SOURCES glomMain.cpp CBufferedFragmentReader.cpp)
nscldaq_use_gengetopt(glom glom)
target_include_directories(glom PRIVATE
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/thread)
target_link_libraries(glom PRIVATE NSCLDAQ::tclPlusHeaders NSCLDAQ::PixieHeaders
  EventBuilderClient dataformat NSCLDAQ::UFMT daqshm NSCLDAQ::Exception)

##
# Make a buffered fragment reader library and install the header in case
# its useful in other contexts:
#

nscldaq_add_library(BufferedFragmentReader
  SOURCES CBufferedFragmentReader.cpp CBufferedFragmentReader.h)
target_link_libraries(BufferedFragmentReader PUBLIC NSCLDAQ::UFMT)

nscldaq_install_headers(CBufferedFragmentReader.h)
