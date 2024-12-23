bin_PROGRAMS=allchans_exttrgtest

bindir=@prefix@/sis3316

allchans_exttrgtest_SOURCES=get_configuration_parameters.cpp \
    sis3316_cern_root_class.cpp \
    sis3316_ch1_to_ch16_external_trigger_test.cpp \
    sis3316_class.cpp \
    sis3316_ethernet_access_class.cpp


noinst_HEADERS=get_configuration_parameters.h \
    project_interface_define.h \
    project_system_define.h \
    rootIncludes.h \
    sis3316_cern_root_class.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h 


allchans_exttrgtest_CPPFLAGS=@ROOT_CFLAGS@ -I@srcdir@/../../vmusb \
    -I@top_srcdir@/base/thread
allchans_exttrgtest_LDADD=@ROOT_LDFLAGS@ @builddir@/../../vmusb/libVMUSB.la \
    @TCL_LDFLAGS@