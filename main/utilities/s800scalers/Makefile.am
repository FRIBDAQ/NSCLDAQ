
#s800scaler_gen - generates a scaler definition for
#                 the VMUSBReadout (e.g.)  based on a
#                 register definition file for the trigger module.
#                 The form of the file is a set of text lines
#                 containing address/size where the address
#                 is the starting address and size the number of
#                 contiguous 32 bit words that can be read.
#  Note:
#     the counters and frequencies are separated in the file.
#
#  See options.ggo for the  run time command def.

bin_PROGRAMS=s800scaler_gen

s800scaler_gen_SOURCES=main.cpp options.h options.cpp
s800scaler_gen_CPPFLAGS = @JSON_CFLAGS@ \
	-DDEFAULT_REGISTER_FILE=\"@datarootdir@/s800trigger/RegisterFile.json\" \
	@TCL_FLAGS@

s800scaler_gen_LDFLAGS=@JSON_LIBS@ @TCL_LDFLAGS@


BUILT_SOURCES=options.h options.cpp

options.h: options.cpp

options.cpp: options.ggo
	@GENGETOPT@ --unnamed-opts \
	--output-dir=@builddir@ \
	--input=@srcdir@/options.ggo \
	--file options --c-extension=cpp

EXTRA_DIST=options.ggo

