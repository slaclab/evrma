#ifndef libevrma_internal_h
#define libevrma_internal_h

#include <string>

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


#endif // libevrma_internal_h

