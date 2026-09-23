#
#  DDAS - XIA Pixie16 digitizer support (only added when ENABLE_DDAS).
#

add_subdirectory(exception)
add_subdirectory(configuration)
add_subdirectory(booter)
add_subdirectory(ddasdumper)
add_subdirectory(readout)
add_subdirectory(pixieplugin)
add_subdirectory(qtscope)
add_subdirectory(utilities)
add_subdirectory(tclconfig)
if(ENABLE_DDAS_DOCS)
  add_subdirectory(docs)
endif()
