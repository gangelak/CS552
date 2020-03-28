#include "types.h"
#include "threads.h"
#include "vga.h"

extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;

void schedule () {
	if (runqueue->next == 0)
	{
		print_s("schedule: nothing to run\n");
		return;
	}
	pcb* tmp = runqueue->next;
	for (;runqueue->next != 0;)
	{
		(tmp->entry)();
//		print_s("running the thread\n");
		runqueue_remove(tmp->tid);
		if( tmp->next == 0 )
		{
			// we reached end of list
			// so we are gonna go to the head again
			tmp = runqueue->next;
		}
		else{
			tmp = tmp->next;
		}	
	}
	print_s("schedule: nothing to run\n");
	return;
}
