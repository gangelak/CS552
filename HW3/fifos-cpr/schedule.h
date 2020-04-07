#ifndef _SCHED
#define _SCHED

pcb * current;     	        // pointing to the current thread running
int intrp;
void yield();
void schedule();
pcb* get_current_thread();


#endif
