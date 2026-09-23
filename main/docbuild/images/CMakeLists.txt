#
#  DocBook admonition/navigation images.
#
#  NOTE: docbuild/Makefile.am lists this directory only in DIST_SUBDIRS, so
#  it was never built/installed by 'make install' and docbuild does not
#  add_subdirectory() it; docbuild/CMakeLists.txt installs these gifs itself.
#  This is the translation of this directory's Makefile.am should it ever be
#  added.  (callouts is likewise DIST_SUBDIRS only.)
#

nscldaq_install_mkdir(share/images)
file(GLOB _gifs CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.gif)
install(FILES ${_gifs} DESTINATION share/images)
