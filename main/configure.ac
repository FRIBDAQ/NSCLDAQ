#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.61)

AC_INIT(nscldaq, 12.1-022, [daqhelp@nscl.msu.edu], [], [https://github.com/FRIBDAQ/NSCLDAQ])

AC_CONFIG_SRCDIR([/base/dataflow/CRingBuffer.h])
AC_CONFIG_HEADER([config.h])
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
AC_PROG_LIBTOOL
AX_CXX_COMPILE_STDCXX_11([noext])

# Set parallel build for incorps:

AC_ARG_WITH([incorp-build-cores],
	AC_HELP_STRING([--with-incorp-build-cores=N], [Number of cores used to build incorporated packages (default=1)]),
	[incorp_cores=$withval],
	[incorp_cores=1])
AC_MSG_NOTICE([Building incorporated packages with ${incorp_cores} cores])

# Is git installed? Used to check incorp'd module versions to export to
# VERSION in the install directory.

AC_CHECK_PROG(GIT, [git], [git], AC_MSG_ERROR([git is required for NSCLDAQ incorporated library management]))
AC_SUBST(GIT) # Only if we have it

# MPI defined/usable to build MPI chunks:

AC_LANG_PUSH(C++)
AX_MPI([AM_CONDITIONAL([MPI], true) AC_DEFINE([HAVE_MPI], 1, [MPI exists])], [AM_CONDITIONAL([MPI], false)])
AC_LANG_POP

# Pandoc is needed for printed versions of the logbook:

AC_CHECK_PROG(HAVE_PANDOC, [pandoc], [true], [false])
if test "$HAVE_PANDOC" == "false"
then
	AC_MSG_FAILURE([pandoc is required to produce PDF output from the logbook utilities])
	
fi

#  CAEN Digitizer support is optional:

AC_ARG_ENABLE([caen-digitizer-support],
	AC_HELP_STRING([--enable-caen-digitizer-support], [Build support for CAEN Digitizers]),
	[enable_caen=$enableval],
	[enable_caen=no])

if test "x$enable_caen" = "xyes"
then
	AC_ARG_WITH([caen-digitizer-libroot],
	AC_HELP_STRING([ --with-caen-digitizer-libroot], [Root directory for CAEN Digitizer/VME/Comm library installation]),
	[caen_root=$withval], [caen_root=""])

	if test "xx$caen_root" != "xx"
	then
		AC_MSG_NOTICE([Enabling CAEN digitizer DPP support])
		CAENCCFLAGS=-I$caen_root/include
		CAENLDFLAGS="-L$caen_root/lib -lCAENDigitizer -lCAENVME -lCAENComm -Wl,-rpath=$caen_root/lib"
		AM_CONDITIONAL([CAENSUPPORT], [true])
		AC_SUBST(CAENCCFLAGS)
		AC_SUBST(CAENLDFLAGS)
	else
		AM_CONDITIONAL([CAENSUPPORT], [false])
		AC_MSG_ERROR([using --enable-caen-digitizer-support also requires --with-caen-dgitizer-libroot to specifiy where the CAEN libraries/headers are])
	fi
else
	AM_CONDITIONAL([CAENSUPPORT], [false])
fi

#  CAEN Nextgen digitizer support is also optional  the CAEN support
#  libraries are awful in that they require installation in
#  specific locations:

AC_ARG_ENABLE([caen-nextgen],
	[AS_HELP_STRING([--enable-caen-nextgen], [To enable support for CAEN Nextgen digitizers.])],
	[AC_MSG_CHECKING([Checking for CAEN FElib installed])
	AC_CHECK_FILE([/usr/local/include/CAEN_FELib.h], 
        [AC_MSG_RESULT([Found header])],
        [AC_MSG_ERROR([cannot find /usr/local/CAEN_FELib.h - install CAEN Nextgen digitizer support libraries])])
	AC_CHECK_FILES([/usr/local/lib/libCAEN_Dig2.so /usr/local/lib/libCAEN_FELib.so],
        [AC_MSG_RESULT([Found CAEN FELib library files])],
        [AC_MSG_ERROR([cannot find the /usr/local/lib/{libCAEN_Dig2.so, libCAEN_FELib.so files - install CAEN nextgen digitizer support libraries}]
        )])
	enable_caennextgen="true"], [enable_caennextgen="false"])  

if test "${enable_caennextgen}" = "true"
then
	AC_MSG_NOTICE([Enabled support for CAEN Nextgen digitizers])

	#  Set the make file variables:

	CAENNG_CPPFLAGS="-I/usr/local/include"
	CAENNG_LDFLAGS="-L/usr/local/lib -lCAEN_FELib -lCAEN_Dig2"
	AC_SUBST([CAENNG_CPPFLAGS])
	AC_SUBST([CAENNG_LDFLAGS])
	AM_CONDITIONAL([CAENNEXTGEN], [true])
else 
     AC_MSG_NOTICE([Disabled support for  CAEN Nextgen digitizers])
     AM_CONDITIONAL([CAENNEXTGEN], [false])
fi

# CAEN Nextgen digitizers, if used require jsoncpp:

PKG_CHECK_MODULES([JSON], [jsoncpp])

# Documentation

AC_ARG_ENABLE([docs],
              AS_HELP_STRING([--enable-docs], [Build the documentation]),
              [], [enable_docs=no])

if test "x$enable_docs" == "xyes"
then
	AC_MSG_NOTICE([Documentation enabled])
	
	AC_CHECK_PROG(HAVE_DOCBOOK2PDF, [docbook2pdf], [true], [false])	
	if test "$HAVE_DOCBOOK2PDF" == "false"
	then
		AC_MSG_FAILURE([docbook2pdf is required to build documentation but was not found])
	else
		AC_SUBST(DOCBOOK2PDF,[docbook2pdf])
	fi

	AC_CHECK_PROG(HAVE_DOCBOOK2HTML, [docbook2html], [true], [false])	
	if test "$HAVE_DOCBOOK2HTML" == "false"
	then
		AC_MSG_FAILURE([docbook2html is required to build documentation but was not found])
	else
		AC_SUBST(DOCBOOK2HTML,[docbook2html])
	fi

	AC_CHECK_PROG(HAVE_XMLTO, [xmlto], [true], [false])	
	if test "$HAVE_XMLTO" == "false"
	then
		AC_MSG_FAILURE([xmlto is required to build documentation but was not found])
	else
		AC_SUBST(DOCBOOK2MAN,["xmlto man"])
	fi

	AC_CHECK_PROG(HAVE_MANDB, mandb, [true], [false])	
	if test "$HAVE_MANDB" == "false"
	then
		AC_MSG_FAILURE([mandb is required to build documentation but was not found])
	else
		AC_SUBST(MANDB,[mandb])
	fi
	AM_CONDITIONAL([BUILD_DOCS],[true])
else
	AM_CONDITIONAL([BUILD_DOCS],[false])
	AC_MSG_NOTICE([Documentation disabled])
fi

# DDAS documentation
# DDAS docs are written in Doxygen, so if enabled check to make sure that
# Doxygen and dot (for graphs) are installed

AC_ARG_ENABLE([ddas-docs],
              AS_HELP_STRING([--enable-ddas-docs], [Build the DDAS documentation]),
              [], [enable_ddas_docs=no])

if test "$enable_ddas_docs" = "yes"
then
	AC_MSG_NOTICE([DDAS documentation enabled])

  	AC_CHECK_PROG(HAVE_DOXYGEN, [doxygen], [true], [false])
  	if test "$HAVE_DOXYGEN" = "false"
	then
		AC_MSG_FAILURE([Doxygen is required to build the DDAS documentation but was not found])
  	else
    		AC_SUBST(DOXYGEN,[doxygen])
  	fi

  	AC_CHECK_PROG(HAVE_DOT, [dot], [true], [false])
  	if test "$HAVE_DOT" = "false"
  	then
		AC_MSG_FAILURE([dot is required by Doxygen to build the DDAS documentation but was not found])
  	fi

  	AM_CONDITIONAL([BUILD_DDAS_DOCS],[true])
else
	AM_CONDITIONAL([BUILD_DDAS_DOCS],[false])
  	AC_MSG_NOTICE([DDAS documentation disabled])
fi

AC_CHECK_LIB([X11], [XSetWindowBackground])
AC_CHECK_LIB([Xt], [XtManage])

AM_PATH_CPPUNIT

# Checksums in eventlog require OpenSSL for the digest libs:

AX_CHECK_OPENSSL([AC_MSG_RESULT([Found OpenSSL])], [AC_MSG_ERROR([cannot find OpenSSL installation required by NSCLDAQ])])

# For compatibility with existing AC_Substs:

CPPUNIT_LDFLAGS="${CPPUNIT_LIBS}"

AC_SUBST([AM_CXXFLAGS], [-fno-strict-aliasing])

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AX_CHECK_COMPILE_FLAG([-Wno-error=date-time], [NODATEWARN="-Wno-error=date-time"])
AC_SUBST(NODATEWARN)

# Check to find the size of address

AC_CHECK_SIZEOF([void*])
AS_IF([test "$ac_cv_sizeof_voidp" -eq 8], AC_DEFINE([ADDR64], 1, [Specifies that memory addresses are 64-bits ]), [])
   
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
		[enable_prompt=$enableval], [enable_prompt="yes"] )

if test "x$enable_prompt" = "xyes"
then
	PROMPT_BUSY="-DVMUSB_PROMPT_BUSY_WORKAROUND"
fi

AC_ARG_ENABLE([vmusb-end-event],
	AS_HELP_STRING([--disable-vmusb-end-event], 
	[Turn off the workaround for the VMUSB not always giving an end of event pulse]),
	[enable_eoe=$enableval], [enable_eoe="yes"])

if test "x$enable_eoe" = "xyes"
then
	END_EVENT="-DVMUSB_END_OF_EVENT_WORKAROUND"
fi	

AC_ARG_ENABLE([vmusb-idle-busy],
	AS_HELP_STRING([--disable-vmusb-idle-busy],
	[Turn off the workaround for the VMUSB not asserting Busy when the run ends]),
	[enable_idlebusy=$enableval], [enable_idlebusy="yes"])

if test "x$enable_idlebusy" = "xyes"
then
	IDLE_BUSY="-DVMUSB_IDLE_BUSY_WORKAROUND"
fi

#  The VMUSB_WORKAROUNDS variable will have all of the conditionals selected:

VMUSB_WORKAROUNDS="${PROMPT_BUSY} ${END_EVENT} ${IDLE_BUSY}"
AC_SUBST(VMUSB_WORKAROUNDS)

	
# Ensure swig is installed: -- this is used to generate wrappers for the
# VMUSB and CCUSB libraries for use within Tcl drivers.
#
# Earlier swig versions than 1.3 may well work, however this is the
# version I used when first successfully building the wrappers
# (Ubuntu Lucid Lynx).

AX_PKG_SWIG([1.3], [], [ AC_MSG_ERROR([swig is required to build NSCLDAQ])])

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
    if test $tclname != "-lieee" 
    then 
        TCL_LDFLAGS="$TCL_LDFLAGS $tclname"
    fi
done
TCLSH_CMD=${TCLSH}

# Incorporate and build tcl++:

AC_CHECK_FILE([${srcdir}/libtcl/configure.ac],
		[AC_MSG_NOTICE([Using previously incorprated tcl++ from the source tree at ${srcdir}/libtcl])
		cp -r ${srcdir}/libtcl .],
		[AC_MSG_NOTICE([Incorporating tcl++ for you])
		${srcdir}/tcl++incorp])
AC_MSG_NOTICE([Building and installing tcl++])

(cd libtcl; ./configure --prefix=${prefix}; make clean ; make -j ${incorp_cores} all ; make install)
result=$?
AS_VAR_IF([result], [0], [], AC_MSG_ERROR([Failed to build and install tcl++ with result: ${result}]))
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


(cd ${srcdir}/tclhttpd3.5.1; ./configure --prefix=${prefix}/share/tclhttpd --enable-gcc --with-tclinclude=$tcl_incdir --with-tcl=$tcl_configdir)
(cd ${srcdir}/tclhttpd3.5.1; make clean ; make -j ${incorp_cores} all ; make install)

# Epics
#
#  This is only probed for and build if --enable-epics-tools=yes is provided
#  at configuration time.  Oncethat's done, AX_EPICS provideas all the defs
#  and the EPICS/BUILD_EPICS_TOOLS enables those in the Makefiles.
#

AC_ARG_ENABLE([epics-tools],
	AS_HELP_STRING([--enable-epics-tools], [Build software that requires EPICS (default no)]),
	[], [enable_epics_tools=no])

if test "$enable_epics_tools" = "yes"
then
	AX_EPICS
	AM_CONDITIONAL([EPICS], [true])
	AM_CONDITIONAL(BUILD_EPICS_TOOLS, [true])

else
	AM_CONDITIONAL([EPICS], [false])
	AM_CONDITIONAL(BUILD_EPICS_TOOLS, [false])
fi


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
	cp -r ${srcdir}/unifiedformat .],
	[AC_MSG_NOTICE([Incorporating unified format for you])
	${srcdir}/unifiedfmt-incorp.sh])
rm -rf unifiedformat/build
AS_MKDIR_P(unifiedformat/build)	
AC_MSG_NOTICE([Building unified format])
(cd unifiedformat/build; cmake .. -DCMAKE_INSTALL_PREFIX=${prefix}/unifiedformat; make clean; make -j ${incorp_cores} all ; make install)
result=$?
AS_VAR_IF([result], [0], [], AC_MSG_ERROR([Failed to build and install unified format library with result: ${result}]))
UFMT_CPPFLAGS="-I${prefix}/unifiedformat/include"
UFMT_LDFLAGS="-L${prefix}/unifiedformat/lib -lNSCLDAQFormat -lV10Format -lV11Format -lV12Format -lAbstractFormat -Wl,-rpath=${prefix}/unifiedformat/lib"

AC_SUBST(UFMT_CPPFLAGS)
AC_SUBST(UFMT_LDFLAGS)

# Check that the DDAS format library has been incorporated into the
# source tree and configure/build it into $prefix/ddasformat.
	
AC_CHECK_FILE([${srcdir}/ddasformat/CMakeLists.txt],
	[AC_MSG_NOTICE([Using previously incorprated ddasformat from the source tree at ${srcdir}/ddasformat])
	cp -r ${srcdir}/ddasformat .],
	[AC_MSG_NOTICE([Incorporating ddasformat for you])
	${srcdir}/ddasfmt-incorp.sh])
rm -rf ddasformat/build
AS_MKDIR_P(ddasformat/build)
AC_MSG_NOTICE([Building DDAS format])
(cd ddasformat/build; cmake .. -DCMAKE_INSTALL_PREFIX=${prefix}/ddasformat; make clean; make -j ${incorp_cores} all ; make install)
result=$?
AS_VAR_IF([result], [0], [], AC_MSG_ERROR([Failed to build and install ddasformat with result: ${result}]))
DDASFMT_CPPFLAGS="-I${prefix}/ddasformat/include"
DDASFMT_LDFLAGS="-L${prefix}/ddasformat/lib -lDDASFormat -Wl,-rpath=${prefix}/ddasformat/lib"
AC_SUBST(DDASFMT_CPPFLAGS)
AC_SUBST(DDASFMT_LDFLAGS)

# The DDAS Software will only be built if --enable-ddas=yes is supplied by
# the user. What we provide to the Makefiles:
#
# BUILD_DDAS       - If this flag is provided that's defined to allow the top
#                    level Makefile to enable the ddas support software build.
# PIXIE_API_DIR    - Installation directory of the XIA Pixie API
# PIXIE_CPPFLAGS   - Compiler flags for Pixie API
# PIXIE_LDFLAGS    - Linker flags for Pixie API
# firmwaredir      - Pixie firmware installation directory
# dspdir           - Pixie DSP load image installaion directory

# ROOTSYS          - ROOT installation to compile e.g. ddasdumper against
# ROOT_CFLAGS      - ROOT cflags (automatically generated from ROOT config)
# ROOT_LDFLAGS     - ROOT libraries (automatically generated from ROOT config)

AC_ARG_ENABLE([ddas],
	AS_HELP_STRING([--enable-ddas], [Build the DDAS support software (default no)]),
	[], [enable_ddas=no])

if test "$enable_ddas" = "yes"
then
	AC_MSG_NOTICE([The XIA-DDAS support software will be built])
	AC_DEFINE([USING_DDAS], [1], [DDAS support enabled])

	AC_ARG_WITH([xiaapidir],
	[  --with-xiaapidir=path - Path to the XIA API installation. Required if DDAS support is enabled.],
	[xiaapidir=$withval], [AC_MSG_ERROR([use --with-xiaapidir to point to the actual installation of the XIA API])])

	# Check that the XIA API installation directory is correct by looking
	# for the pixie16.h header there. If found, define vars; if not,
	# print an error message and exit. Note that this version supports
	# only XIA API major version 4 which incorporates "legacy mode"
	# as a supported option as of release 4.x.y and later:
	
	AC_CHECK_FILE([${xiaapidir}/include/pixie16/pixie16.h],
	[PIXIE_LDFLAGS="-L${xiaapidir}/lib -lPixie16Api -lm -Wl,-rpath=${xiaapidir}/lib"],
	[AC_MSG_ERROR([invalid XIA API installation directory ${xiaapidir}, use --with-xiaapidir to point to the correct one (this version of NSCLDAQ supports only XIA API major version 3+)])])

	PIXIE_API_DIR=$xiaapidir
	PIXIE_CPPFLAGS="-I${xiaapidir}/include"

	AC_SUBST(PIXIE_API_DIR)
	AC_SUBST(PIXIE_CPPFLAGS)
	AC_SUBST(PIXIE_LDFLAGS)

	# Also need to know where the Pixie firmware is installed:

	AC_ARG_WITH([firmwaredir],
		[  --with-firmwaredir=path - Specify path to firmware directory tree. Required if DDAS support is enabled.],
		[firmwaredir=$withval])

	if test x$firmwaredir = x
	then
		AC_MSG_ERROR([specifying a firmware install directory using --with-firmwaredir is a mandatory option if DDAS support is enabled])
	fi

	AC_SUBST(firmwaredir)

	# Establish the dsp load image directory. These files are also not
	# redistributable so the user must have licensed them and installed
	# them somewhere.

	AC_ARG_WITH([dspdir],
		[  --with-dspdir=path - Specify path to the DSP software directory tree. Required if DDAS support is enabled.],
		[dspdir=$withval])

	if test x$dspdir = x
	then
		AC_MSG_ERROR([specifying a firmware install directory using --with-dspdir is a mandatory option if DDAS support is enabled])
	fi

	AC_SUBST(dspdir)

	# Check for ROOT. Use rootcling to add ROOT flags and linker options
	# to the Makefile macro set:

	rootdir=$ROOTSYS
	AC_ARG_WITH([rootsys],
		[  --with-rootsys=path - Path to the ROOT installation to use. Required if DDAS support is enabled.],
		[rootdir=$withval])
	
	if test x$rootdir = x 
	then
		AC_MSG_ERROR([use --with-rootsys to specify where ROOT is installed])
	fi
	
	if test ! -d $rootdir
	then
		AC_MSG_ERROR([the specified ROOTSYS directory $rootdir does not exist!])
	fi

	ROOTSYS=$rootdir
	ROOT_CFLAGS=`$rootdir/bin/root-config --cflags`
	ROOT_LD1=`$rootdir/bin/root-config --glibs`
	ROOT_LD2=`$rootdir/bin/root-config --ldflags`
	ROOT_RPATH="-Wl,-rpath=`$rootdir/bin/root-config --libdir`"

	ROOT_LDFLAGS="$ROOT_LD1 $ROOT_LD2 $ROOT_RPATH"

	AC_SUBST(ROOTSYS)
	AC_SUBST(ROOT_CFLAGS)
	AC_SUBST(ROOT_LDFLAGS)

	#  Now export all the variables to the Makefile:

	AM_CONDITIONAL([BUILD_DDAS], [true])
else
	AM_CONDITIONAL([BUILD_DDAS],[false])
fi

# The SBS software will only be built if --enable-sbs was supplied.
# In that case, the configured kernel source path is set using
# --with-sbs-kernel-source-dir and that will be used to compile the
# device driver as well. The default is to _not_ build the sbs stuff.

AC_ARG_ENABLE([sbs],
	AS_HELP_STRING([--enable-sbs], [Build SBS software with a configured kernel source]),
	[],
	[enable_sbs=no])

if test "$enable_sbs" = "yes"
then
	AC_ARG_WITH([sbsdir],
		[  --with-sbsdir=path - Specify path to the configured SBS kernel directory. Required if SBS support is enabled.],
		[sbsdir=$withval])

	if test x$sbsdir = x
	then
		AC_MSG_ERROR([specifiying an SBS kernel directory using --with-sbsdir is a mandatory option if SBS kernel support is enabled])
	fi
	
	if test ! -d $sbsdir
	then
		AC_MSG_ERROR([the specified SBS directory $sbsdir does not exist!])
	fi

	KERNEL_SOURCE_DIR=$sbsdir
	AC_SUBST(KERNEL_SOURCE_DIR)
	AM_CONDITIONAL([BUILD_SBS_DRIVER], [true])
	AC_MSG_NOTICE([Building SBS software with kernel source at: $KERNEL_SOURCE_DIR])
	
else
	AM_CONDITIONAL([BUILD_SBS_DRIVER], [false])
	AC_MSG_NOTICE([SBS software is not enabled.])
fi

# If --enable-usb is turned on, we build the VM/CC-usb support directories

AC_ARG_ENABLE([usb],
	AS_HELP_STRING([--enable-usb], [Build support for CC/VM usb data taking]),
	[],
	[enable_usb=no])

#  If building is turned on we need to locate the headers and the includes.

if test "$enable_usb" = "yes"
then
	AX_LIBUSB
	PKG_CHECK_MODULES([libusb_1_0], [libusb-1.0])
	PKG_CHECK_MODULES([libusb], [libusb])
	AM_CONDITIONAL([BUILD_USB], [true])
else
	AM_CONDITIONAL([BUILD_USB], [false])
fi

# Check for libsqlite3:

PKG_CHECK_MODULES([SQLITE3], [sqlite3])

##
#  Check for zmq presence:
#

AC_SEARCH_LIBS([zmq_init], [zmq], [result=$ac_cv_search_zmq_init], [AC_MSG_ERROR([the ZeroMQ library was not found and is now required for NSCLDAQ])])

if test $result = "none required"
then
	ZMQ_LDFLAGS=""
else
	ZMQ_LDFLAGS=$result
fi

AC_SUBST(ZMQ_LDFLAGS)

##
# need the zmq c++ bindings
#  note in some c++11's we need to include <algorithm> to include zmq.hpp:
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
    cookbooks/Makefile
])

AC_OUTPUT
