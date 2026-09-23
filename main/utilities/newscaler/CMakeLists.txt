#
#  newscaler - the pure Tcl scaler display (ScalerDisplay).
#

set(_newscaler_package_files
  nameMap.tcl channel.tcl scalerconfig.tcl page.tcl singleModel.tcl
  ratioModel.tcl scalermain.tcl header.tcl reports.tcl scalerUtil.tcl
  stripparam.tcl stripratio.tcl emptyModel.tcl zoomPrompt.tcl)
set(_newscaler_image_files red.gif green.gif amber.gif)

# AC_CONFIG_FILES utilities/newscaler/ScalerDisplay.tcl

nscldaq_configure_file(ScalerDisplay.tcl.in ScalerDisplay.tcl)

#--------------------------------------------------------------------------
#  Documentation: the user guide promoted to a standalone book (was built
#  by install-data-hook; built here at build time and installed below).

if(ENABLE_DOCS)
  set(_newscaler_book ${CMAKE_CURRENT_BINARY_DIR}/scalerdisplay_user_autobook.pdf)
  file(GLOB _newscaler_all_figures CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/figures/*)
  add_custom_command(
    OUTPUT ${_newscaler_book}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/figures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/figures ${CMAKE_CURRENT_BINARY_DIR}/figures
    COMMAND ${PYTHON} ${PROJECT_SOURCE_DIR}/docbuild/chapter2book.py
      -s ${CMAKE_CURRENT_SOURCE_DIR}/scalerdisplay_user.xml
      -b ${CMAKE_CURRENT_SOURCE_DIR}/scalerdisplay_user_bookinfo.xml
      -o ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scalerdisplay_user.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/scalerdisplay_user_bookinfo.xml
      ${PROJECT_SOURCE_DIR}/docbuild/chapter2book.py
      ${_newscaler_all_figures}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "chapter2book scalerdisplay_user"
    VERBATIM)
  add_custom_target(newscaler_docs ALL DEPENDS ${_newscaler_book})
endif()

#--------------------------------------------------------------------------
#  install-exec-hook

install(PROGRAMS ${_newscaler_package_files} DESTINATION TclLibs/newscaler)
nscldaq_install_tcl_index(TclLibs/newscaler -verbose "*.tcl")
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ScalerDisplay.tcl
  DESTINATION bin RENAME ScalerDisplay)
install(FILES ${_newscaler_image_files} DESTINATION share/scalerdisplay)

#--------------------------------------------------------------------------
#  install-data-hook (BUILD_DOCS)

if(ENABLE_DOCS)
  nscldaq_install_mkdir(share/pdfs share/scalerdisplay share/html/figures)
  install(FILES ${_newscaler_book} DESTINATION share/pdfs)
  install(PROGRAMS
    figures/ScalerDisplayUserGuide_empty_600.png
    figures/ScalerDisplayUserGuide_data_600.png
    figures/ScalerDisplayUserGuide_stripchart_600.png
    figures/ScalerDisplayUserGuide_GimpStripChart_600.png
    DESTINATION share/html/figures)
endif()

#--------------------------------------------------------------------------
#  check-TESTS

nscldaq_add_test(NAME tclTests
  COMMAND ${TCLSH_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/tclTests.tcl)
