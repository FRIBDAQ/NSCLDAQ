#
# Build the rust stuff in --release mode
#
#  Note, only executables are supported.  libs should be published and
#  pulled in via dependencies in the executable's Cargo.toml
#
#  Note the are two types of Rust packagres.
#  Those which configure brought in from othe4r repositories
#  and those which are in the src dir.
#  The @RUST_SUBDIRS@ config variable has the list of
#  subdirectories in @builddir@ which were brought in by config.
#  Those can be directly built as they are in the @builddir@ to begin with.
#  The second set of rust items have been added to the NSCLDAQ git repo.
#  Those are listed in RUST_BUILTIN and must be maintained in this file
#  as new rust code is added to the repository.  Those are relative to
#  @srcdir@ and must be built with --target-dir on cargo to support out of
#  tree builds.  In the end all will wind up in some
#  @builddir@/package/target/release
#
#  Cargo.toml in the source directory can be parsed to get the name of the
#  file to install:
#  Specifically:
#    cat Cargo.toml | grep "name =" | cut -d= -f2|tr -d " \""
#  Produces the name of the executable produced.

#  Maintain the variable below with the space separated names of the
# subdirs containing rust packages in this tree of the repo.

RUST_BUILTIN=
INSTALL_DIR=@bindir@/rust     # Segragate the rust bins..

all-local:
	for tool in @RUST_SUBDIRS@ ; do (cd $${tool}; @RUST_CARGO@ build --release); done
	for tool in $(RUST_BUILTIN); do \
		(cd @srcdir@/$$tool; @RUST_CARGO@ build --release --target-dir @builddir@/$$tool/target); \
	done


#
# Installing the packages is a matter of parsing the cargos to get the
# list of files to install and then installing them.  See comments above about
# how to parse Cargo.toml.
#

install-exec-local:
	$(mkinstalldirs) $(INSTALL_DIR)
	files=""; \
	for tool in @RUST_SUBDIRS@; do \
		executable=`cat $$tool/Cargo.toml | grep "name =" | cut -d= -f2|tr -d " \""`; \
		executable="@builddir@/$$tool/target/release/$${executable}"; \
		files="$${files} $${executable}";   \
	done; \
	$(INSTALL_PROGRAM) $${files} $(INSTALL_DIR)

