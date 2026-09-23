#
#  DocBook callout images.
#
#  NOTE: docbuild/images/Makefile.am lists this directory only in
#  DIST_SUBDIRS, so it was never built/installed by 'make install' and is not
#  add_subdirectory()'d; docbuild/CMakeLists.txt installs these gifs into
#  share/images/callouts itself.  This is the translation of this
#  directory's Makefile.am should it ever be added - including its quirk of
#  creating share/images/callouts but installing the gifs into share/images.
#

nscldaq_install_mkdir(share/images/callouts)
file(GLOB _gifs CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.gif)
install(FILES ${_gifs} DESTINATION share/images)
