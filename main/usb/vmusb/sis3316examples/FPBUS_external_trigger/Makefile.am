bin_PROGRAMS=FPBUS_external_trigger
bindir=@prefix@/sis3316

FPBUS_external_trigger_SOURCES=sis3316_cern_root_class.cpp \
    sis3316_class.cpp \
    sis3316_ethernet_access_class.cpp \
    sis3316_FP_Bus_external_trigger.cpp 
FPBUS_external_trigger_CPPFLAGS=@ROOT_CFLAGS@ -I@srcdir@/../../vmusb
FPBUS_external_trigger_LDADD=@ROOT_LDFLAGS@ \
    @builddir@/../../vmusb/libVMUSB.la @TCL_LDFLAGS@

noinst_HEADERS=project_interface_define.h \
    project_system_define.h \
    rootIncludes.h \
    sis3316_cern_root_class.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h \
    vme_interface_class.h
