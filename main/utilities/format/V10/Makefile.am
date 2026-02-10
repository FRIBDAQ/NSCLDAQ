lib_LTLIBRARIES = libdataformatv10.la

libdataformatv10_la_SOURCES = CRingItem.cpp \
                           CRingScalerItem.cpp \
                           CRingTimestampedRunningScalerItem.cpp \
                           CRingStateChangeItem.cpp \
                           CPhysicsEventItem.cpp \
                           CRingPhysicsEventCountItem.cpp \
                           CRingTextItem.cpp \
                           CRingFragmentItem.cpp \
                           CUnknownFragment.cpp \
                           CRingItemFactory.cpp \
                           ringitem.cpp

nscldaq10dir = @includedir@/V10

nscldaq10_HEADERS = CRingItem.h \
                  CRingScalerItem.h \
                  CRingTimestampedRunningScalerItem.h \
                  CRingStateChangeItem.h \
                  CPhysicsEventItem.h \
                  CRingPhysicsEventCountItem.h \
                  CRingTextItem.h \
                  CRingFragmentItem.h \
                  CUnknownFragment.h \
                  CRingItemFactory.h \
                  CVoidItem.h \
                  DataFormatV10.h

libdataformatv10_la_CXXFLAGS = -I@srcdir@/.. \
                               -I@top_srcdir@/utilities/Buffer \
				-I@top_srcdir@/base/os	\
                               @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@

libdataformatv10_la_LDFLAGS = @top_builddir@/utilities/Buffer/libbuffer.la \
				@top_builddir@/base/os/libdaqshm.la	\
                              @LIBTCLPLUS_LDFLAGS@ @LIBEXCEPTION_LDFLAGS@ \
				@TCL_LIBS@

check_PROGRAMS = unittests

unittests_SOURCES	= TestRunner.cpp \
                                                                                ringitemtests.cpp teststate.cpp	\
                                                                                scalertests.cpp texttest.cpp physcounttests.cpp		\
                                                                                formatoutputtests.cpp physeventtests.cpp

unittests_LDADD	= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
                        @top_builddir@/utilities/Buffer/libbuffer.la \
			@builddir@/libdataformatv10.la	\
			@top_builddir@/base/os/libdaqshm.la


unittests_CPPFLAGS= -I@srcdir@/.. \
		    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
                    -I@top_srcdir@/utilities/Buffer \
                    -I@top_srcdir@/base/testutils \
			-I@top_srcdir@/base/os/ @PIXIE_CPPFLAGS@ 


unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                            @TCL_LIBS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"


TESTS=./unittests
