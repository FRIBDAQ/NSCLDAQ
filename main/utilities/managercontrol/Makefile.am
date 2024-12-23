MODULE_FILES=config.py cfgwizard.py maingui.py programlist.py loggerlist.py
MODULE_DEST=@prefix@/pythonLibs/nscldaq/manager_control

PROGRAM_FILE=mg_control.py

install-data-local:
	$(MKDIR_P) $(MODULE_DEST)
	for f in $(MODULE_FILES); do $(INSTALL_SCRIPT) @srcdir@/$$f $(MODULE_DEST); done
	touch $(MODULE_DEST)/__init__.py
	$(INSTALL_PROGRAM) @srcdir@/$(PROGRAM_FILE) @bindir@/`basename $(PROGRAM_FILE) .py`

EXTRA_DIST=$(MODULE_FILES) $(PROGRAM_FILE)
