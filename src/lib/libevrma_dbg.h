#ifndef libevrma_dbg_h
#define libevrma_dbg_h

#include <stdint.h>

// --------- To be used in the test procedures, in C++ ----------

#ifdef __cplusplus

bool evrmaTest(EvrmaSession session, int what);
void evrmaThreadRun(EvrmaSession session, int state);

int evrmaGetFd(EvrmaSession session);

#endif


// --------- To be used in the test procedures, in C/C++ ----------

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DBG_MEASURE_TIME_FROM_IRQ_TO_USER

void evrmaTimeDebug(int event, void *data, int acceptableLimitUsec,
	uint32_t userStart, uint32_t userEnd);

#endif

#ifdef __cplusplus
}
#endif

#endif // libevrma_dbg_h



