lib_LTLIBRARIES = libbuffer.la


libbuffer_la_SOURCES =  ByteBuffer.cpp \
                        ByteOrder.cpp

include_HEADERS = ByteBuffer.h \
                  ByteOrder.h \
                  BufferPtr.h \
                  Deserializer.h


libbuffer_la_CXXFLAGS = -I@srcdir@ \
                         @LIBTCLPLUS_CFLAGS@

libbuffer_la_LIBADD = @LIBTCLPLUS_LDFLAGS@

check_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp \
                    translatorptrtest.cpp \
                    byteordertest.cpp\
                    bytebuffertest.cpp \
                    deserializertest.cpp \
                        ByteBuffer.cpp

unittests_LDADD	= $(builddir)/libbuffer.la \
                    @CPPUNIT_LDFLAGS@ 		\
									@LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS= -I@srcdir@ \
                    -I@top_srcdir@/base/testutils \
                    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ 

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
										@LIBEXCEPTION_LDFLAGS@ \
										@TCL_LDFLAGS@ \
										-Wl,"-rpath-link=$(libdir)"

TESTS=./unittests

