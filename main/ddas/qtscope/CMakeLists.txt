#
#  QtScope: the embedded python launcher, the utility library it uses (via
#  ctypes) and the python sources.
#

nscldaq_add_executable(qtscope_bin OUTPUT_NAME qtscope.bin SOURCES qtscope.cpp)
target_compile_definitions(qtscope_bin PRIVATE PREFIX="${prefix}")
target_link_libraries(qtscope_bin PRIVATE NSCLDAQ::Python NSCLDAQ::PixieHeaders)

nscldaq_add_library(PixieUtilities
  SOURCES CPixieDSPUtilities.cpp CPixieRunUtilities.cpp
    CPixieSystemUtilities.cpp CPixieTraceUtilities.cpp CDataGenerator.cpp)
target_include_directories(PixieUtilities PRIVATE
  ${PROJECT_BINARY_DIR}/ddas
  ${PROJECT_SOURCE_DIR}/ddas/configuration
  ${PROJECT_SOURCE_DIR}/ddas/booter
  ${PROJECT_SOURCE_DIR}/ddas/exception)
target_link_libraries(PixieUtilities
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC Configuration SystemBooter DDASException NSCLDAQ::Pixie
    NSCLDAQ::Exception)

nscldaq_configure_file(qtscope.in qtscope)

# install-exec-hook

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/qtscope DESTINATION bin)

# install-data-hook

install(FILES figures/timing_diagram.png DESTINATION ddas/qtscope/figures)
file(GLOB _qtscope_python CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
install(FILES ${_qtscope_python} DESTINATION ddas/qtscope)
