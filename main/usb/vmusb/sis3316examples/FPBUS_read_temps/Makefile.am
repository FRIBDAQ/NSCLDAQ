bin_PROGRAMS=FP_readtemps
bindir = @prefix@/sis3316

FP_readtemps_SOURCES=sis3316_class.cpp \
    sis3316_ethernet_access_class.cpp \
    sis3316_FP_Bus_read_temperature.cpp

FP_readtemps_CPPFLAGS=@ROOT_CFLAGS@ -I@srcdir@/../../vmusb
FP_readtemps_LDADD=@ROOT_LDFLAGS@

noinst_HEADERS=project_interface_define.h \
    project_system_define.h \
    rootIncludes.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h 

