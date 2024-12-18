bin_PROGRAMS=root_gui
bindir = @prefix@/sis3316
IMAGES_DIR=@datarootdir@/sis_images

root_gui_SOURCES=get_configuration_parameters.cpp \
    sis3316_cern_root_class.cpp \
    sis3316_class.cpp \
    sis3316_ethernet_access_class.cpp \
    sis3316_root_gui_flash.cpp \
    sis3316_root_gui_main.cpp \
    sis3316_root_gui_register_access.cpp \
    sis3316_root_gui_test1.cpp \
    sis3316_root_gui_udp_properties.cpp 

noinst_HEADERS=get_configuration_parameters.h \
    project_interface_define.h \
    project_system_define.h \
    rootIncludes.h \
    sis3316_cern_root_class.h \
    sis3316_class.h \
    sis3316_ethernet_access_class.h \
    sis3316.h \
    sis3316_root_gui_flash.h \
    sis3316_root_gui_monitor_size.h \
    sis3316_root_gui_register_access.h \
    sis3316_root_gui_test1.h \
    sis3316_root_gui_udp_properties.h 


root_gui_CPPFLAGS=@ROOT_CFLAGS@ \
	-I@srcdir@/../../vmusb -I@top_srcdir@/base/thread \
	-DIMAGES_DIR=\"$(IMAGES_DIR)\"

root_gui_LDADD=@ROOT_LDFLAGS@ @builddir@/../../vmusb/libVMUSB.la \
    @TCL_LDFLAGS@ 

IMAGES=sis1_sis3316.png  sislogo.bmp

# Install the images in $DAQSHARE/sis_images

install-exec-local:
	$(mkinstalldirs) $(IMAGES_DIR)
	for image in $(IMAGES) ; do $(INSTALL_DATA) @srcdir@/$$image $(IMAGES_DIR); done



EXTRA_DIST=$(IMAGES)
