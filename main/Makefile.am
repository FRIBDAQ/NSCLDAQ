ACLOCAL_AMFLAGS=-I m4
AUTOMAKE_OPTIONS=subdir-objects

if CAENSUPPORT
CAENSUPPORTDIRS=caendigitizers
endif

if CAENNEXTGEN
CAENNGDIR=caen-nextgen
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

if MVLC_GENERATOR
  MVLCGENDIR=mvlc
endif

SRSDIR=srs

# Rust tools:

if BUILD_RUST_TOOLS
	RUSTSUBDIRS=rust_tools
endif


SUBDIRS = base caenfw-common servers base/dataflow \
	daq utilities $(DOCDIR) \
	sbs usb/common/configurableobject $(EPICSDIRS) $(USBDIR) $(DDASDIR) $(CAENSUPPORTDIRS) \
	$(CAENNGDIR) $(SRSDIR) $(MVLCGENDIR) \
	examples $(RUSTSUBDIRS)

DIST_SUBDIRS = base caenfw-common servers base/dataflow daq utilities \
	docbuild docconfig simplesetups sbs epics usb ddas \
	caendigitizers examples caen-nextgen srs mvlc rust_tools

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
	$(INSTALL_SCRIPT) @top_builddir@/ddas/config_pixie16api.h @prefix@/include
	touch @prefix@/pythonLibs/nscldaq/__init__.py
	echo "Version: "NSCLDAQ-$(VERSION) built on `hostname` `date` by `whoami` > @prefix@/VERSION
	head  @top_builddir@/config.log| grep  -- --prefix >> @prefix@/VERSION
	echo libtcl++ `(cd @top_builddir@/libtcl; @GIT@ status | head -1|cut -d" " -f4)` >> @prefix@/VERSION
	echo unifiedformat `(cd @top_builddir@/unifiedformat; @GIT@ status |head -1| cut -d" " -f4)` >> @prefix@/VERSION
	echo ddasformat `(cd @top_builddir@/ddasformat; @GIT@ status |head -1| cut -d" " -f4)` >> @prefix@/VERSION
	$(INSTALL_DATA) @srcdir@/ChangeLog @prefix@
	$(INSTALL_SCRIPT) daqsetup.bash @prefix@
	$(INSTALL_DATA) @srcdir@/figures/*.png @prefix@/share/html/figures
	$(mkinstalldirs) @prefix@/share/tclhttpd
	(cd @srcdir@/tclhttpd3.5.1; tar czf - .)|(cd @prefix@/share/tclhttpd; tar xzf -)
	chmod a+x @prefix@/share/tclhttpd/bin/*

EXTRA_DIST=dotests introduction.xml libtcl $(INTROFIGURES) cookbooks \
	tclhttpd3.5.1 unifiedformat ddasformat

#check-TESTS:
#	@top_srcdir@/dotests $(SUBDIRS)
