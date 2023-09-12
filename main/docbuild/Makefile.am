EXTRA_DIST = alltsts.tcl docbuilder.tcl \
	fragmenttest.tcl mansectests.tcl \
	metadirtest.tcl walkdirtest.tcl \
	tutorial.xml chapter2book.py

DIST_SUBDIRS = images

TOPDF = @DOCBOOK2PDF@
TOMAN = @DOCBOOK2MAN@
TOHTML = @DOCBOOK2HTML@
UPDATEMAN = @MANDB@

MANSECTS = 1compatibility.xml 1daq.xml 1epics.xml 1evb.xml 1tcl.xml 1utilities.xml \
	3ccusb.xml 3daq.xml 3evb.xml 3provider.xml 3python.xml 3rdogui.xml 3sbsReadout.xml 3tcl.xml 3vmusb.xml \
	5daq.xml 5tcl.xml 5vmusb.xml

install-exec-local: daq.html daq.mans tutorial.pdf tutorial.html
	$(mkinstalldirs) @prefix@/share
	$(mkinstalldirs) @prefix@/share/man
	$(mkinstalldirs) @prefix@/share/html
	$(mkinstalldirs) @prefix@/share/tuthtml
	$(mkinstalldirs) @prefix@/share/pdfs
	for i in 1 2 3 4 5 6 7 8; do $(mkinstalldirs) @prefix@/share/man/man$$i; done
	- for i in 1 2 3 4 5 6 7 8; do for f in man/*.$$i*; do  $(INSTALL_DATA) $$f @prefix@/share/man/man$$i; done; done
	$(UPDATEMAN) @prefix@/share/man
	- $(INSTALL_DATA) html/* @prefix@/share/html
	- $(mkinstalldirs) @prefix@/share/images @prefix@/share/images/callouts
	- $(INSTALL_DATA) @srcdir@/images/*.gif @prefix@/share/images;
	- $(INSTALL_DATA) @srcdir@/images/callouts/*.gif  @prefix@/share/images/callouts
	- $(INSTALL_DATA) tutorial.pdf @prefix@/share/pdfs
	- $(INSTALL_DATA) tuthtml/*   @prefix@/share/tuthtml



clean-local:
	-rm -f *.pdf
	-rm -f daq.xml
	-rm -f $(MANSECTS)
	-rm -f daq.html daq.mans tutorial.pdf tutorial.html 1usbReadout.xml 1sbsReadout.xml
	-rm -rf tuthtml
	-rm -rf man

daq.pdf: daq.xml
	$(TOPDF) daq.xml


#	$(TOMAN) -o man --skip-validation 1usbReadout.xml
daq.mans: daq.xml
	- for f in $(MANSECTS); do $(TOMAN) -o man --skip-validation $$f; done				

#	touch daq.mans

BOOKDIRS = 	@top_srcdir@ \
            @top_srcdir@/base/dataflow\
	@top_srcdir@/daq/format \
	@top_srcdir@/utilities/manager \
	@top_srcdir@/utilities/readoutREST \
	@top_srcdir@/usb \
	@top_srcdir@/usb/usb1 \
	@top_srcdir@/usb/vmusb \
	@top_srcdir@/usb/ccusb \
	@top_srcdir@/daq/readoutgui \
	@top_srcdir@/utilities/logbook \
	@top_srcdir@/daq/eventbuilder \
	@top_srcdir@/daq/evbtools/evblite \
	@top_srcdir@/daq/evbtools/ringsource\
	@top_srcdir@/utilities/newscaler \
	@top_srcdir@/simplesetups/vmusb \
	@top_srcdir@/simplesetups/v775 \
	@top_srcdir@/simplesetups/v785 \
	@top_srcdir@/utilities/bufdump \
	@top_srcdir@/utilities/eventlog \
	@top_srcdir@/utilities/scalersum \
	@top_srcdir@/servers/portmanager \
	@top_srcdir@/servers/tclserver \
	@top_srcdir@/usb/mesytec/MCFD16 \
	@top_srcdir@/usb/mesytec/MSCF16 \
	@top_srcdir@/usb/vmusb/controlpanels \
	@top_srcdir@/usb/ccusb/controlpanels \
	@top_srcdir@/usb/ccusb/tclpackage \
	@top_srcdir@/usb/common/tcldrivers \
	@top_srcdir@/sbs \
	@top_srcdir@/sbs/readout \
	@top_srcdir@/sbs/vmemodules  \
	@top_srcdir@/sbs/slowcontrols \
	@top_srcdir@/sbs/slowcontrols/caenv812 \
	@top_srcdir@/sbs/slowcontrols/bcnaf \
	@top_srcdir@/sbs/slowcontrols/n568b \
	@top_srcdir@/sbs/slowcontrols/vhs \
	@top_srcdir@/sbs/slowcontrols/vhq \
	@top_srcdir@/sbs/puretcl \
	@top_srcdir@/sbs/tclpackage \
	@top_srcdir@/usb/loaders \
	@top_srcdir@/usb/caenfwloader \
	@top_srcdir@/utilities/filter \
	@top_srcdir@/daq/actions \
	@top_srcdir@/utilities/sequencer \
	@top_srcdir@/utilities/ringselector \
	@top_srcdir@/utilities/compatibility \
	@top_srcdir@/utilities/tkbufdump \
	@top_srcdir@/utilities/reglom	\
	@top_srcdir@/epics/controlpush \
	@top_srcdir@/epics/epicsdisplay \
	@top_srcdir@/epics/chanlog \
	@top_srcdir@/base/cvt \
	@top_srcdir@/base/thread \
	@top_srcdir@/base/security \
	@top_srcdir@/base/plotcontainer \
	@top_srcdir@/base/uri \
	@top_srcdir@/base/CopyrightTools \
	@top_srcdir@/base/os \
	@top_srcdir@/base/plotchart	\
	@top_srcdir@/base/tclutils \
	@top_srcdir@/base/tclwidgets  \
	@top_srcdir@/base/tcpip		\
	@top_srcdir@/libtcl/tclplus  \
	@top_srcdir@/libtcl/exception \
	@top_srcdir@/examples \
	@top_srcdir@/ddas/readoutdriver\
	@top_srcdir@/ddas/sorter \
	@top_srcdir@/ddas/pixieplugin \
	@top_srcdir@/caen-nextgen



daq.html: daq.xml
	- rm -rf html
	mkdir html
	- $(TOHTML) -o @builddir@/html daq.xml
	touch daq.html

daq.xml:
	$(TCLSH_CMD) @srcdir@/docbuilder.tcl @top_srcdir@/docconfig "NSCL DAQ Software Documentation" ${BOOKDIRS} >daq.xml


tutorial.pdf: @srcdir@/tutorial.xml
	$(TOPDF) @srcdir@/tutorial.xml

tutorial.html: @srcdir@/tutorial.xml
	- rm -rf tuthtml
	mkdir @builddir@/tuthtml
	- $(TOHTML) -o @builddir@/tuthtml @srcdir@/tutorial.xml
	touch tutorial.html




