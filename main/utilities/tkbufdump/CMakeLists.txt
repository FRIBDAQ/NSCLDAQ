#  install-exec-local

file(GLOB _tkbufdump_tcl CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.tcl)
file(GLOB _tkbufdump_html CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.html)
install(PROGRAMS ${_tkbufdump_tcl} DESTINATION tkbufdump)
install(FILES ${_tkbufdump_html} DESTINATION tkbufdump)
install(FILES packets.def DESTINATION tkbufdump)

#  The tkdumper launcher script (was echo'd into @prefix@/bin/tkdumper).

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tkdumper
  "#!/bin/bash\n${prefix}/tkbufdump/tkbufdump.tcl\n")
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/tkdumper DESTINATION bin)
