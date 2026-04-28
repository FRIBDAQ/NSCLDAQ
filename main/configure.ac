#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.71])

AC_INIT([nscldaq],[12.2-008],[daqhelp@nscl.msu.edu],[],[https://github.com/FRIBDAQ/NSCLDAQ])

AC_CONFIG_SRCDIR([/base/dataflow/CRingBuffer.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AH_TOP([
#ifndef CONFIG_H
#define CONFIG_H
])
AH_BOTTOM([
#endif
])

# Checks for programs:

AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
LT_INIT
AX_CXX_COMPILE_STDCXX_11([noext])
PKG_PROG_PKG_CONFIG 

# Set parallel build for incorps:

AC_ARG_WITH([incorp-build-cores],
	AS_HELP_STRING([--with-incorp-build-cores=N],
		[Number of cores used to build incorporated packages (default=1)]),
	[incorp_cores=$withval],
	[incorp_cores=1])
AC_MSG_NOTICE([Building incorporated packages with ${incorp_cores} cores])

# Is git installed? Used to check incorp'd module versions to export to
# VERSION in the install directory.

AC_CHECK_PROG(GIT, [git], [git], AC_MSG_ERROR([git is required for NSCLDAQ incorporated library management]))
AC_SUBST(GIT) # Only if we have it

# MPI defined/usable to build MPI chunks:

AC_LANG_PUSH(C++)
AX_MPI(
	[have_mpi=yes AC_DEFINE([HAVE_MPI], 1, [MPI exists])],
	[have_mpi=no]
)
AM_CONDITIONAL([MPI], [test "x$have_mpi" = "xyes"])
AC_LANG_POP

# Pandoc is needed for printed versions of the logbook:

AC_CHECK_PROG(HAVE_PANDOC, [pandoc], [yes], [no])
AS_IF([test "x$HAVE_PANDOC" = "xno"], [AC_MSG_FAILURE([pandoc is required to produce PDF output from the logbook utilities])])

#  CAEN Digitizer support is optional:

AC_ARG_ENABLE([caen-digitizer-support],
	AS_HELP_STRING([--enable-caen-digitizer-support],
		[Build support for CAEN Digitizers]),
	[enable_caen=$enableval],
	[enable_caen=no])

AC_ARG_WITH([caen-digitizer-libroot],
	AS_HELP_STRING([--with-caen-digitizer-libroot],
		[Root directory for CAEN Digitizer/VME/Comm library installation]),
	[caen_root=$withval],
	[caen_root=""])

AS_IF([test "x$enable_caen" = "xyes"], [
	AS_IF([test "x$caen_root" != "x"], [
		AC_MSG_NOTICE([Enabling CAEN digitizer DPP support])
		CAENCCFLAGS=-I$caen_root/include
		CAENLDFLAGS="-L$caen_root/lib -lCAENDigitizer -lCAENVME -lCAENComm -Wl,-rpath=$caen_root/lib"
		AC_SUBST(CAENCCFLAGS)
		AC_SUBST(CAENLDFLAGS)
	], [
		AC_MSG_ERROR([using --enable-caen-digitizer-support also requires --with-caen-digitizer-libroot to specify where the CAEN libraries/headers are])
	])
])
AM_CONDITIONAL([CAENSUPPORT], [test "x$enable_caen" = "xyes"])

#  Enabled MVLC code generation:

AC_ARG_ENABLE([mvlc-stack-generator],
	AS_HELP_STRING([--enable-mvlc-stack-generator],
		[Build support for generating MVLX fribdaq-readout config files requires --with-mvlcdir]),
	[enable_mvlc=$enableval], 
	[enable_mvlc=no])

AC_ARG_WITH([mvlcdir],
	AS_HELP_STRING([--with-mvlcdir],
		[Root directory of the MVLC driver installation]),
	[mvlcdir=$withval],
	[mvlcdir=""])

mvlc_generator=false # Conditional to check

AS_IF([test "x$enable_mvlc" = "xyes"], [
	AC_MSG_NOTICE([Including mvlc usb config generation support])

	mvlc_generator=true # Set the conditional

	# MVLC Generator needs to have the yaml-cpp package and its flags defined:

	PKG_CHECK_MODULES([YAML], [yaml-cpp])

	#  We need to know where the mvlc driver is... so mvlcdir can't be blank.

	AS_IF([test "x$mvlcdir" != "x"], [
		MVLC_ROOT="${mvlcdir}"
		MVLC_INC="${mvlcdir}/include"
		MVLC_LIB="${mvlcdir}/lib"
		MVLC_BIN="${mvlcdir}/bin"
		MVLC_CPPFLAGS="-I${MVLC_INC} -I${MVLC_INC}/mesytec-mvlc"
		MVLC_LDFLAGS="-L${MVLC_LIB} -lmesytec-mvlc -Wl,-rpath=${MVLC_LIB}"
		AC_SUBST(MVLC_ROOT)
		AC_SUBST(MVLC_INC)
		AC_SUBST(MVLC_LIB)
		AC_SUBST(MVLC_BIN)
		AC_SUBST(MVLC_CPPFLAGS)
		AC_SUBST(MVLC_LDFLAGS)
	], [
		AC_MSG_ERROR([--enable-mvlc-stack-generator also requires --with-mvlcdir])
	])
])
AM_CONDITIONAL([MVLC_GENERATOR], [test "x$mvlc_generator" = "xtrue"])

#  CAEN Nextgen digitizer support is also optional  the CAEN support
#  libraries are awful in that they require installation in
#  specific locations:

AC_ARG_ENABLE([caen-nextgen],
	[AS_HELP_STRING([--enable-caen-nextgen],
		[To enable support for CAEN Nextgen digitizers.])],	
	[enable_caennextgen=$enableval],
	[enable_caennextgen="no"]) 

AS_IF([test "x$enable_caennextgen" = "xyes"], [
	AC_MSG_NOTICE([Enabled support for CAEN Nextgen digitizers])

	# Check for the files if enabled:

	AC_MSG_CHECKING([for CAEN FElib header])

	##
	#  (ASC 1/15/26): Check for the header/libs using the compiler, not filesystem
	#

	AC_MSG_CHECKING([for CAEN FElib header])
	AC_CHECK_HEADER([CAEN_FELib.h], [CAENNG_CPPFLAGS="-I/usr/local/include"], [AC_MSG_ERROR([cannot find /usr/local/include/CAEN_FELib.h - install CAEN Nextgen digitizer support libraries])])

	AC_MSG_CHECKING([for CAEN FElib libraries])
	AC_CHECK_LIB([CAEN_FELib], [CAEN_FELib_GetLibInfo], [CAENNG_LDFLAGS="-L/usr/local/lib -lCAEN_FELib"], [AC_MSG_ERROR([cannot find /usr/local/lib/libCAEN_FELib.so - install CAEN nextgen digitizer support libraries])])
	AC_CHECK_LIB([CAEN_Dig2], [CAENDig2_GetLibInfo], [CAENNG_LDFLAGS="$CAENNG_LDFLAGS -lCAEN_Dig2"], [AC_MSG_ERROR([cannot find /usr/local/lib/libCAEN_Dig2.so - install CAEN nextgen digitizer support libraries])])

	#  Set the makefile variables:

	AC_SUBST(CAENNG_CPPFLAGS)
	AC_SUBST(CAENNG_LDFLAGS)
], [
	AC_MSG_NOTICE([Disabled support for  CAEN Nextgen digitizers])
])
AM_CONDITIONAL([CAENNEXTGEN], [test "x$enable_caennextgen" = "xyes"])

# CAEN Nextgen digitizers, if used require jsoncpp:

PKG_CHECK_MODULES([JSON], [jsoncpp])

# Documentation

AC_ARG_ENABLE([docs],
    AS_HELP_STRING([--enable-docs], 
		[Build the documentation]),
    [enable_docs=$enableval],
	[enable_docs="no"])

AS_IF([test "x$enable_docs" = "xyes"], [
	AC_MSG_NOTICE([Documentation enabled])

	AC_CHECK_PROG(HAVE_DOCBOOK2PDF, [docbook2pdf], [yes], [no])
	AS_IF([test "x$HAVE_DOCBOOK2PDF" = "xno"], [
		AC_MSG_FAILURE([docbook2pdf is required to build documentation but was not found])
	], [
		AC_SUBST(DOCBOOK2PDF, [docbook2pdf])
	])

	AC_CHECK_PROG(HAVE_DOCBOOK2HTML, [docbook2html], [yes], [no])	
	AS_IF([test "x$HAVE_DOCBOOK2HTML" = "xno"], [
		AC_MSG_FAILURE([docbook2html is required to build documentation but was not found])
	], [
		AC_SUBST(DOCBOOK2HTML, [docbook2html])
	])

	AC_CHECK_PROG(HAVE_XMLTO, [xmlto], [yes], [no])	
	AS_IF([test "x$HAVE_XMLTO" = "xno"], [
		AC_MSG_FAILURE([xmlto is required to build documentation but was not found])
	], [
		AC_SUBST(DOCBOOK2MAN, ["xmlto man"])
	])

	AC_CHECK_PROG(HAVE_MANDB, mandb, [yes], [no])	
	AS_IF([test "x$HAVE_MANDB" = "xno"], [
		AC_MSG_FAILURE([mandb is required to build documentation but was not found])
	], [
		AC_SUBST(MANDB, [mandb])
	])
], [
	AC_MSG_NOTICE([Documentation disabled])
])
AM_CONDITIONAL([BUILD_DOCS], [test "x$enable_docs" = "xyes"])

# DDAS documentation
# DDAS docs are written in Doxygen, so if enabled check to make sure that
# Doxygen and dot (for graphs) are installed

AC_ARG_ENABLE([ddas-docs],
    AS_HELP_STRING([--enable-ddas-docs], 
		[Build the DDAS documentation]),
    [enable_ddas_docs=$enableval], 
	[enable_ddas_docs=no])

AC_CHECK_PROG(HAVE_DOXYGEN, [doxygen], [yes], [no])
AC_CHECK_PROG(HAVE_DOT, [dot], [yes], [no])

AS_IF([test "x$enable_ddas_docs" = "xyes"], [
	AC_MSG_NOTICE([DDAS documentation enabled])
	AS_IF([test "x$HAVE_DOXYGEN" = "xfalse"], [
		AC_MSG_FAILURE([Doxygen is required to build the DDAS documentation but was not found])
	], [
		AC_SUBST(DOXYGEN, [doxygen])
	])
	AS_IF([test "x$HAVE_DOT" = "xno"], [
		AC_MSG_FAILURE([dot is required by Doxygen to build the DDAS documentation but was not found])
	])
], [
	AC_MSG_NOTICE([DDAS documentation disabled])
])
AM_CONDITIONAL([BUILD_DDAS_DOCS], [test "x$enable_ddas_docs" = "xyes"])

AC_CHECK_LIB([X11], [XSetWindowBackground])
AC_CHECK_LIB([Xt], [XtManage])

AM_PATH_CPPUNIT

# Checksums in eventlog require OpenSSL for the digest libs:

AX_CHECK_OPENSSL([AC_MSG_RESULT([Found OpenSSL])], [AC_MSG_ERROR([cannot find OpenSSL installation required by NSCLDAQ])])

# For compatibility with existing AC_Substs:

CPPUNIT_LDFLAGS="${CPPUNIT_LIBS}"

AC_SUBST([AM_CXXFLAGS], [-fno-strict-aliasing])
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_TYPE_UID_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AX_CHECK_COMPILE_FLAG([-Wno-error=date-time], [NODATEWARN="-Wno-error=date-time"])
AC_SUBST(NODATEWARN)

# Check to find the size of address

AC_CHECK_SIZEOF([void*])
AS_IF([test "$ac_cv_sizeof_voidp" -eq 8], [AC_DEFINE([ADDR64], 1, [Specifies that memory addresses are 64-bits ])], [])
   
# Checks for library functions.

AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 gethostbyaddr gethostbyname gettimeofday inet_ntoa memmove memset regcomp socket strchr strcspn strdup strerror strspn strtol])

AC_CHECK_FUNCS([ftruncate munmap])
AC_FUNC_MMAP
AC_FUNC_STAT

# Additional checks from DDAS/main/configure.ac added for refactoring:
# --ASC 4/6/23

AC_TYPE_INT16_T
AC_TYPE_INT64_T
AC_TYPE_UINT64_T
AC_CHECK_TYPES([ptrdiff_t])
AC_CHECK_FUNCS([floor pow select sqrt strstr])
AC_CHECK_HEADERS([float.h values.h])

# Bits and pieces of NSCLDAQ can do logging via boostlog. This stuff tells us
# if that's present and if so, how to incorporate it:

AX_BOOST_BASE([],
	[BOOST_EXTRA_CPPFLAGS="-DBOOST_ALL_DYN_LINK"],
	[BOOST_EXTRA_CPPFLAGS=""])
AX_BOOST_LOG
AC_SUBST(BOOST_EXTRA_CPPFLAGS)

# Automake:

AM_INIT_AUTOMAKE([tar-ustar foreign subdir-objects])

# Makefile exports:

SOVERSION="11:0:0"

#---------------------------------------------------------------------
#
#  Local tests:
#

# Ensure python is 3.0 or greater, and the packages/dependencies we need:

AM_PATH_PYTHON([3.0],
	[],
	[AC_MSG_ERROR([python 3.0 or higher is needed for NSCLDAQ])])
AX_PYQT
PKG_CHECK_MODULES(PYTHON3, python3)

# Also need to set compilation/link flags for QtScope:
# Thanks to Giordano for figuring this portable way to do it:

PKG_CHECK_MODULES(PYTHON,
	[python3-embed],
	[PYTHON3_CXXFLAGS=`pkg-config --cflags python3-embed`; PYTHON3_LIBS=`pkg-config --libs python3-embed`],
	[PYTHON3_CXXFLAGS=`pkg-config python3 --cflags`, PYTHON3_LIBS=`pkg-config python3 --libs`])
AC_SUBST(PYTHON3_CXXFLAGS)
AC_SUBST(PYTHON3_LIBS)

#
#  See Issue #294 - Since the compilation flags for the VMUSBReadout
#  VMUSB firmware problem workarounds must span directories, they
#  need to be configured.  All are enabled by default.
#
#   --disable-vmusb-prompt-busy - turns off workaround for the prompt prompt_busy
#      firmware bug.  If this is enabled, the stack -delay option is implemented
#      by waits in the stack rather than a value written to the trigger delay
#      field of the acquisition config register.
#   --disable-vmusb-end-event - turns off workaround for the requirement that
#       in order to get an end of event pulse, the last element of the stack
#       must put something in the event.  If enabled, a marker is transparently
#       added to the stack by CStack and removed from the event by COutuptThread.
#   --disable-vmusb-idle-busy - Turns off the workaround for the state of the
#       busy when the VMUBS is not in autonomous mode.  If the user has selected
#       -nimo1 busy, on end run, NIM01 is programmed to either be the inverse of the
#       end of event (should always be asserted), or if inverted was selected, end of event itself.

AC_ARG_ENABLE([vmusb-prompt-busy], 
	[AS_HELP_STRING(
		[--disable-vmusb-prompt-busy], 
		[Turn off the workaround for VMUSB not giving a prompt busy])], 
	[enable_prompt=$enableval], 
	[enable_prompt=yes])
AS_IF([test "x$enable_prompt" = "xyes"], [PROMPT_BUSY="-DVMUSB_PROMPT_BUSY_WORKAROUND"])

AC_ARG_ENABLE([vmusb-end-event],
	AS_HELP_STRING([--disable-vmusb-end-event], 
		[Turn off the workaround for the VMUSB not always giving an end of event pulse]),
	[enable_eoe=$enableval], 
	[enable_eoe=yes])
AS_IF([test "x$enable_eoe" = "xyes"], [END_EVENT="-DVMUSB_END_OF_EVENT_WORKAROUND"])

AC_ARG_ENABLE([vmusb-idle-busy],
	AS_HELP_STRING([--disable-vmusb-idle-busy],
		[Turn off the workaround for the VMUSB not asserting Busy when the run ends]),
	[enable_idlebusy=$enableval], 
	[enable_idlebusy=yes])
AS_IF([test "x$enable_idlebusy" = "xyes"], [IDLE_BUSY="-DVMUSB_IDLE_BUSY_WORKAROUND"])

#  The VMUSB_WORKAROUNDS variable will have all of the conditionals selected:

VMUSB_WORKAROUNDS="${PROMPT_BUSY} ${END_EVENT} ${IDLE_BUSY}"
AC_SUBST(VMUSB_WORKAROUNDS)

	
# Ensure swig is installed: -- this is used to generate wrappers for the
# VMUSB and CCUSB libraries for use within Tcl drivers.
#
# Earlier swig versions than 1.3 may well work, however this is the
# version I used when first successfully building the wrappers
# (Ubuntu Lucid Lynx).

AX_PKG_SWIG([1.3], [], [AC_MSG_ERROR([swig is required to build NSCLDAQ])])

# We use gengetopt in a few places.  Ideally it's been found by now in the
# path if not we'll need to do the withval thing... or prompt the user to
# give us a --with-gengetopt-path

AX_GENGETOPT

# Tcl/Tk is pervasive in NSCLDAQ so we need to have it too:

AX_TCL

#  The substs below are for compatibility sake 
#  eventually we should migrate Makefile.am defs to use
#  those in m4/tcl.m4

TCL_FLAGS="${TCL_CPPFLAGS} ${TK_CPPFLAGS}"

# Check for redundancy of libieee in the LD_LIBS

TCL_LDFLAGS=${TK_LIBS}
for tclname in ${TCL_LIBS}
do
	AS_IF([test "x$tclname" != "x-lieee"], [TCL_LDFLAGS="$TCL_LDFLAGS $tclname"])
done
TCLSH_CMD=${TCLSH}

# Incorporate and build tcl++:

AC_CHECK_FILE([${srcdir}/libtcl/configure.ac],
	[AC_MSG_NOTICE([Using previously incorprated tcl++ from the source tree at ${srcdir}/libtcl])
		cp -r ${srcdir}/libtcl . ; chmod -R u+w libtcl],
	[AC_MSG_NOTICE([Incorporating tcl++ for you])
		${srcdir}/tcl++incorp])

rm -rf libtcl/build
AS_MKDIR_P([libtcl/build])
AC_MSG_NOTICE([Building and installing tcl++])

(cd libtcl/build; ../configure --prefix=${prefix}; make clean ; make -j ${incorp_cores} all ; make install)
result=$?
AS_VAR_IF([result], [0], [AC_MSG_NOTICE([tcl++ built and installed])], [AC_MSG_ERROR([Failed to build and install tcl++ with result: ${result}])])
LIBTCLPLUS_CFLAGS="-I${prefix}/include"
LIBEXCEPTION_LDFLAGS="-L${prefix}/lib -lException -Wl,\"-rpath=${prefix}/lib\""
LIBTCLPLUS_LDFLAGS="-L${prefix}/lib -ltclPlus ${LIBEXCEPTION_LDFLAGS}"
AC_SUBST(LIBTCLPLUS_CFLAGS)
AC_SUBST(LIBTCLPLUS_LDFLAGS)
AC_SUBST(LIBEXCEPTION_LDFLAGS)

#  Configure and install tclhttpd:
#  /usr/lib/tclx.y     - Has tclConfig.
#  /usr/include/tclx.y - Has the headers.

tcl_basename=tcl${TCL_VERSION}
tcl_configdir="/usr/lib/$tcl_basename"
tcl_incdir="/usr/include/$tcl_basename"

AC_CHECK_FILE([${srcdir}/tclhttpd3.5.1/configure],
	[AC_MSG_NOTICE([Using previously incorprated tclhttp from the source tree at ${srcdir}/tclhttpd3.5.1])
		cp -r ${srcdir}/tclhttpd3.5.1 . ; chmod -R u+w tclhttpd3.5.1],
	[AC_MSG_ERROR([Cannot find tclhttp in source tree!])])

rm -rf tclhttpd3.5.1/build
AS_MKDIR_P([tclhttpd3.5.1/build])

(cd tclhttpd3.5.1/build; ../configure --prefix=${prefix}/share/tclhttpd --enable-gcc --with-tclinclude=$tcl_incdir --with-tcl=$tcl_configdir)
result=$?
AS_VAR_IF([result], [0], [AC_MSG_NOTICE([Configured tclhttpd])], [AC_MSG_ERROR([Failed to configure tclhttp prior to installation])])
(cd tclhttpd3.5.1/build; make clean ; make -j ${incorp_cores} all ; make install)
result=$?
AS_VAR_IF([result], [0], [AC_MSG_NOTICE([Installed tclhttpd])], [AC_MSG_ERROR([Failed to install tclhttpd])])

# Epics
#
#  This is only probed for and build if --enable-epics-tools=yes is provided
#  at configuration time.  Oncethat's done, AX_EPICS provideas all the defs
#  and the EPICS/BUILD_EPICS_TOOLS enables those in the Makefiles.
#

AC_ARG_ENABLE([epics-tools],
	AS_HELP_STRING([--enable-epics-tools], 
		[Build software that requires EPICS (default no)]),
	[enable_epics_tools=$enableval], 
	[enable_epics_tools=no])
AS_IF([test "x$enable_epics_tools" = "xyes"], [AX_EPICS])
AM_CONDITIONAL([EPICS], [test "x$enable_epics_tools" = "xyes"])
AM_CONDITIONAL([BUILD_EPICS_TOOLS], [test "x$enable_epics_tools" = "xyes"])

#   Build the format libraries.
#
# UFMT_CPPFLAGS    - Unified format compiler flags
# UFMT_LDFLAGS     - Unified format linker flags
# DDASFMT_CPPFLAGS - DDAS format compiler flags
# DDASFMT_LDFLAGS  - DDAS format linker flags
# The unified format and DDAS format libraries have builds that
# require cmake, so check for that:

AC_CHECK_PROG([have_cmake], [cmake], [yes])
AS_VAR_IF([have_cmake], [yes], [], AC_MSG_ERROR([cmake must be in the path to build DDAS support software]))

# Check that the unified format library has been incorporated into
# the source tree and configure/build it into $prefix/unifiedformat

AC_CHECK_FILE([${srcdir}/unifiedformat/CMakeLists.txt],
	[AC_MSG_NOTICE([Using previously incorporated unified format library at ${srcdir}/unifiedformat])
		cp -r ${srcdir}/unifiedformat . ; chmod -R u+w unifiedformat],
	[AC_MSG_NOTICE([Incorporating unified format for you])
		${srcdir}/unifiedfmt-incorp.sh])

rm -rf unifiedformat/build
AS_MKDIR_P(unifiedformat/build)	
AC_MSG_NOTICE([Building unified format])

(cd unifiedformat/build; cmake .. -DCMAKE_INSTALL_PREFIX=${prefix}/unifiedformat; make clean; make -j ${incorp_cores} all ; make install)
result=$?
AS_VAR_IF([result], [0], [AC_MSG_NOTICE([Built and installed unified format pre-requisite])], [AC_MSG_ERROR([Failed to build and install unified format library with result: ${result}])])
UFMT_CPPFLAGS="-I${prefix}/unifiedformat/include"
UFMT_LDFLAGS="-L${prefix}/unifiedformat/lib -lNSCLDAQFormat -lV10Format -lV11Format -lV12Format -lAbstractFormat -Wl,-rpath=${prefix}/unifiedformat/lib"
AC_SUBST(UFMT_CPPFLAGS)
AC_SUBST(UFMT_LDFLAGS)

# Check for ROOT. Use rootcling to add ROOT flags and linker options
# to the Makefile macro set:

# Check that the DDAS format library has been incorporated into the
# source tree and configure/build it into $prefix/ddasformat.
	
AC_CHECK_FILE([${srcdir}/ddasformat/CMakeLists.txt],
	[AC_MSG_NOTICE([Using previously incorprated ddasformat from the source tree at ${srcdir}/ddasformat])
		cp -r ${srcdir}/ddasformat . ; chmod -R u+w ddasformat],
	[AC_MSG_NOTICE([Incorporating ddasformat for you])
		${srcdir}/ddasfmt-incorp.sh])

rm -rf ddasformat/build
AS_MKDIR_P(ddasformat/build)
AC_MSG_NOTICE([Building DDAS format])

(cd ddasformat/build; cmake .. -DCMAKE_INSTALL_PREFIX=${prefix}/ddasformat; make clean; make -j ${incorp_cores} all ; make install)
result=$?
AS_VAR_IF([result], [0], [AC_MSG_NOTICE([Built and installed DDAS format pre-requisite])], [AC_MSG_ERROR([Failed to build and install DDAS format library with result: ${result}])])
DDASFMT_CPPFLAGS="-I${prefix}/ddasformat/include"
DDASFMT_LDFLAGS="-L${prefix}/ddasformat/lib -lDDASFormat -Wl,-rpath=${prefix}/ddasformat/lib"
AC_SUBST(DDASFMT_CPPFLAGS)
AC_SUBST(DDASFMT_LDFLAGS)

##
# DDAS requires ROOT so check for that here:
#

rootdir=$ROOTSYS
AC_ARG_WITH([rootsys],
	AS_HELP_STRING([--with-rootsys=path], 
		[Path to the ROOT installation to use. Required if DDAS support is enabled.]),
	[rootdir=$withval])
AS_IF([test "x$rootdir" = "x"], [AC_MSG_ERROR([use --with-rootsys to specify where ROOT is installed])])
AS_IF([test ! -d $rootdir], [AC_MSG_ERROR([the specified ROOTSYS directory $rootdir does not exist!])])	

ROOTSYS=$rootdir
ROOT_CFLAGS=`$rootdir/bin/root-config --cflags`
ROOT_LD1=`$rootdir/bin/root-config --glibs`
ROOT_LD2=`$rootdir/bin/root-config --ldflags`
ROOT_RPATH="-Wl,-rpath=`$rootdir/bin/root-config --libdir`"

ROOT_LDFLAGS="$ROOT_LD1 $ROOT_LD2 $ROOT_RPATH"

AC_SUBST(ROOTSYS)
AC_SUBST(ROOT_CFLAGS)
AC_SUBST(ROOT_LDFLAGS)

# The DDAS Software will only be built if --enable-ddas=yes is supplied by
# the user. What we provide to the Makefiles:
#
# BUILD_DDAS       - If this flag is provided that's defined to allow the top
#                    level Makefile to enable the ddas support software build.
# PIXIE_API_DIR    - Installation directory of the XIA Pixie API
# PIXIE_CPPFLAGS   - Compiler flags for Pixie API
# PIXIE_LDFLAGS    - Linker flags for Pixie API
# firmwaredir      - Pixie firmware installation directory
# ROOTSYS          - ROOT installation to compile e.g. ddasdumper against
# ROOT_CFLAGS      - ROOT cflags (automatically generated from ROOT config)
# ROOT_LDFLAGS     - ROOT libraries (automatically generated from ROOT config)

AC_ARG_ENABLE([ddas],
	AS_HELP_STRING([--enable-ddas], 
		[Build the DDAS support software (default no)]),
	[enable_ddas=$enableval], 
	[enable_ddas=no])

AC_ARG_WITH([xiaapidir],
	AS_HELP_STRING([--with-xiaapidir=path], 
		[Path to the XIA API installation. Required if DDAS support is enabled.]),
	[xiaapidir=$withval])

AC_ARG_WITH([firmwaredir],
	AS_HELP_STRING([--with-firmwaredir=path], 
		[Specify path to firmware directory tree. Required if DDAS support is enabled.]),
	[firmwaredir=$withval])

AS_IF([test "x$enable_ddas" = "xyes"], [
	AC_MSG_NOTICE([The XIA-DDAS support software will be built])
	AC_DEFINE([USING_DDAS], [1], [DDAS support enabled])

	# Check that the XIA API installation directory is correct by looking
	# for the pixie16.h header there. If found, define vars; if not,
	# print an error message and exit. Note that this version supports
	# only XIA API major version 4 which incorporates "legacy mode"
	# as a supported option as of release 4.x.y and later:

	# Autoconf-generated test code compiler and linker flags:

	default_CPPFLAGS="$CPPFLAGS"
	default_LDFLAGS="$LDFLAGS"
	CPPFLAGS="$CPPFLAGS -I${xiaapidir}/include"
	LDFLAGS="$LDFLAGS -L${xiaapidir}/lib -Wl,-rpath=${xiaapidir}/lib"

	AC_CHECK_HEADER([pixie16/pixie16.h], [PIXIE_CPPFLAGS="-I${xiaapidir}/include"], [AC_MSG_ERROR([cannot find pixie16/pixie16.h in ${xiaapidir}, use --with-xiaapidir to point to the correct one (version 3+)])])
	AC_CHECK_LIB([Pixie16Api], [Pixie16InitSystem], [PIXIE_LDFLAGS="-L${xiaapidir}/lib -lPixie16Api -lm -Wl,-rpath=${xiaapidir}/lib"], [AC_MSG_ERROR([cannot find libPixie16Api in ${xiaapidir}/lib, use --with-xiaapidir to point to the correct one (version 3+)])])

	# Restore default flags:

	CPPFLAGS="$defult_CPPFLAGS"
	LDFLAGS="$default_LDFLAGS"

	PIXIE_API_DIR=$xiaapidir
	AC_SUBST(PIXIE_API_DIR)
	AC_SUBST(PIXIE_CPPFLAGS)
	AC_SUBST(PIXIE_LDFLAGS)

	AS_IF([test "x$firmwaredir" != "x"], [
		AC_SUBST(firmwaredir)
	], [
		AC_MSG_ERROR([specifying a firmware install directory using --with-firmwaredir is a mandatory option if DDAS support is enabled])
	])
])
AM_CONDITIONAL([BUILD_DDAS], [test "x$enable_ddas" = "xyes"])

# The SBS software will only be built if --enable-sbs was supplied.
# In that case, the configured kernel source path is set using
# --with-sbs-kernel-source-dir and that will be used to compile the
# device driver as well. The default is to _not_ build the sbs stuff.

AC_ARG_ENABLE([sbs],
	AS_HELP_STRING([--enable-sbs], 
		[Build SBS software with a configured kernel source]),
	[enable_sbs=$enableval],
	[enable_sbs=no])

AC_ARG_WITH([sbsdir],
	AS_HELP_STRING([--with-sbsdir=path], 
		[Specify path to the configured SBS kernel directory. Required if SBS support is enabled.]),
	[sbsdir=$withval])

AS_IF([test "x$enable_sbs" = "xyes"], [
	AS_IF([test "x$sbsdir" = "x"], [
		AC_MSG_ERROR([specifiying an SBS kernel directory using --with-sbsdir is a mandatory option if SBS kernel support is enabled])
	])
	AS_IF([test ! -d $sbsdir], [
		AC_MSG_ERROR([the specified SBS directory $sbsdir does not exist!])
	])
	KERNEL_SOURCE_DIR=$sbsdir
	AC_SUBST(KERNEL_SOURCE_DIR)
	AC_MSG_NOTICE([Building SBS software with kernel source at: $KERNEL_SOURCE_DIR])
], [
	AC_MSG_NOTICE([SBS software is not enabled.])
])
AM_CONDITIONAL([BUILD_SBS_DRIVER], test ["x$enable_sbs" = "xyes"])

# If --enable-usb is turned on, we build the VM/CC-usb support directories

AC_ARG_ENABLE([usb],
	AS_HELP_STRING([--enable-usb], 
		[Build support for CC/VM usb data taking]),
	[enable_usb=$enableval],
	[enable_usb=no])

#  If building is turned on we need to locate the headers and the includes.

AS_IF([test "x$enable_usb" = "xyes"], [
	AX_LIBUSB
	PKG_CHECK_MODULES([libusb_1_0], [libusb-1.0])
	PKG_CHECK_MODULES([libusb], [libusb])
])
AM_CONDITIONAL([BUILD_USB], [test "x$enable_usb" = "xyes"])

# Check for libsqlite3:

PKG_CHECK_MODULES([SQLITE3], [sqlite3])

##
#  Check for zmq presence:
#  (ASC 1/15/26): zmq_init is deprected by zmq_ctx_new in ZMQ major version > 3 e.g., on bookworm
#

AC_CHECK_LIB([zmq], [zmq_ctx_new], [ZMQ_LDFLAGS="-lzmq"], [AC_MSG_ERROR([the ZeroMQ library was not found and is now required for NSCLDAQ])])
AC_SUBST(ZMQ_LDFLAGS)

##
#  need the zmq c++ bindings
#  note in some c++11's we need to include <algorithm> to include zmq
#

AC_LANG_PUSH(C++)
AC_CHECK_HEADERS([algorithm zmq.hpp], [], [AC_MSG_ERROR([the ZeroMQ C++ header zmq.hpp is missing and is required for NSCLDAQ])], [#include <algorithm>])
AC_LANG_POP

#----------------------------------------------------------------------------

# Exports to the makefiles:

AC_DEFINE(__STDC_LIMIT_MACROS, [], [Needed to define UINT64_C])
AC_DEFINE(__STDC_CONSTANT_MACROS, [], [Needed to define UINT64_C])

AC_SUBST(SOVERSION)

AC_SUBST(TCL_FLAGS)
AC_SUBST(TCL_LDFLAGS)
AC_SUBST(TCLSH_CMD)

AC_SUBST(CPPUNIT_LDFLAGS)

#  Threading is hard coded to -pthread now.

THREADCXX_FLAGS="-pthread"
THREADC_FLAGS="-pthread"
THREADLD_FLAGS="-lpthread -lrt"

AC_SUBST(THREADCXX_FLAGS)
AC_SUBST(THREADC_FLAGS)
AC_SUBST(THREADLD_FLAGS)

# Ensure that resources for testing are accessible for out of tree builds

AC_CONFIG_LINKS([utilities/filter/run-0000-00.evt:utilities/filter/run-0000-00.evt])

#  Start support for integrating rust tools:

AC_ARG_ENABLE([rust-tools],
	AS_HELP_STRING([--enable-rust-tools], 
		[Build RUST tools and toys]),
	[enable_rust=$enableval], 
	[enable_rust=no]
)

AS_IF([test "x$enable_rust" = "xyes"], [
	# Figure out where the rust toolchains are. For FRIB containers,
	# it's in /usr/cargo/bin for user installs it may be in ~/.cargo/bin
	# Those are the things we support -- if neither exists,
	# the usr needs to install RUST; we won't do it for them.
	AC_MSG_CHECKING([Checking for Rust toolchain])
	AS_IF([test -x /usr/cargo/bin/cargo], [
		RUST_TOOL_DIR=/usr/cargo/bin
		BUILD_RUST_TOOLS=1
	], [
		AS_IF([test -x ~/.cargo/bin/cargo], [
			RUST_TOOL_DIR=~/.cargo/bin
			BUILD_RUST_TOOLS=1
		], [
			AC_MSG_ERROR([You need to install the rust toolchain to --enable-rust-tools see https://man7.org/linux/man-pages/man1/test.1.html])
		])
	])
	RUST_CARGO="${RUST_TOOL_DIR}/cargo"

	#  Select the default tool chain to be stable:	

	${RUST_TOOL_DIR}/rustup default stable
	AC_MSG_NOTICE([The default rust toolchain has been set to stable])	
	AC_SUBST(RUST_TOOL_DIR)
	AC_SUBST(RUST_CARGO)

	# Incorporate the rust directory tree into builddir:
	# The next symbol is a list of the 
	# Git repositories to clone into rust_tools:

	rust_repos="https://github.com/FRIBDAQ/rust-portman"
	rust_repos="${rust_repos} https://github.com/FRIBDAQ/rust-ringtostdout"
	rust_repos="${rust_repos} https://github.com/FRIBDAQ/rust-ringmaster"
	RUST_SUBDIRS=""

	mkdir -p rust_tools

	for repo in ${rust_repos}
	do 
		AC_MSG_NOTICE([Cloning rust tool from ${repo}])
		subdir=`basename ${repo}`
		rm -rf rust_tools/${subdir}
		(cd rust_tools; git clone  ${repo})
		RUST_SUBDIRS="${RUST_SUBDIRS} ${subdir}"
	done
	AC_SUBST(RUST_SUBDIRS)
])
AM_CONDITIONAL(BUILD_RUST_TOOLS, [test "x$enable_rust" = "xyes"])
AC_MSG_NOTICE([RUST stuff done])

#---------------------------------------------------------------------------
# Generate the following from their .in's (note that Automake takes
# *.am -> *.in.
#

# Note removed utilities/scalerdisplay in favor of utilities/newscaler
# Sept 25, 2014

AC_CONFIG_FILES([
	Makefile
	nscldaq
	daqsetup.bash
	base/pugi/Makefile
	base/iniparser/Makefile
    base/Makefile
    base/factories/Makefile
    base/os/Makefile
    base/CopyrightTools/Makefile
    base/bltsubst/Makefile
    base/cvt/Makefile
    base/thread/Makefile
    base/headers/Makefile
    base/security/Makefile
    base/tclwidgets/Makefile
    base/uri/Makefile
    base/dataflow/Makefile
    base/plotchart/Makefile
    base/plotcontainer/Makefile
    base/tcpip/Makefile
    base/tclutils/Makefile
    base/testutils/Makefile
    base/sqliteplus/Makefile
    base/udp/Makefile
    caenfw-common/Makefile
    servers/Makefile
    servers/tclserver/Makefile
    servers/portmanager/Makefile
    servers/tclhttpdlaunch/Makefile
    utilities/Makefile
    utilities/common/Makefile
    utilities/Buffer/Makefile
    utilities/daqstart/Makefile
    utilities/dvdburn/Makefile
    utilities/scalerdisplay/ScalerDisplay
    utilities/sequencer/Makefile
    utilities/ringselector/Makefile
    utilities/bufdump/Makefile
    utilities/eventlog/Makefile
    utilities/sclclient/Makefile
    utilities/tkbufdump/Makefile
    utilities/filter/Makefile
    utilities/filter/Makefile-template
    utilities/filter/FilterIncludes
    utilities/compatibility/Makefile
    utilities/compatibility/eventlog-compat
    utilities/compatibility/spectcldaq
    utilities/compatibility/s800toring
    utilities/compatibility/bufdump
    utilities/conversion/Makefile
    utilities/fileanalyzer/Makefile
    utilities/format/Makefile
    utilities/format/V8/Makefile
    utilities/format/V10/Makefile
    utilities/format/V11/Makefile
    utilities/FormattedIO/Makefile
    utilities/newscaler/Makefile
    utilities/newscaler/ScalerDisplay.tcl
    utilities/scalersum/Makefile
    utilities/reglom/Makefile
    utilities/scalerReader/Makefile
    utilities/swtrigger/Makefile
    utilities/logbook/Makefile
    utilities/manager/Makefile
    utilities/managercontrol/Makefile
    utilities/readoutREST/Makefile
    utilities/udpBroker/Makefile
    utilities/ringdiagnostics/Makefile
    utilities/statsview/Makefile
    utilities/s800scalers/Makefile
    epics/epicslib/Makefile
    epics/chanlog/Makefile
    epics/controlpush/Makefile
    epics/epicsdisplay/Makefile
    epics/epicsdisplay/epicsdisplay
    epics/Makefile
    daq/Makefile
    daq/format/Makefile
    daq/IO/Makefile
    daq/readoutgui/Makefile
    daq/readoutgui/InstallRoot.tcl
    daq/readoutgui/ReadoutShell.tcl
    daq/s800callouts/Makefile
    daq/eventbuilder/Makefile
    daq/eventbuilder/Makefile_Skeleton
    daq/eventbuilder/startOrderer
    daq/evbtools/Makefile
    daq/evbtools/ringsource/Makefile
    daq/evbtools/teering/Makefile
    daq/evbtools/glom/Makefile
    daq/evbtools/unglom/Makefile
    daq/evbtools/offlineorderer/Makefile
    daq/evbtools/offlineorderer/figures/Makefile
    daq/evbtools/evts2frags/Makefile
    daq/actions/Makefile
    daq/evbtools/evblite/Makefile
    simplesetups/Makefile
    simplesetups/v775/Makefile
    simplesetups/v785/Makefile
    simplesetups/vmusb/Makefile
    simplesetups/vmusb/src/Makefile
    simplesetups/vmusb/src/Example_Makefile
    sbs/Makefile
    sbs/driver/Makefile
    sbs/driver/src/Makefile
    sbs/driver/include/Makefile
    sbs/driver/dd/GNUmakefile
    sbs/nsclapi/Makefile
    sbs/tclpackage/Makefile
    sbs/puretcl/Makefile
    sbs/vmemodules/Makefile
    sbs/readout/Makefile
    sbs/readout/UserMakefile
    sbs/readout/SBSRdoMakeIncludes
    sbs/slowcontrols/Makefile
    sbs/slowcontrols/vhq/Makefile
    sbs/slowcontrols/caenv812/Makefile
    sbs/slowcontrols/n568b/Makefile
    sbs/slowcontrols/bcnaf/Makefile
    sbs/slowcontrols/vhs/Makefile
    sbs/scripted/Makefile
    sbs/VmeDevice/Makefile
    sbs/caenfwloader/Makefile
    usb/Makefile
    usb/usb1/Makefile
    usb/usb0.1/Makefile
    usb/loaders/Makefile
    usb/vmusb/Makefile
    usb/vmusb/vmusb/Makefile 
    usb/vmusb/core/Makefile 
    usb/vmusb/ctlconfig/Makefile 
    usb/vmusb/daqconfig/Makefile  
    usb/vmusb/driverkit/Makefile 
    usb/vmusb/driverkit/Makefile-template
    usb/vmusb/tcldrivers/Makefile
    usb/vmusb/driverkit/VMUSBDriverIncludes
    usb/vmusb/controlpanels/Makefile 
    usb/vmusb/controlpanels/cv6533/Makefile
    usb/vmusb/controlpanels/figures/Makefile
    usb/vmusb/devtests/Makefile
    usb/vmusb/vmusbPackage/Makefile
    usb/vmusb/tclservertests/Makefile
	usb/vmusb/sis3316examples/Makefile
	usb/vmusb/sis3316examples/rebootall/Makefile
	usb/vmusb/sis3316examples/ch1_to_ch16_external_trigger_test/Makefile
	usb/vmusb/sis3316examples/internaltrigger_externalgate_dt/Makefile
	usb/vmusb/sis3316examples/intclk_onemodule/Makefile
	usb/vmusb/sis3316examples/FPBUS_threads_allmodules/Makefile
	usb/vmusb/sis3316examples/FPBUS_external_trigger/Makefile
	usb/vmusb/sis3316examples/FPBUS_read_temps/Makefile
	usb/vmusb/sis3316examples/FPBUS_appl_allmods/Makefile
	usb/vmusb/sis3316examples/register_rw/Makefile
	usb/vmusb/sis3316examples/fwupdate/Makefile
	usb/vmusb/sis3316examples/access_rate/Makefile
	usb/vmusb/sis3316examples/rootgui/Makefile
	usb/vmusb/sis3316examples/inventory/Makefile
	usb/vmusb/SIS3316Readout/Makefile
    usb/ccusb/Makefile
    usb/ccusb/ccusb/Makefile 
    usb/ccusb/core/Makefile
    usb/ccusb/daqconfig/Makefile 
    usb/ccusb/ctlconfig/Makefile 
    usb/ccusb/ctlconfig/driver_Makefile
    usb/ccusb/tclpackage/Makefile
    usb/ccusb/tcldrivers/Makefile
    usb/ccusb/driverkit/Makefile
    usb/ccusb/driverkit/Makefile-template
    usb/ccusb/driverkit/CCUSBDriverIncludes
    usb/ccusb/controlpanels/Makefile
    usb/ccusb/devtests/Makefile
    usb/ccusb/tclservertests/Makefile
    usb/caenfwloader/Makefile
    usb/common/Makefile
    usb/common/tcldrivers/Makefile
    usb/common/devices/Makefile
    usb/common/configurableobject/Makefile
    usb/common/slowcontrols/Makefile
    usb/mesytec/Makefile
    usb/mesytec/MCFD16/Makefile
    usb/mesytec/MCFD16/figures/Makefile
    usb/mesytec/MSCF16/Makefile
    usb/mesytec/MSCF16/figures/Makefile
    docbuild/Makefile
    docbuild/images/Makefile
    docbuild/images/callouts/Makefile
    docconfig/Makefile
    examples/Makefile
    examples/ReadNSCLDAQFiles/Makefile
    ddas/config_pixie16api.h
    ddas/Makefile
    ddas/exception/Makefile
    ddas/configuration/Makefile
    ddas/booter/Makefile
    ddas/ddasdumper/Makefile
    ddas/ddasdumper/ddasdumper
    ddas/readout/Makefile
    ddas/readout/DDASFirmwareVersions.txt
    ddas/readout/pixieserver.tcl
    ddas/pixieplugin/Makefile
    ddas/pixieplugin/definitions.tcl
    ddas/qtscope/Makefile
    ddas/qtscope/qtscope
    ddas/utilities/Makefile
    ddas/utilities/simulator/Makefile
    ddas/tclconfig/Makefile
    ddas/docs/Makefile
    ddas/docs/Doxyfile
    ddas/docs/DoxygenLayout.xml
    caendigitizers/Makefile
    caendigitizers/dpp-pha/Makefile
    caendigitizers/dpp-pha/skel/Makefile
    caendigitizers/dpp-psd/Makefile
    caendigitizers/dpp-psd/skel/Makefile
    caendigitizers/caendump/Makefile
    caendigitizers/tweaker/Makefile
    caen-nextgen/Makefile
    caen-nextgen/tclreadout/Makefile
    srs/Makefile
    srs/maps/Makefile
    srs/broker/Makefile
    srs/readout/Makefile
    srs/sorter/Makefile
	mvlc/Makefile
    cookbooks/Makefile
	rust_tools/Makefile
])

AC_OUTPUT
