#
#  usb/vmusb/controlpanels: Tcl/Tk and Python control panels for VM-USB
#  slow controls devices.
#

add_subdirectory(cv6533)
add_subdirectory(figures)

#  install-exec-local

nscldaq_install_mkdir(TclLibs TclLibs/vmusbcontrol share/images)
install(PROGRAMS client.tcl configfile.tcl gdgcontrol.tcl gdgwidget.tcl
  sclclient.tcl xlm72scalerpanel.tcl MDGG16Gui.tcl MDGG16Proxy.tcl
  MDGG16GuiApp.tcl AGD16XLM72Panel.tcl ALevel3XLM72.tcl
  AXLM72GenericProxy.tcl vmelocal.tcl vmeremote.tcl MDPPProxy.tcl
  DESTINATION TclLibs/vmusbcontrol)
nscldaq_install_tcl_index(TclLibs/vmusbcontrol -verbose "*.tcl")

#  Applications are installed both without and (for compatibility) with
#  their extension.

set(_controlpanels_apps gdgpanel.tcl v812usbcontrol.tcl MDGG16Control.tcl
  monal3.tcl XLM72ScalerGUI.tcl XLM72GateDelayControl.tcl)
set(_controlpanels_pyapps MDPPTriggerControl.py)
foreach(_app IN LISTS _controlpanels_apps _controlpanels_pyapps)
  get_filename_component(_base ${_app} NAME_WE)
  install(PROGRAMS ${_app} DESTINATION bin RENAME ${_base})
  install(PROGRAMS ${_app} DESTINATION bin)
endforeach()
install(PROGRAMS MDPPTriggerControl.ui MDPPTriggerControl_about.ui DESTINATION bin)
install(PROGRAMS v812usbcontrol.tcl DESTINATION bin RENAME v895usbcontrol.tcl)
install(PROGRAMS v812usbcontrol.tcl DESTINATION bin RENAME v895usbcontrol)
nscldaq_install_mkdir(share/html/figures)
install(FILES gd16_startup.png DESTINATION share/html/figures)
install(FILES l3trigicon.png DESTINATION share/images)

# Not registered as a test: Makefile.am defines a check-TESTS rule but no
# TESTS variable, so 'make check' never ran it.  The rule was:
#   TCLLIBPATH=@prefix@/TclLibs ... tclsh @srcdir@/tclTests.tcl
