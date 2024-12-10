bin_PROGRAMS=sis3316_rebootall

exec_prefix=@prefix@/sis3316

sis3316_rebootall_SOURCES=sis3316_class.cpp \
sis3316_ethernet_access_class.cpp \
sis3316_reboot_all_modules.cpp

sis3316_rebootall_CXXFLAGS=-I@srcdir@/../../vmusb
sis3316_rebootall_LDADD=@builddir@/../../vmusb/libVMUSB.la \
    @TCL_LDFLAGS@

noinst_HEADERS=project_interface_define.h  \
    sis3316_ethernet_access_class.h \
    project_system_define.h     \
    sis3316.h \
    sis3316_class.h             
