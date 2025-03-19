bin_PROGRAMS=SRSReadout

SRSReadout_SOURCES=CReadoutAppSRS.cpp CReadoutAppSRS.h 			\
	CEventSegmentSRS.cpp CEventSegmentSRS.h 			\
	CTriggerSRS.cpp CTriggerSRS.h 				\
	CEndCommandSRS.cpp CEndCommandSRS.h         \
	CTcpClient.cpp CTcpClient.h

#	CBusySRS.cpp CBusySRS.h 					
#	CScalerSRS.cpp CScalerSRS.h 				

SRSReadout_CPPFLAGS= \
	-I@top_srcdir@ 								\
	-I@top_srcdir@/sbs/readout 					\
	-I@top_srcdir@/sbs/nsclapi					\
	-I@top_srcdir@/base/dataflow				\
	-I@top_srcdir@/base/os						\
	-I@top_srcdir@/daq/format					\
	-I@top_srcdir@/daq/IO						\
	-I@top_srcdir@/base/udp         			\
	-I@top_srcdir@/utilities/udpBroker/			\
	-I@top_srcdir@/srs/broker/					\
	-I@top_srcdir@/srs/maps/					\
	@LIBTCLPLUS_CFLAGS@							\
	@TCL_FLAGS@         						\
	-DSORTING

SRSReadout_LDFLAGS= \
	@top_builddir@/sbs/readout/libSBSProductionReadout.la   \
	@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la				\
	@top_builddir@/base/thread/libdaqthreads.la				\
	@top_builddir@/base/os/libdaqshm.la						\
	@top_builddir@/utilities/udpBroker/libUDPBrokerBase.la	\
	@top_builddir@/srs/broker/libUDPBrokerDerived.la	 	\
	@top_builddir@/srs/maps/libSRSMaps.la	 				\
	@LIBTCLPLUS_LDFLAGS@ \
	@TCL_LDFLAGS@
