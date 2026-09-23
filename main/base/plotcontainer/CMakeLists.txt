install(PROGRAMS marker.tcl pkgIndex.tcl series.tcl xyplot.tcl series.test
  DESTINATION TclLibs/plotcontainer)

# Tests: (cd @srcdir@ && @TCLSH@ tclTests.tcl)

nscldaq_add_test(NAME tclTests
  COMMAND ${TCLSH} ${CMAKE_CURRENT_SOURCE_DIR}/tclTests.tcl
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
