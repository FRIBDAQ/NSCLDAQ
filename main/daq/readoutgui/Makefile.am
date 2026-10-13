install-exec-hook:
	$(mkinstalldirs) $(prefix)/bin
	$(mkinstalldirs) $(prefix)/TclLibs/Stager
	$(mkinstalldirs) @prefix@/share/html
	for f in  $(TCLPACKAGES); do install @srcdir@/$$f $(prefix)/TclLibs/Stager; done
	$(INSTALL_SCRIPT) @builddir@/InstallRoot.tcl $(prefix)/TclLibs/Stager
	for f in $(TCLAPPS); do $(INSTALL_SCRIPT) @srcdir@/$$f  $(prefix)/bin/`basename $$f .tcl`; done
	$(INSTALL_SCRIPT) @srcdir@/RunReadout.sh  $(prefix)/TclLibs
	echo "pkg_mkIndex -verbose $(prefix)/TclLibs/Stager *.tcl" | $(TCLSH_CMD)
	$(INSTALL_DATA) @srcdir@/RunState.jpg @srcdir@/RunState.eps @prefix@/share/html
	$(INSTALL_DATA) @srcdir@/readoutshell.jpg @srcdir@/readoutshell.eps @prefix@/share/html
	$(INSTALL_DATA) @srcdir@/eventlogsettings.jpg @srcdir@/eventlogsettings.eps @prefix@/share/html
	$(INSTALL_SCRIPT) ReadoutShell.tcl @bindir@/ReadoutShell
	$(INSTALL_SCRIPT) @srcdir@/start.bash @bindir@/start.bash
	$(INSTALL_SCRIPT) @srcdir@/evbwizard.tcl @bindir@/evbwizard


TCLPACKAGES=Configuration.tcl \
	DAQParameters.tcl \
	ExpFileSystemConfig.tcl \
	InitializeConfiguration.tcl \
	directories.tcl \
	DataSourceManager.tcl \
	RunstateMachine.tcl \
	rdoCalloutsBundle.tcl \
	eventLogBundle.tcl \
	DataSourceUI.tcl     \
	ui.tcl	\
	sshProvider.tcl sshPrompt.tcl \
	janusSshProvider.tcl janusSshPrompt.tcl \
	 s800Provider.tcl \
	ReadoutGui.tcl \
	state.tcl \
	DataSourceMonitor.tcl \
	remoteControl.tcl \
	ReadoutGuiClient.tcl readoutGuiProvider.tcl \
	error.png \
  delayProvider.tcl \
  offlineProvider.tcl \
	multilogger.tcl \
	stageareaValidation.tcl \
	ReadoutGUIAppLauncher.tcl \
	evbparameters.tcl evbsourceprompt.tcl sourceprompt.tcl  \
	BundleManager.tcl runStateObserver.tcl



# sshpipe.test - that can really only run if a system and the user are properly# set up.

TCLTESTS =    filesystem.test \
	initconfig.test installroot.test system.test tests.test  ostest.test \
	s800.test  rstatemachine.test rdocallouts.test \
	  state.test  delayprovider.test \
		applauncher.test bundlemgr.test eventloglifecycle.test

# to enable sshpipe.test uncomment the next line:
#TCLTESTS+=sshpipe.test



TKTESTS =	readoutguitest.tk  scalerparametergui.tk \
	selectreadoutest.tk  tclTests.tcl \
	testrunner.tk

#
#  Note that A=1234 below is for the state.test restore-6

check-TESTS:
	A=1234 TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ @TCLSH_CMD@ @srcdir@/tclTests.tcl
	@@ echo "readoutcontrol.test and tests.test load packages that produce output on stderr"
	@@ echo "This is the source of the "Test files exiting with errors"  message."


EXTRA_DIST = $(TCLPACKAGES) $(TCLAPPS) $(TCLTESTS) $(TKTESTS)\
	RunReadout.sh \
	InstallRoot.tcl.in \
	readoutgui_user.xml RunState.eps RunState.jpg s800fake.tcl\
	readoutshell.jpg readoutshell.eps \
	sshsimulator.testscript start.bash \
	eventlogsettings.jpg eventlogsettings.eps error.png evbwizard.tcl \
	readoutgui_man.xml


bin_PROGRAMS =  SecretTk
COMPILATION_FLAGS =	-I@top_srcdir@/base/headers	\
		@LIBTCLPLUS_CFLAGS@ \
		-I@top_srcdir@/base/CopyrightTools \
		$(TCL_FLAGS) -DHOME=\"$(prefix)\"

SecretTk_SOURCES = SecretTk.c
SecretTk_CPPFLAGS=$(COMPILATION_FLAGS)

SecretTk_LDFLAGS= $(TCL_LDFLAGS) $(X11LIBS)



check: check-TESTS
