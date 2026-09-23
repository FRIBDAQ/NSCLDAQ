#  NOTE: this directory is only in the parent's DIST_SUBDIRS, never in
#  SUBDIRS, so the parent does not add_subdirectory() it (it was never
#  built or installed by autotools either).  Translated for completeness.
#
#  install-exec-local: the rule copied @srcdir@/figures/*.png, a directory
#  that does not exist here, so the glob is taken relative to that path.

nscldaq_install_mkdir(share/html/figures)
file(GLOB _offlineorderer_figures CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/figures/*.png)
install(FILES ${_offlineorderer_figures} DESTINATION share/html/figures)
