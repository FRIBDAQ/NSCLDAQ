
HELPFILES=add.html alarmconfig.html channelmenu.html \
	config.html filemenu.html gui.html \
	helpmenu.html overview.html

HELPDIR=@prefix@/TclLibs/data/epicsdisplay

PKGDIR=@prefix@/TclLibs/epicsdisplay

if EPICS


install-exec-local:
	$(mkinstalldirs)   $(bindir)
	$(mkinstalldirs)   $(HELPDIR)
	$(mkinstalldirs)   $(PKGDIR)



	$(INSTALL_SCRIPT)  @srcdir@/displayepics.tcl $(PKGDIR)
	$(INSTALL_SCRIPT)  @srcdir@/EpicsDisplay.tcl $(bindir)
	$(INSTALL_SCRIPT)  epicsdisplay     $(bindir)
	for f in $(HELPFILES); do $(INSTALL_DATA) @srcdir@/$$f $(HELPDIR); done

endif

EXTRA_DIST = channels.txt  EpicsDisplay.tcl displayepics.tcl \
	epicsdisplay.xml epicsdisplay.in \
	$(HELPFILES)



