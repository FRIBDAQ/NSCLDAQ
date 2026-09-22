TESTABLE_MODULES=StateMachine.py DataSourceManager.py
MODULES=$(TESTABLE_MODULES) StateView.py RunTimerView.py RunParamsView.py \
	DataSource.py DelaySource.py
INTERACTIVE_TESTS=testRunStateController.py
MODULE_DIR=@prefix@/pythonLibs/nscldaq/readoutgui
install-exec-local:
	$(mkinstalldirs) $(MODULE_DIR)
	for f in $(MODULES); \
	do \
	  $(INSTALL_SCRIPT) @srcdir@/$$f $(MODULE_DIR); \
	done


TESTS_ENVIRONMENT=PYTHONPATH=@prefix@/pythonLibs
TESTS=$(TESTABLE_MODULES)

EXTRA_DIST=$(MODULES) $(INTERActive_TESTS)

