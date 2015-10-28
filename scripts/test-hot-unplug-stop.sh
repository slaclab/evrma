#!/bin/sh

### See the Makefile for the test environment

### stop the unplug test

pkill -f test-hot-unplug-1.sh
pkill -f test-hot-unplug-2.sh
pkill -f test-fiducial.sh
pkill test_virt_dev


exit 0
