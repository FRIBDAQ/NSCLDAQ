lib_LTLIBRARIES = libpixie16.la
SHAREDIR=@prefix@/share/ddasreadout

libpixie16_la_SOURCES = Pixie16Package.cpp \
	CTclCommand.h CTclCommand.cpp	   \
	InitSystem.h InitSystem.cpp	   \
	Inventory.h Inventory.cpp	   \
	Release.h Release.cpp		   \
	Boot.h Boot.cpp			   \
	ReadChanPar.h ReadChanPar.cpp 	   \
	ReadModPar.h ReadModPar.cpp	   \
	WriteChanPar.h WriteChanPar.cpp    \
	WriteModPar.h WriteModPar.cpp	   \
	SaveParams.h SaveParams.cpp	   \
	RestoreParams.h RestoreParams.cpp  \
	IsActive.h IsActive.cpp            \
	CAdjustOffsets.h CAdjustOffsets.cpp

libpixie16_la_CXXFLAGS = @TCL_CPPFLAGS@    \
	-I@top_srcdir@/ddas/configuration  \
	-I@top_builddir@/ddas              \
	@PIXIE_CPPFLAGS@                   \
	-I@top_srcdir@/ddas/exception \
	@LIBTCLPLUS_CFLAGS@ \
	-g                                 \
	-DFIRMWARE_FILE=\"${SHAREDIR}/DDASFirmwareVersions.txt\"

libpixie16_la_LDFLAGS = @top_builddir@/ddas/configuration/libConfiguration.la 	\
	@PIXIE_LDFLAGS@ @TCL_LIBS@                              		\
	@top_builddir@/ddas/exception/libDDASException.la 	\
	@LIBEXCEPTION_LDFLAGS@ \
	-Wl,-rpath=@libdir@ -g

install-exec-hook:
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/pixie16
	$(INSTALL_PROGRAM) @libdir@/libpixie16.so @prefix@/TclLibs/pixie16
	$(INSTALL_PROGRAM) @srcdir@/tkNscope.tcl @bindir@/tkNscope
	$(INSTALL_DATA)    @srcdir@/Pixie16Server.tcl @prefix@/TclLibs/pixie16
	echo package ifneeded pixie16 1.1 [list load [file join \$$dir libpixie16.so]] > @prefix@/TclLibs/pixie16/pkgIndex.tcl
	echo package ifneeded pixieserver 1.0 [list source [file join \$$dir Pixie16Server.tcl]] >> @prefix@/TclLibs/pixie16/pkgIndex.tcl

EXTRA_DIST=Pixie16Server.tcl tkNscope.tcl
