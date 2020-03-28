#include "types.h"
#include "threads.h"
#include "vga.h"

extern pcb fifos_threads[MAX_THREADS];
extern pcb * runqueue;

void schedule () {
	for (;;)
	{
		if (!runqueue)
		{
			print_s("schedule: nothing to run\n");
			break;
		}
		else {
			(void*)(runqueue->entry)();
//			print_s("run the thread\n");
			runqueue_remove();
			// do context switch here for now bc the thread is done
			// remove this from runqueue
		}
	}
/*	for (i =0; i < MAX_THREADS; i++)
	{
		if (runqueue)
		{
			print_s("scheduling threads\n");
			(void*)(eads[i].entry)();
			fifos_threads[i].status = 1;
		}
	}
	*/
	return;
}
