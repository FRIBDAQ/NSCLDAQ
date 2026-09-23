#
#  V785 simple setup guide: chapter2book.py turns the chapter into a book
#  and runs docbook2pdf on it.  This was done by install-exec-local; here it
#  is built with 'all' in the binary directory.
#

set(_book ${CMAKE_CURRENT_BINARY_DIR}/v785setup_autobook.pdf)
add_custom_command(
  OUTPUT ${_book}
  COMMAND ${PYTHON} ${PROJECT_SOURCE_DIR}/docbuild/chapter2book.py
    -s ${CMAKE_CURRENT_SOURCE_DIR}/v785setup.xml
    -b ${CMAKE_CURRENT_SOURCE_DIR}/v785bookinfo.xml
    -o ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${PROJECT_SOURCE_DIR}/docbuild/chapter2book.py
    ${CMAKE_CURRENT_SOURCE_DIR}/v785setup.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/v785bookinfo.xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "chapter2book: v785setup"
  VERBATIM)
add_custom_target(simplesetups_v785_book ALL DEPENDS ${_book})

nscldaq_install_mkdir(share/html share/html/figures share/pdfs)
install(FILES
  figures/ss785_amplified_signal_with_gate.png
  figures/ss785_amplified_signal_with_logic_pulse.png
  figures/ss785_amplifier_output.png
  figures/ss785_electronics_block_diagram_new.png
  figures/ss785_pulser_output.png
  DESTINATION share/html/figures)
install(FILES ${_book} DESTINATION share/pdfs)
