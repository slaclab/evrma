### NOTE: SETTABLE_CONFIG
INSTALL_LOCATION ?= ../../buildroot-2019.08-x86_64
KERNEL_MODULE_EVRMA ?= $(PACKAGE_SITE_TOP)/linuxKernel_Modules/evrmaDriver/$(EVRMA_DRIVER_VERSION)/buildroot-2019.08-x86_64


# ===============================================================
# We don't want to use the compiler cache; 
# especially since not everyone has write access 
# to the buildroot installation.
#  ===============================================================
export CCACHE_DISABLE=1

include ../../src/Makefile.common


