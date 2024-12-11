bin_PROGRAMS=FPBUS_threads_allmodules

FPBUS_threads_allmodules_SOURCES=get_configuration_parameter_appl.cpp \
    sis3316_cern_root_class.cpp \
    sis3316_class.cpp \
    sis3316_ethernet_access_class.cpp \
    sis3316_FP_BUS_Threads_appl_allModules.cpp
FPBUS_threads_allmodules_CPPFLAGS=@ROOT_CFLAGS@ -I@srcdir@/../../vmusb
FPBUS_threads_allmodules_LDADD=@ROOT_LDFLAGS@


noinst_HEADERS=get_configuration_parameter_appl.h \
    project_interface_define.h \
    project_system_define.h \
    rootIncludes.h \
    sis3316_cern_root_class.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h 