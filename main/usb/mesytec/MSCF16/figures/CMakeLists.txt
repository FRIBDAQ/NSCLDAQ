# install-exec-local

file(GLOB _figures CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
install(FILES ${_figures} DESTINATION share/html/figures)
