SUBDIRS=cv6533 figures

PACKAGES=client.tcl configfile.tcl gdgcontrol.tcl gdgwidget.tcl sclclient.tcl xlm72scalerpanel.tcl \
				 MDGG16Gui.tcl MDGG16Proxy.tcl MDGG16GuiApp.tcl AGD16XLM72Panel.tcl \
				ALevel3XLM72.tcl AXLM72GenericProxy.tcl \
				ALevel3XLM72.tcl vmelocal.tcl vmeremote.tcl MDPPProxy.tcl


APPS=gdgpanel.tcl v812usbcontrol.tcl MDGG16Control.tcl monal3.tcl  \
	XLM72ScalerGUI.tcl MDGG16Control.tcl XLM72GateDelayControl.tcl

PYAPPS=MDPPTriggerControl.py
UIS=MDPPTriggerControl.ui MDPPTriggerControl_about.ui

IMAGES=l3trigicon.png

TCL_TESTS = tclTests.tcl mdgg16gui.test mdgg16guiapp.test

install-exec-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @prefix@/TclLibs/vmusbcontrol
	$(mkinstalldirs) @prefix@/share/images
	for f in $(PACKAGES); do $(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/TclLibs/vmusbcontrol; done
	echo pkg_mkIndex -verbose @prefix@/TclLibs/vmusbcontrol "*.tcl" | tclsh
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .tcl`;  done
	for f in $(APPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@;  done    # Compatible installation
	for f in $(PYAPPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@/`basename $$f .py`;  done
	for f in $(PYAPPS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@;  done    # Compatible installation
	for f in $(UIS); do $(INSTALL_SCRIPT) @srcdir@/$$f @bindir@;  done
	$(INSTALL_SCRIPT) @srcdir@/v812usbcontrol.tcl @bindir@/v895usbcontrol.tcl	
	$(INSTALL_SCRIPT) @srcdir@/v812usbcontrol.tcl @bindir@/v895usbcontrol
	$(mkinstalldirs) @prefix@/share/html/figures
	$(INSTALL_DATA) @srcdir@/gd16_startup.png @prefix@/share/html/figures/.
	$(INSTALL_DATA) @srcdir@/l3trigicon.png  @prefix@/share/images

check-TESTS:
	TCLLIBPATH=@prefix@/TclLibs tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST= $(APPS) $(PYAPPS) $(UIS) $(PACKAGES) XLM72ScalerGUI.tcl $(TCL_TESTS) mdgg16control.xml \
	XLM72GateDelayControl.tcl gd16_startup.png xlm72gatedelaycontrol.xml $(IMAGES)
