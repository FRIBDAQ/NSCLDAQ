bin_PROGRAMS=register_rw
bindir = @prefix@/sis3316
register_rw_SOURCES=sis3316_class.cpp \
    sis3316_ethernet_access_class.cpp \
    sis3316_eth_register_read_write.cpp

register_rw_CPPFLAGS=-I@srcdir@/../../vmusb \
    -I@top_srcdir@/base/thread
register_rw_LDADD=@builddir@/../../vmusb/libVMUSB.la @TCL_LDFLAGS@

noinst_HEADERS=project_interface_define.h \
    project_system_define.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h 
    
