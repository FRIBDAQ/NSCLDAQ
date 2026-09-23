#
#  libConfigurableObject - added from the top level CMakeLists.txt.
#

nscldaq_add_library(ConfigurableObject SOURCES XXUSBConfigurableObject.cpp)
target_link_libraries(ConfigurableObject
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::tclPlus NSCLDAQ::Tcl m NSCLDAQ::Threads)

nscldaq_install_headers(XXUSBConfigurableObject.h)
