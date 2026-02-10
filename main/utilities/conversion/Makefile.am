lib_LTLIBRARIES = libConversion.la
bin_PROGRAMS = format_converter

libConversion_la_SOURCES = CTransformMediator.h \
                           C8p0to10p0Mediator.cpp \
                           C10p0to8p0Mediator.cpp \
			   C10p0to11p0Mediator.cpp \
                           CTransformMediator.hpp \
                           CTransform8p0to10p0.cpp \
                           CTransform10p0to8p0.cpp \
                           CTransform10p0to11p0.cpp \
                           CTransform11p0to10p0.cpp \
                           CTransformFactory.cpp \
			CCompositePredicate.cpp 

                           #CTransform11p0to11p0.cpp


include_HEADERS = CTransformMediator.h \
                           C8p0to10p0Mediator.h \
                           C10p0to8p0Mediator.h \
			   C10p0to11p0Mediator.h \
			   CTransform8p0to10p0.h \
                           CTransform10p0to8p0.h \
                           CTransform10p0to11p0.h \
                           CTransform11p0to10p0.h \
                           CTransformFactory.h \
				CPredicate.h  \
				CCompositePredicate.h 

#                           CTransform11p0to11p0.h

libConversion_la_CPPFLAGS = -I@srcdir@ \
                            -I@top_srcdir@/utilities/format \
                            -I@top_srcdir@/utilities/filter \
                            -I@top_srcdir@/daq/IO \
                            -I@top_srcdir@/utilities/FormattedIO \
                            -I@top_srcdir@/utilities/Buffer \
                            -I@top_srcdir@/base/headers \
				-I@top_srcdir@/base/factories \
                            @LIBTCLPLUS_CFLAGS@ @PIXIE_CPPFLAGS@

libConversion_la_LIBADD = @top_builddir@/utilities/format/V8/libdataformatv8.la \
			  @top_builddir@/utilities/format/V10/libdataformatv10.la \
			  @top_builddir@/utilities/format/V11/libdataformatv11.la \
			  @top_builddir@/utilities/filter/libfilter.la \
			  @top_builddir@/daq/IO/libdaqio.la \
			  @top_builddir@/utilities/FormattedIO/libdaqformatio.la \
			  @LIBTCLPLUS_LDFLAGS@


check_PROGRAMS = unittests

unittests_SOURCES = TestRunner.cpp \
                    ctransform8p0to10p0tests.cpp \
                    ctransform10p0to8p0tests.cpp \
                    ctransform10p0to11p0tests.cpp \
                    ctransform11p0to10p0tests.cpp \
		    ctransformfactorytests.cpp \
		    c10p0to8p0mediatortests.cpp

unittests_LDADD	= -L$(libdir) $(CPPUNIT_LDFLAGS) 		\
                   @builddir@/libConversion.la	\
                   @top_builddir@/utilities/Buffer/libbuffer.la	\
		   @top_builddir@/utilities/format/V8/libdataformatv8.la	\
		   @top_builddir@/utilities/format/V10/libdataformatv10.la	\
		   @top_builddir@/utilities/format/V11/libdataformatv11.la	\
                   @top_builddir@/daq/IO/libdaqio.la	\
                   @top_builddir@/utilities/FormattedIO/libdaqformatio.la	\
                   @top_builddir@/utilities/filter/libfilter.la	\
                   @top_builddir@/utilities/Buffer/libbuffer.la	\
                   @LIBEXCEPTION_LDFLAGS@

unittests_CPPFLAGS= -I@srcdir@ \
                    -I@top_srcdir@/utilities/Buffer \
                    -I@top_srcdir@/utilities/filter \
                    -I@top_srcdir@/daq/IO \
                    -I@top_srcdir@/utilities/FormattedIO \
                    -I@top_srcdir@/utilities/format \
                    -I@top_srcdir@/base/headers \
			-I@top_srcdir@/base/factories \
                    @CPPUNIT_CFLAGS@ \
                    @LIBTCLPLUS_CFLAGS@ \
                    @TCL_FLAGS@ \
                    -I@top_srcdir@/base/testutils @PIXIE_CPPFLAGS@

unittests_LDFLAGS	= @CPPUNIT_LDFLAGS@ \
                            @TCL_LDFLAGS@ \
                          @LIBEXCEPTION_LDFLAGS@ \
                          -Wl,"-rpath-link=$(libdir)"

TESTS=./unittests


BUILT_SOURCES = format_converter_options.c format_converter_options.h

format_converter_options.c : format_converter_options.h

format_converter_options.h: format_converter_options.ggo
	gengetopt -i $< -F format_converter_options -a cmdLineOpts --output-dir=@builddir@

format_converter_SOURCES = Main.h Main.cpp 
nodist_format_converter_SOURCES = format_converter_options.c format_converter_options.h

format_converter_LDADD = @builddir@/libConversion.la \
                @top_builddir@/utilities/Buffer/libbuffer.la \
	       @top_builddir@/utilities/format/V8/libdataformatv8.la \
	       @top_builddir@/utilities/format/V10/libdataformatv10.la \
	       @top_builddir@/utilities/format/V11/libdataformatv11.la \
	       @top_builddir@/utilities/filter/libfilter.la \
	       @top_builddir@/daq/IO/libdaqio.la \
	       @top_builddir@/utilities/FormattedIO/libdaqformatio.la

format_converter_CPPFLAGS = -I@srcdir@ \
                    -I@top_srcdir@/utilities/Buffer \
		    -I@top_srcdir@/utilities/filter \
		    -I@top_srcdir@/daq/IO \
		    -I@top_srcdir@/utilities/FormattedIO \
		    -I@top_srcdir@/utilities/format \
		    -I@top_srcdir@/base/headers	\
			-I@top_srcdir@/base/factories @PIXIE_CPPFLAGS@

EXTRA_DIST = format_converter_options.ggo \
	 convert_format.py config.h


install-exec-local:
	${INSTALL_SCRIPT} @srcdir@/convert_format.py @prefix@/bin/convert_format 
	chmod a+x @prefix@/bin/convert_format
