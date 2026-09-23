if(ENABLE_EPICS_TOOLS)
  nscldaq_add_library(Epicslib SOURCES epicslib.cpp epicslib.h)
  # (-I@top_srcdir@/Headers named a directory that does not exist.)
  target_include_directories(Epicslib PRIVATE ${PROJECT_SOURCE_DIR}/base/os)
  target_link_libraries(Epicslib
    PRIVATE NSCLDAQ::EPICSBin
    PUBLIC daqshm NSCLDAQ::EPICS NSCLDAQ::Threads NSCLDAQ::Tcl)
endif()
