#
# Build the rust stuff in --release mode
#
all-local:
	for tool in @RUST_SUBDIRS@ ; do (cd $${tool}; @RUST_CARGO@ build --release); done

install-exec-local:
