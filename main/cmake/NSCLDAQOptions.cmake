#
#  NSCLDAQOptions.cmake
#
#  Cache variables that correspond to the old configure options.  Naming is
#  mechanical so existing build recipes are easy to translate:
#
#    --enable-foo-bar[=yes|no]   ->  -DENABLE_FOO_BAR=ON|OFF
#    --disable-foo-bar           ->  -DENABLE_FOO_BAR=OFF
#    --with-foo-bar=value        ->  -DWITH_FOO_BAR=value
#

include_guard(GLOBAL)

# The default build type and its flags are set in the top level
# CMakeLists.txt before project() - see there.

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message(STATUS "CMAKE_INSTALL_PREFIX not set; using ${CMAKE_INSTALL_PREFIX}")
endif()

#---------------------------------------------------------------------------
#  Incorporated packages.

set(WITH_INCORP_BUILD_CORES 1 CACHE STRING
  "Number of cores used to build incorporated packages")
set(NSCLDAQ_LIBTCLPLUS_TAG "libtclplus-v4.3-004" CACHE STRING
  "libtcl++ git tag used when main/libtcl is not already present")
set(NSCLDAQ_UNIFIEDFORMAT_TAG "2.4-001" CACHE STRING
  "UnifiedFormat git tag used when main/unifiedformat is not already present")
set(NSCLDAQ_DDASFORMAT_TAG "2.1-001" CACHE STRING
  "DDASFormat git tag used when main/ddasformat is not already present")

#---------------------------------------------------------------------------
#  Optional subsystems (all default off as in configure.ac).

option(ENABLE_CAEN_DIGITIZER_SUPPORT "Build support for CAEN Digitizers" OFF)
set(WITH_CAEN_DIGITIZER_LIBROOT "" CACHE PATH
  "Root directory for CAEN Digitizer/VME/Comm library installation")

option(ENABLE_MVLC_STACK_GENERATOR
  "Build support for generating MVLC fribdaq-readout config files (needs WITH_MVLCDIR)" OFF)
set(WITH_MVLCDIR "" CACHE PATH "Root directory of the MVLC driver installation")

option(ENABLE_CAEN_NEXTGEN "Enable support for CAEN Nextgen digitizers" OFF)

option(ENABLE_DOCS "Build the documentation" OFF)
option(ENABLE_DDAS_DOCS "Build the DDAS documentation" OFF)

option(ENABLE_DDAS "Build the DDAS support software" OFF)
set(WITH_XIAAPIDIR "" CACHE PATH
  "Path to the XIA API installation. Required if DDAS support is enabled")
set(WITH_FIRMWAREDIR "" CACHE PATH
  "Path to the DDAS firmware directory tree. Required if DDAS support is enabled")
set(WITH_ROOTSYS "$ENV{ROOTSYS}" CACHE PATH "Path to the ROOT installation to use")

option(ENABLE_SBS "Build SBS software with a configured kernel source" OFF)
set(WITH_SBSDIR "" CACHE PATH
  "Path to the configured SBS kernel directory. Required if ENABLE_SBS")

option(ENABLE_USB "Build support for CC/VM usb data taking" OFF)
set(WITH_USB_HEADERDIR "" CACHE PATH "Path to (legacy libusb) usb.h")
set(WITH_USB_LIBDIR "" CACHE PATH "Path to (legacy) libusb.a")

option(ENABLE_EPICS_TOOLS "Build software that requires EPICS" OFF)
set(WITH_EPICS_ROOTDIR "" CACHE PATH "Top level directory of the EPICS install")

option(ENABLE_RUST_TOOLS "Build RUST tools and toys" OFF)

#  VMUSB firmware workarounds (issue #294) - all enabled by default.

option(ENABLE_VMUSB_PROMPT_BUSY
  "Workaround for VMUSB not giving a prompt busy" ON)
option(ENABLE_VMUSB_END_EVENT
  "Workaround for the VMUSB not always giving an end of event pulse" ON)
option(ENABLE_VMUSB_IDLE_BUSY
  "Workaround for the VMUSB not asserting Busy when the run ends" ON)

#  Tool locations (configure: --with-gengetopt-path; --with-tclconfig and
#  --with-tkconfig are WITH_TCLCONFIG/WITH_TKCONFIG in NSCLDAQDependencies).
#  Other tools can be pointed at with the usual <PKG>_ROOT / cache variables
#  (e.g. -DTCLSH_CMD=..., -DBOOST_ROOT=..., -DOPENSSL_ROOT_DIR=...).

set(WITH_GENGETOPT_PATH "" CACHE FILEPATH "Path to the gengetopt executable")

#---------------------------------------------------------------------------
#  Consistency checks done by configure.

if(ENABLE_CAEN_DIGITIZER_SUPPORT AND NOT WITH_CAEN_DIGITIZER_LIBROOT)
  message(FATAL_ERROR "ENABLE_CAEN_DIGITIZER_SUPPORT also requires "
    "WITH_CAEN_DIGITIZER_LIBROOT to specify where the CAEN libraries/headers are")
endif()
if(ENABLE_MVLC_STACK_GENERATOR AND NOT WITH_MVLCDIR)
  message(FATAL_ERROR "ENABLE_MVLC_STACK_GENERATOR also requires WITH_MVLCDIR")
endif()
if(ENABLE_DDAS AND NOT WITH_FIRMWAREDIR)
  message(FATAL_ERROR "specifying a firmware install directory using "
    "WITH_FIRMWAREDIR is mandatory if DDAS support is enabled")
endif()
if(ENABLE_SBS)
  if(NOT WITH_SBSDIR)
    message(FATAL_ERROR "specifying an SBS kernel directory using WITH_SBSDIR "
      "is mandatory if SBS kernel support is enabled")
  endif()
  if(NOT IS_DIRECTORY "${WITH_SBSDIR}")
    message(FATAL_ERROR "the specified SBS directory ${WITH_SBSDIR} does not exist!")
  endif()
endif()
