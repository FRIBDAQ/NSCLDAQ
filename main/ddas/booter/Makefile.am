bin_PROGRAMS = boot boot_extcl
lib_LTLIBRARIES = libSystemBooter.la
SHAREDIR=@prefix@/share/ddasreadout
include_HEADERS = SystemBooter.h

libSystemBooter_la_SOURCES = SystemBooter.cpp
libSystemBooter_la_CPPFLAGS = -I@top_srcdir@/ddas/configuration \
	-I@top_builddir@/ddas \
	-I@top_srcdir@/ddas/exception \
	@LIBTCLPLUS_CFLAGS@ \
	@PIXIE_CPPFLAGS@ \
	-DFIRMWARE_PATH=\"@firmwaredir@\"
libSystemBooter_la_LIBADD = \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@top_builddir@/ddas/exception/libDDASException.la \
	@PIXIE_LDFLAGS@  \
	@LIBEXCEPTION_LDFLAGS@

boot_SOURCES = boot.cpp
boot_CPPFLAGS = -I@srcdir@ -I@top_builddir@/ddas \
	-I@top_srcdir@/ddas/configuration \
	@PIXIE_CPPFLAGS@
boot_LDADD = @builddir@/libSystemBooter.la \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@PIXIE_LDFLAGS@ \
	-ldl

boot_extcl_SOURCES = boot.cpp
boot_extcl_CPPFLAGS = -I@srcdir@ -I@top_srcdir@/ddas/configuration \
	@PIXIE_CPPFLAGS@
boot_extcl_LDADD = @builddir@/libSystemBooter.la \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@PIXIE_LDFLAGS@ \
	-ldl

##
# unittests
#

check_PROGRAMS = unittests
unittests_SOURCES = TestRunner.cpp SystemBooterTest.cpp
unittests_CPPFLAGS = -I@top_builddir@/ddas \
	-I@top_srcdir@/ddas/configuration \
	@PIXIE_CPPFLAGS@ @CPPUNIT_CFLAGS@ 
unittests_LDADD = @builddir@/libSystemBooter.la \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@PIXIE_LDFLAGS@ @CPPUNIT_LIBS@

TESTS = unittests
