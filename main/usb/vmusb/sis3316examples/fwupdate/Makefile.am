bin_PROGRAMS = fwupdate
bindir=@prefix@/sis3316
fwupdate_SOURCES=sis3316_class.cpp \
    sis3316_ethernet_access_class.cpp \
    sis3316_eth_fpga_update.cpp

noinst_HEADERS=project_interface_define.h \
    project_system_define.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h 

fwupdate_CPPFLAGS=-I@srcdir@/../../vmusb \
    -I@top_srcdir@/base/thread
fwupdate_LDADD=@builddir@/../../vmusb/libVMUSB.la @TCL_LDFLAGS@