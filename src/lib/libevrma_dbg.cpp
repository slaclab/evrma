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

int evrmaGetFd(EvrmaSession session)
{
	Session *pSession = (Session *)session;
	return pSession->fd;
}





#ifdef DBG_MEASURE_TIME_FROM_IRQ_TO_USER

#define MHZ 119
#define ACCEPTABLE_LIMIT (MHZ * acceptableLimitUsec)

void evrmaTimeDebug(int event, void *data, int acceptableLimitUsec,
		uint32_t userStart, uint32_t userEnd)
{
	struct evr_data_fifo_event *evData = (struct evr_data_fifo_event *)data;

	enum SpecialEvent {
		SE_PUT_BUFF = 256,
		SE_COPY_TO_USER,
		SE_USER_START,
		SE_USER_END,
		
		SE_COUNT
	};
		
	{
		uint32_t inttime = evData->dbg_timestamp[0] / MHZ;
		static uint32_t mint[SE_COUNT];
		static uint32_t maxt[SE_COUNT];
		static bool initialized = false;
		
		static int counter;
		
		if(!initialized) {
			for(int i = 0; i < SE_COUNT; i ++) {
				maxt[i] = 0;
				mint[i] = 2000*1000*1000;
			}
			initialized = true;
			counter = 0;
		}
		
		if(inttime > maxt[event]) 
			maxt[event] = inttime;
		
		if(inttime < mint[event]) 
			mint[event] = inttime;
		
		int se;
		uint32_t val;
		
		
		val = evData->dbg_timestamp[1] / MHZ - inttime;
		se = SE_PUT_BUFF;
		
		if(val > maxt[se]) maxt[se] = val;
		if(val < mint[se]) mint[se] = val;
		
		
		val = evData->dbg_timestamp[2] / MHZ - inttime;
		se = SE_COPY_TO_USER;
		
		if(val > maxt[se]) maxt[se] = val;
		if(val < mint[se]) mint[se] = val;
		
		
		val = userStart / MHZ - inttime;
		se = SE_USER_START;
		
		if(val > maxt[se]) maxt[se] = val;
		if(val < mint[se]) mint[se] = val;
		

		val = userEnd / MHZ - inttime;
		se = SE_USER_END;
		
		if(val > maxt[se]) maxt[se] = val;
		if(val < mint[se]) mint[se] = val;
		
		

		counter ++;
		if(counter > 4*4096) {
			initialized = false;
			
			for(int i = 0; i < SE_COUNT; i ++) {
				if(maxt[i] > 0) {
					// only for the existing ones
					
					AMARK("{%03d:%5d-%5d}", i, mint[i], maxt[i]);
				}
			}
			
			AMARK("\n");
		}
	}
}


#endif // DBG_MEASURE_TIME_FROM_IRQ_TO_USER

