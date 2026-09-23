#  Header only; install-data-local copied @srcdir@/*.h to @prefix@/include.

file(GLOB _factories_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
nscldaq_install_headers(${_factories_headers})
