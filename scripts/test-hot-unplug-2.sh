#!/bin/sh

RANDOM=1
while true; do 
	RANDOM=$(( RANDOM + 1 ))
	sleep $(( ( RANDOM % 40 )  + 1 ))
	echo "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"
	dmesg -c
	echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
	echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ HOT-UNPLUG @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
	echo 1 > /sys/bus/pci/devices/0000:04:00.0/remove
	sleep 1
	pkill test_virt_dev
	pkill test-fiducial.sh
	sleep 1
	echo "Rescaning the PCI to bring the device back"
	echo 1 > /sys/bus/pci/rescan
	echo "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"
	dmesg -c
	echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
done
