nscldaq_gengetopt(evbtagger evbtagger.ggo C_EXTENSION cpp)

nscldaq_add_executable(evbtagger
  SOURCES evbtagger.h evbtaggerMain.cpp ItemTagger.h ItemTagger.cpp)
nscldaq_use_gengetopt(evbtagger evbtagger)
target_include_directories(evbtagger PRIVATE
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder
  ${PROJECT_SOURCE_DIR}/daq/format)
target_link_libraries(evbtagger PRIVATE daqshm dataformat NSCLDAQ::UFMT)

# install-exec-hook:

install(PROGRAMS evblitePackage.tcl DESTINATION TclLibs/evb-lite)
nscldaq_install_tcl_index(TclLibs/evb-lite -verbose "*.tcl")
