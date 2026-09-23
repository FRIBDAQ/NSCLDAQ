#
#  The pixie16 Tcl package: libpixie16 (lib and TclLibs/pixie16) and the
#  pixieserver package.
#

nscldaq_add_library(pixie16
  SOURCES Pixie16Package.cpp CTclCommand.cpp InitSystem.cpp Inventory.cpp
    Release.cpp Boot.cpp ReadChanPar.cpp ReadModPar.cpp WriteChanPar.cpp
    WriteModPar.cpp SaveParams.cpp RestoreParams.cpp IsActive.cpp
    CAdjustOffsets.cpp)
target_include_directories(pixie16 PRIVATE
  ${PROJECT_SOURCE_DIR}/ddas/configuration
  ${PROJECT_BINARY_DIR}/ddas
  ${PROJECT_SOURCE_DIR}/ddas/exception)
target_link_libraries(pixie16
  PRIVATE NSCLDAQ::tclPlusHeaders
  PUBLIC Configuration NSCLDAQ::Pixie NSCLDAQ::TclOnly DDASException
    NSCLDAQ::Exception)

# The package index is written (echo ... > pkgIndex.tcl), not made by
# pkg_mkIndex.

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
  "package ifneeded pixie16 1.1 [list load [file join \$dir libpixie16.so]]\n"
  "package ifneeded pixieserver 1.0 [list source [file join \$dir Pixie16Server.tcl]]\n")

# install-exec-hook

nscldaq_install_copy(lib/libpixie16.so TclLibs/pixie16/libpixie16.so)
install(FILES Pixie16Server.tcl ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
  DESTINATION TclLibs/pixie16)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(pixie16)
