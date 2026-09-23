lib_LTLIBRARIES = libcaenfwloadCommon.la

libcaenfwloadCommon_la_SOURCES = CAENComm_Common.cpp \
	CFASegment.c \
	cvupgrade.c \
	flash.c

noinst_HEADERS = CFASegment.h flash.h CAENComm.h


install-exec-local:
	$(mkinstalldirs) @prefix@/etc/caenfw
	for f in @top_srcdir@/caenfw-common/*.txt; do \
		test "`basename $$f`" = CMakeLists.txt || $(INSTALL_DATA) $$f @prefix@/etc/caenfw; \
	done



EXTRA_DIST = CVupgrade_params_Digitizers_PLL.txt \
CVupgrade_params_Digitizers.txt \
CVupgrade_params_SY2791.txt \
CVupgrade_params_V1190_V1290.txt \
CVupgrade_params_V1495_USER.txt \
CVupgrade_params_V1495_VME.txt \
CVupgrade_params_V6533.txt	\
V1720_template.txt \
V1721_template.txt \
V1724_template.txt \
V1731_template.txt \
V1740_template.txt \
V1751_template.txt \
caenfwloaders.xml
