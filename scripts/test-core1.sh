#!/bin/sh

### See the Makefile for the test environment

### Tests:
### - insmod, rmmod
### - creation/destruction of VIRT_DEVs

. utils.sh

syfs_devs () {
	
	to_cmp=
	
	for c in `ls /sys/class/modac-mng | sort -dr`; do
		to_cmp=${to_cmp}$c
		to_cmp=${to_cmp}" "
	done
	
	echo $to_cmp
	
	to_cmp2=
	
	for c in $1; do
		to_cmp2=${to_cmp2}$c
		to_cmp2=${to_cmp2}" "
	done
	
	if [ "$to_cmp" = "$to_cmp2" ]; then
		true
	else
		error "1: The sysfs dir content does not match: '$to_cmp' != '$to_cmp2'"
	fi
	
	
	to_cmp=
	
	for c in `ls /sys/class/modac-virt`; do
		to_cmp=${to_cmp}$c
		to_cmp=${to_cmp}" "
	done
	
	to_cmp2=
	
	for c in $2; do
		to_cmp2=${to_cmp2}$c
		to_cmp2=${to_cmp2}" "
	done
	
	echo $to_cmp
	
	if [ "$to_cmp" = "$to_cmp2" ]; then
		true
	else
		error "2: The sysfs dir content does not match: '$to_cmp' != '$to_cmp2'"
	fi
	
	
}

syfs_res () {
	
	to_cmp=
	
	for c in `cat /sys/class/modac-mng/$SIM_MNG_DEV/alloc`; do
		to_cmp=${to_cmp}$c
		to_cmp=${to_cmp}" "
	done
	
	echo $to_cmp
	
	### see the extra space, it was add in the for loop, too
	if [ "$to_cmp" = "$1 " ]; then
		true
	else
		error "The sysfs dir content does not match: '$to_cmp' != '$1'"
	fi
}

re_setup

### this id was auto chosen on create
VEVR_PULSEGEN_ID=1


echo "---- 1A ---- "
command_quiet "evrManager /dev/$SIM_MNG_DEV create vevrh-pulsgen-test"
syfs_devs "$SIM_MNG_DEV $EVR_MNG_DEVS" "vevrh-pulsgen-test"

echo "---- 1B ---- "
create_vevr_sim_3
syfs_devs "$SIM_MNG_DEV $EVR_MNG_DEVS" "vevrh-pulsgen-test vevrh-sim-3"

X0_1="0 p 0 -1 1 p 1 -1"
X3_7="3 p 3 -1 4 p 4 -1 5 p 5 -1 6 p 6 -1 7 p 7 -1"
X9_19="9 p 9 -1 10 p 10 -1 11 p 11 -1 12 p 12 -1 13 p 13 -1 14 p 14 -1 15 p 15 -1 16 o 0 -1 17 o 1 -1 18 o 2 -1 19 o 3 -1"
X21_="21 o 5 -1 22 o 6 -1 23 o 7 -1 24 o 8 -1 25 o 9 -1"
X9_="$X9_19 20 o 4 -1 $X21_"
OUT_X9_="$X9_19 20 o 4 $VEVR_PULSEGEN_ID $X21_"

echo "---- 1C ---- "
syfs_res "$X0_1 2 p 2 -1 $X3_7 8 p 8 -1 $X9_"

command_verbose "evrManager /dev/$SIM_MNG_DEV alloc vevrh-pulsgen-test  pulsegen 0"
syfs_res "$X0_1 2 p 2 -1 $X3_7 8 p 8 $VEVR_PULSEGEN_ID $X9_"

echo "---- 1D ---- "
command_verbose "evrManager /dev/$SIM_MNG_DEV alloc vevrh-pulsgen-test  pulsegen 24"
syfs_res "$X0_1 2 p 2 $VEVR_PULSEGEN_ID $X3_7 8 p 8 $VEVR_PULSEGEN_ID $X9_"

echo "---- 1E ---- "
command_verbose "evrManager /dev/$SIM_MNG_DEV alloc vevrh-pulsgen-test  output 4"
syfs_res "$X0_1 2 p 2 $VEVR_PULSEGEN_ID $X3_7 8 p 8 $VEVR_PULSEGEN_ID $OUT_X9_"

### allocating the same output again must fail
command_verbose_must_fail "evrManager /dev/$SIM_MNG_DEV alloc vevrh-pulsgen-test  output 4"


echo "---- 1F ---- "

command_verbose "evrManager /dev/$SIM_MNG_DEV destroy vevrh-pulsgen-test"
syfs_devs "$SIM_MNG_DEV $EVR_MNG_DEVS" "vevrh-sim-3"

echo "---- 1G ---- "
create_vevr_sim_6
syfs_devs "$SIM_MNG_DEV $EVR_MNG_DEVS" "vevrh-sim-3 vevrh-sim-6"


command_quiet "rmmod evrma"

if [ -e /sys/class/modac ]; then
	error "sysfs modac still exists!"
fi

exit 0
