ACLOCAL_AMFLAGS=-I m4
AUTOMAKE_OPTIONS=subdir-objects

if CAENSUPPORT
CAENSUPPORTDIRS=caendigitizers
endif

if CAENNEXTGEN
CAENNGDIR=caen-nextgen
else
CAENNGDIR=
endif

if BUILD_EPICS_TOOLS
if EPICS
     EPICSDIRS=epics
endif
endif

if BUILD_USB
   USBDIR=usb
endif

if BUILD_DOCS
   DOCDIR=docbuild docconfig simplesetups cookbooks
endif

if BUILD_DDAS
  DDASDIR=ddas
endif


SUBDIRS = base caenfw-common servers base/dataflow \
	daq utilities $(DOCDIR) \
	sbs usb/common/configurableobject $(EPICSDIRS) $(USBDIR) $(DDASDIR) $(CAENSUPPORTDIRS) \
	$(CAENNGDIR) \
	examples

DIST_SUBDIRS = base caenfw-common servers base/dataflow daq utilities \
	docbuild docconfig simplesetups sbs epics usb ddas \
	caendigitizers examples caen-nextgen

INTROFIGURES = @srcdir@/figures/intro-data-flow-diagram.png \
	@srcdir@/figures/intro-data-flow-diagram-diagnostics.png

install-data-local:
	$(mkinstalldirs) @prefix@/TclLibs
	$(mkinstalldirs) @includedir@
	$(mkinstalldirs) @prefix@/pythonLibs @prefix@/pythonLibs/nscldaq
	$(mkinstalldirs) @prefix@/share/html/figures
	echo "pkg_mkIndex @prefix@/TclLibs *.so" >makeindex.tcl
	$(TCLSH_CMD) <makeindex.tcl
	$(INSTALL_SCRIPT) config.h @prefix@/include
	$(INSTALL_SCRIPT) @top_srcdir@/config_pixie16api.h @prefix@/include
	touch @prefix@/pythonLibs/nscldaq/__init__.py
	echo "Version: "NSCLDAQ-$(VERSION) built on `hostname` `date` > @prefix@/VERSION
	$(INSTALL_DATA) @srcdir@/ChangeLog @prefix@
	$(INSTALL_SCRIPT) daqsetup.bash @prefix@
	$(INSTALL_DATA) @srcdir@/figures/*.png @prefix@/share/html/figures
	$(mkinstalldirs) @prefix@/share/tclhttpd
	(cd @srcdir@/tclhttpd3.5.1; tar czf - .)|(cd @prefix@/share/tclhttpd; tar xzf -)
	chmod a+x @prefix@/share/tclhttpd/bin/*

EXTRA_DIST=dotests introduction.xml libtcl $(INTROFIGURES) cookbooks \
	tclhttpd3.5.1 config_pixie16api.h unifiedformat

#check-TESTS:
#	@top_srcdir@/dotests $(SUBDIRS)
