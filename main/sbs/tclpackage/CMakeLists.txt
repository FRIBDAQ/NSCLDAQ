nscldaq_add_library(SBSVme VERSION_INFO 1:0:0
  SOURCES VMECommand.cpp VMEMapCommand.cpp VMEPackage.cpp)
target_include_directories(SBSVme PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/sbs/nsclapi
  ${PROJECT_SOURCE_DIR}/base/CopyrightTools)
target_compile_definitions(SBSVme PRIVATE HOME="${prefix}")
target_link_libraries(SBSVme
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::tclPlus SBSVmeAPI License NSCLDAQ::Tcl btp pthread
    m stdc++ gcc c)

# install-exec-local: the package also lives in TclLibs/SBSVme.
# (The libtool static archive libSBSVme.a that was copied there too is not
# built by CMake.)

nscldaq_install_mkdir(TclLibs/SBSVme)
install(TARGETS SBSVme LIBRARY DESTINATION TclLibs/SBSVme)
install(CODE "file(APPEND \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/TclLibs/SBSVme/pkgIndex.tcl\"
  \"package ifneeded Vme 2.0 [list load [file join \\\$dir libSBSVme.so]]\\n\")")
