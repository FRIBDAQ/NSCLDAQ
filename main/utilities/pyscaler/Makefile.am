# Install the pytyhon compiled module
# Were we want it noty where autotools
# wants it:

pymodule_dir = @prefix@/pythonLibs/nscldaq

lib_LTLIBRARIES=libformat.la

#
#  format.la is a partial? encapsulation of
#  the unified format library
#  for python

libformat_la_CPPFLAGS=@UFMT_CPPFLAGS@ @PYTHON_CFLAGS@
libformat_la_LDFLAGS=@UFMT_LDFLAGS@ @PYTHON_LIBS@
libformat_la_SOURCES=format_Module.cpp


install-exec-hook:
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq
	$(INSTALL_SCRIPT) @libdir@/libformat.so $(pymodule_dir)/format.so
