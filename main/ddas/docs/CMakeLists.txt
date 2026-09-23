#
#  DDAS Doxygen documentation (only added when ENABLE_DDAS_DOCS).
#
#  The Makefile ran doxygen at install time; here the html is generated at
#  build time and installed as it was: every file under html/ flattened into
#  share/ddasdocs/html and the html/search files also into
#  share/ddasdocs/html/search.
#

nscldaq_configure_file(Doxyfile.in Doxyfile)
nscldaq_configure_file(DoxygenLayout.xml.in DoxygenLayout.xml)

file(GLOB _ddasdocs_inputs CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.dox
  ${CMAKE_CURRENT_SOURCE_DIR}/figures/*)

set(_ddasdocs_stamp ${CMAKE_CURRENT_BINARY_DIR}/html.stamp)
add_custom_command(
  OUTPUT ${_ddasdocs_stamp}
  COMMAND ${DOXYGEN} Doxyfile
  COMMAND ${CMAKE_COMMAND} -E touch ${_ddasdocs_stamp}
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    ${CMAKE_CURRENT_BINARY_DIR}/DoxygenLayout.xml ${_ddasdocs_inputs}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "doxygen: DDAS documentation"
  VERBATIM)
add_custom_target(ddas_docs ALL DEPENDS ${_ddasdocs_stamp})

# install-exec-local

install(CODE "
  set(_dest \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/share/ddasdocs/html\")
  file(MAKE_DIRECTORY \"\${_dest}\" \"\${_dest}/search\")
  file(GLOB_RECURSE _files LIST_DIRECTORIES false
    \"${CMAKE_CURRENT_BINARY_DIR}/html/*\")
  if(_files)
    file(INSTALL \${_files} DESTINATION \"\${_dest}\")
  endif()
  file(GLOB_RECURSE _files LIST_DIRECTORIES false
    \"${CMAKE_CURRENT_BINARY_DIR}/html/search/*\")
  if(_files)
    file(INSTALL \${_files} DESTINATION \"\${_dest}/search\")
  endif()
")
