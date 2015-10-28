#!/bin/sh

### See the Makefile for the test environment

### Tests:
### - subscribing to events
### - debug write
### - reading events
### - normal and notifying events

. utils.sh

re_setup

create_vevr_sim_3
create_vevr_sim_6
DBG_SYS=/sys/class/modac-mng/evr-sim-mng/dbg

echo "---- 2A + 2B ---- "
command_verbose "test_virt_dev2   /dev/vevrh-sim-3   /dev/vevrh-sim-6    test2"

echo "---- 2C ---- "
### event code 1 as notifying event
### NOTE: vevrh-sim-6 will not be used...
command_verbose "test_virt_dev2   /dev/vevrh-sim-3   /dev/vevrh-sim-6    test2c"

echo "---- 2D ---- "
### more event codes as notifying events
### NOTE: vevrh-sim-6 will not be used...
command_verbose "test_virt_dev2   /dev/vevrh-sim-3   /dev/vevrh-sim-6    test2d"


exit 0

