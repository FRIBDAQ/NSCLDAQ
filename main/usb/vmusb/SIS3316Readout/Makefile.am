bin_PROGRAMS=VMUSBnostackReadout

includedir=@prefix@/include/vmusbnostack

VMUSBnostackReadout_SOURCES=CConfigurableCompoundEventSegment.cpp \
CSIS3316EventSegment.cpp \
CSIS3316EventSegment.h \
CSIS3316Trigger.cpp \
CSIS3316Trigger.h \
CSIS3820ScalerBank.cpp \
CSIS3820ScalerBank.h \
CSIS3820Scaler.cpp \
CSIS3820Scaler.h \
CSIS3820TimestampEventSegment.cpp \
CSIS3820TimestampEventSegment.h \
CV977EventSegment.cpp \
CV977EventSegment.h \
CVMUSBBusy.cpp \
CVMUSBBusy.h \
CVMUSBEventSegment.cpp \
CVMUSBEventSegment.h \
sis3316_class.cpp \
sis3316_class.h \
sis3316.h \
sis3316Registers.h \
Skeleton.cpp \
Skeleton.h


include_HEADERS=CConfigurableCompoundEventSegment.h

## Compilation and link stuff:

VMUSBnostackReadout_CPPFLAGS=-I@srcdir@/../VMUSB -I@srcdir@/.. \
    -I@top_srcdir@/sbs/readout @LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@

VMUSBnostackReadout_LDADD=@top_builddir@/sbs/readout/libSBSProductionReazdout \
    @builddir@/../VMUSB @LIBTCLPLUS_LDFLAGS@ @TCL_LDFLAGS@