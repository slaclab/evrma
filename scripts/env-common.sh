export TESTS_DIR=.

export WRITABLE_TMP_DIR=/home/laci/tmp
mkdir -p $WRITABLE_TMP_DIR

export EVR_DRIVERS_PATH=/afs/slac/g/lcls/package/linuxKernel_Modules/evrmaDriver/R1-0-p3/buildroot-2014.08-glibc-x86_64
export EVRMA_PATH=/afs/slac/g/lcls/package/evrma/R1-0-p3/buildroot-2014.08-glibc-x86_64
export EVR_MANAGER_PATH=/afs/slac/g/lcls/package/evrManager/R1-0-p3/buildroot-2014.08-glibc-x86_64

export PATH=$PATH:$TESTS_DIR:$EVRMA_PATH/bin:$EVR_MANAGER_PATH/bin
export LD_LIBRARY_PATH= ### static linking...

export EVR_MAJOR=247


### this can be left from boot
rmmod evr_device 2>/dev/null >/dev/null
rmmod pci_mrfevr300 2>/dev/null >/dev/null
rmmod pci_mrfevr 2>/dev/null >/dev/null
rmmod emcor 2>/dev/null >/dev/null
