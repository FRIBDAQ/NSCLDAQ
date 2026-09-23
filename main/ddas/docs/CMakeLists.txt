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

# install-exec-local:
#   find ./html/ -type f -exec install {} share/ddasdocs/html
#   find ./html/search/ -type f -exec install {} share/ddasdocs/html/search
# The first find flattens html/search/* into html/ as well, where e.g.
# search/functions_b.html overwrites the class member page
# functions_b.html; which copy wins depends on find's directory order.  Do
# it deterministically with the result the Autotools CI build produced
# (search/ files copied last).

install(CODE "
  set(_dest \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/share/ddasdocs/html\")
  set(_html \"${CMAKE_CURRENT_BINARY_DIR}/html\")
  file(MAKE_DIRECTORY \"\${_dest}\" \"\${_dest}/search\")
  file(GLOB_RECURSE _all LIST_DIRECTORIES false \"\${_html}/*\")
  file(GLOB_RECURSE _search LIST_DIRECTORIES false \"\${_html}/search/*\")
  set(_top \${_all})
  if(_search)
    list(REMOVE_ITEM _top \${_search})
  endif()
  message(STATUS \"Installing: \${_dest} (flattened DDAS html)\")
  foreach(_f IN LISTS _top _search)
    execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy \"\${_f}\" \"\${_dest}\")
  endforeach()
  foreach(_f IN LISTS _search)
    execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy \"\${_f}\" \"\${_dest}/search\")
  endforeach()
")
