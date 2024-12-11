bin_PROGRAMS=intclk_onemodule

bindir=@prefix@/sis3316

intclk_onemodule_SOURCES=get_configuration_parameter_appl.cpp \
    sis3316_cern_root_class.cpp \
    sis3316_class.cpp \
    sis3316_ethernet_access_class.cpp \
    sis3316_InternalClk_appl_oneModule.cpp 

intclk_onemodule_CPPFLAGS=@ROOT_CFLAGS@ -I@srcdir@/../../vmusb
intclk_onemodule_LDADD=@ROOT_LDFLAGS@

noinst_HEADERS=get_configuration_parameter_appl.h \
    project_interface_define.h \
    project_system_define.h \
    rootIncludes.h \
    sis3316_cern_root_class.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h 

