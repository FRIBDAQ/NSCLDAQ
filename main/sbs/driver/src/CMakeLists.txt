nscldaq_add_library(btp VERSION_INFO 1:0:0
  SOURCES bt_bind.c bt_cas.c bt_chke.c bt_clre.c bt_ctrl.c bt_hw_diag.c
    bt_icbr.c bt_info.c bt_init.c bt_ior.c bt_iow.c bt_locking.c bt_mmap.c
    bt_name.c bt_open.c bt_or_io.c bt_perr.c bt_rdwr.c bt_reset.c bt_sema.c
    bt_send_irq.c bt_serr.c bt_stat.c bt_string.c bt_tas.c)
target_include_directories(btp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_definitions(btp PRIVATE BT1003 NDEBUG BT_INLINE
  _POSIX_C_SOURCE _POSIX_THREAD_SEMANTICS _XOPEN_SOURCE)
target_compile_options(btp PRIVATE -Wall -Wstrict-prototypes)
set_target_properties(btp PROPERTIES C_STANDARD 99 C_EXTENSIONS OFF)
target_link_libraries(btp PUBLIC NSCLDAQ::Threads)

nscldaq_install_headers(btpiflib.h)
