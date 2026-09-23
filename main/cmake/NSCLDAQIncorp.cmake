#
#  NSCLDAQIncorp.cmake
#
#  Incorporated packages built with ExternalProject:
#
#     libtcl        libtcl++ (autotools)     -> ${prefix}/{lib,include,bin}
#     unifiedformat UnifiedFormat (CMake)    -> ${prefix}/unifiedformat
#     ddasformat    DDASFormat (CMake)       -> ${prefix}/ddasformat
#     tclhttpd      tclhttpd3.5.1 (autoconf) -> ${prefix}/share/tclhttpd
#
#  If a package has already been incorporated into the source tree
#  (main/libtcl etc., e.g. by ./tcl++incorp) that copy is used.  Otherwise
#  the pinned tag is cloned into the build tree at build time.
#
#  The packages are configured for the final install prefix but installed at
#  build time into a staging area inside the build tree, one per package
#  (DESTDIR=${NSCLDAQ_INCORP_STAGE}/<package>; NSCLDAQ_<PKG>_STAGE_PREFIX is
#  the staged prefix).  NSCLDAQ is compiled and linked against that staged
#  copy.  'cmake --install' then copies the staged files into the real
#  prefix, so nothing is written to the prefix before install (autotools
#  installed them while running configure).  A package's staging area is
#  emptied before it is re-staged, so files removed from its sources don't
#  linger there.
#
#  Targets provided:
#     NSCLDAQ::tclPlus      @LIBTCLPLUS_CFLAGS@ @LIBTCLPLUS_LDFLAGS@
#     NSCLDAQ::Exception    @LIBTCLPLUS_CFLAGS@ @LIBEXCEPTION_LDFLAGS@
#     NSCLDAQ::UFMT         @UFMT_CPPFLAGS@ @UFMT_LDFLAGS@
#     NSCLDAQ::DDASFormat   @DDASFMT_CPPFLAGS@ @DDASFMT_LDFLAGS@
#     NSCLDAQ::PrefixInclude  -I@prefix@/include - the staged incorp headers
#                           plus the staged NSCLDAQ public headers (see
#                           nscldaq_install_headers()).
#
#  Each has a matching ordering dependency on the external build so any
#  target linking them waits for it.
#

include_guard(GLOBAL)

include(ExternalProject)

set(NSCLDAQ_INCORP_STAGE ${PROJECT_BINARY_DIR}/incorp-stage)
foreach(_pkg LIBTCL UFMT DDASFMT TCLHTTPD)
  string(TOLOWER ${_pkg} _dir)
  set(NSCLDAQ_${_pkg}_STAGE ${NSCLDAQ_INCORP_STAGE}/${_dir})
  set(NSCLDAQ_${_pkg}_STAGE_PREFIX ${NSCLDAQ_INCORP_STAGE}/${_dir}${CMAKE_INSTALL_PREFIX})
endforeach()

# Headers NSCLDAQ installs into ${prefix}/include are also staged here at
# configure time so code that compiled against the installed headers
# (-I@prefix@/include) can be built before 'install'.

set(NSCLDAQ_STAGE_INCLUDE_DIR ${PROJECT_BINARY_DIR}/stage/include)
file(MAKE_DIRECTORY ${NSCLDAQ_STAGE_INCLUDE_DIR})

if(CMAKE_GENERATOR MATCHES "Make")
  set(_incorp_make "$(MAKE)")
else()
  set(_incorp_make make -j${WITH_INCORP_BUILD_CORES})
endif()

# Imported shared library in the staging area.
#   _nscldaq_staged_library(<target> <path> <incdir> <external-project>)

function(_nscldaq_staged_library tgt path incdir ep)
  add_library(${tgt} SHARED IMPORTED GLOBAL)
  set_target_properties(${tgt} PROPERTIES IMPORTED_LOCATION ${path})
  # Imported targets require their include directories to exist.
  file(MAKE_DIRECTORY ${incdir})
  target_include_directories(${tgt} INTERFACE ${incdir})
  add_dependencies(${tgt} ${ep})
endfunction()

# Where does a package's source come from?  Sets <var>_SOURCE_ARGS.

function(_nscldaq_incorp_source var dir marker repo tag)
  if(EXISTS ${PROJECT_SOURCE_DIR}/${dir}/${marker})
    message(STATUS "Using previously incorporated ${dir} from ${PROJECT_SOURCE_DIR}/${dir}")
    set(${var}_SOURCE_ARGS SOURCE_DIR ${PROJECT_SOURCE_DIR}/${dir} PARENT_SCOPE)
    set(${var}_FROM_GIT OFF PARENT_SCOPE)
    set(${var}_LOCAL_DIR ${PROJECT_SOURCE_DIR}/${dir} PARENT_SCOPE)
  else()
    message(STATUS "Incorporating ${dir} from ${repo} (${tag}) at build time")
    set(${var}_SOURCE_ARGS
      GIT_REPOSITORY ${repo}
      GIT_TAG ${tag}
      GIT_SHALLOW ON
      GIT_PROGRESS OFF
      UPDATE_DISCONNECTED ON
      PARENT_SCOPE)
    set(${var}_FROM_GIT ON PARENT_SCOPE)
    set(${var}_LOCAL_DIR "" PARENT_SCOPE)
  endif()
endfunction()

# ExternalProject steps are stamp based: once built, changes to a package's
# sources would never be picked up (configure.ac rebuilt incorporated
# packages on every configure).  For a package used from the source tree,
# make <step> of <ep> depend on
#  - its source files, so an edit (or a new file, being newer than the
#    stamp) reruns the step, and
#  - a manifest listing those files, rewritten only when the list changes,
#    so deleting or renaming a file reruns it too (a deleted file simply
#    drops out of the dependencies, which alone would never trigger).
# The globs are CONFIGURE_DEPENDS, so the next build notices added or
# removed files, regenerates, and then reruns the step.  Nothing reruns
# when nothing changed, so dependents are not relinked needlessly.
#   _nscldaq_incorp_track(<ep> <step> <dir> [ALL_FILES])

function(_nscldaq_incorp_track ep step dir)
  cmake_parse_arguments(A "ALL_FILES" "" "" ${ARGN})
  if(NOT dir)
    return()
  endif()
  if(A_ALL_FILES)
    file(GLOB_RECURSE _srcs CONFIGURE_DEPENDS ${dir}/*)
  else()
    file(GLOB_RECURSE _srcs CONFIGURE_DEPENDS
      ${dir}/*.c ${dir}/*.cc ${dir}/*.cpp ${dir}/*.cxx ${dir}/*.h ${dir}/*.hpp
      ${dir}/*.i ${dir}/*.py ${dir}/*.tcl ${dir}/*.in ${dir}/*.m4
      ${dir}/CMakeLists.txt ${dir}/*.cmake ${dir}/Makefile.am ${dir}/configure.ac)
  endif()
  list(FILTER _srcs EXCLUDE REGEX "/(\\.git|autom4te\\.cache)/")
  list(SORT _srcs)
  set(_manifest ${PROJECT_BINARY_DIR}/incorp/${ep}-sources.txt)
  string(REPLACE ";" "\n" _list "${_srcs}")
  file(WRITE ${_manifest}.tmp "${_list}\n")
  # configure_file only rewrites the manifest when its content changed.
  configure_file(${_manifest}.tmp ${_manifest} COPYONLY)
  ExternalProject_Add_StepDependencies(${ep} ${step} ${_manifest} ${_srcs})
endfunction()

#---------------------------------------------------------------------------
#  libtcl++

_nscldaq_incorp_source(LIBTCL libtcl configure.ac
  https://github.com/FRIBDAQ/libtclplus ${NSCLDAQ_LIBTCLPLUS_TAG})

set(_libtcl_libdir ${NSCLDAQ_LIBTCL_STAGE_PREFIX}/lib)

# The source needs 'autoreconf -if' unless tcl++incorp already did it.

set(_libtcl_bootstrap
  sh -c "test -x <SOURCE_DIR>/configure || (cd <SOURCE_DIR> && autoreconf -if)")

ExternalProject_Add(libtcl_incorp
  ${LIBTCL_SOURCE_ARGS}
  PREFIX ${PROJECT_BINARY_DIR}/incorp/libtcl
  BINARY_DIR ${PROJECT_BINARY_DIR}/incorp/libtcl/build
  CONFIGURE_COMMAND ${_libtcl_bootstrap}
    COMMAND <SOURCE_DIR>/configure --prefix=${CMAKE_INSTALL_PREFIX}
      CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
  BUILD_COMMAND ${_incorp_make} all
  # Its install-data-local rules race under parallel make; configure.ac
  # ran 'make install' serially.
  INSTALL_COMMAND ${CMAKE_COMMAND} -E remove_directory ${NSCLDAQ_LIBTCL_STAGE}
    COMMAND make -j1 install DESTDIR=${NSCLDAQ_LIBTCL_STAGE}
  BUILD_BYPRODUCTS
    ${_libtcl_libdir}/libtclPlus.so
    ${_libtcl_libdir}/libException.so
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
  LOG_OUTPUT_ON_FAILURE ON)

_nscldaq_incorp_track(libtcl_incorp build "${LIBTCL_LOCAL_DIR}")

_nscldaq_staged_library(NSCLDAQ::Exception ${_libtcl_libdir}/libException.so
  ${NSCLDAQ_LIBTCL_STAGE_PREFIX}/include libtcl_incorp)
_nscldaq_staged_library(NSCLDAQ::tclPlus ${_libtcl_libdir}/libtclPlus.so
  ${NSCLDAQ_LIBTCL_STAGE_PREFIX}/include libtcl_incorp)
set_property(TARGET NSCLDAQ::tclPlus APPEND PROPERTY
  INTERFACE_LINK_LIBRARIES NSCLDAQ::Exception)

set(LIBTCLPLUS_CFLAGS "-I${CMAKE_INSTALL_PREFIX}/include")
set(LIBEXCEPTION_LDFLAGS "-L${CMAKE_INSTALL_PREFIX}/lib -lException -Wl,\"-rpath=${CMAKE_INSTALL_PREFIX}/lib\"")
set(LIBTCLPLUS_LDFLAGS "-L${CMAKE_INSTALL_PREFIX}/lib -ltclPlus ${LIBEXCEPTION_LDFLAGS}")

add_library(NSCLDAQ::PrefixInclude INTERFACE IMPORTED GLOBAL)
target_include_directories(NSCLDAQ::PrefixInclude INTERFACE
  ${NSCLDAQ_STAGE_INCLUDE_DIR} ${NSCLDAQ_LIBTCL_STAGE_PREFIX}/include)
add_dependencies(NSCLDAQ::PrefixInclude libtcl_incorp)

#---------------------------------------------------------------------------
#  CMake based format libraries.

# No CMAKE_BUILD_TYPE: configure.ac ran a plain 'cmake ..' and these
# projects set their own -g -O2.

set(_incorp_cmake_args
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DPython3_EXECUTABLE=${Python3_EXECUTABLE})
if(EXISTS "${NSCLDAQ_PYTHON3_INCLUDE_DIR}/Python.h" AND EXISTS "${NSCLDAQ_PYTHON3_LIBRARY}")
  list(APPEND _incorp_cmake_args
    -DPython3_INCLUDE_DIR=${NSCLDAQ_PYTHON3_INCLUDE_DIR}
    -DPython3_LIBRARY=${NSCLDAQ_PYTHON3_LIBRARY})
endif()

function(_nscldaq_cmake_incorp name dir repo tag instdir stage libs)
  _nscldaq_incorp_source(_SRC ${dir} CMakeLists.txt ${repo} ${tag})
  set(_libdir ${stage}${CMAKE_INSTALL_PREFIX}/${instdir}/lib)
  set(_byproducts "")
  foreach(_l IN LISTS libs)
    list(APPEND _byproducts ${_libdir}/lib${_l}.so)
  endforeach()
  ExternalProject_Add(${name}
    ${_SRC_SOURCE_ARGS}
    PREFIX ${PROJECT_BINARY_DIR}/incorp/${dir}
    BINARY_DIR ${PROJECT_BINARY_DIR}/incorp/${dir}/build
    CMAKE_ARGS ${_incorp_cmake_args}
      -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/${instdir}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> -j ${WITH_INCORP_BUILD_CORES}
    INSTALL_COMMAND ${CMAKE_COMMAND} -E remove_directory ${stage}
      COMMAND ${CMAKE_COMMAND} -E env DESTDIR=${stage}
      ${CMAKE_COMMAND} --install <BINARY_DIR>
    BUILD_BYPRODUCTS ${_byproducts}
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON)
  _nscldaq_incorp_track(${name} build "${_SRC_LOCAL_DIR}")
endfunction()

_nscldaq_cmake_incorp(unifiedformat_incorp unifiedformat
  https://github.com/FRIBDAQ/UnifiedFormat.git ${NSCLDAQ_UNIFIEDFORMAT_TAG}
  unifiedformat ${NSCLDAQ_UFMT_STAGE}
  "NSCLDAQFormat;V10Format;V11Format;V12Format;AbstractFormat")

add_library(NSCLDAQ::UFMT INTERFACE IMPORTED GLOBAL)
foreach(_l NSCLDAQFormat V10Format V11Format V12Format AbstractFormat)
  _nscldaq_staged_library(NSCLDAQ::UFMT_${_l}
    ${NSCLDAQ_UFMT_STAGE_PREFIX}/unifiedformat/lib/lib${_l}.so
    ${NSCLDAQ_UFMT_STAGE_PREFIX}/unifiedformat/include unifiedformat_incorp)
  target_link_libraries(NSCLDAQ::UFMT INTERFACE NSCLDAQ::UFMT_${_l})
endforeach()

set(UFMT_CPPFLAGS "-I${CMAKE_INSTALL_PREFIX}/unifiedformat/include")
set(UFMT_LDFLAGS "-L${CMAKE_INSTALL_PREFIX}/unifiedformat/lib -lNSCLDAQFormat -lV10Format -lV11Format -lV12Format -lAbstractFormat -Wl,-rpath=${CMAKE_INSTALL_PREFIX}/unifiedformat/lib")

_nscldaq_cmake_incorp(ddasformat_incorp ddasformat
  https://github.com/FRIBDAQ/DDASFormat.git ${NSCLDAQ_DDASFORMAT_TAG}
  ddasformat ${NSCLDAQ_DDASFMT_STAGE} "DDASFormat")

_nscldaq_staged_library(NSCLDAQ::DDASFormat
  ${NSCLDAQ_DDASFMT_STAGE_PREFIX}/ddasformat/lib/libDDASFormat.so
  ${NSCLDAQ_DDASFMT_STAGE_PREFIX}/ddasformat/include ddasformat_incorp)

set(DDASFMT_CPPFLAGS "-I${CMAKE_INSTALL_PREFIX}/ddasformat/include")
set(DDASFMT_LDFLAGS "-L${CMAKE_INSTALL_PREFIX}/ddasformat/lib -lDDASFormat -Wl,-rpath=${CMAKE_INSTALL_PREFIX}/ddasformat/lib")

#---------------------------------------------------------------------------
#  tclhttpd3.5.1 (in the source tree, installs into ${prefix}/share/tclhttpd).

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/tclhttpd3.5.1/configure)
  message(FATAL_ERROR "Cannot find tclhttp in source tree!")
endif()

# NSCLDAQ_TCL_CONFIG_DIR (the directory holding tclConfig.sh) comes from
# NSCLDAQDependencies.cmake.

# tclhttpd's configure rewrites files in its source directory so build a
# private copy, as configure.ac did.

ExternalProject_Add(tclhttpd_incorp
  # Start from a clean copy so files deleted from the source tree don't
  # linger in it (the build directory is inside the copy, so recreate it).
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E remove_directory <SOURCE_DIR>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/tclhttpd3.5.1 <SOURCE_DIR>
    COMMAND ${CMAKE_COMMAND} -E make_directory <BINARY_DIR>
  PREFIX ${PROJECT_BINARY_DIR}/incorp/tclhttpd3.5.1
  # Not <PREFIX>/src: that is where ExternalProject runs the download step
  # and keeps its stamps, and the copy is removed and recreated.
  SOURCE_DIR ${PROJECT_BINARY_DIR}/incorp/tclhttpd3.5.1/source
  BINARY_DIR ${PROJECT_BINARY_DIR}/incorp/tclhttpd3.5.1/source/build
  CONFIGURE_COMMAND ../configure --prefix=${CMAKE_INSTALL_PREFIX}/share/tclhttpd
    --enable-gcc --with-tclinclude=/usr/include/tcl${TCL_VERSION}
    --with-tcl=${NSCLDAQ_TCL_CONFIG_DIR}
  BUILD_COMMAND ${_incorp_make} all
  # Its install rules race when run in parallel (install-htdocs needs
  # installdirs but does not depend on it) so install serially.
  INSTALL_COMMAND ${CMAKE_COMMAND} -E remove_directory ${NSCLDAQ_TCLHTTPD_STAGE}
    COMMAND make -j1 install DESTDIR=${NSCLDAQ_TCLHTTPD_STAGE}
  LOG_CONFIGURE ON
  LOG_BUILD ON
  LOG_INSTALL ON
  LOG_OUTPUT_ON_FAILURE ON)

# Re-copy (and so rebuild/reinstall) tclhttpd when its files change or are
# added, removed or renamed.
_nscldaq_incorp_track(tclhttpd_incorp download ${PROJECT_SOURCE_DIR}/tclhttpd3.5.1 ALL_FILES)

#---------------------------------------------------------------------------
#  Installation of the staged packages.  This is done first so that the
#  pkg_mkIndex runs in later install rules can load libraries that link
#  against libtcl++.

foreach(_pkg LIBTCL UFMT DDASFMT TCLHTTPD)
  install(DIRECTORY ${NSCLDAQ_${_pkg}_STAGE_PREFIX}/
    DESTINATION .
    USE_SOURCE_PERMISSIONS)
endforeach()

# Source directories, recorded in ${prefix}/VERSION.

ExternalProject_Get_Property(libtcl_incorp SOURCE_DIR)
set(NSCLDAQ_LIBTCL_SOURCE_DIR ${SOURCE_DIR})
ExternalProject_Get_Property(unifiedformat_incorp SOURCE_DIR)
set(NSCLDAQ_UNIFIEDFORMAT_SOURCE_DIR ${SOURCE_DIR})
ExternalProject_Get_Property(ddasformat_incorp SOURCE_DIR)
set(NSCLDAQ_DDASFORMAT_SOURCE_DIR ${SOURCE_DIR})
unset(SOURCE_DIR)
