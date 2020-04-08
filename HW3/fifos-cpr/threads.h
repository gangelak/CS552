#ifndef _THREADS
#define _THREADS
#include "types.h"

int Time;


pcb * runqueue;
void init_threads(void);
void runqueue_remove(int);
#endif

