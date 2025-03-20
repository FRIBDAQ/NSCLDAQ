bin_PROGRAMS = boot boot_extcl
lib_LTLIBRARIES = libSystemBooter.la
SHAREDIR=@prefix@/share/ddasreadout
include_HEADERS = SystemBooter.h

libSystemBooter_la_SOURCES = SystemBooter.cpp
libSystemBooter_la_CPPFLAGS = -I@top_srcdir@/ddas/configuration \
	@PIXIE_CPPFLAGS@ \
	-I@top_builddir@/ddas \
	-I@top_srcdir@/ddas/exception @LIBTCLPLUS_CFLAGS@ \
	-DFIRMWARE_PATH=\"@firmwaredir@\"
libSystemBooter_la_LIBADD = \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@PIXIE_LDFLAGS@  \
	@top_builddir@/ddas/exception/libDDASException.la @LIBEXCEPTION_LDFLAGS@

boot_SOURCES = boot.cpp
boot_CPPFLAGS = -I@srcdir@ -I@top_builddir@/ddas \
	-I@top_srcdir@/ddas/configuration \
	@PIXIE_CPPFLAGS@ \
	-DFIRMWARE_FILE=\"${SHAREDIR}/DDASFirmwareVersions.txt\"
boot_LDADD = @builddir@/libSystemBooter.la \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@PIXIE_LDFLAGS@ \
	 -ldl

boot_extcl_SOURCES = boot.cpp
boot_extcl_CPPFLAGS = -I@srcdir@ -I@top_srcdir@/ddas/configuration \
	@PIXIE_CPPFLAGS@ \
	-DFIRMWARE_FILE=\"${SHAREDIR}/DDASFirmwareVersions_ExtClock.txt\"
boot_extcl_LDADD = @builddir@/libSystemBooter.la \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@PIXIE_LDFLAGS@ \
	 -ldl

# unit tests are disabled until they can be updated for the API transition

#noinst_PROGRAMS = unittests
#unittests_SOURCES = TestRunner.cpp SystemBooterTest.cpp
#unittests_CPPFLAGS = -I@top_builddir@/ddas \
	-I@top_srcdir@/ddas/configuration \
	-I@top_srcdir@/ddas/xiatest  \
	@CPPUNIT_CFLAGS@ 
#unittests_LDADD = @builddir@/libSystemBooter.la \
	@top_builddir@/ddas/configuration/libConfiguration.la \
	@top_builddir@/ddas/xiatest/libTestPixie16.la \
	@CPPUNIT_LIBS@

#TESTS = unittests
