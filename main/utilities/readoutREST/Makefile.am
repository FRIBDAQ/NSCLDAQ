
python_programs = rdo_control.py rdo_runFromKv.py rdo_titleFromKv.py
server_scripts=ReadoutControl.tcl ReadoutStatus.tcl ReadoutParameters.tcl
library_scripts=ReadoutREST.tcl ReadoutRESTClient.tcl ReadoutRESTUI.tcl \
	rdo_utils.tcl
share_scripts=rest_init_script.tcl

PythonLibDest = @prefix@/pythonLibs/nscldaq/readoutREST
PythonModules = readoutRestClient.py rdo_utils.py \
	RunInfo.py RunInfoController.py \
	RunState.py RunStateController.py \
	Eventlog.py EventlogController.py \
	ReadoutStatistics.py ReadoutStatisticsController.py \
	TimedRun.py TimedRunController.py

test_scripts=

LIBRARY_DIR=@prefix@/TclLibs/readoutREST
SERVER_DIR=$(LIBRARY_DIR)/server


EXTRA_DIST=$(program_scripts) $(library_scripts) $(test_scripts) \
	$(server_scripts) $(share_scripts) rdoManager.xml $(PythonModules)



install-exec-hook:
	for f in $(python_programs); do $(INSTALL_PROGRAM) @srcdir@/$$f @bindir@/`basename $$f .py`; done
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
	$(mkinstalldirs) $(PythonLibDest)
	for f in $(PythonModules); do \
		$(INSTALL_SCRIPT) @srcdir@/$$f $(PythonLibDest); \
	done


check:
	for script in $(test_scripts); do \
		DAQBIN=@bindir@ SRCDIR=@srcdir@ TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/$$script; \
	done; 
