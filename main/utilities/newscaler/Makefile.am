# The pure TCl parts of this.

PACKAGE_FILES=nameMap.tcl channel.tcl scalerconfig.tcl page.tcl \
	singleModel.tcl ratioModel.tcl scalermain.tcl header.tcl reports.tcl\
	scalerUtil.tcl stripparam.tcl stripratio.tcl emptyModel.tcl zoomPrompt.tcl
IMAGE_FILES=	red.gif green.gif amber.gif
TEST_FILES=nameMap.test channel.test channelcmd.test singleModel.test \
	ratioModel.test stripparam.test stripratio.test emptyModel.test tclTests.tcl
APP_FILES=ScalerDisplay.tcl.in

DOCS=scalerdisplay_user.xml scalerdisplay_man.xml \
		 scalerdisplay_user_bookinfo.xml tclring_user.xml
FIGURES = @srcdir@/figures/ScalerDisplayUserGuide_empty_600.png \
	@srcdir@/figures/ScalerDisplayUserGuide_data_600.png \
	 @srcdir@/figures/ScalerDisplayUserGuide_stripchart_600.png \
	 @srcdir@/figures/ScalerDisplayUserGuide_GimpStripChart_600.png

# auto book
BOOK=scalerdisplay_user_autobook.pdf
CHAPTER2BOOK = @PYTHON@ @top_srcdir@/docbuild/chapter2book.py

INTERACTIVE_TESTS=pagetest.tcl 




INSTDIR=@prefix@/TclLibs/newscaler
GRAPHDIR=@prefix@/share/scalerdisplay
FIGUREDIR=@prefix@/share/html/figures

install-exec-hook:
	$(mkinstalldirs) $(INSTDIR) 
	(cd @srcdir@; $(INSTALL_SCRIPT) $(PACKAGE_FILES) $(INSTDIR))
	echo pkg_mkIndex -verbose $(INSTDIR) "*.tcl" | @TCLSH_CMD@
	install @builddir@/ScalerDisplay.tcl @bindir@/ScalerDisplay
	$(mkinstalldirs) @datarootdir@/scalerdisplay
	(cd @srcdir@; $(INSTALL_DATA) $(IMAGE_FILES) @datarootdir@/scalerdisplay)

if BUILD_DOCS
install-data-hook:
	$(mkinstalldirs) @prefix@/share/pdfs
	$(mkinstalldirs) $(GRAPHDIR)
	$(mkinstalldirs) $(FIGUREDIR)
	mkdir -p figures
	$(INSTALL_DATA)  @srcdir@/figures/* @builddir@/figures
	$(CHAPTER2BOOK) -s @srcdir@/scalerdisplay_user.xml -b @srcdir@/scalerdisplay_user_bookinfo.xml -o @builddir@
	$(INSTALL_DATA)  $(BOOK) @prefix@/share/pdfs
	install $(FIGURES) $(FIGUREDIR)


endif


check-TESTS:
	@TCLSH_CMD@ @srcdir@/tclTests.tcl

EXTRA_DIST=$(PACKAGE_FILES) $(TEST_FILES) $(INTERACTIVE_TESTS) $(APP_FILES)  \
	$(DOCS) $(FIGURES) ${IMAGE_FILES}


check: check-TESTS


