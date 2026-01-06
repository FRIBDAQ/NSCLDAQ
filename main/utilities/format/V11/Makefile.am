lib_LTLIBRARIES = libdataformatv11.la

libdataformatv11_la_SOURCES = CRingItem.cpp \
                           CAbnormalEndItem.cpp \
                           CRingScalerItem.cpp \
                           CRingStateChangeItem.cpp \
                           CPhysicsEventItem.cpp \
                           CRingPhysicsEventCountItem.cpp \
                           CRingTextItem.cpp \
                           CRingFragmentItem.cpp \
                           CGlomParameters.cpp \
                           CUnknownFragment.cpp \
                           CDataFormatItem.cpp \
                           CRingItemFactory.cpp \
                                                                                                        RingItemComparisons.cpp \
													 ringitem.c 
nscldaq11dir = @includedir@/V11

nscldaq11_HEADERS = CRingItem.h \
                  CAbnormalEndItem.h \
                  CRingScalerItem.h \
                  CRingStateChangeItem.h \
                  CPhysicsEventItem.h \
                  CRingPhysicsEventCountItem.h \
                  CRingTextItem.h \
                  CRingFragmentItem.h \
                  CGlomParameters.h \
                  CUnknownFragment.h \
                  CDataFormatItem.h \
                  CRingItemFactory.h \
                  RingItemComparisons.h \
                  DataFormatV11.h 

libdataformatv11_la_CPPFLAGS = -I@srcdir@/.. @PIXIE_CPPFLAGS@

libdataformatv11_la_CXXFLAGS = -I@srcdir@/.. \
				-I@top_srcdir@/base/os \
                               @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@ 


libdataformatv11_la_LDFLAGS = @top_builddir@/base/os/libdaqshm.la \
	@LIBTCLPLUS_LDFLAGS@ @TCL_LIBS@

check_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp \
			ringitemtests.cpp teststate.cpp	\
			scalertests.cpp texttest.cpp physcounttests.cpp		\
			 physeventtests.cpp \
			formatoutputtests.cpp dataformattests.cpp       \
			fragmenttest.cpp glomparamtests.cpp factorytests.cpp 

unittests_LDADD	= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
			@builddir@/libdataformatv11.la	\
			@top_builddir@/base/os/libdaqshm.la \
			@LIBEXCEPTION_LDFLAGS@ @TCL_LIBS@

unittests_CFLAGS= -I@srcdir@/..
unittests_CPPFLAGS= -I@srcdir@/.. \
		    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
		-I@top_srcdir@/base/os \
                    -I@top_srcdir@/base/testutils @PIXIE_CPPFLAGS@

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                            @TCL_LIBS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
