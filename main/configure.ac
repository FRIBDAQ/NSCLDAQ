#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)


AC_INIT(nscldaq,11.3-039, [scientificsoftware@nscl.msu.edu],[], [http://www.sourceforge.net/projects/nscldaq])



AC_CONFIG_SRCDIR([/base/dataflow/CRingBuffer.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AX_CXX_COMPILE_STDCXX_11([noext])

AC_ARG_ENABLE([docs],
              AS_HELP_STRING([--enable-docs],[Build the documentation]),
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

-  AC_CHECK_PROG(HAVE_XMLTO, [xmlto], [true], [false])
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

AC_CHECK_LIB([X11], [XSetWindowBackground])
AC_CHECK_LIB([Xt], [XtManage])

AM_PATH_CPPUNIT

# Checksums in eventlog require openssl for the digest libs:

AX_CHECK_OPENSSL([AC_MSG_RESULT([found openssl])],[AC_MSG_ERROR([OpenSSL is required but cannot be found])])


#
#   For compatibility with existing AC_Substs:

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


# check to find the size of address
AC_CHECK_SIZEOF([void*])
AS_IF([test "$ac_cv_sizeof_voidp" -eq 8],
    AC_DEFINE([ADDR64],1,[Specifies that memory addresses are 64-bits ]),
     [])
   

# Checks for library functions.

AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 gethostbyaddr gethostbyname gettimeofday inet_ntoa memmove memset regcomp socket strchr strcspn strdup strerror strspn strtol])

AC_CHECK_FUNCS([ftruncate munmap])
AC_FUNC_MMAP
AC_FUNC_STAT

# Bits and pieces of NSCDAQ can do logging via boostlog.  This stuff tells us if that's present and if so,
# how to incorporate it:

AX_BOOST_BASE([], [BOOST_EXTRA_CPPFLAGS="-DBOOST_ALL_DYN_LINK"], [BOOST_EXTRA_CPPFLAGS=""])
AX_BOOST_LOG
AC_SUBST(BOOST_EXTRA_CPPFLAGS)



# Automake:

AM_INIT_AUTOMAKE([foreign subdir-objects])

# Makefile exports:

SOVERSION="11:0:0"





#---------------------------------------------------------------------
#
#  Local tests:
#

# Ensure Python is 3.0  or greater, and the packages/dependencies we need

AM_PATH_PYTHON([3.0], [], [AC_MSG_ERROR([Python 3.0 or higher is needed for NSCLDAQ])])


AC_MSG_RESULT([found Python ${PYTHON_VERSION}])


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
#  VMUSB and CCUSB libraries for use within Tcl drivers.
#
#  Earlier swig versions than 1.3 may well work, however this is the
#  version I used when first successfully building the wrappers
# (ubuntu lucid lynx).
#
AX_PKG_SWIG([1.3], [], [ AC_MSG_ERROR([Swig is required to build nscldaq])])


# We use gengetopt in a few places.  Ideally it's been found by now in the
# path if not we'll need to do the withval thing.. or prompt the user to
# give us a --with-gengetopt-path
#
AX_GENGETOPT

# Tcl/Tk is pervasive in NCLDAQ so we need to have it too:

AX_TCL

#
#  The substs below are for compatibility sake 
#  eventually we should migrate Makefile.am defs to use
#  those in m4/tcl.m4
#
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
#TCL_LDFLAGS="${TK_LIBS} ${TCL_LIBS}"
TCLSH_CMD=${TCLSH} 



#
#  IF systemtcl is yes we're pretty much done.. otherwise
#  we need to configure/build/install libtcl into prefix and then set
#  the flags:

AC_MSG_CHECKING([libtcl++ installed as package (e.g. .deb)])

AC_CHECK_FILE([/usr/include/TCLInterpreterObject.h], [systemtcl="yes"], [systemtcl="no"])

if test $systemtcl = "yes"
then
	LIBTCLPLUS_CFLAGS=""
	LIBEXCEPTION_LDFLAGS="-lException"
	LIBTCLPLUS_LDFLAGS="-ltclPlus ${LIBEXCEPTION_LDFLAGS}"
	AC_MSG_RESULT([Found in  system libraries])
else
  AC_CHECK_FILE([${prefix}/lib/libException.so],[libtclinstalled="yes"],[libtclinstalled="no"])
  AC_CHECK_FILE([${prefix}/lib/libtclPlus.so],[libexceptioninstalled="yes"],[libexceptioninstalled="no"])
  AC_CHECK_FILE([${prefix}/include/TCLInterpreterObject.h],[tclinterpobjinstalled="yes"],[tclinterpobjinstalled="no"])
  if [ [ $libtclinstalled = "no" ] || [ $libexceptioninstalled = "no" ] || [ $tclinterpobjinstalled = "no" ] ]
  then
    AC_CHECK_FILE([${srcdir}/libtcl], [havelibtcldir="yes"], [havelibtcldir="no"])
	  if test $havelibtcldir = "yes"
  	then
      AC_MSG_RESULT([Need to build local tagged copy])
      AC_CHECK_FILE([${srcdir}/libtcl/configure], [havelibtcldir="yes"], [havelibtcldir="no"])
		  (cd ${srcdir}/libtcl; ./configure --prefix=${prefix}; make clean install)
	  else
		  AC_MSG_ERROR([I seem to be missing libtcl, please use tcl++incorp.sh to fill it with a tagged release of libtclplus])
	  fi
  else
    AC_MSG_RESULT([Previously built version found in ${prefix}])
  fi
  LIBTCLPLUS_CFLAGS="-I${prefix}/include"
  LIBEXCEPTION_LDFLAGS="-L${prefix}/lib -lException -Wl,\"-rpath=${prefix}/lib\""
	LIBTCLPLUS_LDFLAGS="-L${prefix}/lib -ltclPlus ${LIBEXCEPTION_LDFLAGS}"

fi
AC_SUBST(LIBTCLPLUS_CFLAGS)
AC_SUBST(LIBTCLPLUS_LDFLAGS)
AC_SUBST(LIBEXCEPTION_LDFLAGS)


# Epics
#
#  This is only probed for and build if --enable-epics-tools=yes is provided
#  at configuration time.  Oncethat's done, AX_EPICS provideas all the defs
#  and the EPICS/BUILD_EPICS_TOOLS enables those in the Makefiles.
#


BUILD_EPICS_TOOLS=no
AC_ARG_ENABLE([epics-tools],
	AS_HELP_STRING([--enable-epics-tools=yes/no], 
	[Build software that requires EPICS (default no)]),
	[BUILD_EPICS_TOOLS="yes"])

if test "$BUILD_EPICS_TOOLS" == "yes"
then

	AX_EPICS
	AM_CONDITIONAL([EPICS], [true])
	AM_CONDITIONAL(BUILD_EPICS_TOOLS, [true])

else
	AM_CONDITIONAL([EPICS], [false])
	AM_CONDITIONAL(BUILD_EPICS_TOOLS, [false])

fi

#
#  The DDAS Software will only be built if --enable-ddas=ddas-install-dir
#  is supplied by the user.  This will attempt to locate the plx software
#  but if that can't be found, --with-plx-sdk can be used to provide it.
#  What we provide to the makefiles:
#
#  BUILD_DDAS   - if this flag is provided that's defined
#                  to allow the top level makefile
#                  to enable the ddas support software build.

#   
#  DDAS_INSTDIR - the value provided to ddas-install-dir
#  PLX_LDFLAGS   - The flags needed to build with the PLX driver support libraries.

BUILD_DDAS="no"
AC_ARG_ENABLE([ddas],
    AS_HELP_STRING([--enable-ddas=ddas-install-dir],
                   [Build the DDAS SUpport software using libs etc in ddas-install-dir]),
    [BUILD_DDAS="yes"
    DDAS_INSTDIR="$enableval"]
)


if test "x$BUILD_DDAS" == "xyes"
then
   AC_MSG_NOTICE([The XIA-DDAS Support software will be built support software in ${DDAS_INSTDIR}])

  AC_DEFINE([USING_DDAS], [1], [DDAS Definitions are in use])

   AC_ARG_WITH([xiaapidir],
        [    -with-xiaapidir=path - Path to the XIAAPI installation.],
        [xiaapidir=$withval])
   if test -n -d ${xiaapidir}
   then
        AC_MSG_ERROR([Specify --with-xiaapidir -t point to the actual installation o\
f the XIA API])
   fi
   PIXIE_API_DIR=${xiaapidir}
#  If the xiaapidir/include/pixie16/pixie16.h exists we're using the XIAAPI_V3
#  or higher stuff:
PIXIE_CPPFLAGS="-I${xiaapidir}/include"
AC_CHECK_FILE([${xiaapidir}/include/pixie16/pixie16.h], [
PIXIE_CPPFLAGS="${PIXIE_CPPFLAGS} -DXIAAPI_V3"
PIXIE_LDFLAGS="-L${xiaapidir}/lib -lPixie16Api -lm -Wl,-rpath=${xiaapidir}/lib"
AC_DEFINE([XIAAPI_VERSION], [3], [Pixie16 version 3 api])
], [
PIXIE_LDFLAGS="-L${xiaapidir}/lib -lPixieSDK -lm -Wl,-rpath=${xiaapidir}/lib"
AC_DEFINE([XIAAPI_VERSION], [2], [Pixie16 version 2 or lower])
])

AC_SUBST(PIXIE_API_DIR)
AC_SUBST(PIXIE_CPPFLAGS)
AC_SUBST(PIXIE_LDFLAGS)

   # Specify the location of the Plx installation. This will default to
   # /usr/opt/plx.
   
   AC_ARG_WITH([plx-sdk],
   [  --with-plx-sdk=path - path to the PLX software installation],
   [PLX_SDK_DIR=$withval], [PLX_SDK_DIR=""])

   if test -z "$PLX_SDK_DIR"
   then
      for d in /usr/opt/plx $(prefix)/../../plx
      do
         if test -z "$PLX_SDK_DIR"
         then

	    # Its probably one of these:

            AC_CHECK_FILE([$d/PlxApi/Library/libPlxApi.a],
	    [PLX_SDK_DIR="$d"],[PLX_SDK_DIR=""])
	    
	    AC_CHECK_FILE([$d/PlxApi/Library/PlxApi.so],
	    [PLX_SDK_DIR="$d"],[PLX_SDK_DIR=""])
	    
         fi
      done
   fi
   
   #  If PLX_SDK_DIR is still "" then we toss an error:
   
   if test -z "$PLX_SDK_DIR"
   then
      AC_MSG_FAILURE([Could not locate the PLX SDK directory, use --with-plx-sdk to help me])
   fi

   PLX_CPPFLAGS="-I${PLX_SDK_DIR}/Include -I${PLX_SDK_DIR}/PlxApi"

   # Use the shared library PlxApi.so (plx 8+) otherwise libPlxApi.a:
   
   AC_CHECK_FILE([${PLX_SDK_DIR}/PlxApi/Library/PlxApi.so],
   [PLX_LDFLAGS="-L${PLX_SDK_DIR}/PlxApi/Library -l:PlxApi.so -Wl,-rpath=${PLX_SDK_DIR}/PlxApi/Library"],
   [PLX_LDFLAGS="-L${PLX_SDK_DIR}/PlxApi/Library -lPlxApi"])

   AC_SUBST(PLX_SDK_DIR)
   AC_SUBST(PLX_CPPFLAGS)
   AC_SUBST(PLX_LDFLAGS)

   # Now export all the variables to the makefile:
   
   AM_CONDITIONAL([BUILD_DDAS], [true])
   AC_SUBST(DDAS_INSTDIR)
else
   AM_CONDITIONAL([BUILD_DDAS],[false])
fi

#
#  The SBS software will only be built if
#  --enable-sbs=kernel-source-dir was supplied.
#  In that case, the configured kernel source is assumed to be in 
#  kernel-source-dir and that will be used to compile the device driver
#  as well.  
#  The default is to _not_ build the sbs stuff.
#

BUILD_SBS="no"

AC_ARG_ENABLE([sbs], 
	      AS_HELP_STRING([--enable-sbs=kernel-source-dir], [Build SBS software using kernel-source-dir as directory with configured kernel source]),
	      [BUILD_SBS="yes"
	       KERNEL_SOURCE_DIR="$enableval"])




if test "$BUILD_SBS" == "yes"
then
  AC_MSG_NOTICE([Building SBS software with kernel source at: $KERNEL_SOURCE_DIR])
else
  AC_MSG_NOTICE([SBS software support not selected!])
fi


#
#  If --enable-usb is turned on, we build the VM/CC-usb support directories
#
enableusb="no"
AC_ARG_ENABLE([usb],
	AS_HELP_STRING([--enable-usb], [Build support for CC/VM usb data taking]),
	[enableusb="yes"])

#  If building is turned on we need to locate the headers and the includes.


if test "$enableusb" == "yes"
then
	AX_LIBUSB
	AM_CONDITIONAL([BUILD_USB], [true])
else
	AM_CONDITIONAL([BUILD_USB], [false])
fi

##
#  Check for zmq presence:
#

AC_SEARCH_LIBS([zmq_init], [zmq], [result=$ac_cv_search_zmq_init], [AC_MSG_ERROR([The zeromq library was not found and is now required for nscldaq])])
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
AC_CHECK_HEADERS([algorithm zmq.hpp],[], [AC_MSG_ERROR([The zeromq C++ header (zmq.hpp) is missing and is required for nscldaq])],
			    [#include <algorithm>])
AC_LANG_POP


##
# check for presence of Python zmq package

#AC_PYTHON_MODULE([zmq])
AX_PYTHON_MODULE([zmq])

#----------------------------------------------------------------------------

# Exports to the makefiles:

AC_DEFINE(__STDC_LIMIT_MACROS, [], [Needed to define UINT64_C])
AC_DEFINE(__STDC_CONSTANT_MACROS, [], [Needed to define UINT64_C])

AC_SUBST(SOVERSION)

AC_SUBST(TCL_FLAGS)
AC_SUBST(TCL_LDFLAGS)
AC_SUBST(TCLSH_CMD)






AC_SUBST(CPPUNIT_LDFLAGS)

#
#  Threading is hard coded to -pthread now.
#
THREADCXX_FLAGS="-pthread"
THREADC_FLAGS="-pthread"
THREADLD_FLAGS="-lpthread -lrt"

AC_SUBST(THREADCXX_FLAGS)
AC_SUBST(THREADC_FLAGS)
AC_SUBST(THREADLD_FLAGS)

# SBS stuff:

AC_SUBST(KERNEL_SOURCE_DIR)
AM_CONDITIONAL([BUILD_SBS_DRIVER], [test "$BUILD_SBS" == "yes"])


#
# Ensure that resources for testing are accessible
# for out of tree builds
#
AC_CONFIG_LINKS([utilities/filter/run-0000-00.evt:utilities/filter/run-0000-00.evt])


#---------------------------------------------------------------------------
# Generate the following from their .in's (note that Automake takes
# *.am -> *.in.
#

# Note removed utilities/scalerdisplay in favor of utilities/newscaler
# Sept 25, 2014

AC_CONFIG_FILES([Makefile
		nscldaq
		daqsetup.bash
    base/Makefile
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
    caenfw-common/Makefile
    servers/Makefile
    servers/tclserver/Makefile
    servers/portmanager/Makefile
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
    utilities/IO/Makefile
    utilities/FormattedIO/Makefile
    utilities/newscaler/Makefile
    utilities/newscaler/ScalerDisplay.tcl
    utilities/scalersum/Makefile
    utilities/reglom/Makefile
    utilities/scalerReader/Makefile
    utilities/evtCreator/Makefile
    epics/chanlog/Makefile
    epics/controlpush/Makefile
    epics/epicsdisplay/Makefile
    epics/epicsdisplay/epicsdisplay
    epics/Makefile
    daq/Makefile
    daq/format/Makefile
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
    daq/actions/Makefile
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
    ddas/Makefile ddas/readout/Makefile
    ddas/readout-extclock/Makefile
    ddas/pixieplugin/Makefile
    ddas/pixieplugin/definitions.tcl
    cookbooks/Makefile
    ddascook/Makefile    
    ])

AC_OUTPUT
