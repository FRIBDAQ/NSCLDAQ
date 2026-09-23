#  chanlog : log epics channels to file or stdout.
#  This can only be built if Epics was detected.

if(ENABLE_EPICS_TOOLS)
  nscldaq_add_executable(chanlog SOURCES chanlog.cpp getinfo.cpp)
  # (-I@top_srcdir@/Headers named a directory that does not exist.)
  target_include_directories(chanlog PRIVATE
    ${PROJECT_SOURCE_DIR}/epics/epicslib
    ${PROJECT_SOURCE_DIR}/base/os)
  target_link_libraries(chanlog PRIVATE
    NSCLDAQ::EPICSBin NSCLDAQ::PixieHeaders
    daqshm Epicslib NSCLDAQ::EPICS NSCLDAQ::Threads NSCLDAQ::Tcl)
endif()
