if USING_XIAAPI_V2
SUBDIRS = configuration xiatest booter format ddasdumper nscope readout sorter readoutdriver pixieplugin simple_setups qtscope dumpsetfile xml/tinyxml xml tclconfig
else
SUBDIRS = configuration xiatest booter format ddasdumper nscope readout sorter readoutdriver pixieplugin simple_setups qtscope
endif

if BUILD_DDAS_DOCS
SUBDIRS += docs
endif

# readout-extclock - external clock handling is subsumed into readout.
# traiter - legacy trace iterator class. Not installed for now.
