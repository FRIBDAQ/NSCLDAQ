SUBDIRS = exception configuration booter ddasdumper readout pixieplugin qtscope utilities tclconfig

if BUILD_DDAS_DOCS
SUBDIRS += docs
endif

# 12.1 release will fully migrate to some XIA API 4.x version and many features
# that are available only for legacy API systems will be deprecated. A number
# of programs are no longer built including those to convert binary settings
# files to XML format (and back) and the suite of unittests which faked a
# version of the legacy API to test against.

# Also not built:
# * readout-extclock - external clock handling is subsumed into readout.
# * traiter - legacy trace iterator class. Not installed for now.

# (ASC 5/31/24): Refactored the DDAS format stuff out of FRIBDAQ into its own
# module which can be incorp'd the same way as the unified format library and
# libtcl.
