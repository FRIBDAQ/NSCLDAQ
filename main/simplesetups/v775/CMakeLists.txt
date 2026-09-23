#
#  V775 simple setup guide: chapter2book.py turns the chapter into a book
#  and runs docbook2pdf on it.  This was done by install-exec-local; here it
#  is built with 'all' in the binary directory.
#

set(_book ${CMAKE_CURRENT_BINARY_DIR}/v775setup_autobook.pdf)
add_custom_command(
  OUTPUT ${_book}
  COMMAND ${PYTHON} ${PROJECT_SOURCE_DIR}/docbuild/chapter2book.py
    -s ${CMAKE_CURRENT_SOURCE_DIR}/v775setup.xml
    -b ${CMAKE_CURRENT_SOURCE_DIR}/v775bookinfo.xml
    -o ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${PROJECT_SOURCE_DIR}/docbuild/chapter2book.py
    ${CMAKE_CURRENT_SOURCE_DIR}/v775setup.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/v775bookinfo.xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "chapter2book: v775setup"
  VERBATIM)
add_custom_target(simplesetups_v775_book ALL DEPENDS ${_book})

nscldaq_install_mkdir(share/html share/pdfs)
install(FILES v775ediagram.jpeg v775ediagram.eps DESTINATION share/html)
install(FILES ${_book} DESTINATION share/pdfs)
