program_scripts=rdo_control.tcl rdo_titleFromKv.tcl rdo_runFromKv.tcl \
	rdo_RunControl.tcl
server_scripts=ReadoutControl.tcl ReadoutStatus.tcl ReadoutParameters.tcl
library_scripts=ReadoutREST.tcl ReadoutRESTClient.tcl ReadoutRESTUI.tcl \
	rdo_utils.tcl
share_scripts=rest_init_script.tcl

test_scripts=

LIBRARY_DIR=@prefix@/TclLibs/readoutREST
SERVER_DIR=$(LIBRARY_DIR)/server


EXTRA_DIST=$(program_scripts) $(library_scripts) $(test_scripts) \
	$(server_scripts) $(share_scripts) rdoManager.xml



install-exec-local:
	for f in $(program_scripts); do $(INSTALL_PROGRAM) @srcdir@/$$f @bindir@/`basename $$f .tcl`; done
	$(mkinstalldirs) $(LIBRARY_DIR) $(SERVER_DIR)
	for f in $(library_scripts); do $(INSTALL_DATA) @srcdir@/$$f $(LIBRARY_DIR); done
	echo pkg_mkIndex -verbose $(LIBRARY_DIR)  $(library_scripts) \
	| @TCLSH_CMD@
	for f in $(server_scripts); do \
		$(INSTALL_DATA) @srcdir@/$$f $(SERVER_DIR); \
	done
	$(mkinstalldirs) @datarootdir@/scripts
	for f in $(share_scripts); do \
		$(INSTALL_DATA) @srcdir@/$$f @datarootdir@/scripts; \
	done


check:
	for script in $(test_scripts); do \
		DAQBIN=@bindir@ SRCDIR=@srcdir@ TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/$$script; \
	done; 
