#!/bin/sh

### Tests:
### - setup the VEVR for the EVR230

### NOTE: the kernel must be loaded in

evrManager /dev/evr0mng init
### recreate the vevr from scratch
evrManager /dev/evr0mng destroy vevr0 >/dev/null
evrManager /dev/evr0mng create vevr0

sleep 1 ### wait for vevr0 to appear
chmod 666 /dev/vevr0

evrManager /dev/evr0mng alloc vevr0 output 0
evrManager /dev/evr0mng alloc vevr0 output 1
evrManager /dev/evr0mng alloc vevr0 output 2
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng output vevr0 0 P 0
evrManager /dev/evr0mng output vevr0 1 P 1
evrManager /dev/evr0mng output vevr0 2 P 2 

# test_virt_dev /dev/vevr0 fiducial
