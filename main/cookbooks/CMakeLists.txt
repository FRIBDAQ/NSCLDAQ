#
#  The NSCLDAQ cookbook (recipes).  docbook2html was run by
#  install-exec-local; here the html is built with 'all' in the binary
#  directory.
#

set(_stamp ${CMAKE_CURRENT_BINARY_DIR}/cookbook.html.stamp)
add_custom_command(
  OUTPUT ${_stamp}
  COMMAND ${DOCBOOK2HTML} -o ${CMAKE_CURRENT_BINARY_DIR}/html
    ${CMAKE_CURRENT_SOURCE_DIR}/cookbook.xml
  COMMAND ${CMAKE_COMMAND} -E touch ${_stamp}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cookbook.xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "docbook2html: cookbook.xml"
  VERBATIM)
add_custom_target(cookbooks_html ALL DEPENDS ${_stamp})

# The recipe source trees and the html, copied recursively (dirs 755,
# files 644) into share/recipes.

nscldaq_install_mkdir(share/recipes)
foreach(_dir images process readrings writerings epicspush offlinereglom)
  install(DIRECTORY ${_dir} DESTINATION share/recipes)
endforeach()
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/recipes)
