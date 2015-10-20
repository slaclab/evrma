#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "linux-evrma.h"
#include "linux-evr-regs.h"
 
#include "libevrma.h"
#include "libevrma_internal.h"
#include "libevrma_dbg.h"
#include "utils.h"

// --------- debug only ----------

bool evrmaTest(EvrmaSession session, 
				   int what)
{
	Session *pSession = (Session *)session;
	
	uint8_t *ptr;
	
	#define MMAP_DATA_SIZE (sizeof(struct vevr_mmap_data))
	
	int prot;
	
	bool ret = false;
	
	if(what >= 0 && what < 10) {
		
		// mmap tests
	
		if(what == 1) {
			prot = PROT_READ | PROT_WRITE;
		} else {
			prot = PROT_READ;
		}
		
		ADBG("mmaping: %d", (int)MMAP_DATA_SIZE);
		
		ptr = (uint8_t *)mmap(NULL, MMAP_DATA_SIZE, 
							prot, MAP_SHARED, pSession->fd, 0);
		
		ret = ptr != MAP_FAILED;
		
		if(ret) {
			ADBG("mmap-ed pointer: 0x%x", (int)(size_t)ptr);
		} else {
			AERR("mmap failed with errno=%d", errno);
			return false;
		}
		
		for(unsigned i = 0; i < MMAP_DATA_SIZE; i ++) {
			if(ptr[i] != (uint8_t)(i & 0xFF)) {
				AERR("mem fail at %d: %d != %d", i, (int)ptr[i], i & 0xFF);
				return false;
			}
		}
		
		ADBG("read all");
		
		if(what == 1 || what == 3) {
			for(unsigned i = 0; i < MMAP_DATA_SIZE; i ++) {
				ptr[i] = 33;
			}
			
			ADBG("write all");
		}
	} else if(what == 10) {
		// sleep a lot for the application to stay alive an keep the VIRT_DEV open
		sleep(10000);
	} else if(what == 1000) {
		
		AERR("This is not supported anymore");
		abort(); 

	} else {
	}
		
	return ret;
}

void evrmaThreadRun(EvrmaSession session, int state)
{
	Session *pSession = (Session *)session;
	pSession->pauseThread = !state;
}


