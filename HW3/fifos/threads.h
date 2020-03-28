#ifndef _THREADS
#define _THREADS
#include "types.h"

pcb * runqueue;
void init_threads(void);
void runqueue_remove();
#endif

