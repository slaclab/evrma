#!/bin/sh

SIM_MNG_DEV="evr-sim-mng"
EVR_MNG_DEV0="evr0mng"


error () {
	echo "********************** ERRROR *************************** "
	echo "********************** ERRROR *************************** "
	echo "ERRROR: " $1
	echo "********************** ERRROR *************************** "
	echo "********************** ERRROR *************************** "
	exit 1
}

command_quiet_no_fail () {
	$1 >/dev/null 2>/dev/null
}

command_quiet () {
	if ! $1 > $WRITABLE_TMP_DIR/1.log.tmp; then 
		cat $WRITABLE_TMP_DIR/1.log.tmp
		rm $WRITABLE_TMP_DIR/1.log.tmp
		error "Command failed: $1"
	fi
}

command_verbose () {
	if ! $1; then 
		error "Command failed: $1"
	fi
}

command_verbose_must_fail () {
	if $1; then 
		error "Command shouldn't have succeeded: $1"
	fi
}

command_quiet_must_fail () {
	if $1 > $WRITABLE_TMP_DIR/1.log.tmp; then 
		error "Command shouldn't have succeeded: $1"
	fi
}

cleanup () {
	pkill test_virt_dev
	pkill test_virt_dev
	pkill test_virt_dev
	pkill test_virt_dev
	pkill test_virt_dev
	pkill test_virt_dev
	command_quiet_no_fail "rmmod evrma"
}

re_setup () {
	### first remove the driver
	cleanup
	### re-insert it
	command_verbose "insmod $EVR_DRIVERS_PATH/evrma.ko"
}

create_vevr_sim_3 () {
	command_quiet "evrManager /dev/$SIM_MNG_DEV create vevrh-sim-3"
}

create_vevr_sim_5 () {
	command_quiet "evrManager /dev/$SIM_MNG_DEV create vevrh-sim-5"
}

create_vevr_sim_6 () {
	command_quiet "evrManager /dev/$SIM_MNG_DEV create vevrh-sim-6"
}

echo "--------------------------------------------------------------"
 
