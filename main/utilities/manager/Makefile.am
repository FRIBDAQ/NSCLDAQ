program_scripts=startServer.tcl mg_mkconfig.tcl mg_cfgcontainers.tcl \
	mg_cfgprogram.tcl mg_stateedit.tcl mg_seqedit.tcl mg_loggeredit.tcl \
	mg_kvedit.tcl mg_authedit.tcl mg_config.tcl mg_startManager.tcl \
	mg_kvget.tcl mg_monitorOutput.tcl mg_shutdown.tcl mg_cfgEvlog.tcl \
	mg_startloggers.tcl mg_container_wizard.tcl \
	mg_readout_wizard.tcl


PY_PACKAGES=mg_database.py manager_client.py editablelist.py
PYPROGRAMS=mg_logwizard.py mg_program_wizard.py mg_import.py
server_scripts=mg_statusserver.tcl  mg_programserver.tcl \
	mg_kvserver.tcl mg_evlogserver.tcl

library_scripts=mg_containerpackage.tcl mg_containeredit.tcl \
	mg_programpackage.tcl mg_programedit.tcl mg_sequencepackage.tcl  \
	mg_kvstorepackage.tcl  mg_auth.tcl \
	mg_stateclient.tcl mg_stateclientui.tcl mg_clientutils.tcl \
	mg_pgmstatusclient.tcl mg_programstatusclientui.tcl \
	mg_kvclient.tcl mg_kvwidgets.tcl mg_clientui.tcl mg_serverutils.tcl \
	mg_OutputModel.tcl mg_OutputController.tcl mg_evlogpackage.tcl \
	mg_evlogEditViews.tcl mg_evlogEditModel.tcl mg_evlogEditController.tcl \
	mg_loggerRESTClient.tcl

docs=userguide.xml api.xml


test_scripts=container.test program.test sequence.test transitions.test \
	loggerdb.test kv.test authtest.test evlog.test


install-exec-local:
	for f in $(program_scripts); do $(INSTALL_PROGRAM) @srcdir@/$$f @bindir@/`basename $$f .tcl`; done
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/manager
	for f in $(library_scripts); do $(INSTALL_DATA) @srcdir@/$$f @prefix@/TclLibs/manager; done
	echo pkg_mkIndex -verbose @prefix@/TclLibs/manager $(library_scripts) \
	| @TCLSH_CMD@
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/manager_server
	for f in $(server_scripts); do \
		$(INSTALL_DATA) @srcdir@/$$f @prefix@/TclLibs/manager_server; \
	done
	rm -rf @prefix@/pythonLibs/nscldaq/__pycache__   # Force recompilation.
	(cd @srcdir@; $(INSTALL_SCRIPT) $(PY_PACKAGES) @prefix@/pythonLibs/nscldaq)
	for f in $(PYPROGRAMS); do \
		$(INSTALL_PROGRAM) @srcdir@/$$f @bindir@/`basename $$f .py` ; \
	done
	


check:
	$(INSTALL_PROGRAM)  @srcdir@/singexists.sh ~
	if test "`ssh  localhost  ~/singexists.sh`" = "yes" ; \
	then \
		$(INSTALL_DATA) @srcdir@/minimal.img ~ ;\
		for script in $(test_scripts); do \
		  DAQBIN=@bindir@ SRCDIR=@srcdir@ TCLLIBPATH=@prefix@/TclLibs\
 @TCLSH_CMD@ @srcdir@/$$script; \
		 if test $$? -ne 0; \
			then    \
                           exit -1; \
                 fi ; \
		done; \
	else \
		echo No singularity manager tests will not be done; \
	fi

EXTRA_DIST=$(program_scripts) $(library_scripts) $(test_scripts) \
	$(server_scripts) $(docs) \
	minimal.img singexists.sh $(PYPROGRAMS) $(PY_PACKAGES)
