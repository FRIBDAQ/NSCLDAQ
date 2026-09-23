#
#  CAEN first generation digitizer support (only added when
#  ENABLE_CAEN_DIGITIZER_SUPPORT).
#

add_subdirectory(dpp-pha)
add_subdirectory(dpp-psd)
add_subdirectory(caendump)
add_subdirectory(tweaker)

# install-exec-hook (ENABLE_DOCS): the Makefile ran docbook2html at install
# time; here it runs at build time and its output is installed.

if(ENABLE_DOCS)
  set(_caendpp_html ${CMAKE_CURRENT_BINARY_DIR}/caendpp)
  set(_caendpp_stamp ${CMAKE_CURRENT_BINARY_DIR}/caendpp.stamp)
  add_custom_command(
    OUTPUT ${_caendpp_stamp}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_caendpp_html}
    COMMAND ${DOCBOOK2HTML} -o ${_caendpp_html}
      ${CMAKE_CURRENT_SOURCE_DIR}/caendigitizers.xml
    COMMAND ${CMAKE_COMMAND} -E touch ${_caendpp_stamp}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/caendigitizers.xml
    COMMENT "docbook2html caendigitizers.xml"
    VERBATIM)
  add_custom_target(caendigitizers_docs ALL DEPENDS ${_caendpp_stamp})
  install(DIRECTORY ${_caendpp_html}/ DESTINATION share/caendpp)
endif()
