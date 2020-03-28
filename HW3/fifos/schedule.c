#include "types.h"
#include "threads.h"
#include "vga.h"

extern pcb fifos_threads[MAX_THREADS];
 
void schedule () {
	int i;
	for (i =0; i < MAX_THREADS; i++)
	{
		if (!fifos_threads[i].idle)
		{
			print_s("scheduling threads\n");
			fifos_threads[i].task();
			fifos_threads[i].idle = 1;
		}
	}
	return;
}
