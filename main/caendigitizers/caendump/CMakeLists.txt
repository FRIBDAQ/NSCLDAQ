nscldaq_add_executable(caendump SOURCES caendump.cpp)
target_link_libraries(caendump PRIVATE NSCLDAQ::CAEN)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(caendump)
