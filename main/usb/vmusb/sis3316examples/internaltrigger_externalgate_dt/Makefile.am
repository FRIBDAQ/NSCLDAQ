bin_PROGRAMS=inttrig_extgate_dt
bindir=@prefix@/sis3316

inttrig_extgate_dt_SOURCES=sis3316_cern_root_class.cpp \
sis3316_class.cpp \
sis3316_ethernet_access_class.cpp \
sis3316_internalTrigger_externalGate_deadTime.cpp

noinst_HEADERS=project_interface_define.h \
    sis3316_class.h \
    project_system_define.h \
    sis3316_ethernet_access_class.h \
    rootIncludes.h \
    sis3316.h \
    sis3316_cern_root_class.h


inttrig_extgate_dt_CPPFLAGS=@ROOT_CFLAGS@ -I@srcdir@/../../vmusb \
    -I@top_srcdir@/base/thread
inttrig_extgate_dt_LDADD=@ROOT_LDFLAGS@ \
    @builddir@/../../vmusb/libVMUSB.la @TCL_LDFLAGS@