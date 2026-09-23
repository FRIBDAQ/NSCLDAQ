#  The examples are installed as source into share/examples/<dir>.
#  ReadNSCLDAQFiles/Makefile is configured from its hand written Makefile.in
#  (AC_CONFIG_FILES examples/ReadNSCLDAQFiles/Makefile).
#
#  install-data-local copied everything in the build tree's <dir>/ with
#  'install' (mode 755): for the usual in-tree build those are the example
#  sources plus the configured Makefile.  Here that is the sources from the
#  source directory (not Makefile.in or editor backups) plus the configured
#  Makefile from the binary directory.

nscldaq_configure_file(ReadNSCLDAQFiles/Makefile.in ReadNSCLDAQFiles/Makefile)

foreach(_dir ReadNSCLDAQFiles)
  file(GLOB _example_files CONFIGURE_DEPENDS LIST_DIRECTORIES false
    ${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/*)
  list(FILTER _example_files EXCLUDE REGEX "(/Makefile\\.in|/CMakeLists\\.txt|~)$")
  list(APPEND _example_files ${CMAKE_CURRENT_BINARY_DIR}/${_dir}/Makefile)
  install(PROGRAMS ${_example_files} DESTINATION share/examples/${_dir})
endforeach()
