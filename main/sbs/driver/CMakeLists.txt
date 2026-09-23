add_subdirectory(include)
add_subdirectory(src)

#----------------------------------------------------------------------------
#  The SBS kernel driver (dd/GNUmakefile.am, SUBDIRS += dd if BUILD_SBS_DRIVER).
#
#  all-local:
#     make -C @KERNEL_SOURCE_DIR@ SUBDIRS=<dd> modules BTDRIVER=<dd>/..
#
#  The kbuild 'Makefile' in dd/ compiles with -I$(BTDRIVER)/dd
#  -I$(BTDRIVER)/include.  Kbuild writes its objects next to the sources, so
#  rather than building in the source tree the dd/ sources are copied to the
#  binary directory and built there; BTDRIVER still points at the source
#  tree for the headers.

if(ENABLE_SBS)
  set(_dd_src ${CMAKE_CURRENT_SOURCE_DIR}/dd)
  set(_dd_bin ${CMAKE_CURRENT_BINARY_DIR}/dd)
  file(GLOB _dd_sources CONFIGURE_DEPENDS ${_dd_src}/*.c ${_dd_src}/*.h)

  add_custom_command(
    OUTPUT ${_dd_bin}/btp.ko
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_dd_bin}
    COMMAND ${CMAKE_COMMAND} -E copy ${_dd_sources} ${_dd_src}/Makefile
      ${_dd_src}/mkbtp ${_dd_bin}
    COMMAND make -C ${WITH_SBSDIR} SUBDIRS=${_dd_bin} modules
      BTDRIVER=${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${_dd_sources} ${_dd_src}/Makefile ${_dd_src}/mkbtp
    COMMENT "Building the SBS btp kernel module"
    VERBATIM)
  add_custom_target(sbs_btp_module ALL DEPENDS ${_dd_bin}/btp.ko)

  # install-exec-local: install/load the module when installing as root
  # (the make recipe line was '-' prefixed).

  nscldaq_install_code([[
    cd "$BUILDDIR/dd" &&
    if test `id -u` = 0; then
      install btp.ko mkbtp /lib/modules/`uname -r`;
      rmmod btp;
      (cd /lib/modules/`uname -r`; ./mkbtp 5);
    else
      echo "\a\a" Must be root to install kernel module;
    fi]] NOFAIL)

  # install-data-local: the driver headers go to @prefix@/include.

  file(GLOB _dd_headers CONFIGURE_DEPENDS ${_dd_src}/*.h)
  nscldaq_install_headers(${_dd_headers})
endif()
