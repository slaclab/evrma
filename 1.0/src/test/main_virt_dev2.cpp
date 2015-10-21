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

#define IGNORE_RETVAL(CMD) if(CMD);

#include "libevrma.h"
#include "libevrma_dbg.h"


namespace {
	
std::string rcvStr1, rcvStr2;
	
void add(std::string &s, uint8_t v)
{
	char b[10];
	snprintf(b, 10, "%02X", (int)v);
	s += b;
}

void add32(std::string &s, int v)
{
	add(s, (v >> 0) &0xFF);
	add(s, (v >> 8) &0xFF);
	add(s, (v >> 16) &0xFF);
	add(s, (v >> 24) &0xFF);
}

void evrmaCallback1(EvrmaSession session, void *arg, int event, uint8_t *data, int length)
{
	add32(rcvStr1, event);
	for(int i = 0; i < length; i ++) {
		add(rcvStr1, data[i]);
	}
}

void evrmaCallback2(EvrmaSession session, void *arg, int event, uint8_t *data, int length)
{
	add32(rcvStr2, event);
	for(int i = 0; i < length; i ++) {
		add(rcvStr2, data[i]);
	}
}

void unblockThread(EvrmaSession session)
{
	evrmaThreadRun(session, 1);
}

#define DBG_SYS "/sys/class/modac-mng/evr-sim-mng/dbg"

bool test2(EvrmaSession session1, EvrmaSession session2)
{
	ADBG("test2 1");
	
	unblockThread(session1);
	unblockThread(session2);
		
	evrmaUnsubscribeAll(session1);
	evrmaSubscribe(session1, 19);
	
	ADBG("test2 3");
	
	IGNORE_RETVAL(system("echo i134444444444444488 > " DBG_SYS)); // this will go through for 3
	std::string part1 = "130000004444444444444488";
	
	evrmaUnsubscribeAll(session1);
	evrmaUnsubscribeAll(session2);
	evrmaSubscribe(session1, 20);
	evrmaSubscribe(session2, 19);
	evrmaSubscribe(session2, 20);
	
	ADBG("test2 5");
	
	// this will go through for 3 and 6
	IGNORE_RETVAL(system("echo i144544444444444488 > " DBG_SYS));
	std::string part2 = "140000004544444444444488";

	// this will go through for 6
	IGNORE_RETVAL(system("echo i134644444444444488 > " DBG_SYS));
	std::string part2a = "130000004644444444444488";

	ADBG("test2 8");
	
	evrmaUnsubscribeAll(session1);
	evrmaUnsubscribeAll(session2);
	evrmaSubscribe(session1, 19);
	evrmaSubscribe(session1, 20);

	// this will go through for 3
	IGNORE_RETVAL(system("echo i144711223344554433 > " DBG_SYS));
	std::string part3 = "140000004711223344554433";

	ADBG("test2 11");
	
	// this will go through for 3
	IGNORE_RETVAL(system("echo i134844444444444488 > " DBG_SYS));
	std::string part4 = "130000004844444444444488";
	
	usleep(500*1000);
	
	std::string expectedStr;
	
	expectedStr = part1 + part2 + part3 + part4;
	
	if(rcvStr1 == expectedStr) {
		ADBG("The read %s is ok", rcvStr1.c_str());
	} else {
		ADBG("The read %s is bad, should be %s", rcvStr1.c_str(), expectedStr.c_str());
		return false;
	}
	
	expectedStr = part2 + part2a;
	
	if(rcvStr2 == expectedStr) {
		ADBG("The read %s is ok", rcvStr2.c_str());
	} else {
		ADBG("The read %s is bad, should be %s", rcvStr2.c_str(), expectedStr.c_str());
		return false;
	}
	
	return true;
}

bool test2c(EvrmaSession session1)
{
	unblockThread(session1);

	evrmaUnsubscribeAll(session1);
	evrmaSubscribe(session1, 0x101);
	IGNORE_RETVAL(system("echo n101 > " DBG_SYS));
	
	usleep(500*1000);
	
	if(rcvStr1 == "01010000") {
		ADBG("The read %s is ok", rcvStr1.c_str());
	} else {
		ADBG("The read %s is bad", rcvStr1.c_str());
		return false;
	}
	
	return true;
}

bool test2d(EvrmaSession session1)
{
	evrmaUnsubscribeAll(session1);
	evrmaSubscribe(session1, 0x100);
	evrmaSubscribe(session1, 0x101);
	evrmaSubscribe(session1, 0x102);
	evrmaSubscribe(session1, 0x103);

	IGNORE_RETVAL(system("echo n101 > " DBG_SYS));
	IGNORE_RETVAL(system("echo n101 > " DBG_SYS));
	IGNORE_RETVAL(system("echo n103 > " DBG_SYS));
	IGNORE_RETVAL(system("echo n100 > " DBG_SYS));
	
	// only unblock now so we can see the joined events
	unblockThread(session1);
	
	usleep(500*1000);
	
	// these come out ordered by event number, not by the time they were created
	std::string str1 = "00010000" "01010000" "03010000" ;
	
	if(rcvStr1 == str1) {
		ADBG("The read %s is ok", rcvStr1.c_str());
	} else {
		ADBG("The read '%s' is bad (!= '%s')", rcvStr1.c_str(), str1.c_str());
		return false;
	}
	
	return true;
}

} // unnamed

int main(int argc, const char *argv[])
{
	int argc_used = 1; // the command itself
	bool ret = false;
	std::string virtDevName1, virtDevName2, cmd;
	EvrmaSession session1 = NULL, session2 = NULL;
	
	if(argc < argc_used + 3) {
		AERR("arg[%d,%d]->virtDevName1,virtDevName,cmd", argc_used, argc_used + 1);
		goto LEnd1;
	}
	
	virtDevName1 = argv[argc_used ++];
	virtDevName2 = argv[argc_used ++];
	
	cmd = argv[argc_used ++];
	
	// only read on 'read'
	session1 = evrmaOpenSession(virtDevName1.c_str(), evrmaCallback1, 0);
	session2 = evrmaOpenSession(virtDevName2.c_str(), evrmaCallback2, 0);
	
	evrmaThreadRun(session1, 0);
	evrmaThreadRun(session2, 0);
	
	// time for the thread to really stop running
	sleep(2);
	
	if(session1 == NULL || session2 == NULL) {
		AERR("evrmaOpenSession failed.");
		goto LEnd1;

	
LEnd1:
		if(session1 != NULL) evrmaCloseSession(session1);
		if(session2 != NULL) evrmaCloseSession(session2);
		
		return ret ? 0 : 1;
	}

	if(cmd == "test2") {
		
		ret = test2(session1, session2);

	} else if(cmd == "test2c") {
				
		ret = test2c(session1);

	} else if(cmd == "test2d") {
				
		ret = test2d(session1);

	} else {
		// unknown command
	}

	goto LEnd1;
	
}


 
