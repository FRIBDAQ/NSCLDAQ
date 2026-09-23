nscldaq_add_executable(caendump SOURCES caendump.cpp)
target_link_libraries(caendump PRIVATE NSCLDAQ::CAEN)
