# Install the pytyhon compiled module
# Were we want it noty where autotools
# wants it:

pymodule_dir = @prefix@/pythonLibs/nscldaq

lib_LTLIBRARIES=libpydaqformat.la

#
#  format.la is a partial? encapsulation of
#  the unified format library
#  for python

libpydaqformat_la_CPPFLAGS=@UFMT_CPPFLAGS@ @PYTHON_CFLAGS@
libpydaqformat_la_LDFLAGS=@UFMT_LDFLAGS@ @PYTHON_LIBS@
libpydaqformat_la_SOURCES=format_Module.cpp \
	format_factory.h format_factory.cpp \
	format_ringitem.h format_ringitem.cpp \
	format_abnormalend.h format_abnormalend.cpp \
	format_scaler.h format_scaler.cpp


install-exec-hook:
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq
	$(INSTALL_SCRIPT) @libdir@/libpydaqformat.so $(pymodule_dir)/daqformat.so
