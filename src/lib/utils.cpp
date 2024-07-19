//////////////////////////////////////////////////////////////////////////////
// This file is part of 'evrma'.
// It is subject to the license terms in the LICENSE.txt file found in the 
// top-level directory of this distribution and at: 
//    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
// No part of 'evrma', including this file, 
// may be copied, modified, propagated, or distributed except according to 
// the terms contained in the LICENSE.txt file.
//////////////////////////////////////////////////////////////////////////////
#include <pthread.h>
#include <stdlib.h>

#include "libevrma_internal.h"
#include "libevrma.h"

typedef struct
{
	pthread_t id;
	EvrmaThreadEntryFunction entryFunction;
	void *arg;
} ThreadInternal;

static void *encapsulatingFunction(void *arg)
{
	ThreadInternal *ti = (ThreadInternal *)arg;

	// starts the wanted entry function with the specified argument
	ti->entryFunction(ti->arg);

	return NULL;
}

EvrmaThread evrmaThreadStart(
	EvrmaThreadEntryFunction entryFunction,
	void *arg, int realtime, const char *name)
{
	ThreadInternal *ti= (ThreadInternal *)malloc(sizeof(ThreadInternal));

	if(ti==NULL) return NULL;

	// function and arg will not be directly used, because of
	// noncompatibility of function type
	ti->entryFunction = entryFunction;
	ti->arg = arg;

	pthread_attr_t attr;

	if(pthread_attr_init(&attr) != 0) {

LEndOnError:
		free(ti);
		return NULL;
	}

	if(realtime) {
		if(pthread_attr_setschedpolicy(&attr, SCHED_RR)!=0) {

			goto LEndOnError;
		}
	}

	if(pthread_create(
				&(ti->id),
				&attr,
				encapsulatingFunction,
				(void *)ti) != 0) {
		goto LEndOnError;
	} else {
		
		if(name != NULL) {
			// TODO: check for error if the name couldn't be set
			pthread_setname_np(ti->id, name);
		}
		
		return (EvrmaThread)ti;
	}
}

void evrmaThreadJoin(EvrmaThread thread)
{
	ThreadInternal *ti = (ThreadInternal *)thread;
	
	(void) pthread_join(ti->id, NULL);
	free(ti);
}

bool evrmaThreadCurrentIs(EvrmaThread thread)
{
	ThreadInternal *ti = (ThreadInternal *)thread;
	return ti->id == pthread_self();
}

EvrmaMutex evrmaMutexOpen(void)
{
	pthread_mutex_t *pmi = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

	if(pmi == NULL) {
		return NULL;
	}

    if(pthread_mutex_init(pmi, NULL) != 0) {
		free(pmi);
		return NULL;
	}
	
	return (EvrmaMutex)pmi;
}

void evrmaMutexClose(EvrmaMutex mutex)
{
	pthread_mutex_t *pmi = (pthread_mutex_t *)mutex;
	pthread_mutex_destroy(pmi);
	free(pmi);
}

void evrmaMutexLock(EvrmaMutex mutex)
{
	pthread_mutex_t *pmi = (pthread_mutex_t *)mutex;
	pthread_mutex_lock(pmi);
}

void evrmaMutexUnlock(EvrmaMutex mutex)
{
	pthread_mutex_t *pmi = (pthread_mutex_t *)mutex;
	pthread_mutex_unlock(pmi);
}

int waitForRead(int fd, int timeoutMs)
{
	struct timeval restime;
	fd_set fds_r, fds_e;
	int res;

	restime.tv_sec  = timeoutMs/1000;   // seconds
	restime.tv_usec = (timeoutMs%1000)*1000;   // mikro sec
	FD_ZERO(&fds_r);
	FD_SET(fd, &fds_r);
	FD_ZERO(&fds_e);
	FD_SET(fd, &fds_e);

	/*
	 * check for reading and errors
	 */
	res = select(FD_SETSIZE, &fds_r, NULL, &fds_e, timeoutMs<0 ? NULL : &restime);

	/*
	 * res holds the number of fds, e.g. 1 or 0 if none was signaled;
	 * -1 on error
	 */
	return res;
}

int toHex(unsigned char c)
{
	if(c < 10) return '0' + c;
	if(c < 16) return 'A' + c - 10;
	return 'x';
}
