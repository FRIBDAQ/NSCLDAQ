bin_PROGRAMS = inventory
bindir = @prefix@/sis3316


inventory_SOURCES  = inventory.cpp  sis3316_class.cpp
inventory_CPPFLAGS = -I@srcdir@/../../vmusb -I@top_srcdir@/base/thread
inventory_LDADD    = @builddir@/../../vmusb/libVMUSB.la @TCL_LDFLAGS@
