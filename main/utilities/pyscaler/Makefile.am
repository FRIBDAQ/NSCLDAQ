# Install the pytyhon compiled module
# Were we want it noty where autotools
# wants it:

pymodule_dir = @prefix@/pythonLibs/nscldaq
MODULES      = channel.py configfile.py datasource.py RunInfo.py \
	ScalerGui.py ScalerPage.py ScalerPlot.py source_manager.py

MAIN=main.py
install-exec-hook:
	$(mkinstalldirs) $(pymodule_dir)
	$(mkinstalldirs) $(pymodule_dir)/pyscaler
	for f in $(MODULES); do \
		$(INSTALL_SCRIPT) @srcdir@/$$f $(pymodule_dir)/pyscaler; \
	done
	$(INSTALL_SCRIPT) @srcdir@/$(MAIN) @bindir@/pyScaler

# Some modules have tests that can run:

TESTS=channel.py configfile.py

EXTRA_DIST=$(MODULES) $(MAIN)
