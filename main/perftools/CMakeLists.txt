#  DAQ performance / diagnostic tools. Built only when ENABLE_PERFTOOLS.
#  Installed under <prefix>/bin/perftools to keep them separate from the normal
#  NSCLDAQ programs. fragmaker and evbfilecheck also use the unified format
#  library for fragment.h / FragmentIndex.h (previously pulled from SpecTcl).
#
#  Command-line parsing is done with gengetopt: each tool's options live in
#  <tool>.ggo, generated into <tool>_cmdline.c / .h at build time. The _cmdline
#  suffix keeps the generated files from colliding with the program's own
#  <tool>.cpp.

set(perftools_includes
  ${PROJECT_SOURCE_DIR}/base/tcpip
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/daq/IO
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/headers)

set(perftools_libs
  Tcp dataformat daqio DataFlow daqshm
  NSCLDAQ::Exception NSCLDAQ::tclPlusHeaders NSCLDAQ::Threads)

# gengetopt parsers (generated at build time; see the comment above and the
# matching rules in Makefile.am). The generated <tool>_cmdline.h is found via
# CMAKE_INCLUDE_CURRENT_DIR (on while this directory is processed), as for glom.
nscldaq_gengetopt(rdoperf_cmdline         rdoperf.ggo)
nscldaq_gengetopt(fragsrcperf_cmdline     fragsrcperf.ggo)
nscldaq_gengetopt(sockperf_cmdline        sockperf.ggo)
nscldaq_gengetopt(pipeperf_cmdline        pipeperf.ggo)
nscldaq_gengetopt(socksend_cmdline        socksend.ggo)
nscldaq_gengetopt(pipesend_cmdline        pipesend.ggo)
nscldaq_gengetopt(bufferedoutperf_cmdline bufferedoutperf.ggo)
nscldaq_gengetopt(ritemMaker_cmdline      ritemMaker.ggo)
nscldaq_gengetopt(runmaker_cmdline        runmaker.ggo)
nscldaq_gengetopt(checkevfiles_cmdline    checkevfiles.ggo)
nscldaq_gengetopt(fragmaker_cmdline       fragmaker.ggo)
nscldaq_gengetopt(evbfilecheck_cmdline    evbfilecheck.ggo)

# MeterApi convenience library shared by several tools.
nscldaq_add_library(perfmeter NOINST SOURCES MeterApi.cpp MeterApi.h)
target_include_directories(perfmeter PRIVATE ${perftools_includes})
target_link_libraries(perfmeter PUBLIC Tcp NSCLDAQ::Exception NSCLDAQ::tclPlusHeaders)

nscldaq_add_executable(socksend        SOURCES socksend.cpp        DESTINATION bin/perftools)
nscldaq_add_executable(pipesend        SOURCES pipesend.cpp        DESTINATION bin/perftools)
nscldaq_add_executable(bufferedoutperf SOURCES bufferedoutperf.cpp DESTINATION bin/perftools)
nscldaq_add_executable(ritemMaker      SOURCES ritemMaker.cpp      DESTINATION bin/perftools)
nscldaq_add_executable(runmaker        SOURCES runmaker.cpp        DESTINATION bin/perftools)
nscldaq_add_executable(checkevfiles    SOURCES checkevfiles.cpp    DESTINATION bin/perftools)
nscldaq_add_executable(metertest       SOURCES metertestmain.cpp   DESTINATION bin/perftools)
nscldaq_add_executable(rdoperf         SOURCES rdoperf.cpp         DESTINATION bin/perftools)
nscldaq_add_executable(fragsrcperf     SOURCES fragsrcperf.cpp     DESTINATION bin/perftools)
nscldaq_add_executable(sockperf        SOURCES sockperf.cpp        DESTINATION bin/perftools)
nscldaq_add_executable(pipeperf        SOURCES pipeperf.cpp        DESTINATION bin/perftools)

foreach(t socksend pipesend bufferedoutperf ritemMaker runmaker checkevfiles
          metertest rdoperf fragsrcperf sockperf pipeperf)
  target_include_directories(${t} PRIVATE ${perftools_includes})
  target_link_libraries(${t} PRIVATE ${perftools_libs})
endforeach()

# Attach the generated parser to each program (metertest takes no options).
nscldaq_use_gengetopt(rdoperf         rdoperf_cmdline)
nscldaq_use_gengetopt(fragsrcperf     fragsrcperf_cmdline)
nscldaq_use_gengetopt(sockperf        sockperf_cmdline)
nscldaq_use_gengetopt(pipeperf        pipeperf_cmdline)
nscldaq_use_gengetopt(socksend        socksend_cmdline)
nscldaq_use_gengetopt(pipesend        pipesend_cmdline)
nscldaq_use_gengetopt(bufferedoutperf bufferedoutperf_cmdline)
nscldaq_use_gengetopt(ritemMaker      ritemMaker_cmdline)
nscldaq_use_gengetopt(runmaker        runmaker_cmdline)
nscldaq_use_gengetopt(checkevfiles    checkevfiles_cmdline)

# Installed binaries find the core NSCLDAQ shared libraries via the global
# CMAKE_INSTALL_RPATH (cmake/NSCLDAQCompiler.cmake); no per-target -Wl,-rpath
# is needed here, unlike the autotools AM_LDFLAGS.

foreach(t metertest rdoperf fragsrcperf sockperf pipeperf)
  target_link_libraries(${t} PRIVATE perfmeter)
endforeach()

# fragmaker and evbfilecheck also link the unified format library. Core include
# dirs are added first (directly) so common headers resolve to core;
# NSCLDAQ::UFMT supplies the unified-only fragment.h / FragmentIndex.h.
nscldaq_add_executable(fragmaker    SOURCES fragmaker.cpp    DESTINATION bin/perftools)
nscldaq_add_executable(evbfilecheck SOURCES evbfilecheck.cpp DESTINATION bin/perftools)
foreach(t fragmaker evbfilecheck)
  target_include_directories(${t} PRIVATE ${perftools_includes})
  target_link_libraries(${t} PRIVATE ${perftools_libs} NSCLDAQ::UFMT)
endforeach()
nscldaq_use_gengetopt(fragmaker    fragmaker_cmdline)
nscldaq_use_gengetopt(evbfilecheck evbfilecheck_cmdline)

# Install the Tcl meter server + widget next to the tools so perftests can be
# run from an NSCLDAQ installation without the source tree. 0755 via PROGRAMS,
# matching how the rest of the tree installs Tcl scripts.
install(PROGRAMS meterserver.tcl meter.tcl DESTINATION bin/perftools)
