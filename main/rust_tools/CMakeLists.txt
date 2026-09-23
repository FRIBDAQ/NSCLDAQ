#
#  Rust tools, built with 'cargo build --release'.  Only executables are
#  supported; they are installed into bin/rust.
#
#  Two kinds of packages:
#   * External repositories cloned into this binary directory (configure
#     did the cloning; here it happens at build time, only if the clone is
#     not already there).  They are built in place.
#   * Packages in this source directory (RUST_BUILTIN) built with
#     --target-dir in this binary directory.
#
#  The executables to install are the 'name =' entries of each Cargo.toml:
#     grep "name =" Cargo.toml | cut -d= -f2 | tr -d " \""
#
#  configure ran 'rustup default stable'; rather than change the user's
#  default toolchain cargo is run with RUSTUP_TOOLCHAIN=stable.
#

set(_rust_repos
  https://github.com/FRIBDAQ/rust-portman
  https://github.com/FRIBDAQ/rust-ringtostdout
  https://github.com/FRIBDAQ/rust-ringmaster)

# Maintain this with the subdirectories of this source directory that hold
# rust packages.
set(_rust_builtin ring_monitor)

# cargo with the toolchain directory appended to the (build time) PATH.
set(_cargo sh -c
  "RUSTUP_TOOLCHAIN=stable PATH=\"\$PATH:${RUST_TOOL_DIR}\" exec \"\$0\" \"\$@\""
  ${RUST_CARGO})

set(_bin ${CMAKE_CURRENT_BINARY_DIR})

# The cargo builds are serialized (each is internally parallel and they
# share the cargo package cache); _prev chains them.

set(_prev "")
set(_rust_subdirs "")

foreach(_repo IN LISTS _rust_repos)
  get_filename_component(_sub ${_repo} NAME)
  list(APPEND _rust_subdirs ${_sub})
  add_custom_command(
    OUTPUT ${_bin}/${_sub}/Cargo.toml
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${_bin}/${_sub}
    COMMAND ${GIT} clone ${_repo}
    WORKING_DIRECTORY ${_bin}
    COMMENT "Cloning rust tool from ${_repo}"
    VERBATIM)
  add_custom_target(rust_${_sub} ALL
    COMMAND ${_cargo} build --release
    DEPENDS ${_bin}/${_sub}/Cargo.toml
    WORKING_DIRECTORY ${_bin}/${_sub}
    COMMENT "cargo build --release (${_sub})"
    VERBATIM)
  if(_prev)
    add_dependencies(rust_${_sub} ${_prev})
  endif()
  set(_prev rust_${_sub})
endforeach()

foreach(_tool IN LISTS _rust_builtin)
  add_custom_target(rust_${_tool} ALL
    COMMAND ${_cargo} build --release --target-dir ${_bin}/${_tool}/target
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_tool}
    COMMENT "cargo build --release (${_tool})"
    VERBATIM)
  if(_prev)
    add_dependencies(rust_${_tool} ${_prev})
  endif()
  set(_prev rust_${_tool})
endforeach()

#---------------------------------------------------------------------------
#  Installation (was install-exec-local): parse each Cargo.toml at install
#  time (the cloned ones only exist after the build).

nscldaq_install_mkdir(bin/rust)

# _rust_install_code(<Cargo.toml> <release dir>) -> CMake code appending the
# executables' paths to _files.
function(_rust_install_code out toml reldir)
  set(${out} "
    file(STRINGS \"${toml}\" _lines REGEX \"name =\")
    foreach(_l IN LISTS _lines)
      string(REPLACE \"=\" \";\" _fields \"\${_l}\")
      list(LENGTH _fields _n)
      if(_n GREATER 1)
        list(GET _fields 1 _e)
        string(REGEX REPLACE \"[ \\\"]\" \"\" _e \"\${_e}\")
        if(NOT _e STREQUAL \"\")
          list(APPEND _files \"${reldir}/\${_e}\")
        endif()
      endif()
    endforeach()
" PARENT_SCOPE)
endfunction()

set(_code "set(_files \"\")\n")
foreach(_sub IN LISTS _rust_subdirs)
  _rust_install_code(_c ${_bin}/${_sub}/Cargo.toml ${_bin}/${_sub}/target/release)
  string(APPEND _code "${_c}")
endforeach()
string(APPEND _code "
    file(INSTALL \${_files} TYPE PROGRAM
      DESTINATION \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/rust\")
")
install(CODE "${_code}")

set(_code "set(_files \"\")\n")
foreach(_tool IN LISTS _rust_builtin)
  _rust_install_code(_c ${CMAKE_CURRENT_SOURCE_DIR}/${_tool}/Cargo.toml
    ${_bin}/${_tool}/target/release)
  string(APPEND _code "${_c}")
endforeach()
string(APPEND _code "
    if(_files)
      file(INSTALL \${_files} TYPE PROGRAM
        DESTINATION \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/rust\")
    endif()
")
install(CODE "${_code}")

#---------------------------------------------------------------------------
#  Tests (was check-local): cargo test for each builtin.

foreach(_tool IN LISTS _rust_builtin)
  nscldaq_add_test(NAME ${_tool}_cargo_test
    COMMAND ${_cargo} test --target-dir ${_bin}/${_tool}/target -- --test-threads 1
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_tool})
endforeach()
