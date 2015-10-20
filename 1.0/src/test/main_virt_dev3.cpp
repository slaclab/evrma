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

#include "linux-evrma.h"
// only to use utils, no session will be open
#include "libevrma.h"

namespace {

class Context {
public:
	
	explicit Context(void)
		: fd(-1)
		, threadRead(NULL)
	{
	}
	
	virtual ~Context()
	{
	}
	
	int fd;
	EvrmaThread threadRead;
};

void threadReadFunction(void *arg)
{
	Context *ctxt = (Context *)arg;
	
	#define READBUFLEN (8 * 1024)
	uint8_t buff[READBUFLEN];
	
	while(true) {
		int n = read(ctxt->fd, buff, READBUFLEN);
		ADBG("Read returned: %d", n);
	}
}

	
} // unnamed

int main(int argc, const char *argv[])
{
	int argc_used = 1; // the command itself
	bool ret = false;
	std::string virtDevName, cmd;
	Context ctxt;
	
	if(argc < argc_used + 2) {
		AERR("arg[%d,%d]->virtDevName,cmd", argc_used, argc_used + 1);
		goto LEnd1;
	}
	
	virtDevName = argv[argc_used ++];
	
	cmd = argv[argc_used ++];
	
	if(cmd == "close_while_read") {

		// Test a misbehaving close() while the device is in the read sleep.
		
		ctxt.fd = open(virtDevName.c_str(), O_RDWR);
		
		if(ctxt.fd < 0) {
			AERR("Can't open the device: '%s'", virtDevName.c_str());
			goto LBail;
		}
		
		// start two parallel threads
		
		/*ctxt.threadRead = */
		evrmaThreadStart(threadReadFunction, (void *)&ctxt, 1);
		evrmaThreadStart(threadReadFunction, (void *)&ctxt, 1);
		
		// sleep half a second
		usleep(1500*1000);
		
		if(close(ctxt.fd)) {
			ADBG("Close failed, errno=%d", errno);
		}
		
		// sleep half a second
		usleep(500*1000);
		
		ret = true;
		
LBail:
		return ret ? 0 : 1;
	}
	
LEnd1:
	return ret ? 0 : 1;

}


 
