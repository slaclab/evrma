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
#include "utils.h"

namespace { // unnamed
	
enum {
	CLEAN_SESSION,
	CLEAN_FD,
	CLEAN_MMAP,
	CLEAN_THREAD,
	CLEAN_ALL = CLEAN_THREAD
};

	
void cleanup(Session *pSession, int what)
{
	switch(what) {
	case CLEAN_THREAD:
		if(pSession->threadRead != NULL) {
			pSession->threadReadTerminate = 1;
			evrmaThreadJoin(pSession->threadRead);
		}
	case CLEAN_MMAP:
		munmap(NULL, pSession->mmapLength);
	case CLEAN_FD: 
		close(pSession->fd);
	case CLEAN_SESSION:
		delete pSession;
	}
}

void threadReadFunction(void *arg)
{
	Session *pSession = (Session *)arg;
	
	#define READBUFLEN (8 * 1024)
	uint8_t buff[READBUFLEN];
	
	while(!pSession->threadReadTerminate) {
		
		if(pSession->pauseThread) {
			usleep(50*1000);
			continue;
		}
		
		// once a second
		int npr = waitForRead(pSession->fd, 1000);
		
		if(npr <= 0) continue;

		int n = read(pSession->fd, buff, READBUFLEN);

		if(n <= 0) continue;

// // // // // 		int readInx = 0;
		
		uint8_t *buffP = buff;
		int stillToBeUsed = n;
		
		while(stillToBeUsed > 0) {
			
			uint16_t event;
			
			if(stillToBeUsed >= 2) {
				memcpy(&event, buffP, 2);
				buffP += 2;
				stillToBeUsed -= 2;
			} else {
				/*
				 * Not every byte was used. This can't happen anyway.
				 */
				break;
			}
			
			if(event >= EVRMA_FIFO_MIN_EVENT_CODE && event <= EVRMA_FIFO_MAX_EVENT_CODE) {
				
				// EVR fifo event codes
				
				int dataLength = sizeof(struct evr_data_fifo_event);
				uint8_t *dataPtr = buffP;
				
				if(stillToBeUsed >= dataLength) {
					buffP += dataLength;
					stillToBeUsed -= dataLength;
				} else {
					/*
					* Not every byte was used. This can't happen anyway.
					*/
					break;
				}
			
				pSession->evrmaCallback(
						(EvrmaSession)pSession, 
						pSession->callbackArg, 
						event, dataPtr, dataLength);
				
			} else {
				
				pSession->evrmaCallback(
						(EvrmaSession)pSession, 
						pSession->callbackArg, 
						event, NULL, 0);
			}
		}
	}
}

bool subscribe(EvrmaSession session, int event, uint8_t action)
{
	Session *pSession = (Session *)session;
	
	struct vdev_ioctl_subscribe subs = {
		event,
		action
	};
	
	int ret = ioctl(pSession->fd, VIRT_DEV_IOC_SUBSCRIBE, &subs);
	
	if(ret < 0) {
		AERR("VIRT_DEV_IOC_SUBSCRIBE failed");
		return false;
	}
	
	return true;
}

} // unnamed namespace




EvrmaSession evrmaOpenSession(const char *devNodeName, EvrmaCallback evrmaCallback, void *callbackArg)
{
	Session *pSession = new Session;
	
	pSession->evrmaCallback = evrmaCallback;
	pSession->callbackArg = callbackArg;
	
	if(pSession == NULL) return NULL;

	pSession->fd = open(devNodeName, O_RDWR);
	if(pSession->fd < 0) {
		AERR("Can't open the device: '%s'", devNodeName);
		cleanup(pSession, CLEAN_SESSION);
		return NULL;
	}
	
	
	struct vdev_ioctl_status vevrStatus;
	
	int ret = ioctl(pSession->fd, VIRT_DEV_IOC_STATUS_GET, &vevrStatus);
	
	if(ret < 0) {
		AERR("VIRT_DEV_IOC_STATUS_GET failed, errno=%d", errno);
		cleanup(pSession, CLEAN_FD);
		return NULL;
	}

	/* Get some properties of the VEVR for later use. */
	pSession->minor = vevrStatus.minor;
	pSession->major = vevrStatus.major;
	// just in case the string was not obtained terminated
	vevrStatus.name[MODAC_ID_MAX_NAME] = 0;
	pSession->vevrName = vevrStatus.name;
	
	
	int prot = PROT_READ;
	
	pSession->mmapLength = sizeof(struct vevr_mmap_data);
	pSession->mmapPtr = (struct vevr_mmap_data *)mmap(NULL, pSession->mmapLength, 
								prot, MAP_SHARED, pSession->fd, 0);
	
	if(pSession->mmapPtr == MAP_FAILED) {
		AERR("IO mmap failed with errno=%d", errno);
		
		cleanup(pSession, CLEAN_FD);
		return NULL;
	} else {
		ADBG("mmap = 0x%x", (int)(uint64_t)pSession->mmapPtr);
	}

	if(pSession->evrmaCallback != NULL) {
		// realtime priority may also be set later in the scripts
		pSession->threadRead = evrmaThreadStart(threadReadFunction, 
					(void *)pSession, 1, ("evrma_" + pSession->vevrName).c_str());
		if(pSession->threadRead == NULL) {
			cleanup(pSession, CLEAN_MMAP);
			return NULL;
		}
	}

	// now evertything is running, release the thread
	pSession->pauseThread = false;
	
	return (EvrmaSession)pSession;
}

void evrmaCloseSession(EvrmaSession session)
{
	Session *pSession = (Session *)session;
	
	cleanup(pSession, CLEAN_ALL);
}

int evrmaGetDBuf(EvrmaSession session, uint32_t **data, int *size)
{
	Session *pSession = (Session *)session;
	
	*data = pSession->mmapPtr->data_buff.data;
	*size = pSession->mmapPtr->data_buff.size32 * 4;
	bool checksumOk = (pSession->mmapPtr->data_buff.status & (1<<C_EVR_DATABUF_CHECKSUM)) == 0;
	bool complete = (pSession->mmapPtr->data_buff.status & (1<<C_EVR_DATABUF_RXREADY)) != 0;
	
	if(checksumOk) {
		if(complete)
			return 0;
		else
			return -2;
	} else {
		if(complete)
			return -1;
		else
			return -3;
	}
}


int evrmaSubscribe(EvrmaSession session, int event)
{
	return subscribe(session, event, VIRT_DEV_IOCTL_SUBSCRIBE_ACTION_SUBSCRIBE) ? 0 : -1;
}

int evrmaUnsubscribe(EvrmaSession session, int event)
{
	return subscribe(session, event, VIRT_DEV_IOCTL_SUBSCRIBE_ACTION_UNSUBSCRIBE) ? 0 : -1;
}

int evrmaUnsubscribeAll(EvrmaSession session)
{
	return subscribe(session, 0, VIRT_DEV_IOCTL_SUBSCRIBE_ACTION_CLEAR) ? 0 : -1;
}

int evrmaSetPulseParams(EvrmaSession session,
		int pulsegenIndex,
		uint32_t prescaler, uint32_t delay, uint32_t width)
{
	Session *pSession = (Session *)session;
	
	struct vevr_ioctl_pulse_param ppData = {
		{{
			EVR_RES_TYPE_PULSEGEN,
			pulsegenIndex,
		}},
		prescaler,
		delay,
		width,
	};
	
	int ret = ioctl(pSession->fd, VEVR_IOC_PULSE_PARAM_SET, &ppData);
	
	if(ret < 0) {
		AERR("evrmaSetPulseParams failed, errno=%d", errno);
		return -1;
	}
	
	return 0;
}

int evrmaGetPulseParams(EvrmaSession session,
		int pulsegenIndex,
		uint32_t *prescaler, uint32_t *delay, uint32_t *width)
{
	Session *pSession = (Session *)session;
	
	struct vevr_ioctl_pulse_param ppData = {
		{{
			EVR_RES_TYPE_PULSEGEN,
			pulsegenIndex,
		}},
	};
	
	int ret = ioctl(pSession->fd, VEVR_IOC_PULSE_PARAM_GET, &ppData);
	
	if(ret < 0) {
		AERR("evrmaSetPulseParams failed, errno=%d", errno);
		return -1;
	}
	
	*prescaler = ppData.prescaler;
	*delay = ppData.delay;
	*width = ppData.width;
	
	return 0;
}

int evrmaSetPulseProperties(EvrmaSession session, 
		int pulsegenIndex, 
		uint8_t enable, uint8_t polarity, uint8_t pulseCfgBits)
{
	Session *pSession = (Session *)session;
	
	struct vevr_ioctl_pulse_properties ppData = {
		{{
			EVR_RES_TYPE_PULSEGEN,
			pulsegenIndex,
		}},
		enable,
		polarity,
		pulseCfgBits,
	};
	
	int ret = ioctl(pSession->fd, VEVR_IOC_PULSE_PROP_SET, &ppData);
	
	if(ret < 0) {
		AERR("evrmaSetPulseProperties failed, errno=%d", errno);
		return -1;
	}
	
	return 0;
}

int evrmaGetPulseProperties(EvrmaSession session, 
		int pulsegenIndex, 
		uint8_t *enable, uint8_t *polarity, uint8_t *pulseCfgBits)
{
	Session *pSession = (Session *)session;
	
	struct vevr_ioctl_pulse_properties ppData = {
		{{
			EVR_RES_TYPE_PULSEGEN,
			pulsegenIndex,
		}},
	};
	
	int ret = ioctl(pSession->fd, VEVR_IOC_PULSE_PROP_GET, &ppData);
	
	if(ret < 0) {
		AERR("evrmaSetPulseProperties failed, errno=%d", errno);
		return -1;
	}
	
	*enable = ppData.enable;
	*polarity = ppData.polarity;
	*pulseCfgBits = ppData.pulse_cfg_bits;
		
	return 0;
}

int evrmaSetPulseRam(EvrmaSession session, 
		int pulsegenIndex, const uint8_t *data)
{
	Session *pSession = (Session *)session;
	
	struct vevr_ioctl_pulse_map_ram pData = {
		{{
			EVR_RES_TYPE_PULSEGEN,
			pulsegenIndex,
		}},
	};
	
	memcpy(pData.map, data, sizeof(pData.map));
	
	int ret = ioctl(pSession->fd, VEVR_IOC_PULSE_MAP_RAM_SET, &pData);
	
	if(ret < 0) {
		AERR("evrmaSetPulseRam failed, errno=%d", errno);
		return -1;
	}
	
	return 0;
}

int evrmaGetPulseRam(EvrmaSession session, 
		int pulsegenIndex, uint8_t *data)
{
	Session *pSession = (Session *)session;
	
	struct vevr_ioctl_pulse_map_ram pData = {
		{{
			EVR_RES_TYPE_PULSEGEN,
			pulsegenIndex,
		}},
	};
	
	int ret = ioctl(pSession->fd, VEVR_IOC_PULSE_MAP_RAM_GET, &pData);
	
	if(ret < 0) {
		AERR("evrmaSetPulseRam failed, errno=%d", errno);
		return -1;
	}
	
	memcpy(data, pData.map, sizeof(pData.map));
	
	return 0;
}

int evrmaSetPulseRamForEvent(EvrmaSession session, 
		int pulsegenIndex, uint8_t eventCode, uint8_t data)
{
	Session *pSession = (Session *)session;
	
// // // 	ADBG("evrmaSetPulseRamForEvent %d %d %d", pulsegenIndex, eventCode, data);
	
	struct vevr_ioctl_pulse_map_ram_for_event pData = {
		{{
			EVR_RES_TYPE_PULSEGEN,
			pulsegenIndex,
		}},
		eventCode,
		data,
	};
	
	int ret = ioctl(pSession->fd, VEVR_IOC_PULSE_MAP_RAM_SET_FOR_EVENT, &pData);
	
	if(ret < 0) {
		AERR("evrmaSetPulseRamForEvent failed, errno=%d", errno);
		return -1;
	}
	
	return 0;
}

int evrmaGetPulseRamForEvent(EvrmaSession session, 
		int pulsegenIndex, uint8_t eventCode, uint8_t *data)
{
	Session *pSession = (Session *)session;
	
	struct vevr_ioctl_pulse_map_ram_for_event pData = {
		{{
			EVR_RES_TYPE_PULSEGEN,
			pulsegenIndex,
		}},
		eventCode,
	};
	
	int ret = ioctl(pSession->fd, VEVR_IOC_PULSE_MAP_RAM_GET_FOR_EVENT, &pData);
	
	if(ret < 0) {
		AERR("evrmaSetPulseRamForEvent failed, errno=%d", errno);
		return -1;
	}
	
	*data = pData.map;
	
	return 0;
}

static int getStatusAll(Session *pSession, struct vevr_status *status)
{
	struct vevr_ioctl_status pData = {
		{{
			MODAC_RES_TYPE_NONE,
		}},
	};
	
	int ret = ioctl(pSession->fd, VEVR_IOC_STATUS_GET, &pData);
	
	if(ret < 0) {
		AERR("VEVR_IOC_STATUS_GET failed, errno=%d", errno);
		return -1;
	}
	
	*status = pData.status;
	
	return 0;
}

int evrmaGetStatus(EvrmaSession session, 
				   uint32_t *fpgaVersion, int *taxiStatus)
{
	Session *pSession = (Session *)session;
	
	struct vevr_status status;
	if(getStatusAll(pSession, &status) < 0) {
		AERR("evrmaGetStatus failed, errno=%d", errno);
		return -1;
	}

	*fpgaVersion = status.fpga_version;
	*taxiStatus = (status.irq_flags & (1 << C_EVR_IRQFLAG_VIOLATION)) != 0;
	
	return 0;
}

int evrmaGetPulseCount(EvrmaSession session)
{
	Session *pSession = (Session *)session;
	
	struct vdev_ioctl_res_status pData = {
		EVR_RES_TYPE_PULSEGEN,
	};
	
	int ret = ioctl(pSession->fd, VIRT_DEV_IOC_RES_STATUS_GET, &pData);
	
	if(ret < 0) {
		AERR("evrmaGetPulseCount failed, errno=%d", errno);
		return -1;
	}

	return pData.count;
}

int evrmaGetSecondsShift(EvrmaSession session, uint32_t *secSh)
{
	Session *pSession = (Session *)session;
	
	struct vevr_status status;
	if(getStatusAll(pSession, &status) < 0) {
		AERR("evrmaGetPulseCount failed, errno=%d", errno);
		return -1;
	}
	
	*secSh = status.seconds_shift;

	return 0;
}

int evrmaGetTimestampLatch(EvrmaSession session, uint32_t *tLa)
{
	Session *pSession = (Session *)session;
	
	int ret = ioctl(pSession->fd, VEVR_IOC_LATCHED_TIMESTAMP_GET, tLa);

	if(ret < 0) {
		AERR("evrmaGetTimestampLatch failed, errno=%d", errno);
		return -1;
	}

	return 0;
}

int evrmaGetSysfsDevice(EvrmaSession session, char *buf, int bufLen)
{
	Session *pSession = (Session *)session;
	
	int n = snprintf(buf, bufLen, "/sys/dev/char/%d:%d", 
						pSession->major, pSession->minor);
	
	if(n >= bufLen || n < 0) return -1; // too short

	return 0;
}
