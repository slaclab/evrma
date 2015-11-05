#ifndef libevrma_internal_h
#define libevrma_internal_h

#include <string>
#include <stdio.h>

#include "libevrma.h"

class Session {
public:
	
	explicit Session(void)
		: fd(-1)
		, pauseThread(true)
		, threadReadTerminate(false)
		, threadRead(NULL)
		, evrmaCallback(NULL)
		, callbackArg(NULL)
		, mmapPtr(NULL)
		, mmapLength(0)
	{
	}
	
	virtual ~Session()
	{
	}
	
	int fd;
	bool pauseThread;

	bool threadReadTerminate;
	EvrmaThread threadRead;

	EvrmaCallback evrmaCallback;
	void *callbackArg;
	
	struct vevr_mmap_data *mmapPtr;
	int mmapLength;
	
	int major;
	int minor;
	std::string vevrName;

};



/*
 * Convenient macros to handle message printouts.
 */
#define ADBG(FORMAT, ...) printf("DBG: " FORMAT "\n", ## __VA_ARGS__)
#define AINFO(FORMAT, ...) printf("INFO: " FORMAT "\n", ## __VA_ARGS__)
#define AERR(FORMAT, ...) printf("ERROR: " FORMAT "\n", ## __VA_ARGS__)
#define AMARK(FORMAT, ...) {printf(FORMAT, ## __VA_ARGS__); fflush(stdout);}


#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

/* Returns <0 on error, ==0 on timeout, >0 on pending read. */
int waitForRead(int fd, int timeoutMs);

int toHex(unsigned char c);

/* Checks if the current thread is the one defined by 'thread' argument. */
bool evrmaThreadCurrentIs(EvrmaThread thread);




#endif // libevrma_internal_h

