if USING_XIAAPI_V2
SUBDIRS = configuration xiatest booter format ddasdumper nscope readout sorter readoutdriver pixieplugin simple_setups qtscope dumpsetfile xml/tinyxml xml tclconfig
else
SUBDIRS = configuration xiatest booter format ddasdumper readout sorter readoutdriver pixieplugin simple_setups qtscope tclconfig
endif

if BUILD_DDAS_DOCS
SUBDIRS += docs
endif

# readout-extclock - external clock handling is subsumed into readout.
# traiter - legacy trace iterator class. Not installed for now.
# xiatest - removed for xia api 3+ bullseye. test suite is decoupled from the api version and implements a set of api 2 functions for testing which may not be supported into the future. these tests must be reconsidered for api 3+, possibly using the crate emulation xia provides (?).
# nscope - removed for xia api 3+ bullseye. only supporting qtscope.
