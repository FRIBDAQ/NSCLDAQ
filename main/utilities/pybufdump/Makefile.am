
MODULES = pyUI.py pybufdumpController.py
EXAMPLES=samplePlugin.py
PROGRAM_FILES=pybufdump.py

install-exec-local:
	$(mkinstalldirs) @datarootdir@/pydumper
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/pydumper
	for f in $(PROGRAM_FILES) ; do \
		$(INSTALL_PROGRAM) @srcdir@/$$f @bindir@/`basename $$f .py`; \
	done
	for f in $(EXAMPLES); do \
		$(INSTALL_DATA) @srcdir@/$$f @datarootdir@/pydumper; \
	done
	for f in $(MODULES) ; do \
		$(INSTALL_SCRIPT) @srcdir@/$$f @prefix@/pythonLibs/nscldaq/pydumper; \
	done

EXTRA_DIST=$(MODULES) $(EXAMPLES) $(PROGRAMS)

