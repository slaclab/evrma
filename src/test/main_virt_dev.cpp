#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>

#define ADBG(FORMAT, ...) printf("ADBG: " FORMAT "\n", ## __VA_ARGS__)
#define AERR(FORMAT, ...) printf("ERROR: " FORMAT "\n", ## __VA_ARGS__)
#define AMARK(FORMAT, ...) printf(FORMAT, ## __VA_ARGS__)

#define IGNORE_RETVAL(CMD) if(CMD);

#include "linux-evrma.h"
#include "libevrma.h"
#include "libevrma_dbg.h"

namespace {
	
double nowMonotonic(void)
{
	struct timespec tm;

	if (clock_gettime(CLOCK_MONOTONIC, &tm) == -1) {
		// ups
		return -1;
	} else {
		return (double)tm.tv_sec + ((double)tm.tv_nsec) / 1000000000.0;
	}
}


bool doFiducial = false;
int taxi, lost, heart;

class TimeSlot {
	
public:
	
	explicit TimeSlot(void)
		: initalized(false)
	{
	}
	
	
	void reStart(void)
	{
		if(initalized) {
			// check the slot if it's ok
			
			if(!has9 || v10_20_30 == 0 || v40_50_60 == 0 || vX1 == 0) {
				AERR("\nBad slot\n");
			} else if(v10_20_30 == 10 && !has140) {
				AERR("\nBad: no 140 in slot\n");
			}
			
		}
		
		has140 = false;
		has9 = false;
		v10_20_30 = 0;
		v40_50_60 = 0;
		vX1 = 0;
		
		initalized = true;
	}
	
	bool initalized;
	bool has140;
	bool has9;
	int v10_20_30;
	int v40_50_60;
	int vX1;
};


class Fiducial {
public:
	
	explicit Fiducial(void)
		: lastTime1(0)
		, counter1(0)
		, dbCount(0)
		, missing(0)
		, doubled(0)
		, strange(0)
		, strange140(0), strange9(0), strange10_20_30(0), strange40(0), strange41(0)
		, lastRedrawCounter(-1000)

		, lastEvent(-1)
	{
		for(int i = 0; i < 3; i ++) {
			minErr[i] = maxErr[i] = 0;
		}
	}
	
	
	void addEvent(int event)
	{

		double now = nowMonotonic();

		if(event == EVRMA_EVENT_DBUF_DATA) {
			
			dbCount ++;
			
		} else if(event == 1) {
			counter1 ++;
			
			if(lastTime1 != 0) {
				double tDiff = now - lastTime1;
				double err = tDiff - (1 / 360.0);
				
				if(err > 0.001) {
					missing ++;
				} else if(err < -0.001) {
					doubled ++;
				} else {
				
					for(int i = 0; i < 3; i ++) {
						if(err > maxErr[i]) maxErr[i] = err;
						if(err < minErr[i]) minErr[i] = err;
					}
				}
				
// 				AMARK("%20d ", (int)(err * 1000000));
			}
			
			if(counter1 % (360 * 10) == 0) {
				maxErr[0] = 0;
				minErr[0] = 0;
			}
			
			if(counter1 % (360 * 300) == 0) {
				maxErr[1] = 0;
				minErr[1] = 0;
			}
			
			lastTime1 = now;
			
// 			timeSlot.reStart();
			
			
		} else if(lastEvent < 0) {
			// skip, waiting for 1
		} else if(event == 140) {
			if(lastEvent != 1) {
				ADBG("\n140:lastEvent = %d", lastEvent);
			}
		} else if(event == 9) {
			if(lastEvent != 1 && lastEvent != 140) {
				ADBG("\n9:lastEvent = %d", lastEvent);
			}
		} else if(event == 10 || event == 20 || event == 30) {
			if(lastEvent != 9) {
				ADBG("\n10_20_30:lastEvent = %d", lastEvent);
			}
		} else if(event == 40) {
			if(lastEvent != 10) {
				ADBG("\n40:lastEvent = %d", lastEvent);
			}
		} else if(event == 41) {
			if(lastEvent != 40) {
				ADBG("\n41:lastEvent = %d", lastEvent);
			}
		} else if(event == 51) {
			if(lastEvent != 50) {
				ADBG("\n51:lastEvent = %d", lastEvent);
			}
		} else if(event == 61) {
			if(lastEvent != 60) {
				ADBG("\n61:lastEvent = %d", lastEvent);
			}
		} else if(event == 11) {
			if(lastEvent != 40) {
				ADBG("\n11:lastEvent = %d", lastEvent);
			}
		} else if(event == 21) {
			if(lastEvent != 50) {
				ADBG("\n21:lastEvent = %d", lastEvent);
			}
		} else if(event == 31) {
			if(lastEvent != 60) {
				ADBG("\n31:lastEvent = %d", lastEvent);
			}
		}
		
		lastEvent = event;
	}
	
	// event 1
	double lastTime1;
	int counter1;
	
	int dbCount;
	
	// [every 10 sec, every 5 min, eternal]
	double minErr[3], maxErr[3];
	
	int missing;
	int doubled;
	int strange;
	
	int strange140, strange9, strange10_20_30, strange40, strange41;
	
	int lastRedrawCounter;
	int lastEvent;
	
	TimeSlot timeSlot;
	
	void redraw(bool full)
	{
		if(counter1 < lastRedrawCounter + 320) {
			AERR("\nCounter failure");
		}
		
		lastRedrawCounter = counter1;
		
		if(full) {
			AMARK(
					"[** %d dBuf=%d diff=%d**]\t"
					"TAXI=%d,\tLOST=%d\tHEART=%d\tmissing=%d\tdoubled=%d\tstrange=%d(%d,%d,%d,%d,%d)\t"
						"time err: "
						"%10.5f - %10.5f usec   "
						"%10.5f - %10.5f usec   "
						"%10.5f - %10.5f usec   "
						"\n", 
						counter1, dbCount, counter1 - dbCount, taxi, lost, heart, missing, doubled, strange,
						strange140, strange9, strange10_20_30, strange40, strange41,
						minErr[0] * 1000000, maxErr[0] * 1000000,
						minErr[1] * 1000000, maxErr[1] * 1000000,
						minErr[2] * 1000000, maxErr[2] * 1000000
				);
		} else {
			AMARK(
					"[%d]\t"
					"LOST=%d\tHEART=%d\tmissing=%d\tdoubled=%d\tstrange=%d(%d,%d,%d,%d,%d)"
						"\n", 
						counter1, lost, heart, missing, doubled, strange,
						strange140, strange9, strange10_20_30, strange40, strange41
				);
		}
	}

};

Fiducial fiducial;

void evrmaCallback(EvrmaSession session, void *arg, int event, uint8_t *data, int length)
{
	if(doFiducial) {

		if(event == EVRMA_EVENT_ERROR_TAXI) {
			taxi ++;
		} else if(event == EVRMA_EVENT_ERROR_LOST) {
			lost ++;
		} else if(event == EVRMA_EVENT_ERROR_HEART) {
			heart ++;
		} else if(event >= 0 && event < 256) {
			fiducial.addEvent(event);
		} else if(event == EVRMA_EVENT_DBUF_DATA) {
			
			uint32_t *dataDBuf;
			int dbufLength;
			int checksumOk = evrmaGetDBuf(session, &dataDBuf, &dbufLength) == 0;
			
			std::string sd;
			
			for(int i = 0; i < dbufLength && i < 52 / 4; i ++) {
				char bx[10];
				snprintf(bx, 10, "%02X", (uint8_t)dataDBuf[i]);
				sd += bx;
			}
				
			if(!checksumOk) {
				
				ADBG("DBUF: %5.6lf\t%s\t not checksumOk, dbufLength=%d, ", nowMonotonic(), sd.c_str(), dbufLength);
// 				printf("ba"); fflush(stdout);
			} else if(dbufLength != 52) {
				ADBG("DBUF: not 52 bytes");
			} else {
// 				ADBG("DBUF: %5.6lf\t%s\t OK", nowMonotonic(), sd.c_str());
				fiducial.addEvent(EVRMA_EVENT_DBUF_DATA);
			}
			
		} else {
			ADBG("\nStrange event: %d\n", event);
		}
		
	} else 

	{
		
		for(int i = 0; i < length; i ++) {
			AMARK("%02X", (int)data[i]);
		}
	}
}

bool runFiducial(EvrmaSession session, bool full = false)
{
	ADBG("Testing fiducial");
	
	doFiducial = true;
	
	evrmaUnsubscribeAll(session);
	
	evrmaSubscribe(session, EVRMA_EVENT_DBUF_DATA);
	evrmaSubscribe(session, EVRMA_EVENT_ERROR_LOST);
	evrmaSubscribe(session, EVRMA_EVENT_ERROR_HEART);
	if(full) {
		evrmaSubscribe(session, EVRMA_EVENT_ERROR_TAXI);
		ADBG("Subscribed DBUF, LOST, HEART, TAXI");
	} else {
		ADBG("Subscribed DBUF, LOST, HEART");
	}
	
	evrmaSubscribe(session, 1);
	evrmaSubscribe(session, 9);
	evrmaSubscribe(session, 10);
	evrmaSubscribe(session, 11);
	evrmaSubscribe(session, 20);
	evrmaSubscribe(session, 21);
	evrmaSubscribe(session, 30);
	evrmaSubscribe(session, 31);
	evrmaSubscribe(session, 40);
	evrmaSubscribe(session, 41);
	evrmaSubscribe(session, 50);
	evrmaSubscribe(session, 51);
	evrmaSubscribe(session, 60);
	evrmaSubscribe(session, 61);
	evrmaSubscribe(session, 140);
	

	while(true) {
		sleep(1);
		fiducial.redraw(full);
	}
	
	return true;
}

} // unnamed

int main(int argc, const char *argv[])
{
	int argc_used = 1; // the command itself
	bool ret = false;
	std::string virtDevName, cmd;
	EvrmaSession session;
	
	if(argc < argc_used + 2) {
		AERR("arg[%d,%d]->virtDevName,cmd", argc_used, argc_used + 1);
		goto LEnd1;
	}
	
	virtDevName = argv[argc_used ++];
	
	cmd = argv[argc_used ++];
	
	// only read on 'read'
	session = evrmaOpenSession(virtDevName.c_str(), 
					(cmd == "read" || cmd == "fiducial" || cmd == "fiducial_full") 
						? evrmaCallback : NULL, 0);
	if(session == NULL) {
		AERR("evrmaOpenSession failed.");
		goto LEnd1;
	
LEnd2:
		evrmaCloseSession(session);
	
LEnd1:
		return ret ? 0 : 1;
	
	}
	
	if(cmd == "read") {
		sleep(1); // wait till the data arrives
		ret = true;
	} else if(cmd == "subs") {
	
		if(argc < argc_used + 1) {
			// all commands need this at the moment
			AERR("arg[%d]->event", argc_used);
			goto LEnd2;
		}

		std::string eventStr = argv[argc_used ++];
		
		int event;
		sscanf(eventStr.c_str(), "%d", &event);

		ret = evrmaSubscribe(session, event) >= 0;
		if(!ret) {
			AERR("evrmaSubscribe failed.");
			goto LEnd2;
		}
	} else if(cmd == "unsubs") {
	
		if(argc < argc_used + 1) {
			// all commands need this at the moment
			AERR("arg[%d]->event", argc_used);
			goto LEnd2;
		}

		std::string eventStr = argv[argc_used ++];
		
		// < 0 for  Unsubscribe All
		int event;
		sscanf(eventStr.c_str(), "%d", &event);

		if(event < 0) {
			ret = evrmaUnsubscribeAll(session) >= 0;
			if(!ret) {
				AERR("evrmaUnsubscribeAll failed.");
				goto LEnd2;
			}
		} else {
			ret = evrmaUnsubscribe(session, event) >= 0;
			if(!ret) {
				AERR("evrmaUnsubscribe failed.");
				goto LEnd2;
			}
		}
	} else if(cmd == "pulsegen") {
		
		/*
		pulsegen  pulsegen_index    pulsegen_command pulsegen_args
			Executes pulse generator command with arguments. Possibilities:
		… properties   prescaler   delay   width
			Sets the pulse properties.
		*/

		if(argc < argc_used + 2) {
			// all commands need this at the moment
			AERR("arg[%d,%d]->pulsegenIndex, pulsegenCommand ", argc_used, argc_used + 1);
			goto LEnd2;
		}
		
		int pulsegenIndex = ::atoi(argv[argc_used ++]);
		std::string pulsegenCommand = argv[argc_used ++];
		
		if(pulsegenCommand == "properties") {
			
			if(argc < argc_used + 3) {
				// all commands need this at the moment
				AERR("arg[%d,%d,%d]->prescaler, delay, width",
						argc_used, argc_used + 1, argc_used + 2
				);
				goto LEnd2;
			}
			
			uint32_t prescaler = ::atoi(argv[argc_used ++]);
			uint32_t delay = ::atoi(argv[argc_used ++]);
			uint32_t width = ::atoi(argv[argc_used ++]);

			ret = evrmaSetPulseParams(session, pulsegenIndex, prescaler, delay, width) >= 0;
			
			if(!ret) {
				AERR("PULSE_PARAM failed");
				goto LEnd2;
			} else {
				ADBG("PULSE_PARAM done");
			}
			
		} else if(pulsegenCommand == "mapram") {
			
			if(argc < argc_used + 2) {
				// all commands need this at the moment
				AERR("arg[%d,%d]->eventCode, map",
						argc_used, argc_used + 1
				);
				goto LEnd2;
			}
			
			uint8_t eventCode = ::atoi(argv[argc_used ++]);
			uint8_t map = ::atoi(argv[argc_used ++]);

			ret = evrmaSetPulseRamForEvent(session, pulsegenIndex, eventCode, map) >= 0;
			
			if(!ret) {
				AERR("PulseRamForEvent failed");
				goto LEnd2;
			} else {
				ADBG("PulseRamForEvent done");
			}
			
		} else if(pulsegenCommand == "get") {
			
			// gets the info about one pulse gen
			
			uint32_t prescaler, delay, width;

			ret = evrmaGetPulseParams(session, pulsegenIndex, &prescaler, &delay, &width) >= 0;
			
			if(!ret) {
				AERR("PULSE_PARAM get failed");
				goto LEnd2;
			} else {
				ADBG("PULSE_PARAM get donec");
			}
			
			char b[1000];
			snprintf(b, 1000, "echo '%d %d %d' > "
										"/home/laci/tmp"
												"/_result_.tmp", prescaler, delay, width);
			ADBG("System: %s", b);
			IGNORE_RETVAL(system(b));
			
		} else {
			AERR("unknown pulsegen command: %s", pulsegenCommand.c_str());
			goto LEnd2;
		}

	} else if(cmd == "test_subscribe_to_special_events_and_wait") {
		
		evrmaUnsubscribeAll(session);
		evrmaSubscribe(session, EVRMA_EVENT_ERROR_TAXI);
		evrmaSubscribe(session, EVRMA_EVENT_ERROR_LOST);
		evrmaSubscribe(session, EVRMA_EVENT_ERROR_HEART);
// 		evrmaSubscribe(session, EVRMA_EVENT_DELAYED_IRQ);
		evrmaSubscribe(session, EVRMA_EVENT_DBUF_DATA);
		
		evrmaSubscribe(session, 1);
		evrmaSubscribe(session, 0x28);
		evrmaSubscribe(session, 0x70);
		evrmaSubscribe(session, 0x71);
		evrmaSubscribe(session, 0x7a);
		evrmaSubscribe(session, 0x7b);
		evrmaSubscribe(session, 0x7c);
		evrmaSubscribe(session, 0x7d);
		evrmaSubscribe(session, 0x8c);

		// leave device open for 10 seconds to be able to monitor the state
		while(true) sleep(10);
		
	} else if(cmd == "testl") { // test in lib
		
		if(argc < argc_used + 1) {
			// all commands need this at the moment
			AERR("arg[%d]->testPar", argc_used);
			goto LEnd2;
		}
		
		int testPar = ::atoi(argv[argc_used ++]);
		
		ret = evrmaTest(session, testPar);
		
	} else if(cmd == "fiducial") {
				
		ret = runFiducial(session);
		
	} else if(cmd == "fiducial_full") {
				
		ret = runFiducial(session, true);
		
	} else {
		// unknown command
	}

	goto LEnd2;
	
}


 
