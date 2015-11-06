#!/bin/sh

### Test environment for this to run:
### - EVR executables must be on the PATH
### - EVR libs must be accessible on the LD LIBRARY_PATH
### - EVR_DRIVERS_PATH must point to the directory with *.ko files

if ! test-core1.sh; then exit 1; fi
if ! test-core2.sh; then exit 1; fi
if ! test-core3.sh; then exit 1; fi
	
echo "***************** ALL TESTS PASSED ************************"
	
