bin_PROGRAMS=access_rate
bindir=@prefix@/sis3316

access_rate_SOURCES=sis3316_class.cpp \
    sis3316_eth_access_rate_test.cpp \
    sis3316_ethernet_access_class.cpp

noinst_HEADERS=project_interface_define.h \
    project_system_define.h \
    rootIncludes.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h \
    vme_interface_class.h

access_rate_CPPFLAGS=@ROOT_CFLAGS@
access_rate_LDADD=@ROOT_LDFLAGS@