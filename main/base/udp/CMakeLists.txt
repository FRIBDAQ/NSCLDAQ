nscldaq_add_library(Udp SOURCES UDP.cpp CUDPServer.cpp CUDPClient.cpp)

nscldaq_install_headers(UDP.h CUDPServer.h CUDPClient.h)
