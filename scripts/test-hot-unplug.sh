#!/bin/sh

### See the Makefile for the test environment

### test what happens if test-fiducial.sh runs in a loop and the evr0mng
### is destroyed (semi-)randomly


. utils.sh


test-hot-unplug-1.sh &
test-hot-unplug-2.sh &


exit 0
