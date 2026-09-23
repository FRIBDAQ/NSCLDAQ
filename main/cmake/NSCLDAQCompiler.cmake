#
#  NSCLDAQCompiler.cmake
#
#  Global compilation environment - the equivalent of what automake gave
#  every compilation implicitly - and generation of config.h.
#

include_guard(GLOBAL)

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckCXXCompilerFlag)

# AX_CXX_COMPILE_STDCXX_11([noext]) only added -std=c++11 when the
# compiler's default dialect was older than C++11; otherwise the compiler
# default was used (gnu++17 for g++ 11+).  Do the same: no -std flag unless
# the default is too old.  Targets that need more get it through compile
# features (e.g. NSCLDAQ::ROOT requires cxx_std_17).

if(NOT DEFINED CMAKE_CXX_STANDARD AND DEFINED CMAKE_CXX_STANDARD_COMPUTED_DEFAULT)
  if(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT EQUAL 98)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
  endif()
endif()

# libtool builds everything PIC; so do we (convenience libraries included).

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# libtool installed shared libraries executable (0755); CMake on Debian
# would install them 0644.

set(CMAKE_INSTALL_SO_NO_EXE OFF)

# Automake's DEFAULT_INCLUDES: -I. -I$(srcdir) -I$(top_builddir) and
# -DHAVE_CONFIG_H.  AM_CXXFLAGS (-fno-strict-aliasing) is applied per target
# by nscldaq_add_library/nscldaq_add_executable - see NSCLDAQ_AM_CXXFLAGS in
# NSCLDAQHelpers.cmake.

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${PROJECT_BINARY_DIR})
add_compile_definitions(HAVE_CONFIG_H)

# -Wno-error=date-time if the compiler knows about it (NODATEWARN).

check_cxx_compiler_flag(-Wno-error=date-time NSCLDAQ_HAVE_NODATEWARN)
if(NSCLDAQ_HAVE_NODATEWARN)
  set(NODATEWARN "-Wno-error=date-time")
else()
  set(NODATEWARN "")
endif()

# RPATH handling.  Installed binaries find NSCLDAQ libraries and the
# incorporated packages in the install prefix; libraries from outside the
# build tree (ROOT, XIA API, EPICS, CAEN, ...) keep their link directories
# as autotools did with its explicit -Wl,-rpath flags.

set(CMAKE_INSTALL_RPATH
  "${CMAKE_INSTALL_PREFIX}/lib"
  "${CMAKE_INSTALL_PREFIX}/unifiedformat/lib"
  "${CMAKE_INSTALL_PREFIX}/ddasformat/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

#---------------------------------------------------------------------------
#  config.h
#
#  The autoconf checks whose results appear in config.h.  Many of these are
#  historical; they are kept so code testing for them sees the same values.

set(PACKAGE "nscldaq")
set(PACKAGE_NAME "nscldaq")
set(PACKAGE_TARNAME "nscldaq")
set(PACKAGE_VERSION "${NSCLDAQ_VERSION_STRING}")
set(PACKAGE_STRING "nscldaq ${NSCLDAQ_VERSION_STRING}")
set(PACKAGE_BUGREPORT "daqhelp@nscl.msu.edu")
set(PACKAGE_URL "https://github.com/FRIBDAQ/NSCLDAQ")
set(VERSION "${NSCLDAQ_VERSION_STRING}")

foreach(_hdr
    arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stdint.h
    stdlib.h string.h sys/socket.h sys/time.h unistd.h float.h values.h
    sys/wait.h stdbool.h inttypes.h memory.h strings.h sys/stat.h
    sys/types.h stdio.h sys/param.h vfork.h dlfcn.h)
  string(TOUPPER "HAVE_${_hdr}" _var)
  string(REGEX REPLACE "[/.]" "_" _var "${_var}")
  check_include_file(${_hdr} ${_var})
endforeach()

check_include_file_cxx(algorithm HAVE_ALGORITHM)
check_include_file_cxx(zmq.hpp HAVE_ZMQ_HPP)
if(NOT HAVE_ZMQ_HPP)
  message(FATAL_ERROR "the ZeroMQ C++ header zmq.hpp is missing and is required for NSCLDAQ")
endif()

foreach(_fn
    dup2 gethostbyaddr gethostbyname gettimeofday inet_ntoa memmove memset
    regcomp socket strchr strcspn strdup strerror strspn strtol ftruncate
    munmap floor pow select sqrt strstr fork vfork mmap getpagesize)
  string(TOUPPER "HAVE_${_fn}" _var)
  check_function_exists(${_fn} ${_var})
endforeach()
# (floor/pow/sqrt are in libm, which AC_CHECK_FUNCS did not link either, so
#  like configure these checks normally fail.)

# AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK (via AC_FUNC_STAT): true on Linux.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(LSTAT_FOLLOWS_SLASHED_SYMLINK 1)
endif()

set(HAVE_WORKING_FORK ${HAVE_FORK})
set(HAVE_WORKING_VFORK ${HAVE_VFORK})
set(HAVE_MMAP_WORKING ${HAVE_MMAP})

check_type_size("void*" SIZEOF_VOIDP)
if(SIZEOF_VOIDP EQUAL 8)
  set(ADDR64 1)
endif()
check_type_size(ptrdiff_t PTRDIFF_T)
if(HAVE_PTRDIFF_T)
  set(HAVE_PTRDIFF_T 1)
endif()
check_type_size(_Bool _BOOL)
if(HAVE__BOOL)
  set(HAVE__BOOL 1)
endif()
