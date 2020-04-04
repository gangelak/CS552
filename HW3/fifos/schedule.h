#ifndef _SCHED
#define _SCHED

pcb * current;     	        // pointing to the current thread running

void yield();
void schedule();
void pcrschedule();
pcb* get_current_thread();

#endif
