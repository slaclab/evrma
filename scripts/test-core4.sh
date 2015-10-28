#!/bin/sh

### See the Makefile for the test environment

### Tests:
### - mmap

. utils.sh

re_setup

TESTED_VEVR_NUM=6
TESTED_VEVR=vevrh-sim-$TESTED_VEVR_NUM

create_vevr_sim_$TESTED_VEVR_NUM

echo "*" RO user space open, just reading, should pass
command_verbose "test_virt_dev /dev/$TESTED_VEVR testl 0"

echo "*" RW user space open, should fail because of kernel RO
command_verbose_must_fail "test_virt_dev /dev/$TESTED_VEVR testl 1"

echo "*" Should pass because the write must not have changed the table
command_verbose "test_virt_dev /dev/$TESTED_VEVR testl 0"

echo "*" Must fail because RO user space open should have influence
command_verbose_must_fail "test_virt_dev /dev/$TESTED_VEVR testl 3"

echo "*" Should pass because the write must not change the table
command_verbose "test_virt_dev /dev/$TESTED_VEVR testl 0"


exit 0
