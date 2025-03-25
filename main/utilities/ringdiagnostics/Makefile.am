install-exec-local:
	$(mkinstalldirs) @bindir@
	$(INSTALL_PROGRAM) @srcdir@/pidtocommand.py @bindir@/pidtocommand
	rm -rf @prefix@/pythonLibs/nscldaq/__pycache__
	$(INSTALL_SCRIPT) @srcdir@/*.py @prefix@/pythonLibs/nscldaq
	$(INSTALL_PROGRAM) @srcdir@/ringdiagnostics.py @bindir@/ringdiagnostics


DOCS=ringdiags_docs.xml
PY_FILES= nscldaqutils.py  \
	ringdiagnostics.py  \
	RingUsage.py \
	pidtocommand.py  \
	RingModel.py    \
	RingView.py
EXTRA_DIST=$(DOCS) $(PY_FILES)
