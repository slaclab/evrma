#!/bin/sh

### Tests:
### - test the fiducial events comming

### Argument $1: the number of the EVR card (0, 1, ..., 6)

### NOTE: setup must be run before

. utils.sh

command_quiet "evrManager /dev/evr$1mng init"

### destroy if existing for fresh start
command_quiet_no_fail "evrManager /dev/evr$1mng destroy vevr-fid"

command_quiet "evrManager /dev/evr$1mng create vevr-fid"

### wait for the vevr to be created
sleep 1

command_verbose "test_virt_dev /dev/vevr-fid fiducial"

exit 0

