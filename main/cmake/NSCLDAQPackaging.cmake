#
#  NSCLDAQPackaging.cmake
#
#  'make dist' replacement: 'cmake --build <build> --target package_source'
#  makes nscldaq-<version>.tar.gz from the source tree.  Incorporated
#  packages are included when they are present in the source tree (run
#  ./tcl++incorp, ./unifiedfmt-incorp.sh and ./ddasfmt-incorp.sh first to
#  get a self-contained tarball as 'make dist' produced).
#

include_guard(GLOBAL)

set(CPACK_PACKAGE_NAME nscldaq)
set(CPACK_PACKAGE_VERSION "${NSCLDAQ_VERSION_STRING}")
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "nscldaq-${NSCLDAQ_VERSION_STRING}")
set(CPACK_SOURCE_IGNORE_FILES
  "/\\\\.git/"
  "/\\\\.vscode/"
  "/build[^/]*/"
  "/oot/"
  "/autom4te\\\\.cache/"
  "~$")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${PROJECT_SOURCE_DIR};/")
set(CPACK_GENERATOR TGZ)

include(CPack)
