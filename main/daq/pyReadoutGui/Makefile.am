TESTABLE_MODULES=StateMachine.py
MODULES=$(TESTABLE_MODULES) StateView.py

MODULE_DIR=@prefix@/pythonLibs/nscldaq/readoutgui
install-exec-local:
	$(mkinstalldirs) $(MODULE_DIR)
	for f in $(MODULES); \
	do \
	  $(INSTALL_SCRIPT) @srcdir@/$$f $(MODULE_DIR); \
	done

TESTS=$(TESTABLE_MODULES)

EXTRA_DIST=$(MODULES)

