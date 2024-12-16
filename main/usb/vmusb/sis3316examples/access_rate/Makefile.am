bin_PROGRAMS=ethernet_access_rate
bindir=@prefix@/sis3316

ethernet_access_rate_SOURCES=sis3316_class.cpp \
    sis3316_eth_access_rate_test.cpp \
    sis3316_ethernet_access_class.cpp

noinst_HEADERS=project_interface_define.h \
    project_system_define.h \
    rootIncludes.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h


ethernet_access_rate_CPPFLAGS=@ROOT_CFLAGS@ -I@srcdir@/../../vmusb
ethernet_access_rate_LDADD=@ROOT_LDFLAGS@