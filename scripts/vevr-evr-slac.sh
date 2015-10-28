#!/bin/sh

### Tests:
### - setup the VEVR for the SLAC-based-EVR

### NOTE: the kernel must be loaded in

evrManager /dev/evr1mng init
### recreate the vevr from scratch
evrManager /dev/evr1mng destroy vevr1 >/dev/null
evrManager /dev/evr1mng create vevr1

sleep 1 ### wait for vevr1 to appear
chmod 666 /dev/vevr1

evrManager /dev/evr1mng alloc vevr1 output 0
evrManager /dev/evr1mng alloc vevr1 output 1
evrManager /dev/evr1mng alloc vevr1 output 2
evrManager /dev/evr1mng alloc vevr1 output 3
evrManager /dev/evr1mng alloc vevr1 output 4
evrManager /dev/evr1mng alloc vevr1 output 5
evrManager /dev/evr1mng alloc vevr1 output 6
evrManager /dev/evr1mng alloc vevr1 output 7
evrManager /dev/evr1mng alloc vevr1 output 8
evrManager /dev/evr1mng alloc vevr1 output 9
evrManager /dev/evr1mng alloc vevr1 output 10
evrManager /dev/evr1mng alloc vevr1 output 11

evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng alloc vevr1 pulsegen

evrManager /dev/evr1mng output vevr1 0 P 0
evrManager /dev/evr1mng output vevr1 1 P 1
evrManager /dev/evr1mng output vevr1 2 P 2 
evrManager /dev/evr1mng output vevr1 3 P 3
evrManager /dev/evr1mng output vevr1 4 P 4
evrManager /dev/evr1mng output vevr1 5 P 5
evrManager /dev/evr1mng output vevr1 6 P 6
evrManager /dev/evr1mng output vevr1 7 P 7
evrManager /dev/evr1mng output vevr1 8 P 8
evrManager /dev/evr1mng output vevr1 9 P 9
evrManager /dev/evr1mng output vevr1 10 P 10
evrManager /dev/evr1mng output vevr1 11 P 11

# test_virt_dev /dev/vevr1 fiducial
