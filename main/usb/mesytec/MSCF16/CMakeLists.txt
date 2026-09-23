add_subdirectory(figures)

# install-exec-local

install(PROGRAMS MSCF16USB.tcl MSCF16Gui.tcl MSCF16GuiApp.tcl
  MSCF16Memorizer.tcl MSCF16CommandLogger.tcl
  MSCF16ScriptHeaderGenerator.tcl MSCF16StateSaver.tcl MSCF16FileLoader.tcl
  DESTINATION TclLibs/mesytec)
install(PROGRAMS MSCF16Control.tcl DESTINATION bin RENAME MSCF16Control)
nscldaq_install_tcl_index(TclLibs/mesytec -verbose "*.tcl")

# The Tcl tests were only run by 'make installcheck' (installcheck-local),
# never by 'make check' (no TESTS), so no test is registered here.
