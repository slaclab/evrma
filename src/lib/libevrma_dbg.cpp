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

// --------- debug only ----------

#ifdef DBG_MEASURE_TIME_FROM_IRQ_TO_USER

static uint64_t get_precise_time(void)
{	
	struct timespec tm;

	if (clock_gettime(CLOCK_MONOTONIC, &tm) == -1) {
		// ups
		return 0;
	} else {
		return ((uint64_t)tm.tv_sec) * 1000000 + (uint32_t)(tm.tv_nsec / 1000);
	}

}

#endif


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
		SE_START = 600,
		SE_PUT_BUFF = SE_START,
		SE_COPY_TO_USER,
		SE_USER_START,
		SE_USER_END,
		SE_DBUF_READ_TIME,
		SE_PERIOD_1, // the period of the event1
		
		SE_COUNT
	};
	
	const char *seName[] = {
		"PUTBF",
		"CP-US",
		"US_ST",
		"US_EN",
		"DB-RD",
		"PRD-1",
	};
		
	{
		uint32_t inttime = evData->dbg_timestamp[0] / MHZ;
		static uint32_t mint[SE_COUNT];
		static uint32_t maxt[SE_COUNT];
		static bool initialized = false;
		static uint64_t prevTimeEvent1 = 0;
		
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
		
		
		if(event == EVRMA_EVENT_DBUF_DATA) {
			
			val = evData->timestamp / MHZ;
			se = SE_DBUF_READ_TIME;
			
			if(val > maxt[se]) maxt[se] = val;
			if(val < mint[se]) mint[se] = val;
		
		}

		if(event == 1) {
			
			uint64_t timeEvent1 = get_precise_time();
			
			val = timeEvent1 - prevTimeEvent1;
			prevTimeEvent1 = timeEvent1;
			se = SE_PERIOD_1;
			
			if(val > maxt[se]) maxt[se] = val;
			if(val < mint[se]) mint[se] = val;
		
		}

		counter ++;
		if(counter > 4*4096) {
			initialized = false;
			
			for(int i = 0; i < SE_COUNT; i ++) {
				if(maxt[i] > 0) {
					// only for the existing ones
					
					if(i >= SE_START) {
						AMARK("{%s:%5d-%5d}", seName[i - SE_START], mint[i], maxt[i]);
					} else {
						AMARK("{%03d:%5d-%5d}", i, mint[i], maxt[i]);
					}
				}
			}
			
			AMARK("\n");
		}
	}
}


#endif // DBG_MEASURE_TIME_FROM_IRQ_TO_USER

