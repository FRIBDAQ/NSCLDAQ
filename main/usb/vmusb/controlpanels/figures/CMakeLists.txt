#
#  usb/vmusb/controlpanels/figures: documentation figures.
#

nscldaq_install_mkdir(share/html/figures)
file(GLOB _controlpanels_figures CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
install(FILES ${_controlpanels_figures} DESTINATION share/html/figures)
