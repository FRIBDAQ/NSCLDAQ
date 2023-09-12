# Workaround for /usr/bin/install not having recursive copy
install-exec-local:
	$(mkinstalldirs) @prefix@/share/ddasdocs/html
	$(mkinstalldirs) @prefix@/share/ddasdocs/html/search
	$(DOXYGEN) Doxyfile
	find ./html/ -type f -exec $(INSTALL_DATA) "{}" "@prefix@/share/ddasdocs/html" \;
	find ./html/search/ -type f -exec $(INSTALL_DATA) "{}" "@prefix@/share/ddasdocs/html/search" \;

clean-local:
	rm -rf html latex
