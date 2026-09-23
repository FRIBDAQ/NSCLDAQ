#
#  VM-USB simple setup guide: chapter2book.py turns the chapter into a book
#  and runs docbook2pdf on it.  This was done by install-exec-local; here it
#  is built with 'all' in the binary directory.
#

add_subdirectory(src)

set(_book ${CMAKE_CURRENT_BINARY_DIR}/vmusbsetup_autobook.pdf)
add_custom_command(
  OUTPUT ${_book}
  COMMAND ${PYTHON} ${PROJECT_SOURCE_DIR}/docbuild/chapter2book.py
    -s ${CMAKE_CURRENT_SOURCE_DIR}/vmusbsetup.xml
    -b ${CMAKE_CURRENT_SOURCE_DIR}/vmusb_bookinfo.xml
    -o ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${PROJECT_SOURCE_DIR}/docbuild/chapter2book.py
    ${CMAKE_CURRENT_SOURCE_DIR}/vmusbsetup.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/vmusb_bookinfo.xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "chapter2book: vmusbsetup"
  VERBATIM)
add_custom_target(simplesetups_vmusb_book ALL DEPENDS ${_book})

nscldaq_install_mkdir(share/html share/html/figures share/pdfs)
install(FILES
  figures/sis3820blockdiagram.dia
  figures/sis3820blockdiagram.jpg
  figures/sis3820blockdiagram.eps
  figures/sis3820blockdiagram.png
  figures/v785v775sis3820setup.png
  figures/v785v775sis3820setup.eps
  figures/v785v775sis3820setup.dia
  figures/treegui_1d_0.png
  figures/treegui_1d_1.png
  figures/treegui_1d_2.png
  figures/treegui_connect.png
  figures/xamine_0.png
  figures/xamine_1.png
  DESTINATION share/html/figures)
install(FILES ${_book} DESTINATION share/pdfs)
