#!/bin/sh

### See the Makefile for the test environment

### Tests:
### - allocating pulsegens
### - setting pulsegen properties
### - checking pulsegen properties with sysfs and ioctl get
### - allocating outputs
### - setting outputs wiring

. utils.sh

check_config () {
	CFG_STATE=$(cat /sys/class/modac-virt/$TESTED_VEVR/config)

	if [ "$CFG_STATE" = "$1" ]; then
		true
	else
		error "The config content does not match: $CFG_STATE != $1"
	fi
}

check_result_tmp () {
	RES_STATE=$(cat $WRITABLE_TMP_DIR/_result_.tmp)

	if [ "$RES_STATE" = "$1" ]; then
		true
	else
		error "The _result_.tmp content does not match: $RES_STATE != $1"
	fi
}


re_setup

### auto chosen on create
TESTED_VEVR_NUM=1
TESTED_VEVR=vevrh-sim-pg

command_quiet "evrManager /dev/$SIM_MNG_DEV create vevrh-sim-pg"

command_verbose "evrManager /dev/$SIM_MNG_DEV alloc $TESTED_VEVR  pulsegen 0"
command_verbose "evrManager /dev/$SIM_MNG_DEV alloc $TESTED_VEVR  pulsegen 24"

### make sure the settings remain to be viewed
echo 1 > /sys/class/modac-virt/$TESTED_VEVR/config

PGPARS0="21,22,23"
PGPARS1="110,120,130"

echo "---- 3A ---- "

command_verbose "test_virt_dev /dev/$TESTED_VEVR pulsegen  0 properties 21 22 23"
check_config "0[0{$PGPARS0}1{0,0,0}]1[], subs[], res_set_cfg=1"
command_verbose "test_virt_dev /dev/$TESTED_VEVR pulsegen  0 get"
check_result_tmp "21 22 23"

PGCFG="0[0{$PGPARS0}1{$PGPARS1}]"

echo "---- 3B ---- "

command_verbose "test_virt_dev /dev/$TESTED_VEVR pulsegen  1 properties 110 120 130"
check_config "${PGCFG}1[], subs[], res_set_cfg=1"
command_verbose "test_virt_dev /dev/$TESTED_VEVR pulsegen  1 get"
check_result_tmp "110 120 130"

echo "---- 3C ---- "

### not allocated resource
command_quiet_must_fail "test_virt_dev /dev/$TESTED_VEVR pulsegen  2 properties 4 5 6"

echo "---- 3D ---- "

### no such physical output
command_quiet_must_fail "evrManager /dev/$SIM_MNG_DEV alloc $TESTED_VEVR  output 14"

TESTED_ABS_OUTPUT=4
TESTED_REL_OUTPUT=0

echo "---- 3E ---- "

### alloc phys. output 4
command_verbose "evrManager /dev/$SIM_MNG_DEV alloc $TESTED_VEVR  output $TESTED_ABS_OUTPUT"
check_config "${PGCFG}1[0{0}], subs[], res_set_cfg=1"

echo "---- 3E2 ---- "
### Set output to the source num.98
command_verbose "evrManager /dev/$SIM_MNG_DEV output $TESTED_VEVR  $TESTED_REL_OUTPUT  S 98"
check_config "${PGCFG}1[0{98}], subs[], res_set_cfg=1"

echo "---- 3E3 ---- "
### fails because the source can not be 12 (settable as pulsegen only)
command_quiet_must_fail "evrManager /dev/$SIM_MNG_DEV output $TESTED_VEVR  $TESTED_REL_OUTPUT  S 12"

echo "---- 3F ---- "

### fails because the pulsegen is not allocated
command_quiet_must_fail "evrManager /dev/$SIM_MNG_DEV output $TESTED_VEVR  $TESTED_REL_OUTPUT  P 2"

echo "---- 3G ---- "

### Set output to the pulsegen (with abs index=8)
command_verbose "evrManager /dev/$SIM_MNG_DEV output $TESTED_VEVR  $TESTED_REL_OUTPUT  P 0"
check_config "${PGCFG}1[0{8}], subs[], res_set_cfg=1"




exit 0
