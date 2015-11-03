#!/bin/sh

### Tests:
### - setup the VEVR for the EVR300

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
evrManager /dev/evr0mng alloc vevr0 output 3
evrManager /dev/evr0mng alloc vevr0 output 4
evrManager /dev/evr0mng alloc vevr0 output 5
evrManager /dev/evr0mng alloc vevr0 output 6
evrManager /dev/evr0mng alloc vevr0 output 7
evrManager /dev/evr0mng alloc vevr0 output 8
evrManager /dev/evr0mng alloc vevr0 output 9
evrManager /dev/evr0mng alloc vevr0 output 10
evrManager /dev/evr0mng alloc vevr0 output 11
evrManager /dev/evr0mng alloc vevr0 output 12
evrManager /dev/evr0mng alloc vevr0 output 13
evrManager /dev/evr0mng alloc vevr0 output 14
evrManager /dev/evr0mng alloc vevr0 output 15

### This is the output of 
###     cat /sys/class/modac-mng/evr
###
# abs:0,bits:presc=16;delay=32;width=32
# abs:1,bits:presc=16;delay=32;width=32
# abs:2,bits:presc=16;delay=32;width=32
# abs:3,bits:presc=16;delay=32;width=32
# abs:4,bits:presc=0;delay=32;width=16
# abs:5,bits:presc=0;delay=32;width=16
# abs:6,bits:presc=0;delay=32;width=16
# abs:7,bits:presc=0;delay=32;width=16
# abs:8,bits:presc=0;delay=32;width=16
# abs:9,bits:presc=0;delay=32;width=16
# abs:10,bits:presc=0;delay=32;width=16
# abs:11,bits:presc=0;delay=32;width=16
# abs:12,bits:presc=0;delay=32;width=16
# abs:13,bits:presc=0;delay=32;width=16
# abs:14,bits:presc=0;delay=32;width=16
# abs:15,bits:presc=0;delay=32;width=16

evrManager /dev/evr0mng alloc vevr0 pulsegen 16 32 32
evrManager /dev/evr0mng alloc vevr0 pulsegen 16 32 32
evrManager /dev/evr0mng alloc vevr0 pulsegen 16 32 32
evrManager /dev/evr0mng alloc vevr0 pulsegen 16 32 32
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng alloc vevr0 pulsegen
# evrManager /dev/evr0mng alloc vevr0 pulsegen
# evrManager /dev/evr0mng alloc vevr0 pulsegen

evrManager /dev/evr0mng output vevr0 0 P 0
evrManager /dev/evr0mng output vevr0 1 P 1
evrManager /dev/evr0mng output vevr0 2 P 2 
evrManager /dev/evr0mng output vevr0 3 P 3
evrManager /dev/evr0mng output vevr0 4 P 4
evrManager /dev/evr0mng output vevr0 5 P 5
evrManager /dev/evr0mng output vevr0 6 P 6
evrManager /dev/evr0mng output vevr0 7 P 7
evrManager /dev/evr0mng output vevr0 8 P 8
evrManager /dev/evr0mng output vevr0 9 P 9
evrManager /dev/evr0mng output vevr0 10 P 10
evrManager /dev/evr0mng output vevr0 11 P 11
evrManager /dev/evr0mng output vevr0 12 P 12
evrManager /dev/evr0mng output vevr0 13 P 13
### set the last two outputs to the first two pulse gens to test the multiple
### pulsegen use
evrManager /dev/evr0mng output vevr0 14 P 0
evrManager /dev/evr0mng output vevr0 15 P 1

# test_virt_dev /dev/vevr0 fiducial

exit 0

for out in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17; do evrManager /dev/evr0mng output vevr0 $out S 62; echo $out; done
