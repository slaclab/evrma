### NOTE: SETTABLE_CONFIG
INSTALL_LOCATION ?= ../../buildroot-2016.11.1-x86_64
KERNEL_MODULE_EVRMA ?= $(PACKAGE_TOP)/linuxKernel_Modules/evrmaDriver/R1.0.12/buildroot-2016.11.1-x86_64


# ===============================================================
# We don't want to use the compiler cache; 
# especially since not everyone has write access 
# to the buildroot installation.
#  ===============================================================
export CCACHE_DISABLE=1

include ../../src/Makefile.common


