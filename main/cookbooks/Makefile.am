install-exec-local:
	$(mkinstalldirs) @prefix@/share/recipes
	@DOCBOOK2HTML@ -o @builddir@/html @srcdir@/cookbook.xml
	for dir in images process readrings writerings epicspush offlinereglom; do \
		(cd @srcdir@ && find $$dir -type d -exec $(INSTALL) -d -m 755 @prefix@/share/recipes/{} \;) ; \
		(cd @srcdir@ && find $$dir -type f -exec $(INSTALL) -m 644 {} @prefix@/share/recipes/{} \;) \
	done
	(cd @builddir@ && find html -type d -exec $(INSTALL) -d -m 755 @prefix@/share/recipes/{} \;)
	(cd @builddir@ && find html -type f -exec $(INSTALL) -m 644 {} @prefix@/share/recipes/{} \;)