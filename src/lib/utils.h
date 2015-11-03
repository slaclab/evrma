#ifndef evrma_utils_h 
#define evrma_utils_h

#include <stdio.h>

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



#endif // evrma_utils_h

