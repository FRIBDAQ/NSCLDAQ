#
#  Top level CMake build for NSCLDAQ.
#
#  This replaces configure.ac/Makefile.am.  Configure options map 1:1 onto
#  cache variables:
#
#     --enable-foo-bar        ->  -DENABLE_FOO_BAR=ON
#     --with-foo-bar=value    ->  -DWITH_FOO_BAR=value
#
#  see cmake/NSCLDAQOptions.cmake and README-cmake.md for the full list.
#
#  Typical use:
#     cmake -S main -B build -DCMAKE_INSTALL_PREFIX=/usr/opt/nscldaq/12.2-010 ...
#     cmake --build build -j
#     cmake --install build
#     cd build && ctest             (tests, like 'make check', expect an install)
#

cmake_minimum_required(VERSION 3.18.4)

# The version is the AC_INIT version; CMake needs a numeric project version
# so the full string is kept separately.

set(NSCLDAQ_VERSION_STRING "12.2-010")
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)-0*([0-9]+)$" "\\1.\\2.\\3"
  _nscldaq_numeric_version "${NSCLDAQ_VERSION_STRING}")

# Default build type: RelWithDebInfo, whose flags NSCLDAQCompiler.cmake sets
# to the Autotools "-g -O2" (no -DNDEBUG).

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
    "Build type (Debug Release RelWithDebInfo MinSizeRel)")
endif()

project(nscldaq
  VERSION ${_nscldaq_numeric_version}
  DESCRIPTION "NSCL/FRIB data acquisition system"
  HOMEPAGE_URL "https://github.com/FRIBDAQ/NSCLDAQ"
  LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(NSCLDAQOptions)       # Cache variables mirroring the configure options.
include(NSCLDAQCompiler)      # Language standard, global flags, config.h.
include(NSCLDAQDependencies)  # External packages -> NSCLDAQ:: targets.
include(NSCLDAQIncorp)        # libtcl++, unifiedformat, ddasformat, tclhttpd.
include(NSCLDAQHelpers)       # add_library/executable/test/gengetopt/... helpers.

#----------------------------------------------------------------------------
#  Top level configured files.

nscldaq_configure_file(nscldaq.in nscldaq)
nscldaq_configure_file(daqsetup.bash.in daqsetup.bash)

# ddas/config_pixie16api.h is installed unconditionally (by the top level)
# even when DDAS is not built, so it is configured here.

configure_file(ddas/config_pixie16api.h.in
  ${PROJECT_BINARY_DIR}/ddas/config_pixie16api.h COPYONLY)

#----------------------------------------------------------------------------
#  Subdirectories - same order as SUBDIRS in the old Makefile.am as install
#  order matters for the pkg_mkIndex runs.

add_subdirectory(base)
add_subdirectory(caenfw-common)
add_subdirectory(servers)
add_subdirectory(base/dataflow)
add_subdirectory(daq)
add_subdirectory(utilities)
if(ENABLE_DOCS)
  add_subdirectory(docbuild)
  add_subdirectory(docconfig)
  add_subdirectory(simplesetups)
  add_subdirectory(cookbooks)
endif()
add_subdirectory(sbs)
add_subdirectory(usb/common/configurableobject)
if(ENABLE_EPICS_TOOLS)
  add_subdirectory(epics)
endif()
if(ENABLE_USB)
  add_subdirectory(usb)
endif()
if(ENABLE_DDAS)
  add_subdirectory(ddas)
endif()
if(ENABLE_CAEN_DIGITIZER_SUPPORT)
  add_subdirectory(caendigitizers)
endif()
if(ENABLE_CAEN_NEXTGEN)
  add_subdirectory(caen-nextgen)
endif()
add_subdirectory(srs)
if(ENABLE_MVLC_STACK_GENERATOR)
  add_subdirectory(mvlc)
endif()
add_subdirectory(examples)
if(ENABLE_RUST_TOOLS)
  add_subdirectory(rust_tools)
endif()

# libtool's static libFoo.a beside every installed libFoo.so (needs every
# library's final settings, so after all subdirectories).  Those targets are
# created here but carry their own directory's -I. -I$(srcdir), so don't
# also add this (top level) directory's.

set(CMAKE_INCLUDE_CURRENT_DIR OFF)
nscldaq_add_static_libraries()

#----------------------------------------------------------------------------
#  Top level installation (was install-data-local in Makefile.am).
#  Declared after the subdirectories so it runs last, as it did before.

nscldaq_install_mkdir(TclLibs include pythonLibs pythonLibs/nscldaq share/html/figures)
nscldaq_install_tcl_index(TclLibs "*.so")

install(PROGRAMS ${PROJECT_BINARY_DIR}/config.h DESTINATION include)
install(PROGRAMS ${PROJECT_BINARY_DIR}/ddas/config_pixie16api.h DESTINATION include)
install(CODE "file(TOUCH \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/pythonLibs/nscldaq/__init__.py\")")

# The VERSION file records how/when this was built and the versions of the
# incorporated packages.

cmake_host_system_information(RESULT _nscldaq_host QUERY HOSTNAME)
get_cmake_property(_nscldaq_cache_vars CACHE_VARIABLES)
set(NSCLDAQ_CONFIGURE_OPTIONS "")
foreach(_v IN LISTS _nscldaq_cache_vars)
  if(_v MATCHES "^(ENABLE|WITH)_" AND NOT "${${_v}}" STREQUAL "")
    string(APPEND NSCLDAQ_CONFIGURE_OPTIONS " -D${_v}=${${_v}}")
  endif()
endforeach()
set(NSCLDAQ_VERSION_SCRIPT ${PROJECT_BINARY_DIR}/cmake/write_version.cmake)
configure_file(cmake/write_version.cmake.in ${NSCLDAQ_VERSION_SCRIPT} @ONLY)
install(SCRIPT ${NSCLDAQ_VERSION_SCRIPT})

install(FILES ChangeLog DESTINATION .)
install(PROGRAMS ${PROJECT_BINARY_DIR}/daqsetup.bash DESTINATION .)
file(GLOB _nscldaq_figures ${PROJECT_SOURCE_DIR}/figures/*.png)
install(FILES ${_nscldaq_figures} DESTINATION share/html/figures)
install(FILES QT5LICENSE DESTINATION .)

#----------------------------------------------------------------------------
#  Source packaging (replaces 'make dist').

include(NSCLDAQPackaging)

nscldaq_print_summary()
