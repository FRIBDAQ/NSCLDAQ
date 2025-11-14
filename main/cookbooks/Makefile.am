install-exec-local:
	$(mkinstalldirs) @prefix@/share/recipes
	@DOCBOOK2HTML@ -o @builddir@/html @srcdir@/cookbook.xml
	(cd @srcdir@; tar czf - images process readrings writerings epicspush offlinereglom) \
		| (cd @prefix@/share/recipes; tar xzf -)
	(cd @builddir@; tar czf - html) | (cd @prefix@/share/recipes; tar xzf -)

EXTRA_DIST=cookbook.xml eventbuilt images process readrings writerings
