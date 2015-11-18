#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include <string>

// TODO: temporarily, to set DBG_MEASURE_TIME_FROM_IRQ_TO_USER
#include "linux-evrma.h"

#ifndef DBG_MEASURE_TIME_FROM_IRQ_TO_USER
#warning **********************************************************************
#warning **********************************************************************
#warning **********************************************************************
#warning Please, remove #include "linux-evrma.h" now that it is not needed anymore.
#warning **********************************************************************
#warning **********************************************************************
#warning **********************************************************************
#endif

#include "libevrma.h"
#include "libevrma_dbg.h"

#define ADBG(FORMAT, ...) {printf("ADBG: " FORMAT "\n", ## __VA_ARGS__); fflush(stdout);}
#define AERR(FORMAT, ...) {printf("ERROR: " FORMAT "\n", ## __VA_ARGS__); fflush(stdout);}
#define AINFO(FORMAT, ...) {printf("INFO: " FORMAT "\n", ## __VA_ARGS__); fflush(stdout);}
#define AMARK(FORMAT, ...) {printf(FORMAT, ## __VA_ARGS__); fflush(stdout);}

/*
 * Prerequisites:
 * - at least one pulse generator allocated.
 * - the test must be done on a VEVR of a real EVR device. The simulation is
 *   not enough.
 */
int main(int argc, const char *argv[])
{
	int argc_used = 1; // the command itself
	bool ret = false;
	std::string virtDevName, cmd;
	EvrmaSession session;
	
	if(argc < argc_used + 1) {
		AERR("arg[%d]->virtDevName", argc_used);
		goto LEnd2;
	}
	
	virtDevName = argv[argc_used ++];
	
	AINFO("evrmaOpenSession ...");
	
	session = evrmaOpenSession(virtDevName.c_str(), NULL, 0);
	if(session == NULL) {
		AERR("evrmaOpenSession failed.");
		goto LEnd2;
	}
 
 
	AINFO("evrmaSubscribe ...");
	if(evrmaSubscribe(session, 40) < 0) {
		goto LEnd1;
	}
		
	AINFO("evrmaUnsubscribe ...");
	if(evrmaUnsubscribe(session, 40) < 0) {
		goto LEnd1;
	}
		
	AINFO("evrmaUnsubscribeAll ...");
	if(evrmaUnsubscribeAll(session) < 0) {
		goto LEnd1;
	}
		
	{
		int pc = evrmaGetPulseCount(session);
		if(pc < 0) {
			AINFO("evrmaGetPulseCount ...");
			goto LEnd1;
		} else {
			AINFO("evrmaGetPulseCount = %d ...", pc);
		}
	}
	
	{
		uint32_t prescaler = 10, delay = 0, width = 0;
		
		AINFO("evrmaSetPulseParams ...");
		/* Set prescaler to 0 so any pulsegen will accept it */
		if(evrmaSetPulseParams(session, 0, 0, 1, 1) < 0) {
			goto LEnd1;
		}
			
		AINFO("evrmaGetPulseParams ...");
		if(evrmaGetPulseParams(session, 0, &prescaler, &delay, &width) < 0) {
			goto LEnd1;
		}
		
		if(prescaler != 0 || delay != 1 || width != 1) {
			AINFO("evrmaGetPulseParams bad result %d %d %d != 0 1 1 ...", prescaler, delay, width);
			goto LEnd1;
		}
	}

	{
		uint8_t enable = 0, polarity = 0, pulseCfgBits = 0;
		
		AINFO("evrmaSetPulseProperties ...");
		if(evrmaSetPulseProperties(session, 0, 1, 1, 1) < 0) {
			goto LEnd1;
		}
			
		AINFO("evrmaGetPulseProperties ...");
		if(evrmaGetPulseProperties(session, 0, &enable, &polarity, &pulseCfgBits) < 0) {
			goto LEnd1;
		}
		
		if(enable != 1 || polarity != 1 || pulseCfgBits != 1) {
			AINFO("evrmaGetPulseProperties bad result ...");
			goto LEnd1;
		}
	}
	
	{
		uint8_t data1[256], data2[256];
		
		memset(data1, 0, sizeof(data1));
		memset(data2, 0, sizeof(data1));
		
		data1[60] = 3;
		
		AINFO("evrmaSetPulseRam ...");
		if(evrmaSetPulseRam(session, 0, data1) < 0) {
			goto LEnd1;
		}
			
		AINFO("evrmaGetPulseRam ...");
		if(evrmaGetPulseRam(session, 0, data2) < 0) {
			goto LEnd1;
		}
		
		if(memcmp(data1, data2, sizeof(data1)) != 0) {
			AINFO("evrmaGetPulseRam bad result ...");
			goto LEnd1;
		}
	}
	
	{
		uint8_t data2 = 10;
		
		AINFO("evrmaSetPulseRamForEvent ...");
		if(evrmaSetPulseRamForEvent(session, 0, 70, 5) < 0) {
			goto LEnd1;
		}
			
		AINFO("evrmaGetPulseRamForEvent ...");
		if(evrmaGetPulseRamForEvent(session, 0, 70, &data2) < 0) {
			goto LEnd1;
		}
		
		if(data2 != 5) {
			AINFO("evrmaGetPulseRamForEvent bad result %d != 5...", data2);
			goto LEnd1;
		}
	}
	
	
	{
		uint32_t fpgaVersion;
		int taxiStatus;
		uint32_t secSh;
		uint32_t tLa;
		char sysfsDev[100];
		
		
		AINFO("evrmaGetStatus ...");
		if(evrmaGetStatus(session, &fpgaVersion, &taxiStatus) < 0) {
			goto LEnd1;
		}
		
		AINFO("evrmaGetSecondsShift ...");
		if(evrmaGetSecondsShift(session, &secSh) < 0) {
			goto LEnd1;
		}
		
		AINFO("evrmaGetTimestampLatch ...");
		if(evrmaGetTimestampLatch(session, &tLa) < 0) {
			goto LEnd1;
		}
		
		AINFO("evrmaGetSysfsDevice ...");
		if(evrmaGetSysfsDevice(session, sysfsDev, 100) < 0) {
			goto LEnd1;
		}
		
		AINFO("STATUS: fpgaVersion=0x%08X, taxiStatus=%d, secSh=%d, tLa=%d sysfsDev=%s", 
			  fpgaVersion, taxiStatus, secSh, tLa, sysfsDev);
	}
	
	{
		uint32_t *data;
		int size;
		int status;
		
		status = evrmaGetDBuf(session, &data, &size);
		AINFO("evrmaGetDBuf, status = %d ...", status);
	}	

	ret = true;
	
#ifdef DBG_MEASURE_TIME_FROM_IRQ_TO_USER
	evrmaSubscribe(session, 40);
	for(int i = 0; i < 10; i ++) {
		uint32_t tLa;
		evrmaGetTimestampLatch(session, &tLa);
		ADBG("timestamp: %d", tLa);
	}
#endif
	
LEnd1:

	AINFO("evrmaCloseSession ...");
	evrmaCloseSession(session);
	
LEnd2:

	if(!ret) {
		
		AERR("***********************************");
		AERR("*** ERROR * The API test failed ***");
		AERR("***********************************");
		
	} else {
		AINFO("---------------------------");
		AINFO("OK.");
		AINFO("---------------------------");
		
	}
	
	return ret ? 0 : 1;
}

/*
int main_test(int argc, const char *argv[])
{
	int argc_used = 1; // the command itself
	bool ret = false;
	std::string virtDevName, cmd;
	EvrmaSession session;
	
	if(argc < argc_used + 1) {
		AERR("arg[%d]->virtDevName", argc_used);
		goto LEnd1;
	}
	
	virtDevName = argv[argc_used ++];
	
	session = evrmaOpenSession(virtDevName.c_str(), NULL, 0);
	if(session == NULL) {
		AERR("evrmaOpenSession failed.");
		goto LEnd1;
	}
 
	ADBG("Opened");
	
	{
	
		int fd = evrmaGetFd(session);
		
		for(int i = 0; i < 100*1000*1000; i ++) {
			ioctl(fd, 55); 
			AMARK(".");
		}
	
	}
	
	ADBG("Closing");
	evrmaCloseSession(session);
	
	ret = true;
	
LEnd1:
	return ret ? 0 : 1;
}

*/

