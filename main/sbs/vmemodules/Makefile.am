lib_LTLIBRARIES	=	libSBSDeviceSupport.la

libSBSDeviceSupport_la_SOURCES =	CADC2530.cpp		\
					CAENcard_767.cpp	\
					CAENcard.cpp		\
					CaenIO.cpp		\
					camacmap.cpp		\
					CCAENChain.cpp		\
					CCAENV1x90.cpp		\
					CCAENV560.cpp		\
					CCAENV830.cpp		\
					CCAENV977.cpp		\
					CModule32.cpp	\
					CVMEScaler.cpp	\
					CSIS3600.cpp	\
					CSIS3820.cpp	\
					CVMEScalerLRS1151.cpp	\
					Nimout.cpp	\
					sis3300.cpp	\
					v890.cpp	\
					CMemoryMappedCAMACBranch.cpp	\
					CESCBD8210.cpp	\
					CVC32CC32.cpp	\
					CCAMACModule.cpp

#
#  This prevents collision with vmusb daqconfig
#  headers of the same name when building e.g. mvlc_generate
#  after an install...
#
includedir=@prefix@/include/sbsmodules


include_HEADERS =	CADC2530.h	\
			CAENcard_767.h	\
			CAENcard.h	\
			CaenIO.h	\
			camac.h		\
			camacmap.h	\
			CCAENChain.h	\
			CCAENV1x90Data.h	\
			CCAENV1x90.h	\
			CCAENV1x90Opcodes.h	\
			CCAENV1x90Registers.h	\
			CCAENV560.h	\
			CCAENV560Registers.h	\
		 	CCAENV820Registers.h	\
			CCAENV830.h	\
			CCAENV977.h	\
			cescamac.h		\
			cesmacros.h	\
			CModule32.h	\
			CVMEScaler.h	\
			CSIS3600.h	\
			CSIS3820.h	\
			CVMEScalerLRS1151.h	\
			macros.h	\
			Nimout.h	\
			sis3300.h	\
			v890.h		\
			wienercamac.h	\
			wienermacros.h	\
			CCAMACBranch.h	\
			CMemoryMappedCAMACBranch.h \
			CESCBD8210.h		\
			CVC32CC32.h	\
			CCAMACModule.h


libSBSDeviceSupport_la_CPPFLAGS	=	-I@top_srcdir@/base/headers 	\
			@LIBTCLPLUS_CFLAGS@	\
			-I@top_srcdir@/sbs/nsclapi	\
			-I@top_srcdir@/sbs/driver/include \
			-DHAVE_VME_MAPPING -DHAVE_SBSVME_INTERFACE \
			-I@top_srcdir@/base/os @PIXIE_CPPFLAGS@


libSBSDeviceSupport_la_LIBADD =	@LIBTCLPLUS_LDFLAGS@	\
				@top_builddir@/base/os/libdaqshm.la	\
				@top_builddir@/sbs/nsclapi/libSBSVmeAPI.la	\
				@top_builddir@/sbs/driver/src/libbtp.la

libSBSDeviceSupport_la_LDFLAGS=	-version-info	1:0:0

EXTRA_DIST=DeviceSupport.xml
