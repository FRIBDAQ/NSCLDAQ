# Building NSCLDAQ with CMake

NSCLDAQ builds with CMake 3.18.4 or newer. The Autotools build
(`configure.ac`, `Makefile.am`, `m4/`) is still supported alongside it, and
CI builds and packages both (see below).

```bash
cmake -S main -B build -DCMAKE_INSTALL_PREFIX=/usr/opt/nscldaq/12.2-010 \
      -DWITH_ROOTSYS=/usr/opt/root/6.32.04 [options...]
cmake --build build -j$(nproc)
cmake --install build
cd build && ctest --output-on-failure        # or: cmake --build build --target check
```

`ctest` plays the role of `make check` and runs the same tests. As before,
many tests expect NSCLDAQ to be installed first (the Tcl tests load packages from
`${prefix}/TclLibs`). They also expect a port manager and ring master to be
running, and Tk tests need a display (CI uses `xvfb-run`). Test programs are
built with `all`; pass `-DBUILD_TESTING=OFF` to skip them.

## Options

Configure options map mechanically onto cache variables:

| configure                         | CMake                                    |
|-----------------------------------|------------------------------------------|
| `--prefix=DIR`                    | `-DCMAKE_INSTALL_PREFIX=DIR`             |
| `--enable-foo-bar[=yes]`          | `-DENABLE_FOO_BAR=ON`                    |
| `--disable-foo-bar`               | `-DENABLE_FOO_BAR=OFF`                   |
| `--with-foo-bar=VALUE`            | `-DWITH_FOO_BAR=VALUE`                   |
| `CXX=mpicxx`                      | `-DCMAKE_CXX_COMPILER=mpicxx`            |

The full list:

| Option | Default | Meaning |
|---|---|---|
| `ENABLE_USB` | OFF | VM-USB/CC-USB support (`WITH_USB_HEADERDIR`, `WITH_USB_LIBDIR` locate legacy libusb) |
| `ENABLE_DDAS` | OFF | XIA Pixie16 support; needs `WITH_XIAAPIDIR`, `WITH_FIRMWAREDIR` |
| `ENABLE_DDAS_DOCS` | OFF | DDAS Doxygen documentation (doxygen, dot) |
| `ENABLE_DOCS` | OFF | DocBook documentation (docbook2pdf, docbook2html, xmlto, mandb) |
| `ENABLE_EPICS_TOOLS` | OFF | EPICS tools; `WITH_EPICS_ROOTDIR` |
| `ENABLE_CAEN_DIGITIZER_SUPPORT` | OFF | CAEN first-generation digitizers; needs `WITH_CAEN_DIGITIZER_LIBROOT` |
| `ENABLE_CAEN_NEXTGEN` | OFF | CAEN next-gen digitizers (CAEN_FELib/CAEN_Dig2 in /usr/local) |
| `ENABLE_MVLC_STACK_GENERATOR` | OFF | MVLC config generation; needs `WITH_MVLCDIR`, yaml-cpp |
| `ENABLE_SBS` | OFF | SBS kernel driver; needs `WITH_SBSDIR` |
| `ENABLE_RUST_TOOLS` | OFF | Rust tools (cargo from /usr/cargo/bin, ~/.cargo/bin or PATH) |
| `ENABLE_VMUSB_PROMPT_BUSY` / `_END_EVENT` / `_IDLE_BUSY` | ON | VMUSB firmware workarounds (issue #294) |
| `WITH_ROOTSYS` | `$ROOTSYS` | ROOT installation (required, as it was for configure) |
| `WITH_INCORP_BUILD_CORES` | 1 | Parallel jobs for the incorporated packages |
| `WITH_GENGETOPT_PATH` | | gengetopt executable |
| `WITH_TCLCONFIG`, `WITH_TKCONFIG` | | tclConfig.sh / tkConfig.sh to use (found from `tclsh` otherwise) |
| `BUILD_TESTING` | ON | Build the unit tests |

Other tools are located the usual CMake way, for example `-DTCLSH_CMD=...`,
`-DBOOST_ROOT=...` and `-DOPENSSL_ROOT_DIR=...`.

CI runs both builds on every push (`.github/workflows/main.yml`):
`build.yml` is the Autotools build and `build-cmake.yml` the CMake build.
Each uploads its own artifacts. The CMake ones carry `cmake` in their names
(`nscldaq-<tag>-cmake-<os>.tar.gz`, `nscldaq-<tag>-dist-cmake-<os>.tar.gz`,
`usropt-nscldaq-<tag>-cmake-<os>.tar.gz`). A tagged release attaches the
artifacts of both builds.

The CMake CI recipe:

```bash
cmake -S main -B main/oot -DCMAKE_INSTALL_PREFIX=/usr/opt/nscldaq/$VERSION \
  -DENABLE_USB=ON -DENABLE_CAEN_DIGITIZER_SUPPORT=ON -DENABLE_DOCS=ON \
  -DENABLE_EPICS_TOOLS=ON \
  -DWITH_CAEN_DIGITIZER_LIBROOT=`cat /usr/opt/CAENDIGITIZERLIBS_PATH` \
  -DWITH_EPICS_ROOTDIR=`cat /usr/opt/EPICS_PATH` \
  -DENABLE_DDAS=ON -DENABLE_DDAS_DOCS=ON -DWITH_INCORP_BUILD_CORES=2 \
  -DWITH_XIAAPIDIR=`cat /usr/opt/XIAAPI_PATH` \
  -DWITH_FIRMWAREDIR=`cat /usr/opt/DDASFIRMWARE_PATH-12.2` \
  -DWITH_ROOTSYS=`cat /usr/opt/ROOT_PATH` \
  -DENABLE_MVLC_STACK_GENERATOR=ON -DWITH_MVLCDIR=`cat /usr/opt/MESYTECMVLC_PATH` \
  -DENABLE_RUST_TOOLS=ON -DCMAKE_CXX_COMPILER=`cat /usr/opt/MPI_PATH`
```

## Incorporated packages

libtcl++, UnifiedFormat, DDASFormat and tclhttpd are built as
`ExternalProject`s during `cmake --build`. If a package is already in the
source tree (`main/libtcl`, `main/unifiedformat`, `main/ddasformat`, put
there for example by `./tcl++incorp`), that copy is used. Otherwise the
pinned tag (`NSCLDAQ_LIBTCLPLUS_TAG`, `NSCLDAQ_UNIFIEDFORMAT_TAG`,
`NSCLDAQ_DDASFORMAT_TAG`) is cloned into the build tree.

Each package is configured for the final prefix but installed into
`build/incorp-stage` while building. NSCLDAQ compiles and links against that
staged copy, and `cmake --install` copies it into the prefix. Unlike
`configure`, nothing is written to the prefix before the install step.
`DESTDIR` staging works.

## Source packages

`cmake --build build --target package_source` replaces `make dist` and writes
`nscldaq-<version>.tar.gz`. To get a self-contained tarball, as `make dist`
produced, incorporate the packages into the source tree first.

## Differences from the Autotools build

With the CI options, the install tree was compared file by file against an
Autotools install of the same source. The file lists match apart from the
items below, and all installed ELF files resolve their libraries.

- Only shared libraries are built. libtool also built static `.a` archives
  (e.g. `TclLibs/SBSVme/libSBSVme.a`), and no `.la`/`.lai` files are
  installed.
- `share/examples/ReadNSCLDAQFiles` also gets the example sources. The
  Makefile copied that directory from the build tree, so an out-of-tree
  Autotools build (CI) installed only the configured `Makefile`.
- `rustup default stable` is no longer run at configure time. Cargo runs
  with `RUSTUP_TOOLCHAIN=stable` instead, so the user's default toolchain
  is not changed.
- Documentation (DocBook, Doxygen, chapter2book) is built during
  `cmake --build`, not during `make install`.
- The SBS kernel module (`ENABLE_SBS`) is built in a copy under the build
  directory, not in the source tree.
- Test executables are built by `all` (when `BUILD_TESTING` is ON), not only
  by `make check`.

Unchanged on purpose: the compiler's default C++ dialect is used
(AX_CXX_COMPILE_STDCXX_11 added no flag on modern compilers), the default
flags are `-g -O2` without `-DNDEBUG` (some code relies on `assert()` side
effects), and Tcl/Tk flags come from `tclConfig.sh`/`tkConfig.sh`, as
`m4/tcl.m4` did.

## Layout of the CMake files

- `CMakeLists.txt`: top level, subdirectory order, top-level installs
- `cmake/NSCLDAQOptions.cmake`: the options above
- `cmake/NSCLDAQCompiler.cmake`: language standard, global flags, config.h checks
- `cmake/NSCLDAQDependencies.cmake`: external packages as `NSCLDAQ::<Name>`
  targets. `NSCLDAQ::<Name>Headers` are header-only variants, for targets
  that used only the `..._CFLAGS` half of a flag pair.
- `cmake/NSCLDAQIncorp.cmake`: the incorporated packages
- `cmake/NSCLDAQHelpers.cmake`: `nscldaq_add_library`,
  `nscldaq_add_executable`, `nscldaq_add_test`, `nscldaq_add_tcl_test`,
  `nscldaq_gengetopt`, `nscldaq_install_headers`,
  `nscldaq_install_tcl_index`, `nscldaq_configure_file`, ...
