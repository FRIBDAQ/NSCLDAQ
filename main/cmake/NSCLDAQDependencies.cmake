#
#  NSCLDAQDependencies.cmake
#
#  Locates the external software NSCLDAQ needs.  Each dependency is exposed
#  as an INTERFACE IMPORTED target in the NSCLDAQ:: namespace so a
#  Makefile.am flag pair such as @TCL_FLAGS@ ... @TCL_LDFLAGS@ becomes a
#  single target_link_libraries(... NSCLDAQ::Tcl).
#
#  The old AC_SUBST'd strings (TCL_FLAGS, LIBTCLPLUS_LDFLAGS, ...) are also
#  defined as ordinary variables with their autoconf names.  Those are used
#  only to configure the Makefile skeletons/includes we install for users
#  (FilterIncludes, VMUSBDriverIncludes, ...), never for our own targets.
#
#  Targets:
#    NSCLDAQ::Tcl          Tcl + Tk (TCL_FLAGS/TCL_LDFLAGS)
#    NSCLDAQ::TclOnly      Tcl only (TCL_CPPFLAGS/TCL_LIBS)
#    NSCLDAQ::Threads      -pthread / -lpthread -lrt
#    NSCLDAQ::CppUnit      CPPUNIT_CFLAGS/CPPUNIT_LDFLAGS
#    NSCLDAQ::OpenSSL      OPENSSL_INCLUDES/OPENSSL_LIBS
#    NSCLDAQ::BoostLog     BOOST_CPPFLAGS BOOST_EXTRA_CPPFLAGS/BOOST_LDFLAGS BOOST_LOG_LIB
#    NSCLDAQ::MPI          MPI (only if found; see NSCLDAQ_HAVE_MPI)
#    NSCLDAQ::ZMQ          ZMQ_LDFLAGS
#    NSCLDAQ::SQLite3      SQLITE3_CFLAGS/SQLITE3_LIBS
#    NSCLDAQ::JsonCpp      JSON_CFLAGS/JSON_LIBS
#    NSCLDAQ::YamlCpp      YAML_CFLAGS/YAML_LIBS           (MVLC only)
#    NSCLDAQ::Python       PYTHON(3)_CFLAGS/PYTHON(3)_LIBS (python3-embed)
#    NSCLDAQ::ROOT         ROOT_CFLAGS/ROOT_LDFLAGS (including its -std=c++NN)
#    NSCLDAQ::Pixie        PIXIE_CPPFLAGS/PIXIE_LDFLAGS    (empty unless DDAS)
#    NSCLDAQ::EPICS        EPICS_INCLUDES/EPICS_LDFLAGS    (EPICS tools only)
#    NSCLDAQ::EPICSBin     EPICS_BIN (the -DEPICS_BIN=... definition)
#    NSCLDAQ::USB          USBSWITCHES/USB_LIBS (legacy libusb, USB only)
#    NSCLDAQ::libusb_1_0   libusb_1_0_CFLAGS/libusb_1_0_LIBS (USB only)
#    NSCLDAQ::libusb       libusb_CFLAGS/libusb_LIBS (USB only)
#    NSCLDAQ::CAEN         CAENCCFLAGS/CAENLDFLAGS         (CAEN digitizers)
#    NSCLDAQ::CAENNG       CAENNG_CPPFLAGS/CAENNG_LDFLAGS  (CAEN nextgen)
#    NSCLDAQ::MVLC         MVLC_CPPFLAGS/MVLC_LDFLAGS      (MVLC generator)
#    NSCLDAQ::VMUSBWorkarounds  VMUSB_WORKAROUNDS definitions
#
#  The incorporated packages (NSCLDAQ::tclPlus, NSCLDAQ::Exception,
#  NSCLDAQ::UFMT, NSCLDAQ::DDASFormat) are in NSCLDAQIncorp.cmake.
#

include_guard(GLOBAL)

find_package(PkgConfig REQUIRED)
include(CheckLibraryExists)

# Create an empty global interface target.

function(_nscldaq_interface name)
  add_library(NSCLDAQ::${name} INTERFACE IMPORTED GLOBAL)
endfunction()

# Resolve a list of linker flags (-L... -l... other) to full library paths
# where possible so CMake can manage build/install rpaths for them.
#   _nscldaq_resolve_libs(<out-var> <flags...>)

function(_nscldaq_resolve_libs out)
  set(_dirs "")
  set(_result "")
  foreach(_f IN LISTS ARGN)
    if(_f MATCHES "^-L(.+)")
      list(APPEND _dirs "${CMAKE_MATCH_1}")
    endif()
  endforeach()
  foreach(_f IN LISTS ARGN)
    if(_f MATCHES "^-L")
      continue()
    elseif(_f MATCHES "^-l(.+)")
      set(_name "${CMAKE_MATCH_1}")
      string(MAKE_C_IDENTIFIER "NSCLDAQ_LIB_${_name}_${_dirs}" _cache)
      if(_dirs)
        find_library(${_cache} NAMES ${_name} PATHS ${_dirs} NO_DEFAULT_PATH)
      endif()
      if(NOT ${_cache})
        find_library(${_cache} NAMES ${_name})
      endif()
      if(${_cache})
        list(APPEND _result "${${_cache}}")
      else()
        list(APPEND _result "${_name}")
      endif()
    else()
      list(APPEND _result "${_f}")
    endif()
  endforeach()
  set(${out} "${_result}" PARENT_SCOPE)
endfunction()

# Turn compiler flags into usage requirements on an interface target.
#   _nscldaq_cflags_to_target(<target> <flags...>)

function(_nscldaq_cflags_to_target tgt)
  foreach(_f IN LISTS ARGN)
    if(_f MATCHES "^-I(.+)")
      target_include_directories(${tgt} SYSTEM INTERFACE "${CMAKE_MATCH_1}")
    elseif(_f MATCHES "^-D(.+)")
      target_compile_definitions(${tgt} INTERFACE "${CMAKE_MATCH_1}")
    elseif(_f MATCHES "^-std=")
      # Keep the exact dialect (e.g. root-config's -std=c++17, not gnu++17).
      target_compile_options(${tgt} INTERFACE "${_f}")
    elseif(_f STREQUAL "-pthread")
      target_link_libraries(${tgt} INTERFACE Threads::Threads)
    else()
      target_compile_options(${tgt} INTERFACE "${_f}")
    endif()
  endforeach()
endfunction()

#---------------------------------------------------------------------------
#  Programs.

find_package(Git REQUIRED)
set(GIT "${GIT_EXECUTABLE}")

find_program(PANDOC pandoc)
if(NOT PANDOC)
  message(FATAL_ERROR "pandoc is required to produce PDF output from the logbook utilities")
endif()

if(WITH_GENGETOPT_PATH)
  set(GENGETOPT "${WITH_GENGETOPT_PATH}")
else()
  find_program(GENGETOPT gengetopt)
  if(NOT GENGETOPT)
    message(FATAL_ERROR "gnu gengetopt tested for but not found")
  endif()
endif()

find_package(SWIG 1.3)
if(NOT SWIG_FOUND)
  message(FATAL_ERROR "swig is required to build NSCLDAQ")
endif()
set(SWIG "${SWIG_EXECUTABLE}")

# @CC@/@CXX@ for configured files are provided by nscldaq_configure_file()
# only: a global variable named CXX breaks if(CXX IN_LIST ...) tests in
# CMake's own modules (FindMPI in CMake 3.18 silently skips the component).

#---------------------------------------------------------------------------
#  Threads: THREADCXX_FLAGS=-pthread, THREADLD_FLAGS=-lpthread -lrt

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_library(NSCLDAQ_RT_LIBRARY rt)

_nscldaq_interface(Threads)
target_compile_options(NSCLDAQ::Threads INTERFACE -pthread)
target_link_libraries(NSCLDAQ::Threads INTERFACE Threads::Threads)
if(NSCLDAQ_RT_LIBRARY)
  target_link_libraries(NSCLDAQ::Threads INTERFACE ${NSCLDAQ_RT_LIBRARY})
endif()
set(THREADCXX_FLAGS "-pthread")
set(THREADC_FLAGS "-pthread")
set(THREADLD_FLAGS "-lpthread -lrt")

#---------------------------------------------------------------------------
#  X11/Xt - configure did AC_CHECK_LIB([X11], [XSetWindowBackground]) and
#  AC_CHECK_LIB([Xt], [XtManage]), putting each library that passed into LIBS
#  so everything linked it.  (XtManage does not exist, so in practice Xt is
#  never linked - reproduce the checks, not just the library search.)

find_library(NSCLDAQ_X11_LIBRARY X11)
find_library(NSCLDAQ_XT_LIBRARY Xt)
if(NSCLDAQ_X11_LIBRARY)
  check_library_exists(${NSCLDAQ_X11_LIBRARY} XSetWindowBackground ""
    HAVE_LIBX11)
endif()
if(NSCLDAQ_XT_LIBRARY)
  check_library_exists(${NSCLDAQ_XT_LIBRARY} XtManage "" HAVE_LIBXT)
endif()
if(HAVE_LIBX11)
  link_libraries(${NSCLDAQ_X11_LIBRARY})
endif()
if(HAVE_LIBXT)
  link_libraries(${NSCLDAQ_XT_LIBRARY})
endif()

#---------------------------------------------------------------------------
#  Tcl/Tk - the same discovery m4/tcl.m4 (AX_TCL) did: find tclsh, locate
#  tclConfig.sh/tkConfig.sh (or take them from WITH_TCLCONFIG/WITH_TKCONFIG)
#  and use the flags they define.

set(WITH_TCLCONFIG "" CACHE FILEPATH "Use tcl defined by provided tclConfig.sh")
set(WITH_TKCONFIG "" CACHE FILEPATH "Use tk defined by provided tkConfig.sh")

# Source a *Config.sh and return the requested variables as <prefix><VAR>.

function(_nscldaq_source_config file prefix)
  set(_script "LIB_RUNTIME_DIR=unused; . \"${file}\"")
  foreach(_v IN LISTS ARGN)
    string(APPEND _script "; printf '%s=%s\\n' ${_v} \"\$${_v}\"")
  endforeach()
  execute_process(COMMAND sh -c "${_script}"
    OUTPUT_VARIABLE _out RESULT_VARIABLE _status)
  if(NOT _status EQUAL 0)
    message(FATAL_ERROR "Could not source ${file}")
  endif()
  string(REPLACE ";" "\;" _out "${_out}")
  string(REPLACE "\n" ";" _lines "${_out}")
  foreach(_l IN LISTS _lines)
    if(_l MATCHES "^([A-Z_]+)=(.*)$")
      set(${prefix}${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" PARENT_SCOPE)
    endif()
  endforeach()
endfunction()

find_program(TCLSH_CMD NAMES tclsh8.6 tclsh8.5 tclsh8.4 tclsh8.3 tclsh8.2
  tclsh8.1 tclsh8.0 tclsh DOC "Tcl shell")
if(NOT TCLSH_CMD)
  message(FATAL_ERROR "Can't find Tcl installation (tclsh).")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E echo "puts \$tcl_version"
  COMMAND ${TCLSH_CMD}
  OUTPUT_VARIABLE _tclsh_version OUTPUT_STRIP_TRAILING_WHITESPACE)

if(WITH_TCLCONFIG)
  set(NSCLDAQ_TCLCONFIG "${WITH_TCLCONFIG}")
else()
  execute_process(COMMAND ${CMAKE_COMMAND} -E echo "puts \$tcl_library"
    COMMAND ${TCLSH_CMD}
    OUTPUT_VARIABLE _tcl_library OUTPUT_STRIP_TRAILING_WHITESPACE)
  get_filename_component(_tclClaims "${_tcl_library}" DIRECTORY)
  set(NSCLDAQ_TCLCONFIG "")
  foreach(_dir "${_tclClaims}" "/usr/lib/tcl${_tclsh_version}")
    if(EXISTS "${_dir}/tclConfig.sh")
      set(NSCLDAQ_TCLCONFIG "${_dir}/tclConfig.sh")
      break()
    endif()
  endforeach()
  if(NOT NSCLDAQ_TCLCONFIG)
    message(FATAL_ERROR "Can't find tclConfig.sh you'll need to use -DWITH_TCLCONFIG to tell me where it is")
  endif()
endif()
get_filename_component(NSCLDAQ_TCL_CONFIG_DIR "${NSCLDAQ_TCLCONFIG}" DIRECTORY)
message(STATUS "Using Tcl configuration ${NSCLDAQ_TCLCONFIG}")

_nscldaq_source_config("${NSCLDAQ_TCLCONFIG}" _tclcfg_
  TCL_VERSION TCL_PREFIX TCL_EXEC_PREFIX TCL_LD_SEARCH_FLAGS TCL_LIB_SPEC
  TCL_LIBS TCL_DEFS)
set(TCL_VERSION "${_tclcfg_TCL_VERSION}")
set(TCL_DEFS "${_tclcfg_TCL_DEFS}")
set(TCLSH "${TCLSH_CMD}")

foreach(_inc "${_tclcfg_TCL_PREFIX}/include/tcl${TCL_VERSION}"
    "${_tclcfg_TCL_PREFIX}/include/tcl" "${_tclcfg_TCL_PREFIX}/include")
  if(EXISTS "${_inc}/tcl.h")
    set(NSCLDAQ_TCL_INCLUDE_DIR "${_inc}")
    break()
  endif()
endforeach()
if(NOT NSCLDAQ_TCL_INCLUDE_DIR)
  message(FATAL_ERROR "can't find Tcl includes (tcl.h) under ${_tclcfg_TCL_PREFIX}/include")
endif()
set(TCL_INCLUDE_PATH "${NSCLDAQ_TCL_INCLUDE_DIR}")
set(TCL_CPPFLAGS "-I${NSCLDAQ_TCL_INCLUDE_DIR}")
set(TCL_LIBS "${_tclcfg_TCL_LD_SEARCH_FLAGS} ${_tclcfg_TCL_LIB_SPEC} ${_tclcfg_TCL_LIBS}")

if(WITH_TKCONFIG)
  set(NSCLDAQ_TKCONFIG "${WITH_TKCONFIG}")
else()
  set(NSCLDAQ_TKCONFIG "")
  foreach(_guess "${NSCLDAQ_TCL_CONFIG_DIR}/../tk${TCL_VERSION}"
      "${NSCLDAQ_TCL_CONFIG_DIR}/../tk"
      "${NSCLDAQ_TCL_CONFIG_DIR}/../../tk${TCL_VERSION}/lib"
      "${NSCLDAQ_TCL_CONFIG_DIR}/../tk${TCL_VERSION}/lib")
    if(EXISTS "${_guess}/tkConfig.sh")
      get_filename_component(NSCLDAQ_TKCONFIG "${_guess}/tkConfig.sh" ABSOLUTE)
      break()
    endif()
  endforeach()
  if(NOT NSCLDAQ_TKCONFIG)
    message(FATAL_ERROR "Can't find tkConfig.sh, use -DWITH_TKCONFIG to help me out")
  endif()
endif()
message(STATUS "Using Tk configuration ${NSCLDAQ_TKCONFIG}")

_nscldaq_source_config("${NSCLDAQ_TKCONFIG}" _tkcfg_
  TK_LD_SEARCH_FLAGS TK_LIB_SPEC TK_LIBS TK_INCLUDE_SPEC TK_DEFS)
set(TK_DEFS "${_tkcfg_TK_DEFS}")
set(TK_LIBS "${_tkcfg_TK_LD_SEARCH_FLAGS} ${_tkcfg_TK_LIB_SPEC} ${_tkcfg_TK_LIBS}")
set(TK_CPPFLAGS "${_tkcfg_TK_INCLUDE_SPEC}")
set(WISH "")

set(TCL_FLAGS "${TCL_CPPFLAGS} ${TK_CPPFLAGS}")
# configure.ac dropped -lieee from the Tcl libraries.
set(TCL_LDFLAGS "${TK_LIBS}")
separate_arguments(_tcl_libs_words UNIX_COMMAND "${TCL_LIBS}")
foreach(_w IN LISTS _tcl_libs_words)
  if(NOT _w STREQUAL "-lieee")
    string(APPEND TCL_LDFLAGS " ${_w}")
  endif()
endforeach()

# The targets use exactly those flags.

_nscldaq_interface(TclOnly)
target_include_directories(NSCLDAQ::TclOnly SYSTEM INTERFACE ${NSCLDAQ_TCL_INCLUDE_DIR})
separate_arguments(_w UNIX_COMMAND "${TCL_LIBS}")
list(FILTER _w EXCLUDE REGEX "^-lieee$")
_nscldaq_resolve_libs(_w ${_w})
target_link_libraries(NSCLDAQ::TclOnly INTERFACE ${_w})

_nscldaq_interface(Tcl)
separate_arguments(_w UNIX_COMMAND "${TK_CPPFLAGS}")
_nscldaq_cflags_to_target(NSCLDAQ::Tcl ${_w})
separate_arguments(_w UNIX_COMMAND "${TK_LIBS}")
_nscldaq_resolve_libs(_w ${_w})
target_link_libraries(NSCLDAQ::Tcl INTERFACE ${_w} NSCLDAQ::TclOnly)

#---------------------------------------------------------------------------
#  CppUnit (required: configure required it).

pkg_check_modules(CPPUNIT REQUIRED IMPORTED_TARGET GLOBAL cppunit)
_nscldaq_interface(CppUnit)
target_link_libraries(NSCLDAQ::CppUnit INTERFACE PkgConfig::CPPUNIT)

#---------------------------------------------------------------------------
#  OpenSSL (eventlog checksums).

find_package(OpenSSL)
if(NOT OPENSSL_FOUND)
  message(FATAL_ERROR "cannot find OpenSSL installation required by NSCLDAQ")
endif()
_nscldaq_interface(OpenSSL)
target_link_libraries(NSCLDAQ::OpenSSL INTERFACE OpenSSL::SSL OpenSSL::Crypto)

#---------------------------------------------------------------------------
#  Boost/Boost::log - optional; logging turns off without it.

find_package(Boost QUIET)
_nscldaq_interface(BoostLog)
if(Boost_FOUND)
  set(HAVE_BOOST 1)
  target_link_libraries(NSCLDAQ::BoostLog INTERFACE Boost::headers)
  target_compile_definitions(NSCLDAQ::BoostLog INTERFACE BOOST_ALL_DYN_LINK)
  find_package(Boost QUIET COMPONENTS log)
  if(Boost_LOG_FOUND)
    set(HAVE_BOOST_LOG 1)
    target_link_libraries(NSCLDAQ::BoostLog INTERFACE Boost::log)
  else()
    message(WARNING "Could not find a version of boost::log - logging disabled")
  endif()
endif()

#---------------------------------------------------------------------------
#  MPI (optional; enables the MPI parts of swtrigger).

find_package(MPI QUIET COMPONENTS CXX)
_nscldaq_interface(MPI)
if(MPI_CXX_FOUND)
  set(HAVE_MPI 1)
  set(NSCLDAQ_HAVE_MPI ON)
  target_link_libraries(NSCLDAQ::MPI INTERFACE MPI::MPI_CXX)
else()
  set(NSCLDAQ_HAVE_MPI OFF)
endif()

#---------------------------------------------------------------------------
#  ZeroMQ.

find_library(ZMQ_LIBRARY zmq)
if(ZMQ_LIBRARY)
  check_library_exists(${ZMQ_LIBRARY} zmq_ctx_new "" NSCLDAQ_ZMQ_HAS_CTX_NEW)
endif()
if(NOT ZMQ_LIBRARY OR NOT NSCLDAQ_ZMQ_HAS_CTX_NEW)
  message(FATAL_ERROR "the ZeroMQ library was not found and is now required for NSCLDAQ")
endif()
_nscldaq_interface(ZMQ)
target_link_libraries(NSCLDAQ::ZMQ INTERFACE ${ZMQ_LIBRARY})
set(ZMQ_LDFLAGS "-lzmq")

#---------------------------------------------------------------------------
#  pkg-config based packages.

pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET GLOBAL sqlite3)
_nscldaq_interface(SQLite3)
target_link_libraries(NSCLDAQ::SQLite3 INTERFACE PkgConfig::SQLITE3)

pkg_check_modules(JSON REQUIRED IMPORTED_TARGET GLOBAL jsoncpp)
_nscldaq_interface(JsonCpp)
target_link_libraries(NSCLDAQ::JsonCpp INTERFACE PkgConfig::JSON)

#---------------------------------------------------------------------------
#  Python 3 (+PyQt5).

find_package(Python3 3.0 COMPONENTS Interpreter)
if(NOT Python3_Interpreter_FOUND)
  message(FATAL_ERROR "python 3.0 or higher is needed for NSCLDAQ")
endif()
set(PYTHON "${Python3_EXECUTABLE}")

execute_process(COMMAND ${Python3_EXECUTABLE} -c "from PyQt5 import *"
  RESULT_VARIABLE _pyqt_status OUTPUT_QUIET ERROR_QUIET)
if(NOT _pyqt_status EQUAL 0)
  message(FATAL_ERROR "NSCLDAQ requires the PyQt5 bindings to Qt5")
endif()

# The same interpreter's headers/library, handed to the incorporated CMake
# packages (unifiedformat builds python bindings) so they use this python
# too.  It also lets older CMake FindPython modules that don't know a newer
# python version (e.g. CMake 3.18 with python 3.11) accept it.

execute_process(COMMAND ${Python3_EXECUTABLE} -c
  "import sysconfig,os; print(sysconfig.get_paths()['include']); print(os.path.join(sysconfig.get_config_var('LIBDIR') or '', sysconfig.get_config_var('LDLIBRARY') or ''))"
  OUTPUT_VARIABLE _py_paths OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REPLACE "\n" ";" _py_paths "${_py_paths}")
list(GET _py_paths 0 NSCLDAQ_PYTHON3_INCLUDE_DIR)
list(GET _py_paths 1 NSCLDAQ_PYTHON3_LIBRARY)

pkg_check_modules(PYTHON3 REQUIRED python3)
pkg_check_modules(PYTHON_EMBED IMPORTED_TARGET GLOBAL python3-embed)
_nscldaq_interface(Python)
if(PYTHON_EMBED_FOUND)
  target_link_libraries(NSCLDAQ::Python INTERFACE PkgConfig::PYTHON_EMBED)
else()
  pkg_check_modules(PYTHON_NOEMBED REQUIRED IMPORTED_TARGET GLOBAL python3)
  target_link_libraries(NSCLDAQ::Python INTERFACE PkgConfig::PYTHON_NOEMBED)
endif()

#---------------------------------------------------------------------------
#  ROOT - configure insisted on it even without DDAS.

if(NOT WITH_ROOTSYS)
  message(FATAL_ERROR "use -DWITH_ROOTSYS=<dir> (or set ROOTSYS) to specify where ROOT is installed")
endif()
if(NOT IS_DIRECTORY "${WITH_ROOTSYS}")
  message(FATAL_ERROR "the specified ROOTSYS directory ${WITH_ROOTSYS} does not exist!")
endif()
set(ROOTSYS "${WITH_ROOTSYS}")
set(ROOT_CONFIG "${ROOTSYS}/bin/root-config")

foreach(_opt cflags glibs ldflags libdir)
  execute_process(COMMAND ${ROOT_CONFIG} --${_opt}
    OUTPUT_VARIABLE _root_${_opt} OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _status)
  if(NOT _status EQUAL 0)
    message(FATAL_ERROR "${ROOT_CONFIG} --${_opt} failed")
  endif()
endforeach()
set(ROOT_CFLAGS "${_root_cflags}")
set(ROOT_LDFLAGS "${_root_glibs} ${_root_ldflags} -Wl,-rpath=${_root_libdir}")

_nscldaq_interface(ROOT)
separate_arguments(_root_cflags_list UNIX_COMMAND "${_root_cflags}")
_nscldaq_cflags_to_target(NSCLDAQ::ROOT ${_root_cflags_list})
separate_arguments(_root_libs_list UNIX_COMMAND "${_root_glibs} ${_root_ldflags}")
list(FILTER _root_libs_list EXCLUDE REGEX "^-m64$|^-pthread$")
_nscldaq_resolve_libs(_root_libs_list ${_root_libs_list})
target_link_libraries(NSCLDAQ::ROOT INTERFACE ${_root_libs_list})
set(NSCLDAQ_ROOTCLING "${ROOTSYS}/bin/rootcling")

#---------------------------------------------------------------------------
#  DDAS / XIA API.  NSCLDAQ::Pixie is used everywhere PIXIE_CPPFLAGS was and
#  is empty unless DDAS is enabled.

_nscldaq_interface(Pixie)
set(PIXIE_CPPFLAGS "")
set(PIXIE_LDFLAGS "")
if(ENABLE_DDAS)
  set(USING_DDAS 1)
  find_path(NSCLDAQ_PIXIE_INCLUDE_DIR pixie16/pixie16.h
    PATHS ${WITH_XIAAPIDIR}/include NO_DEFAULT_PATH)
  if(NOT NSCLDAQ_PIXIE_INCLUDE_DIR)
    message(FATAL_ERROR "cannot find pixie16/pixie16.h in ${WITH_XIAAPIDIR}, "
      "use -DWITH_XIAAPIDIR to point to the correct one (version 3+)")
  endif()
  find_library(NSCLDAQ_PIXIE_LIBRARY Pixie16Api
    PATHS ${WITH_XIAAPIDIR}/lib NO_DEFAULT_PATH)
  if(NOT NSCLDAQ_PIXIE_LIBRARY)
    message(FATAL_ERROR "cannot find libPixie16Api in ${WITH_XIAAPIDIR}/lib, "
      "use -DWITH_XIAAPIDIR to point to the correct one (version 3+)")
  endif()
  target_include_directories(NSCLDAQ::Pixie SYSTEM INTERFACE ${NSCLDAQ_PIXIE_INCLUDE_DIR})
  target_link_libraries(NSCLDAQ::Pixie INTERFACE ${NSCLDAQ_PIXIE_LIBRARY} m)
  set(PIXIE_API_DIR "${WITH_XIAAPIDIR}")
  set(PIXIE_CPPFLAGS "-I${WITH_XIAAPIDIR}/include")
  set(PIXIE_LDFLAGS "-L${WITH_XIAAPIDIR}/lib -lPixie16Api -lm -Wl,-rpath=${WITH_XIAAPIDIR}/lib")
  set(firmwaredir "${WITH_FIRMWAREDIR}")
endif()

#---------------------------------------------------------------------------
#  EPICS.

_nscldaq_interface(EPICS)
_nscldaq_interface(EPICSBin)
if(ENABLE_EPICS_TOOLS)
  set(_epicsroot "${WITH_EPICS_ROOTDIR}")
  if(NOT _epicsroot)
    foreach(_dir /usr/local/epics /opt/epics /usr/lib/epics)
      if(EXISTS ${_dir}/include/cadef.h)
        set(_epicsroot ${_dir})
        break()
      endif()
    endforeach()
  endif()
  if(NOT _epicsroot)
    message(FATAL_ERROR "Cannot locate the EPICS root directory, help me out with -DWITH_EPICS_ROOTDIR")
  endif()
  string(TOLOWER "${CMAKE_SYSTEM_NAME}" _kernel)
  set(_bits "${CMAKE_SYSTEM_PROCESSOR}")
  if(_bits MATCHES "^i[36]86$")
    set(_bits x86)
  endif()
  set(NSCLDAQ_EPICS_ARCH "${_kernel}-${_bits}")
  set(NSCLDAQ_EPICS_ROOT "${_epicsroot}")
  find_library(NSCLDAQ_EPICS_CA_LIBRARY ca
    PATHS ${_epicsroot}/lib/${NSCLDAQ_EPICS_ARCH} NO_DEFAULT_PATH)
  if(NOT NSCLDAQ_EPICS_CA_LIBRARY)
    message(FATAL_ERROR "Can't find libca in ${_epicsroot}/lib/${NSCLDAQ_EPICS_ARCH}")
  endif()
  find_library(NSCLDAQ_EPICS_COM_LIBRARY Com
    PATHS ${_epicsroot}/lib/${NSCLDAQ_EPICS_ARCH} NO_DEFAULT_PATH)
  target_include_directories(NSCLDAQ::EPICS SYSTEM INTERFACE
    ${_epicsroot}/include ${_epicsroot}/include/os/Linux
    ${_epicsroot}/include/compiler/gcc)
  target_link_libraries(NSCLDAQ::EPICS INTERFACE ${NSCLDAQ_EPICS_CA_LIBRARY})
  set(NSCLDAQ_EPICS_BIN_DIR "${_epicsroot}/bin/${NSCLDAQ_EPICS_ARCH}")
  target_compile_definitions(NSCLDAQ::EPICSBin INTERFACE
    "EPICS_BIN=\"${NSCLDAQ_EPICS_BIN_DIR}\"")
  set(EPICS_INCLUDES "-I${_epicsroot}/include -I${_epicsroot}/include/os/Linux -I${_epicsroot}/include/compiler/gcc")
  set(EPICS_LDFLAGS "-L${_epicsroot}/lib/${NSCLDAQ_EPICS_ARCH} -lca -Wl,\"-rpath=${_epicsroot}/lib/${NSCLDAQ_EPICS_ARCH}\"")
endif()

#---------------------------------------------------------------------------
#  USB (VM-USB/CC-USB).

_nscldaq_interface(USB)
_nscldaq_interface(libusb_1_0)
_nscldaq_interface(libusb)
if(ENABLE_USB)
  # AX_LIBUSB: legacy libusb (usb.h, libusb).
  if(WITH_USB_HEADERDIR)
    set(_usbhdrdirs ${WITH_USB_HEADERDIR})
  else()
    set(_usbhdrdirs /usr/include /usr/local/include)
  endif()
  find_path(NSCLDAQ_USB_INCLUDE_DIR usb.h PATHS ${_usbhdrdirs} NO_DEFAULT_PATH)
  if(NOT NSCLDAQ_USB_INCLUDE_DIR)
    message(FATAL_ERROR "Can't find usb.h install libusb development or try "
      "using -DWITH_USB_HEADERDIR to help me find it")
  endif()
  if(WITH_USB_LIBDIR)
    set(_usblibdirs ${WITH_USB_LIBDIR})
  else()
    set(_usblibdirs /lib /usr/lib /usr/local/lib
      /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE})
  endif()
  find_library(NSCLDAQ_USB_LIBRARY usb PATHS ${_usblibdirs} NO_DEFAULT_PATH)
  if(NOT NSCLDAQ_USB_LIBRARY)
    message(FATAL_ERROR "Can't find libusb install libusb or try using "
      "-DWITH_USB_LIBDIR to tell me where to find it")
  endif()
  get_filename_component(_usblibdir ${NSCLDAQ_USB_LIBRARY} DIRECTORY)
  target_include_directories(NSCLDAQ::USB SYSTEM INTERFACE ${NSCLDAQ_USB_INCLUDE_DIR})
  target_link_libraries(NSCLDAQ::USB INTERFACE ${NSCLDAQ_USB_LIBRARY})
  set(USBSWITCHES "-I${NSCLDAQ_USB_INCLUDE_DIR}")
  set(USB_LIBS "-L${_usblibdir} -lusb")

  pkg_check_modules(libusb_1_0 REQUIRED IMPORTED_TARGET GLOBAL libusb-1.0)
  target_link_libraries(NSCLDAQ::libusb_1_0 INTERFACE PkgConfig::libusb_1_0)
  pkg_check_modules(libusb REQUIRED IMPORTED_TARGET GLOBAL libusb)
  target_link_libraries(NSCLDAQ::libusb INTERFACE PkgConfig::libusb)
endif()

# VMUSB firmware workarounds (VMUSB_WORKAROUNDS).

_nscldaq_interface(VMUSBWorkarounds)
set(VMUSB_WORKAROUNDS "")
if(ENABLE_VMUSB_PROMPT_BUSY)
  target_compile_definitions(NSCLDAQ::VMUSBWorkarounds INTERFACE VMUSB_PROMPT_BUSY_WORKAROUND)
  string(APPEND VMUSB_WORKAROUNDS " -DVMUSB_PROMPT_BUSY_WORKAROUND")
endif()
if(ENABLE_VMUSB_END_EVENT)
  target_compile_definitions(NSCLDAQ::VMUSBWorkarounds INTERFACE VMUSB_END_OF_EVENT_WORKAROUND)
  string(APPEND VMUSB_WORKAROUNDS " -DVMUSB_END_OF_EVENT_WORKAROUND")
endif()
if(ENABLE_VMUSB_IDLE_BUSY)
  target_compile_definitions(NSCLDAQ::VMUSBWorkarounds INTERFACE VMUSB_IDLE_BUSY_WORKAROUND)
  string(APPEND VMUSB_WORKAROUNDS " -DVMUSB_IDLE_BUSY_WORKAROUND")
endif()

#---------------------------------------------------------------------------
#  CAEN first generation digitizers.

_nscldaq_interface(CAEN)
set(CAENCCFLAGS "")
set(CAENLDFLAGS "")
if(ENABLE_CAEN_DIGITIZER_SUPPORT)
  set(_caenroot "${WITH_CAEN_DIGITIZER_LIBROOT}")
  set(_caenlibs "")
  foreach(_lib CAENDigitizer CAENVME CAENComm)
    find_library(NSCLDAQ_${_lib}_LIBRARY ${_lib} PATHS ${_caenroot}/lib NO_DEFAULT_PATH)
    if(NOT NSCLDAQ_${_lib}_LIBRARY)
      message(FATAL_ERROR "Can't find lib${_lib} in ${_caenroot}/lib")
    endif()
    list(APPEND _caenlibs ${NSCLDAQ_${_lib}_LIBRARY})
  endforeach()
  target_include_directories(NSCLDAQ::CAEN SYSTEM INTERFACE ${_caenroot}/include)
  target_link_libraries(NSCLDAQ::CAEN INTERFACE ${_caenlibs})
  set(CAENCCFLAGS "-I${_caenroot}/include")
  set(CAENLDFLAGS "-L${_caenroot}/lib -lCAENDigitizer -lCAENVME -lCAENComm -Wl,-rpath=${_caenroot}/lib")
endif()

#---------------------------------------------------------------------------
#  CAEN nextgen digitizers.

_nscldaq_interface(CAENNG)
if(ENABLE_CAEN_NEXTGEN)
  find_path(NSCLDAQ_CAEN_FELIB_INCLUDE_DIR CAEN_FELib.h PATHS /usr/local/include)
  find_library(NSCLDAQ_CAEN_FELIB_LIBRARY CAEN_FELib PATHS /usr/local/lib)
  find_library(NSCLDAQ_CAEN_DIG2_LIBRARY CAEN_Dig2 PATHS /usr/local/lib)
  if(NOT NSCLDAQ_CAEN_FELIB_INCLUDE_DIR)
    message(FATAL_ERROR "cannot find /usr/local/include/CAEN_FELib.h - install CAEN Nextgen digitizer support libraries")
  endif()
  if(NOT NSCLDAQ_CAEN_FELIB_LIBRARY OR NOT NSCLDAQ_CAEN_DIG2_LIBRARY)
    message(FATAL_ERROR "cannot find libCAEN_FELib.so/libCAEN_Dig2.so - install CAEN nextgen digitizer support libraries")
  endif()
  target_include_directories(NSCLDAQ::CAENNG SYSTEM INTERFACE ${NSCLDAQ_CAEN_FELIB_INCLUDE_DIR})
  target_link_libraries(NSCLDAQ::CAENNG INTERFACE
    ${NSCLDAQ_CAEN_FELIB_LIBRARY} ${NSCLDAQ_CAEN_DIG2_LIBRARY})
endif()

#---------------------------------------------------------------------------
#  MVLC stack generator.

_nscldaq_interface(MVLC)
_nscldaq_interface(YamlCpp)
set(MVLC_ROOT "")
set(MVLC_INC "")
set(MVLC_LIB "")
set(MVLC_BIN "")
if(ENABLE_MVLC_STACK_GENERATOR)
  pkg_check_modules(YAML REQUIRED IMPORTED_TARGET GLOBAL yaml-cpp)
  target_link_libraries(NSCLDAQ::YamlCpp INTERFACE PkgConfig::YAML)

  set(MVLC_ROOT "${WITH_MVLCDIR}")
  set(MVLC_INC "${WITH_MVLCDIR}/include")
  set(MVLC_LIB "${WITH_MVLCDIR}/lib")
  set(MVLC_BIN "${WITH_MVLCDIR}/bin")
  set(MVLC_CPPFLAGS "-I${MVLC_INC} -I${MVLC_INC}/mesytec-mvlc")
  set(MVLC_LDFLAGS "-L${MVLC_LIB} -lmesytec-mvlc -Wl,-rpath=${MVLC_LIB}")
  find_library(NSCLDAQ_MVLC_LIBRARY mesytec-mvlc PATHS ${MVLC_LIB} NO_DEFAULT_PATH)
  if(NOT NSCLDAQ_MVLC_LIBRARY)
    message(FATAL_ERROR "Can't find libmesytec-mvlc in ${MVLC_LIB}")
  endif()
  target_include_directories(NSCLDAQ::MVLC SYSTEM INTERFACE ${MVLC_INC} ${MVLC_INC}/mesytec-mvlc)
  target_link_libraries(NSCLDAQ::MVLC INTERFACE ${NSCLDAQ_MVLC_LIBRARY})
endif()

#---------------------------------------------------------------------------
#  Documentation tools.

if(ENABLE_DOCS)
  foreach(_tool docbook2pdf docbook2html xmlto mandb)
    string(TOUPPER ${_tool} _var)
    find_program(NSCLDAQ_${_var} ${_tool})
    if(NOT NSCLDAQ_${_var})
      message(FATAL_ERROR "${_tool} is required to build documentation but was not found")
    endif()
  endforeach()
  set(DOCBOOK2PDF "${NSCLDAQ_DOCBOOK2PDF}")
  set(DOCBOOK2HTML "${NSCLDAQ_DOCBOOK2HTML}")
  set(DOCBOOK2MAN "${NSCLDAQ_XMLTO};man")
  set(MANDB "${NSCLDAQ_MANDB}")
endif()

if(ENABLE_DDAS_DOCS)
  find_package(Doxygen COMPONENTS dot)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is required to build the DDAS documentation but was not found")
  endif()
  if(NOT DOXYGEN_DOT_EXECUTABLE)
    message(FATAL_ERROR "dot is required by Doxygen to build the DDAS documentation but was not found")
  endif()
  set(DOXYGEN "${DOXYGEN_EXECUTABLE}")
endif()

#---------------------------------------------------------------------------
#  Rust toolchain.

if(ENABLE_RUST_TOOLS)
  find_program(RUST_CARGO cargo PATHS /usr/cargo/bin $ENV{HOME}/.cargo/bin)
  if(NOT RUST_CARGO)
    message(FATAL_ERROR "You need to install the rust toolchain to ENABLE_RUST_TOOLS "
      "(looked in /usr/cargo/bin, ~/.cargo/bin and PATH)")
  endif()
  get_filename_component(RUST_TOOL_DIR ${RUST_CARGO} DIRECTORY)
endif()

#---------------------------------------------------------------------------
#  config.h now that everything it records is known.

configure_file(${PROJECT_SOURCE_DIR}/cmake/config.h.cmake.in
  ${PROJECT_BINARY_DIR}/config.h)
