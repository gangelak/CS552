#ifndef _SCHED
#define _SCHED

pcb * current;     	        // pointing to the current thread running
pcb * prev_node; 			//pointing to the previous thread running

void schedule();
pcb* get_current_thread();


#endif
