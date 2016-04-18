#ifndef libevrma_h
#define libevrma_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @file */

/**
 * @defgroup g_libevrma_api EVRMA User Space Library API.
 * 
 * @short EVRMA User Space Library API.
 *
 * All objects are defined using:
 * @code
 *   typedef struct XXX {} *Type
 * @endcode
 * This is a general pointer which is nevertheless type-checkable.
 * 
 * NULL means an invalid object.
 * 
 * Generally the functions return 0 on success, negative value on error.
 * 
 * The library is not thread-safe.
 * 
 * @warning
 * 
 * For maximal RT performance care must be taken when calling the library
 * functions. The library calls the kernel's IOCTLs that are protected by
 * mutexes. High priority RT threads may be blocked by low priority ones if
 * such functions are called. The only function that is designed to be called
 * from the high priority task is @ref evrmaGetTimestampLatch.
 * 
 * @{
 */


// --------- General evrma stuff ----------

/**
 * The session context object type.
 */
typedef struct EvrmaSessionS {} *EvrmaSession;

/**
 * The type of the function that is used for the event callback.
 * 
 * The parameters that are passed:
 * 
 * @par session 
 * The session context object.
 * 
 * @par callbackArg
 * Additional argument given on evrmaOpenSession.
 * 
 * @par event
 * The event code.
 * 
 * @par data and dataLength
 * Used in case the event has attached data to it. The ‘data’ can be NULL in 
 * which case the ‘dataLength’ is 0.
 */
typedef void (* EvrmaCallback)(
		EvrmaSession session, 
		void *callbackArg, 
		int event, 
		uint8_t *data,
		int length
							  );

/**
 * This function initalizes and starts the library session.
 * 
 * Events that can happen include the events that happened in the kernel driver
 * (see the kernel HW support for the given HW) and are transparently 
 * transferred to the user space.
 * 
 * @note
 * - This function is called from the context of a special thread so the 
 *   thread safety measures should be considered.
 * - The function must be processed as quickly as possible not to block the 
 *   thread processing. Failing to do so could lead to the event information loss.
 *   It is advisable to raise the RT priority of this task after it's been
 *   started. Only call the EVRMA API functions that are designed to be called from
 *   high prioirity tasks.
 * - The parameters passed from the system are valid only during the callback 
 *   call. Any information that they carry should be copied if later 
 *   use is required.
 * 
 * @return
 * The function returns the object reference that contains the session context.
 * 
 * In case of an error the returned value is NULL. Possibilities for an error are:
 * - The underlying HW type is not supported by the library.
 * - Memory allocation failure.
 */
EvrmaSession evrmaOpenSession(
		/**
		 * The name of the Linux device node.
		 */
		const char *devNodeName,
		
		/**
		 * The function that is called whenever a virtual event happens. 
		 * One event per call is obtained. See the EvrmaEventCallback type 
		 * description. If NULL the events are not read from the device in a loop. 
		 */
		EvrmaCallback evrmaCallback,
		
		/** 
		 * The argument passed on callback.
		 */
		void *callbackArg
			);

/**
 * Closes the library session.
 */
void evrmaCloseSession(
		/**
		 * The session context object obtained with the call to evrmaOpenSession().
		 */
		EvrmaSession session
			 );


/** 
 * Subscribes an event.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaSubscribe(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The event to subscribe.
		 */
		int event);

/** 
 * Unsubscribes an event.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaUnsubscribe(
		/**
		 * The session context object.
		 */
		EvrmaSession session, 
		/**
		 * The event to unsubscribe.
		 */
		int event
					);

/** 
 * Unsubscribes all events.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaUnsubscribeAll(
		/**
		 * The session context object.
		 */
		EvrmaSession session
					   );

/** 
 * Returns the number of allocated pulse generators for the VEVR.
 */
int evrmaGetPulseCount(
		/**
		 * The session context object.
		 */
		EvrmaSession session
					   );

/**
 * Sets the pulse parameters for the given pulse generator resourse.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaSetPulseParams(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The index of the pulse resource to be set, must be allocated to the VIRT_DEV.
		 */
		int pulsegenIndex,
		/**
		 * The 'prescaler' property of the pulse generator.
		 */
		uint32_t prescaler, 
		/**
		 * The 'delay' property of the pulse generator.
		 */
		uint32_t delay, 
		/**
		 * The 'width' property of the pulse generator.
		 */
		uint32_t width);

/**
 * Gets the pulse parameters for the given pulse generator resourse.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaGetPulseParams(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The index of the pulse resource to be set, must be allocated to the VIRT_DEV.
		 */
		int pulsegenIndex,
		/**
		 * The 'prescaler' property of the pulse generator.
		 */
		uint32_t *prescaler, 
		/**
		 * The 'delay' property of the pulse generator.
		 */
		uint32_t *delay, 
		/**
		 * The 'width' property of the pulse generator.
		 */
		uint32_t *width);


/**
 * Sets the pulse properties for the given pulse generator resourse.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaSetPulseProperties(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The index of the pulse resource to be set, must be allocated to the VIRT_DEV.
		 */
		int pulsegenIndex,
		/**
		 * Enable / disable the pulse generator.
		 */
		uint8_t enable,
		/**
		 * The polarity of the pulse generator.
		 */
		uint8_t polarity, 
		/** 
		 * The pulse generator configuration bits (see EVR_PULSE_CFG_BIT_...) 
		 */
		uint8_t pulseCfgBits);

/**
 * Gets the pulse properties for the given pulse generator resourse.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaGetPulseProperties(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The index of the pulse resource to be set, must be allocated to the VIRT_DEV.
		 */
		int pulsegenIndex, 
		/**
		 * Enable / disable status of the pulse generator.
		 */
		uint8_t *enable, 
		/**
		 * The polarity of the pulse generator.
		 */
		uint8_t *polarity, 
		/** 
		 * The pulse generator configuration bits (see EVR_PULSE_CFG_BIT_...).
		 */
		uint8_t *pulseCfgBits);

/**
 * Sets the pulse generator Event Mapping RAM.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaSetPulseRam(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The index of the pulse resource to be set, must be allocated to the VIRT_DEV.
		 */
		int pulsegenIndex,
		/**
		 * The data would hold 256 entries each one having defined 3 bits, 
		 * ‘trigger’, ‘set’ and ‘clear’. For the pulse generator this would 
		 * program the bits that correspond to it  in all of the event code 
		 * entries in the Event Mapping RAM.
		 * (see EVR_PULSE_CFG_BIT_...).
		 */
		const uint8_t *data);

/**
 * Gets the pulse generator Event Mapping RAM.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaGetPulseRam(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The index of the pulse resource to be set, must be allocated to the VIRT_DEV.
		 */
		int pulsegenIndex,
		/**
		 * See evrmaSetPulseRam's 'data'.
		 */
		uint8_t *data);

/**
 * Sets the pulse generator Event Mapping RAM for one event code only.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaSetPulseRamForEvent(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The index of the pulse resource to be set, must be allocated to the VIRT_DEV.
		 */
		int pulsegenIndex,
		/**
		 * The event which the pulse generator is set for.
		 */
		uint8_t eventCode,
		/** 
		 * The data holds the entry for the given event code one having defined 
		 * 3 bits, ‘trigger’, ‘set’ and ‘clear’. 
		 * See also evrmaSetPulseRam / evrmaGetPulseRam.
		 */
		uint8_t data);

/**
 * Gets the pulse generator Event Mapping RAM for one event code only.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaGetPulseRamForEvent(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The index of the pulse resource to be set, must be allocated to the VIRT_DEV.
		 */
		int pulsegenIndex, 
		/**
		 * The event which the pulse generator is obtained for.
		 */
		uint8_t eventCode,
		/** 
		 * The data holds the entry for the given event code one having defined 
		 * 3 bits, ‘trigger’, ‘set’ and ‘clear’. 
		 * See also evrmaSetPulseRam / evrmaGetPulseRam.
		 */
		uint8_t *data);

/**
 * Obtains different statuses from the VEVR.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaGetStatus(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * Returned FGPA version.
		 */
		uint32_t *fpgaVersion,
		/**
		 * Returned status of the 'TAXI' error.
		 */
		int *taxiStatus);

/**
 * Obtains the value of the Seconds Shift Register.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaGetSecondsShift(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The returned value.
		 */
		uint32_t *secSh);

/**
 * Obtains the value of the Seconds Shift Register.
 * 
 * @return
 * A negative value on error, 0 on success.
 * 
 * @note
 * This function can be called in high priority RT tasks.
 * 
 */
int evrmaGetTimestampLatch(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		uint32_t *tLa);


int evrmaGetTemperature(
		EvrmaSession session,
		uint32_t *tT);
int evrmaGetMaxTemperature(
		EvrmaSession session,
		uint32_t *tT);

/** 
 * Gets the DataBuf current data.
 * 
 * @return
 * - 0 on success
 * - (-1) on checksum error
 * - (-2) on error: not Data Buffer Transmition Complete.
 * - (-3) on both errors
 */
int evrmaGetDBuf(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The pointer to the current data.
		 */
		uint32_t **data, 
		/**
		 * The current length of the data in bytes.
		 */
		int *size);

/** 
 * Gets the name of the sysfs device associated with the VEVR.
 * 
 * @return
 * A negative value on error, 0 on success.
 */
int evrmaGetSysfsDevice(
		/**
		 * The session context object.
		 */
		EvrmaSession session,
		/**
		 * The returned name.
		 */
		char *buf,
		/**
		 * The available length of the 'buf'.
		 */
		int bufLen);

// --------- Some of the wrapped OS functionality is exported for general use ----------

/**
 * The thread object type.
 */
typedef struct EvrmaThreadS {} *EvrmaThread;

/**
 * The mutex object type.
 */
typedef struct EvrmaMutexS {} *EvrmaMutex;

/**
 * The type of the function that is used for the thread callback.
 * 
 * The parameters that are passed:
 * 
 * @par arg 
 * The argument given on thread start.
 */
typedef void (*EvrmaThreadEntryFunction)(void *arg);

/**
 * Starts a thread.
 */
EvrmaThread evrmaThreadStart(
		/**
		* The start function that is run in the thread context.
		*/
		EvrmaThreadEntryFunction entryFunction,
		/**
		* The argument to the start function.
		*/
		void *arg,
		/**
		* 1 if the thread should be a realtime thread.
		*/
		int realtime,
		/**
		 * The name of the thread. Can be NULL to inherit the name from the parent.
		 * Because of the underlying pthread implementation, the length is 
		 * restricted to 16 characters, including the terminating null byte ('\0').
		 */
		const char *name);

/**
 * Joins the thread.
 */
void evrmaThreadJoin(
		/**
		 * Thread to be joined.
		 */
		EvrmaThread thread);

/**
 * Opens a new mutex.
 */
EvrmaMutex evrmaMutexOpen(void);

/**
 * Closes the mutex.
 */
void evrmaMutexClose(
		/**
		* Mutex to be closed.
		*/
		EvrmaMutex mutex);

/**
 * Locks the mutex.
 */
void evrmaMutexLock(
		/**
		* Mutex to be locked.
		*/
		EvrmaMutex mutex);

/**
 * Unlocks the mutex.
 */
void evrmaMutexUnlock(
		/**
		* Mutex to be unlocked.
		*/
		EvrmaMutex mutex);


 
/** @} */

#ifdef __cplusplus
}
#endif


#endif // libevrma_h



