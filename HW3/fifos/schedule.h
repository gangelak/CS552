#ifndef _SCHED
#define _SCHED

pcb * current;     	        // pointing to the current thread running

void yield();
void schedule();
pcb* get_current_thread();


#endif
