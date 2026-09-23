#  BLT substitution widgets -> TclLibs/blt and the prebuilt RBC toolkit
#  -> TclLibs/rbc.

install(PROGRAMS tabset.tcl DESTINATION TclLibs/blt)
nscldaq_install_tcl_index(TclLibs/blt "*.tcl")

# The Makefile chose the prebuilt librbc by `getconf LONG_BIT`.

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(_rbc_lib librbc0.1.so-32)
else()
  set(_rbc_lib librbc0.1.so-64)
endif()
install(PROGRAMS ${_rbc_lib} DESTINATION TclLibs/rbc RENAME librbc0.1.so)
install(PROGRAMS graph.tcl pkgIndex.tcl rbcCanvEps.pro rbcGraph.pro
  DESTINATION TclLibs/rbc)
